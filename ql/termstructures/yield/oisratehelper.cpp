/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2012 Roland Lichters
 Copyright (C) 2009, 2012 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/instruments/makeois.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/yield/oisratehelper.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

namespace QuantLib {

    OISRateHelper::OISRateHelper(Natural settlementDays,
                                 const Period& tenor, // swap maturity
                                 const Handle<Quote>& fixedRate,
                                 std::shared_ptr<OvernightIndex> overnightIndex,
                                 Handle<YieldTermStructure> discount,
                                 bool telescopicValueDates,
                                 Natural paymentLag,
                                 BusinessDayConvention paymentConvention,
                                 Frequency paymentFrequency,
                                 Calendar paymentCalendar,
                                 const Period& forwardStart,
                                 const Spread overnightSpread,
                                 Pillar::Choice pillar,
                                 Date customPillarDate,
                                 RateAveraging::Type averagingMethod,
                                 std::optional<bool> endOfMonth)
    : RelativeDateRateHelper(fixedRate), pillarChoice_(pillar), settlementDays_(settlementDays),
      tenor_(tenor), overnightIndex_(std::move(overnightIndex)),
      discountHandle_(std::move(discount)), telescopicValueDates_(telescopicValueDates),
      paymentLag_(paymentLag), paymentConvention_(paymentConvention),
      paymentFrequency_(paymentFrequency), paymentCalendar_(std::move(paymentCalendar)),
      forwardStart_(forwardStart), overnightSpread_(overnightSpread),
      averagingMethod_(averagingMethod), endOfMonth_(endOfMonth) {
        registerWith(overnightIndex_);
        registerWith(discountHandle_);

        pillarDate_ = customPillarDate;
        OISRateHelper::initializeDates();
    }

    void OISRateHelper::initializeDates() {

        // dummy OvernightIndex with curve/swap arguments
        // review here
        std::shared_ptr<IborIndex> clonedIborIndex =
            overnightIndex_->clone(termStructureHandle_);
        std::shared_ptr<OvernightIndex> clonedOvernightIndex =
            std::dynamic_pointer_cast<OvernightIndex>(clonedIborIndex);
        // input discount curve Handle might be empty now but it could
        //    be assigned a curve later; use a RelinkableHandle here
        MakeOIS tmp = MakeOIS(tenor_, clonedOvernightIndex, 0.0, forwardStart_)
            .withDiscountingTermStructure(discountRelinkableHandle_)
            .withSettlementDays(settlementDays_)
            .withTelescopicValueDates(telescopicValueDates_)
            .withPaymentLag(paymentLag_)
            .withPaymentAdjustment(paymentConvention_)
            .withPaymentFrequency(paymentFrequency_)
            .withPaymentCalendar(paymentCalendar_)
            .withOvernightLegSpread(overnightSpread_)
            .withAveragingMethod(averagingMethod_);
        if (endOfMonth_) {
            swap_ = tmp.withEndOfMonth(*endOfMonth_);
        } else {
            swap_ = tmp;
        }

        earliestDate_ = swap_->startDate();
        maturityDate_ = swap_->maturityDate();

        Date lastPaymentDate = std::max(swap_->overnightLeg().back()->date(),
                                        swap_->fixedLeg().back()->date());
        latestRelevantDate_ = std::max(maturityDate_, lastPaymentDate);

        switch (pillarChoice_) {
          case Pillar::MaturityDate:
            pillarDate_ = maturityDate_;
            break;
          case Pillar::LastRelevantDate:
            pillarDate_ = latestRelevantDate_;
            break;
          case Pillar::CustomDate:
            // pillarDate_ already assigned at construction time
            QL_REQUIRE(pillarDate_ >= earliestDate_,
                       "pillar date (" << pillarDate_ << ") must be later "
                       "than or equal to the instrument's earliest date (" <<
                       earliestDate_ << ")");
            QL_REQUIRE(pillarDate_ <= latestRelevantDate_,
                       "pillar date (" << pillarDate_ << ") must be before "
                       "or equal to the instrument's latest relevant date (" <<
                       latestRelevantDate_ << ")");
            break;
          default:
            QL_FAIL("unknown Pillar::Choice(" << Integer(pillarChoice_) << ")");
        }

        latestDate_ = std::max(swap_->maturityDate(), lastPaymentDate);
    }

    void OISRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        std::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        if (discountHandle_.empty())
            discountRelinkableHandle_.linkTo(temp, observer);
        else
            discountRelinkableHandle_.linkTo(*discountHandle_, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    Real OISRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != nullptr, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->recalculate();
        return swap_->fairRate();
    }

    void OISRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<OISRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

    DatedOISRateHelper::DatedOISRateHelper(const Date& startDate,
                                           const Date& endDate,
                                           const Handle<Quote>& fixedRate,
                                           const std::shared_ptr<OvernightIndex>& overnightIndex,
                                           Handle<YieldTermStructure> discount,
                                           bool telescopicValueDates,
                                           RateAveraging::Type averagingMethod)
    : RateHelper(fixedRate), discountHandle_(std::move(discount)),
      telescopicValueDates_(telescopicValueDates),
      averagingMethod_(averagingMethod) {

        registerWith(overnightIndex);
        registerWith(discountHandle_);

        // dummy OvernightIndex with curve/swap arguments
        // review here
        std::shared_ptr<IborIndex> clonedIborIndex =
            overnightIndex->clone(termStructureHandle_);
        std::shared_ptr<OvernightIndex> clonedOvernightIndex =
            std::dynamic_pointer_cast<OvernightIndex>(clonedIborIndex);

        // input discount curve Handle might be empty now but it could
        //    be assigned a curve later; use a RelinkableHandle here
        swap_ = MakeOIS(Period(), clonedOvernightIndex, 0.0)
            .withDiscountingTermStructure(discountRelinkableHandle_)
            .withEffectiveDate(startDate)
            .withTerminationDate(endDate)
            .withTelescopicValueDates(telescopicValueDates_)
            .withAveragingMethod(averagingMethod_);

        earliestDate_ = swap_->startDate();
        Date lastPaymentDate = std::max(swap_->overnightLeg().back()->date(),
                                        swap_->fixedLeg().back()->date());
        latestDate_ = std::max(swap_->maturityDate(), lastPaymentDate);
    }

    void DatedOISRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        std::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        if (discountHandle_.empty())
            discountRelinkableHandle_.linkTo(temp, observer);
        else
            discountRelinkableHandle_.linkTo(*discountHandle_, observer);

        RateHelper::setTermStructure(t);
    }

    Real DatedOISRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != nullptr, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->recalculate();
        return swap_->fairRate();
    }

    void DatedOISRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<DatedOISRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

}
