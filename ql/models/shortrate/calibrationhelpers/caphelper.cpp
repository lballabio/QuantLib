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
    : CalibrationHelper(volatility,termStructure,errorType) {

        Period indexTenor = index->tenor();
        Rate fixedRate = 0.04; // dummy value
        Date startDate, maturity;
        if (includeFirstSwaplet) {
            startDate = termStructure->referenceDate();
            maturity = termStructure->referenceDate() + length;
        } else {
            startDate = termStructure->referenceDate() + indexTenor;
            maturity = termStructure->referenceDate() + length;
        }
        boost::shared_ptr<IborIndex> dummyIndex(new
            IborIndex("dummy",
                      indexTenor,
                      index->fixingDays(),
                      index->currency(),
                      index->fixingCalendar(),
                      index->businessDayConvention(),
                      index->endOfMonth(),
                      termStructure->dayCounter(),
                      termStructure));

        std::vector<Real> nominals(1,1.0);

        Schedule floatSchedule(startDate, maturity,
                               index->tenor(), index->fixingCalendar(),
                               index->businessDayConvention(),
                               index->businessDayConvention(),
                               DateGeneration::Forward, false);
        Leg floatingLeg = IborLeg(floatSchedule, index)
            .withNotionals(nominals)
            .withPaymentAdjustment(index->businessDayConvention())
            .withFixingDays(0);

        Schedule fixedSchedule(startDate, maturity, Period(fixedLegFrequency),
                               index->fixingCalendar(),
                               Unadjusted, Unadjusted,
                               DateGeneration::Forward, false);
        Leg fixedLeg = FixedRateLeg(fixedSchedule)
            .withNotionals(nominals)
            .withCouponRates(fixedRate, fixedLegDayCounter)
            .withPaymentAdjustment(index->businessDayConvention());

        Swap swap(floatingLeg, fixedLeg);
        swap.setPricingEngine(boost::shared_ptr<PricingEngine>(
                            new DiscountingSwapEngine(termStructure, false)));
        Rate fairRate = fixedRate - swap.NPV()/(swap.legBPS(1)/1.0e-4);
        engine_ = boost::shared_ptr<PricingEngine>();
        cap_ = boost::shared_ptr<Cap>(new Cap(floatingLeg,
                                              std::vector<Rate>(1, fairRate)));
        marketValue_ = blackPrice(volatility_->value());
    }

    void CapHelper::addTimesTo(std::list<Time>& times) const {
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
        cap_->setPricingEngine(engine_);
        return cap_->NPV();
    }

    Real CapHelper::blackPrice(Volatility sigma) const {
        boost::shared_ptr<Quote> vol(new SimpleQuote(sigma));
        boost::shared_ptr<PricingEngine> black(
                                 new BlackCapFloorEngine(termStructure_,
                                                         Handle<Quote>(vol)));
        cap_->setPricingEngine(black);
        Real value = cap_->NPV();
        cap_->setPricingEngine(engine_);
        return value;
    }

}
