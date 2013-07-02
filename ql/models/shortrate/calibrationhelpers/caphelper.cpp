/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/models/shortrate/calibrationhelpers/caphelper.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/pricingengines/capfloor/discretizedcapfloor.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/time/schedule.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/cashflows/cashflowvectors.hpp>

namespace QuantLib {

    CapHelper::CapHelper(const Period& length,
                         const Handle<Quote>& volatility,
                         const boost::shared_ptr<IborIndex>& index,
                         Frequency fixedLegFrequency,
                         const DayCounter& fixedLegDayCounter,
                         bool includeFirstSwaplet,
                         const Handle<YieldTermStructure>& termStructure,
                         CalibrationHelper::CalibrationErrorType errorType)
        : CalibrationHelper(volatility,termStructure,errorType),
        length_(length), index_(index), fixedLegFrequency_(fixedLegFrequency),
        fixedLegDayCounter_(fixedLegDayCounter),
        includeFirstSwaplet_(includeFirstSwaplet)
    {

        registerWith(index_);

    }

    void CapHelper::addTimesTo(std::list<Time>& times) const {
        calculate();
        CapFloor::arguments args;
        cap_->setupArguments(&args);
        std::vector<Time> capTimes =
            DiscretizedCapFloor(args,
                                termStructure_->referenceDate(),
                                termStructure_->dayCounter()).mandatoryTimes();
        times.insert(times.end(),
                     capTimes.begin(), capTimes.end());
    }

    Real CapHelper::modelValue() const {
        calculate();
        cap_->setPricingEngine(engine_);
        return cap_->NPV();
    }

    Real CapHelper::blackPrice(Volatility sigma) const {
        calculate();
        boost::shared_ptr<Quote> vol(new SimpleQuote(sigma));
        boost::shared_ptr<PricingEngine> black(
                                 new BlackCapFloorEngine(termStructure_,
                                                         Handle<Quote>(vol)));
        cap_->setPricingEngine(black);
        Real value = cap_->NPV();
        cap_->setPricingEngine(engine_);
        return value;
    }

    void CapHelper::performCalculations() const {

        Period indexTenor = index_->tenor();
        Rate fixedRate = 0.04; // dummy value
        Date startDate, maturity;
        if (includeFirstSwaplet_) {
            startDate = termStructure_->referenceDate();
            maturity = termStructure_->referenceDate() + length_;
        } else {
            startDate = termStructure_->referenceDate() + indexTenor;
            maturity = termStructure_->referenceDate() + length_;
        }
        boost::shared_ptr<IborIndex> dummyIndex(new
            IborIndex("dummy",
                      indexTenor,
                      index_->fixingDays(),
                      index_->currency(),
                      index_->fixingCalendar(),
                      index_->businessDayConvention(),
                      index_->endOfMonth(),
                      termStructure_->dayCounter(),
                      termStructure_));

        std::vector<Real> nominals(1,1.0);

        Schedule floatSchedule(startDate, maturity,
                               index_->tenor(), index_->fixingCalendar(),
                               index_->businessDayConvention(),
                               index_->businessDayConvention(),
                               DateGeneration::Forward, false);
        Leg floatingLeg = IborLeg(floatSchedule, index_)
            .withNotionals(nominals)
            .withPaymentAdjustment(index_->businessDayConvention())
            .withFixingDays(0);

        Schedule fixedSchedule(startDate, maturity, Period(fixedLegFrequency_),
                               index_->fixingCalendar(),
                               Unadjusted, Unadjusted,
                               DateGeneration::Forward, false);
        Leg fixedLeg = FixedRateLeg(fixedSchedule)
            .withNotionals(nominals)
            .withCouponRates(fixedRate, fixedLegDayCounter_)
            .withPaymentAdjustment(index_->businessDayConvention());

        Swap swap(floatingLeg, fixedLeg);
        swap.setPricingEngine(boost::shared_ptr<PricingEngine>(
                            new DiscountingSwapEngine(termStructure_, false)));
        Rate fairRate = fixedRate - swap.NPV()/(swap.legBPS(1)/1.0e-4);
        cap_ = boost::shared_ptr<Cap>(new Cap(floatingLeg,
                                              std::vector<Rate>(1, fairRate)));

        CalibrationHelper::performCalculations();

    }


}
