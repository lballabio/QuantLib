/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Kyrylo Protsenko

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/cashflows/stubiborcoupon.hpp>
#include <ql/settings.hpp>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <utility>

namespace QuantLib {

    const ext::shared_ptr<IborIndex>&
    StubIborCoupon::WeightedIndex::firstComponent(const std::vector<Component>& components) {
        QL_REQUIRE(!components.empty(), "no components given for weighted index");
        return components.front().first;
    }

    Period
    StubIborCoupon::WeightedIndex::longestTenor(const std::vector<Component>& components) {
        QL_REQUIRE(!components.empty(), "no components given for weighted index");
        const auto longest = std::max_element(
            components.begin(), components.end(),
            [](const auto& a, const auto& b) { return a.first->tenor() < b.first->tenor(); });
        return longest->first->tenor();
    }


    StubIborCoupon::WeightedIndex::WeightedIndex(std::vector<Component> components)
    : IborIndex("Weighted",
                longestTenor(components),
                firstComponent(components)->fixingDays(),
                firstComponent(components)->currency(),
                firstComponent(components)->fixingCalendar(),
                firstComponent(components)->businessDayConvention(),
                firstComponent(components)->endOfMonth(),
                firstComponent(components)->dayCounter()),
      components_(std::move(components)) {

        /* The combination fixes on a single date and its clients derive
           dates from a single set of conventions.
        */
        const auto& first = components_.front().first;
        for (const auto& component : components_) {
            const auto& index = component.first;
            QL_REQUIRE(index, "null component index");
            QL_REQUIRE(index->fixingDays() == first->fixingDays(),
                       index->name() << " fixing days (" << index->fixingDays()
                                     << ") do not match " << first->name() << " ("
                                     << first->fixingDays() << ")");
            QL_REQUIRE(index->fixingCalendar() == first->fixingCalendar(),
                       index->name() << " fixing calendar does not match " << first->name());
            QL_REQUIRE(index->dayCounter() == first->dayCounter(),
                       index->name() << " day counter does not match " << first->name());
            QL_REQUIRE(index->currency() == first->currency(),
                       index->name() << " currency does not match " << first->name());
            registerWith(index);
        }

        std::ostringstream out;
        out << std::setprecision(4) << std::fixed;
        for (Size i = 0; i < components_.size(); ++i) {
            if (i > 0)
                out << " + ";
            out << components_[i].first->name() << "(" << components_[i].second << ")";
        }
        name_ = out.str();
        registerWith(notifier());
    }

    Date StubIborCoupon::WeightedIndex::maturityDate(const Date& valueDate) const {
        Date maturity = Date::minDate();
        for (const auto& component : components_)
            maturity = std::max(maturity, component.first->maturityDate(valueDate));
        return maturity;
    }

    Rate StubIborCoupon::WeightedIndex::forecastFixing(const Date& fixingDate) const {
        // as in SwapSpreadIndex, this also handles the case where one of the
        // components has a historic fixing on the evaluation date
        Rate fixing = 0.0;
        for (const auto& component : components_)
            fixing += component.second * component.first->fixing(fixingDate, false);
        return fixing;
    }

    Rate StubIborCoupon::WeightedIndex::pastFixing(const Date& fixingDate) const {
        Rate fixing = 0.0;
        for (const auto& component : components_) {
            const Rate componentFixing = component.first->pastFixing(fixingDate);
            // a missing component means a missing fixing for the combination
            if (componentFixing == Null<Real>())
                return Null<Real>();
            fixing += component.second * componentFixing;
        }
        return fixing;
    }

    ext::shared_ptr<IborIndex>
    StubIborCoupon::WeightedIndex::clone(const Handle<YieldTermStructure>&) const {
        QL_FAIL("StubIborCoupon::WeightedIndex cannot be relinked to a single forwarding curve; "
                "relink its components instead");
    }


    StubIndexSelection::StubIndexSelection(Convention convention,
                                           std::vector<ext::shared_ptr<IborIndex> > indices)
    : convention_(convention), indices_(std::move(indices)) {
        QL_REQUIRE(!indices_.empty(), "no candidate indices provided for stub index selection");
        for (const auto& candidate : indices_)
            QL_REQUIRE(candidate, "null candidate index for stub index selection");
    }


    namespace {

        /*! Returns the indices an accrual period fixes on, with their weights.
            All candidates should start the rate on accrualStartDate, and their
            maturities must be distinct.
        */
        std::vector<StubIborCoupon::WeightedIndex::Component>
        selectComponents(const StubIndexSelection& selection,
                         const Date& fixingDate,
                         const Date& accrualStartDate,
                         const Date& accrualEndDate) {
            QL_REQUIRE(!selection.empty(),
                       "no candidate indices provided for stub index selection");

            // (maturity, index) pairs, sorted by maturity so selection reduces to
            // a nearest-neighbour or bracket search.
            std::vector<std::pair<Date, ext::shared_ptr<IborIndex> > > candidates;
            candidates.reserve(selection.indices().size());
            for (const auto& candidate : selection.indices()) {
                const Date candidateValueDate = candidate->valueDate(fixingDate);
                QL_REQUIRE(candidateValueDate == accrualStartDate,
                           candidate->name() << " value date " << candidateValueDate
                                             << " does not match accrual start date "
                                             << accrualStartDate);
                candidates.emplace_back(candidate->maturityDate(candidateValueDate), candidate);
            }
            std::sort(candidates.begin(), candidates.end(),
                      [](const auto& a, const auto& b) { return a.first < b.first; });
            for (Size i = 1; i < candidates.size(); ++i)
                QL_REQUIRE(candidates[i - 1].first < candidates[i].first,
                           "duplicate candidate index maturity " << candidates[i].first);

            switch (selection.convention()) {

              case StubIndexSelection::ClosestIndex: {
                  // Since candidates are sorted, ties select the smaller maturity.
                  const auto closest = std::min_element(
                      candidates.begin(), candidates.end(),
                      [accrualEndDate](const auto& a, const auto& b) {
                          return std::abs(a.first - accrualEndDate) <
                                 std::abs(b.first - accrualEndDate);
                      });
                  return {{closest->second, 1.0}};
              }

              case StubIndexSelection::Interpolated: {
                  const auto longer = std::lower_bound(
                      candidates.begin(), candidates.end(), accrualEndDate,
                      [](const auto& candidate, const Date& d) { return candidate.first < d; });
                  QL_REQUIRE(longer != candidates.end(),
                             "index maturities do not bracket irregular coupon end date "
                             << accrualEndDate);
                  if (longer->first == accrualEndDate)
                      return {{longer->second, 1.0}};

                  QL_REQUIRE(longer != candidates.begin(),
                             "index maturities do not bracket irregular coupon end date "
                             << accrualEndDate);
                  const auto shorter = std::prev(longer);
                  const Real weight = Real(accrualEndDate - shorter->first) /
                      Real(longer->first - shorter->first);
                  return {{shorter->second, 1.0 - weight}, {longer->second, weight}};
              }

              default:
                QL_FAIL("unknown stub index convention");
            }
        }

        ext::shared_ptr<StubIborCoupon::WeightedIndex>
        makeStubIndex(const StubIndexSelection& selection,
                      const Date& startDate,
                      const Date& endDate,
                      Natural fixingDays,
                      bool isInArrears,
                      BusinessDayConvention fixingConvention) {
            QL_REQUIRE(!selection.empty(),
                       "StubIborCoupon requires a non-empty stub index selection");
            QL_REQUIRE(!isInArrears,
                       "stub index selection is not supported for in-arrears coupons");

            const Date fixingDate = selection.indices().front()->fixingCalendar().advance(
                startDate, -static_cast<Integer>(fixingDays), Days, fixingConvention);

            return ext::make_shared<StubIborCoupon::WeightedIndex>(
                selectComponents(selection, fixingDate, startDate, endDate));
        }

    }


    StubIborCoupon::StubIborCoupon(const Date& paymentDate,
                                   Real nominal,
                                   const Date& startDate,
                                   const Date& endDate,
                                   Natural fixingDays,
                                   StubIndexSelection stubIndexSelection,
                                   Real gearing,
                                   Spread spread,
                                   const Date& refPeriodStart,
                                   const Date& refPeriodEnd,
                                   const DayCounter& dayCounter,
                                   bool isInArrears,
                                   const Date& exCouponDate,
                                   BusinessDayConvention fixingConvention)
    : StubIborCoupon(paymentDate, nominal, startDate, endDate, fixingDays,
                     makeStubIndex(stubIndexSelection, startDate, endDate, fixingDays, isInArrears,
                                   fixingConvention),
                     stubIndexSelection, gearing, spread, refPeriodStart, refPeriodEnd, dayCounter,
                     isInArrears, exCouponDate, fixingConvention) {}

    StubIborCoupon::StubIborCoupon(const Date& paymentDate,
                                   Real nominal,
                                   const Date& startDate,
                                   const Date& endDate,
                                   Natural fixingDays,
                                   ext::shared_ptr<WeightedIndex> weightedIndex,
                                   StubIndexSelection stubIndexSelection,
                                   Real gearing,
                                   Spread spread,
                                   const Date& refPeriodStart,
                                   const Date& refPeriodEnd,
                                   const DayCounter& dayCounter,
                                   bool isInArrears,
                                   const Date& exCouponDate,
                                   BusinessDayConvention fixingConvention)
    : IborCoupon(paymentDate, nominal, startDate, endDate, fixingDays, weightedIndex, gearing,
                 spread, refPeriodStart, refPeriodEnd, dayCounter, isInArrears, exCouponDate,
                 fixingConvention),
      stubIndexSelection_(std::move(stubIndexSelection)),
      weightedIndex_(std::move(weightedIndex)) {
        for (const auto& component : weightedIndex_->components())
            QL_REQUIRE(component.first->valueDate(fixingDate()) == accrualStartDate(),
                       component.first->name()
                           << " value date for the coupon fixing date " << fixingDate()
                           << " does not match accrual start date " << accrualStartDate());
    }

    Rate StubIborCoupon::indexFixing() const {
        return weightedIndex_->fixing(fixingDate());
    }

    bool StubIborCoupon::hasFixed() const {
        const Date today = QuantLib::Settings::instance().evaluationDate();
        if (fixingDate() > today)
            return false;
        if (fixingDate() < today)
            return true;
        if (QuantLib::Settings::instance().enforcesTodaysHistoricFixings())
            return true;
        const auto& components = weightedIndex_->components();
        return std::all_of(components.begin(), components.end(),
                           [this](const auto& component) {
                               return component.first->hasHistoricalFixing(fixingDate());
                           });
    }

    void StubIborCoupon::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<StubIborCoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            IborCoupon::accept(v);
    }

}
