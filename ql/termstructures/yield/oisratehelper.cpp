/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano

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

using boost::shared_ptr;

namespace QuantLib {

    namespace {
        void no_deletion(YieldTermStructure*) {}
    }

    OISRateHelper::OISRateHelper(
                    Natural settlementDays,
                    const Period& tenor, // swap maturity
                    const Handle<Quote>& fixedRate,
                    const boost::shared_ptr<OvernightIndex>& overnightIndex)
    : RelativeDateRateHelper(fixedRate),
      settlementDays_(settlementDays), tenor_(tenor),
      overnightIndex_(overnightIndex) {
        registerWith(overnightIndex_);
        initializeDates();
    }

    void OISRateHelper::initializeDates() {

        // dummy OvernightIndex with curve/swap arguments
        // review here
        boost::shared_ptr<IborIndex> clonedIborIndex =
            overnightIndex_->clone(termStructureHandle_);
        shared_ptr<OvernightIndex> clonedOvernightIndex =
            boost::dynamic_pointer_cast<OvernightIndex>(clonedIborIndex);

        swap_ = MakeOIS(tenor_, clonedOvernightIndex, 0.0)
            .withSettlementDays(settlementDays_)
            .withDiscountingTermStructure(termStructureHandle_);

        earliestDate_ = swap_->startDate();
        latestDate_ = swap_->maturityDate();
    }

    void OISRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        termStructureHandle_.linkTo(
                         shared_ptr<YieldTermStructure>(t,no_deletion),
                         false);
        RelativeDateRateHelper::setTermStructure(t);
    }

    Real OISRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->recalculate();
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
                    const boost::shared_ptr<OvernightIndex>& overnightIndex)
    : RateHelper(fixedRate) {

        registerWith(overnightIndex);

        // dummy OvernightIndex with curve/swap arguments
        // review here
        boost::shared_ptr<IborIndex> clonedIborIndex =
            overnightIndex->clone(termStructureHandle_);
        shared_ptr<OvernightIndex> clonedOvernightIndex =
            boost::dynamic_pointer_cast<OvernightIndex>(clonedIborIndex);

       swap_ = MakeOIS(Period(), clonedOvernightIndex, 0.0)
           .withEffectiveDate(startDate)
           .withTerminationDate(endDate)
           .withDiscountingTermStructure(termStructureHandle_);

        earliestDate_ = swap_->startDate();
        latestDate_ = swap_->maturityDate();
    }

    void DatedOISRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        termStructureHandle_.linkTo(
                         shared_ptr<YieldTermStructure>(t,no_deletion),
                         false);
        RateHelper::setTermStructure(t);
    }

    Real DatedOISRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->recalculate();
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
