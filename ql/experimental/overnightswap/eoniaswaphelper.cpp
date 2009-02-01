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

#include <ql/termstructures/yield/eoniaswaphelper.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/quote.hpp>
#include <ql/currency.hpp>

using boost::shared_ptr;

namespace QuantLib {

    namespace {
        void no_deletion(YieldTermStructure*) {}
    }

    EoniaSwapHelper::EoniaSwapHelper(
                                     const Handle<Quote>& fixedRate,
                                     const Period& tenor, // swap maturity
                                     Natural settlementDays,
                                     const Calendar& calendar,
                                     // eonia leg
                                     const Period& eoniaPeriod,
                                     BusinessDayConvention eoniaConvention,
                                     const boost::shared_ptr<Eonia>& index,
                                     // fixed leg
                                     const Period& fixedPeriod,
                                     BusinessDayConvention fixedConvention,
                                     const DayCounter& fixedDayCount)
        : RelativeDateRateHelper(fixedRate),
          tenor_(tenor), settlementDays_(settlementDays),
          calendar_(calendar),
          eoniaPeriod_(eoniaPeriod),
          eoniaConvention_(eoniaConvention),
          index_(index),
          fixedPeriod_(fixedPeriod),
          fixedConvention_(fixedConvention),
          fixedDayCount_(fixedDayCount) {
        registerWith(index_);
        initializeDates();
    }

    void EoniaSwapHelper::initializeDates() {
        earliestDate_ = calendar_.advance(evaluationDate_,
                                          settlementDays_*Days,
                                          Following);

        Date maturity = earliestDate_ + tenor_;

        // dummy Eonia index with curve/swap arguments
        shared_ptr<Eonia> clonedIndex(new Eonia(termStructureHandle_));

        Schedule eoniaSchedule = MakeSchedule(earliestDate_,
                                            maturity,
                                            eoniaPeriod_,
                                            index_->fixingCalendar(),
                                            eoniaConvention_).backwards();

        Schedule fixedSchedule =
            MakeSchedule(earliestDate_,
                         maturity,
                         fixedPeriod_,
                         index_->fixingCalendar(),
                         fixedConvention_).backwards();

        swap_ = shared_ptr<EoniaSwap>(new EoniaSwap(EoniaSwap::Payer, 100.0,
                                                    eoniaSchedule,
                                                    0.0,
                                                    clonedIndex,
                                                    fixedSchedule,
                                                    0.0,
                                                    fixedDayCount_));
        swap_->setPricingEngine(shared_ptr<PricingEngine>(new
            DiscountingSwapEngine(clonedIndex->termStructure())));

        latestDate_ = swap_->maturityDate();
    }

    void EoniaSwapHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        termStructureHandle_.linkTo(
                         shared_ptr<YieldTermStructure>(t,no_deletion),
                         false);
        RelativeDateRateHelper::setTermStructure(t);
    }

    Real EoniaSwapHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->recalculate();
        return swap_->fairRate();
    }

    void EoniaSwapHelper::accept(AcyclicVisitor& v) {
        Visitor<EoniaSwapHelper>* v1 =
            dynamic_cast<Visitor<EoniaSwapHelper>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }
}
