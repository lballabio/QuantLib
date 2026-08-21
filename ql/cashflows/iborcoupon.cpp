/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2010, 2011 Ferdinando Ametrano
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer
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

#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/indexes/interestrateindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/optional.hpp>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>

namespace QuantLib {

    IborCoupon::IborCoupon(const Date& paymentDate,
                           Real nominal,
                           const Date& startDate,
                           const Date& endDate,
                           Natural fixingDays,
                           const ext::shared_ptr<IborIndex>& iborIndex,
                           Real gearing,
                           Spread spread,
                           const Date& refPeriodStart,
                           const Date& refPeriodEnd,
                           const DayCounter& dayCounter,
                           bool isInArrears,
                           const Date& exCouponDate,
                           BusinessDayConvention fixingConvention)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, iborIndex, gearing, spread,
                         refPeriodStart, refPeriodEnd,
                         dayCounter, isInArrears, exCouponDate,
                         fixingConvention),
      iborIndex_(iborIndex) {
        fixingDate_ = FloatingRateCoupon::fixingDate();
    }

    void IborCoupon::initializeCachedData() const {
        auto p = ext::dynamic_pointer_cast<IborCouponPricer>(pricer_);
        QL_REQUIRE(p, "IborCoupon: pricer not set or not derived from IborCouponPricer");
        p->initializeCachedData(*this);
    }

    const Date& IborCoupon::fixingValueDate() const {
        initializeCachedData();
        return fixingValueDate_;
    }

    const Date& IborCoupon::fixingEndDate() const {
        initializeCachedData();
        return fixingEndDate_;
    }

    const Date& IborCoupon::fixingMaturityDate() const {
        initializeCachedData();
        return fixingMaturityDate_;
    }

    Time IborCoupon::spanningTime() const {
        initializeCachedData();
        return spanningTime_;
    }

    Time IborCoupon::spanningTimeIndexMaturity() const {
        initializeCachedData();
        return spanningTimeIndexMaturity_;
    }

    Date IborCoupon::fixingDate() const {
        return fixingDate_;
    }

    bool IborCoupon::hasFixed() const {
        Date today = QuantLib::Settings::instance().evaluationDate();

        if (fixingDate_ > today) {
            return false;
        } else if (fixingDate_ < today) {
            return true;
        } else {
            // fixingDate_ == today
            if (QuantLib::Settings::instance().enforcesTodaysHistoricFixings()) {
                return true;
            } else {
                return index_->hasHistoricalFixing(fixingDate_);
            }
        }
    }

    Rate IborCoupon::indexFixing() const {
        initializeCachedData();

        /* instead of just returning index_->fixing(fixingValueDate_)
           its logic is duplicated here using a specialized iborIndex
           forecastFixing overload which
           1) allows to save date/time recalculations, and
           2) takes into account par coupon needs
        */

        if (hasFixed()) {
            Rate result = index_->pastFixing(fixingDate_);
            QL_REQUIRE(result != Null<Real>(),
                       "Missing " << index_->name() << " fixing for " << fixingDate_);
            return result;
        } else {
            return iborIndex_->forecastFixing(fixingValueDate_, fixingEndDate_, spanningTime_);
        }
    }

    void IborCoupon::setPricer(const ext::shared_ptr<FloatingRateCouponPricer>& pricer) {
        cachedDataIsInitialized_ = false;
        FloatingRateCoupon::setPricer(pricer);
    }

    void IborCoupon::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<IborCoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }


    StubIndexConfig::StubIndexConfig(StubIndexConvention convention,
                                     std::vector<ext::shared_ptr<IborIndex> > indices)
    : convention_(convention), indices_(std::move(indices)) {
        QL_REQUIRE(!indices_.empty(), "no candidate indices provided for stub index selection");
        for (const auto& candidate : indices_)
            QL_REQUIRE(candidate, "null candidate index for stub index selection");
    }

    std::vector<WeightedIborIndex::Component>
    StubIndexConfig::components(const Date& fixingDate,
                                const Date& accrualStartDate,
                                const Date& accrualEndDate) const {
        QL_REQUIRE(!empty(), "no candidate indices provided for stub index selection");

        // (maturity, index) pairs, sorted by maturity so selection reduces to
        // a nearest-neighbour or bracket search.
        std::vector<std::pair<Date, ext::shared_ptr<IborIndex> > > candidates;
        candidates.reserve(indices_.size());
        for (const auto& candidate : indices_) {
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

        if (convention_ == StubIndexConvention::ClosestIndex) {
            // Since candidates are sorted, ties select the smaller maturity.
            const auto closest = std::min_element(
                candidates.begin(), candidates.end(),
                [accrualEndDate](const auto& a, const auto& b) {
                    return std::abs(a.first - accrualEndDate) <
                           std::abs(b.first - accrualEndDate);
                });
            return {{closest->second, 1.0}};
        }

        QL_REQUIRE(convention_ == StubIndexConvention::Interpolated,
                   "unknown stub index convention");

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

    ext::shared_ptr<WeightedIborIndex>
    StubIndexConfig::makeIndex(const Date& fixingDate,
                               const Date& accrualStartDate,
                               const Date& accrualEndDate) const {
        return ext::make_shared<WeightedIborIndex>(
            components(fixingDate, accrualStartDate, accrualEndDate));
    }

    static ext::shared_ptr<WeightedIborIndex>
    makeStubIndex(const StubIndexConfig& config,
                  const Date& startDate,
                  const Date& endDate,
                  Natural fixingDays,
                  bool isInArrears,
                  BusinessDayConvention fixingConvention) {
        QL_REQUIRE(!config.empty(),
                   "StubIborCoupon requires a non-empty stub index configuration");
        QL_REQUIRE(!isInArrears,
                   "stub index selection is not supported for in-arrears coupons");

        const Date fixingDate = config.indices().front()->fixingCalendar().advance(
            startDate, -static_cast<Integer>(fixingDays), Days, fixingConvention);

        return config.makeIndex(fixingDate, startDate, endDate);
    }


    StubIborCoupon::StubIborCoupon(const Date& paymentDate,
                                   Real nominal,
                                   const Date& startDate,
                                   const Date& endDate,
                                   Natural fixingDays,
                                   StubIndexConfig stubIndexConfig,
                                   Real gearing,
                                   Spread spread,
                                   const Date& refPeriodStart,
                                   const Date& refPeriodEnd,
                                   const DayCounter& dayCounter,
                                   bool isInArrears,
                                   const Date& exCouponDate,
                                   BusinessDayConvention fixingConvention)
    : StubIborCoupon(paymentDate, nominal, startDate, endDate, fixingDays,
                     makeStubIndex(stubIndexConfig, startDate, endDate, fixingDays, isInArrears,
                                   fixingConvention),
                     stubIndexConfig, gearing, spread, refPeriodStart, refPeriodEnd, dayCounter,
                     isInArrears, exCouponDate, fixingConvention) {}

    StubIborCoupon::StubIborCoupon(const Date& paymentDate,
                                   Real nominal,
                                   const Date& startDate,
                                   const Date& endDate,
                                   Natural fixingDays,
                                   ext::shared_ptr<WeightedIborIndex> weightedIndex,
                                   StubIndexConfig stubIndexConfig,
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
      stubIndexConfig_(std::move(stubIndexConfig)),
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


    void IborCoupon::Settings::createAtParCoupons() {
         usingAtParCoupons_ = true;
    }

    void IborCoupon::Settings::createIndexedCoupons() {
         usingAtParCoupons_ = false;
    }

    bool IborCoupon::Settings::usingAtParCoupons() const {
        return usingAtParCoupons_;
    }

    IborLeg::IborLeg(Schedule schedule, ext::shared_ptr<IborIndex> index)
    : schedule_(std::move(schedule)), index_(std::move(index)) {
        QL_REQUIRE(index_, "no index provided");
    }

    IborLeg& IborLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

    IborLeg& IborLeg::withNotionals(const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    IborLeg& IborLeg::withPaymentDayCounter(const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

    IborLeg& IborLeg::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    IborLeg& IborLeg::withPaymentLag(Integer lag) {
        paymentLag_ = lag;
        return *this;
    }

    IborLeg& IborLeg::withPaymentCalendar(const Calendar& cal) {
        paymentCalendar_ = cal;
        return *this;
    }

    IborLeg& IborLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1,fixingDays);
        return *this;
    }

    IborLeg& IborLeg::withFixingDays(const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

    IborLeg& IborLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1,gearing);
        return *this;
    }

    IborLeg& IborLeg::withGearings(const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    IborLeg& IborLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1,spread);
        return *this;
    }

    IborLeg& IborLeg::withSpreads(const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    IborLeg& IborLeg::withCaps(Rate cap) {
        caps_ = std::vector<Rate>(1,cap);
        return *this;
    }

    IborLeg& IborLeg::withCaps(const std::vector<Rate>& caps) {
        caps_ = caps;
        return *this;
    }

    IborLeg& IborLeg::withFloors(Rate floor) {
        floors_ = std::vector<Rate>(1,floor);
        return *this;
    }

    IborLeg& IborLeg::withFloors(const std::vector<Rate>& floors) {
        floors_ = floors;
        return *this;
    }

    IborLeg& IborLeg::inArrears(bool flag) {
        inArrears_ = flag;
        return *this;
    }

    IborLeg& IborLeg::withZeroPayments(bool flag) {
        zeroPayments_ = flag;
        return *this;
    }

	IborLeg& IborLeg::withExCouponPeriod(const Period& period,
                                         const Calendar& cal,
                                         BusinessDayConvention convention,
                                         bool endOfMonth) {
        exCouponPeriod_ = period;
        exCouponCalendar_ = cal;
        exCouponAdjustment_ = convention;
        exCouponEndOfMonth_ = endOfMonth;
        return *this;
	}

    IborLeg& IborLeg::withFixingConvention(BusinessDayConvention convention) {
        fixingConvention_ = convention;
        return *this;
    }

    IborLeg& IborLeg::withIndexedCoupons(std::optional<bool> b) {
        useIndexedCoupons_ = b;
        return *this;
    }

    IborLeg& IborLeg::withAtParCoupons(bool b) {
        useIndexedCoupons_ = !b;
        return *this;
    }

    IborLeg& IborLeg::withStubIndexConfig(const StubIndexConfig& config) {
        stubIndexConfig_ = config;
        return *this;
    }

    IborLeg::operator Leg() const {

        Leg leg = FloatingLeg<IborIndex, IborCoupon, CappedFlooredIborCoupon>(
                         schedule_, notionals_, index_, paymentDayCounter_,
                         paymentAdjustment_, fixingDays_, gearings_, spreads_,
                         caps_, floors_, inArrears_, zeroPayments_, paymentLag_, paymentCalendar_, 
			             exCouponPeriod_, exCouponCalendar_, exCouponAdjustment_, exCouponEndOfMonth_,
			             fixingConvention_);

        if (!stubIndexConfig_.empty()) {
            QL_REQUIRE(caps_.empty() && floors_.empty(),
                       "stub index conventions are not supported for capped/floored Ibor legs");
            QL_REQUIRE(!inArrears_,
                       "stub index conventions are not supported for in-arrears Ibor legs");
            QL_REQUIRE(useIndexedCoupons_.value_or(
                           !IborCoupon::Settings::instance().usingAtParCoupons()),
                       "stub index conventions require indexed coupons");
            QL_REQUIRE(schedule_.hasIsRegular(),
                       "schedule does not provide regularity information for stub index selection");

            for (Size i = 0; i < leg.size(); ++i) {
                if (schedule_.isRegular(i + 1))
                    continue;

                // periods with null gearing are fixed-rate coupons; leave them alone
                auto coupon = ext::dynamic_pointer_cast<IborCoupon>(leg[i]);
                if (!coupon)
                    continue;

                leg[i] = ext::make_shared<StubIborCoupon>(
                    coupon->date(), coupon->nominal(), coupon->accrualStartDate(),
                    coupon->accrualEndDate(), coupon->fixingDays(), stubIndexConfig_,
                    coupon->gearing(), coupon->spread(), coupon->referencePeriodStart(),
                    coupon->referencePeriodEnd(), coupon->dayCounter(), coupon->isInArrears(),
                    coupon->exCouponDate(), coupon->fixingConvention());
            }
        }

        if (caps_.empty() && floors_.empty() && !inArrears_) {
            ext::shared_ptr<IborCouponPricer> pricer = ext::make_shared<BlackIborCouponPricer>(
                Handle<OptionletVolatilityStructure>(),
                BlackIborCouponPricer::TimingAdjustment::Black76,
                Handle<Quote>(ext::make_shared<SimpleQuote>(1.0)), useIndexedCoupons_);
            setCouponPricer(leg, pricer);
        }

        return leg;
    }

}
