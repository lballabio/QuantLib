/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters

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

#include <ql/experimental/overnightswap/eoniaswaphelper.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/quote.hpp>
#include <ql/currency.hpp>

using boost::shared_ptr;

namespace QuantLib {

    namespace {
        void no_deletion(YieldTermStructure*) {}
    }

    OISRateHelper::OISRateHelper(const Handle<Quote>& fixedRate,
                                 const Period& tenor, // swap maturity
                                 Natural settlementDays,
                                 const Calendar& calendar,
                                 // Overnight Indexed leg
                                 const Period& overnightPeriod,
                                 BusinessDayConvention overnightConvention,
                                 const boost::shared_ptr<OvernightIndex>& overnightIndex,
                                 // fixed leg
                                 const Period& fixedPeriod,
                                 BusinessDayConvention fixedConvention,
                                 const DayCounter& fixedDayCount)
    : RelativeDateRateHelper(fixedRate),
      tenor_(tenor), settlementDays_(settlementDays),
      calendar_(calendar),
      overnightPeriod_(overnightPeriod),
      overnightConvention_(overnightConvention),
      overnightIndex_(overnightIndex),
      fixedPeriod_(fixedPeriod),
      fixedConvention_(fixedConvention),
      fixedDayCount_(fixedDayCount) {
        registerWith(overnightIndex_);
        initializeDates();
    }

    void OISRateHelper::initializeDates() {
        earliestDate_ = calendar_.advance(evaluationDate_,
                                          settlementDays_*Days,
                                          Following);

        Date maturity = earliestDate_ + tenor_;

        Schedule overnightSchedule =
            MakeSchedule().from(earliestDate_)
                          .to(maturity)
                          .withTenor(overnightPeriod_)
                          .withCalendar(overnightIndex_->fixingCalendar())
                          .withConvention(overnightConvention_)
                          .backwards();

        Schedule fixedSchedule =
            MakeSchedule().from(earliestDate_)
                          .to(maturity)
                          .withTenor(fixedPeriod_)
                          .withCalendar(overnightIndex_->fixingCalendar())
                          .withConvention(fixedConvention_)
                          .backwards();

        // dummy OvernightIndex with curve/swap arguments
        boost::shared_ptr<IborIndex> clonedIborIndex =
            overnightIndex_->clone(termStructureHandle_);
        shared_ptr<OvernightIndex> clonedOvernightIndex = 
            boost::dynamic_pointer_cast<OvernightIndex>(clonedIborIndex);

        swap_ = shared_ptr<OvernightIndexedSwap>(new
            OvernightIndexedSwap(OvernightIndexedSwap::Payer,
                                 100.0,
                                 overnightSchedule,
                                 0.0,
                                 clonedOvernightIndex,
                                 fixedSchedule,
                                 0.0,
                                 fixedDayCount_));
        swap_->setPricingEngine(shared_ptr<PricingEngine>(new
            DiscountingSwapEngine(clonedOvernightIndex->termStructure())));

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

}
