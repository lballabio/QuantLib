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

#include <ql/termstructures/yield/oisratehelper.hpp>
#include <ql/instruments/makeois.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>

#include <ql/utilities/null_deleter.hpp>

using boost::shared_ptr;

namespace QuantLib {

    OISRateHelper::OISRateHelper(
                    Natural settlementDays,
                    const Period& tenor, // swap maturity
                    const Handle<Quote>& fixedRate,
                    const boost::shared_ptr<OvernightIndex>& overnightIndex,
                    const Handle<YieldTermStructure>& discount,
                    bool telescopicValueDates,
                    Natural paymentLag,
                    BusinessDayConvention paymentConvention,
                    Frequency paymentFrequency,
                    const Calendar& paymentCalendar,
                    const Period& forwardStart, 
                    const Spread overnightSpread)
    : RelativeDateRateHelper(fixedRate),
      settlementDays_(settlementDays), tenor_(tenor),
      overnightIndex_(overnightIndex), discountHandle_(discount),
      telescopicValueDates_(telescopicValueDates),
      paymentLag_(paymentLag), paymentConvention_(paymentConvention),
      paymentFrequency_(paymentFrequency),
      paymentCalendar_(paymentCalendar),
      forwardStart_(forwardStart), overnightSpread_(overnightSpread) {
        registerWith(overnightIndex_);
        registerWith(discountHandle_);
        initializeDates();
    }

    void OISRateHelper::initializeDates() {

        // dummy OvernightIndex with curve/swap arguments
        // review here
        boost::shared_ptr<IborIndex> clonedIborIndex =
            overnightIndex_->clone(termStructureHandle_);
        shared_ptr<OvernightIndex> clonedOvernightIndex =
            boost::dynamic_pointer_cast<OvernightIndex>(clonedIborIndex);

        // input discount curve Handle might be empty now but it could
        //    be assigned a curve later; use a RelinkableHandle here
        swap_ = MakeOIS(tenor_, clonedOvernightIndex, 0.0, forwardStart_)
            .withDiscountingTermStructure(discountRelinkableHandle_)
            .withSettlementDays(settlementDays_)
            .withTelescopicValueDates(telescopicValueDates_)
            .withPaymentLag(paymentLag_)
            .withPaymentAdjustment(paymentConvention_)
            .withPaymentFrequency(paymentFrequency_)
            .withPaymentCalendar(paymentCalendar_)
            .withOvernightLegSpread(overnightSpread_);

        earliestDate_ = swap_->startDate();
        latestDate_ = swap_->maturityDate();
    }

    void OISRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        if (discountHandle_.empty())
            discountRelinkableHandle_.linkTo(temp, observer);
        else
            discountRelinkableHandle_.linkTo(*discountHandle_, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    Real OISRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->deepUpdate();
        return swap_->fairRate();
    }

    void OISRateHelper::accept(AcyclicVisitor& v) {
        Visitor<OISRateHelper>* v1 =
            dynamic_cast<Visitor<OISRateHelper>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

    DatedOISRateHelper::DatedOISRateHelper(
                    const Date& startDate,
                    const Date& endDate,
                    const Handle<Quote>& fixedRate,
                    const boost::shared_ptr<OvernightIndex>& overnightIndex,
                    const Handle<YieldTermStructure>& discount,
                    bool telescopicValueDates)
        : RateHelper(fixedRate), discountHandle_(discount),
          telescopicValueDates_(telescopicValueDates) {

        registerWith(overnightIndex);
        registerWith(discountHandle_);

        // dummy OvernightIndex with curve/swap arguments
        // review here
        boost::shared_ptr<IborIndex> clonedIborIndex =
            overnightIndex->clone(termStructureHandle_);
        shared_ptr<OvernightIndex> clonedOvernightIndex =
            boost::dynamic_pointer_cast<OvernightIndex>(clonedIborIndex);

        // input discount curve Handle might be empty now but it could
        //    be assigned a curve later; use a RelinkableHandle here
        swap_ = MakeOIS(Period(), clonedOvernightIndex, 0.0)
            .withDiscountingTermStructure(discountRelinkableHandle_)
            .withEffectiveDate(startDate)
            .withTerminationDate(endDate)
            .withTelescopicValueDates(telescopicValueDates_);

        earliestDate_ = swap_->startDate();
        latestDate_ = swap_->maturityDate();
    }

    void DatedOISRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        if (discountHandle_.empty())
            discountRelinkableHandle_.linkTo(temp, observer);
        else
            discountRelinkableHandle_.linkTo(*discountHandle_, observer);

        RateHelper::setTermStructure(t);
    }

    Real DatedOISRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->deepUpdate();
        return swap_->fairRate();
    }

    void DatedOISRateHelper::accept(AcyclicVisitor& v) {
        Visitor<DatedOISRateHelper>* v1 =
            dynamic_cast<Visitor<DatedOISRateHelper>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

}
