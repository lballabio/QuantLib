/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/time/schedule.hpp>
#include <ql/instruments/swap.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    CapHelper::CapHelper(const Period& length,
                         const Handle<Quote>& volatility,
                         const boost::shared_ptr<IborIndex>& index,
                         Frequency fixedLegFrequency,
                         const DayCounter& fixedLegDayCounter,
                         bool includeFirstSwaplet,
                         const Handle<YieldTermStructure>& termStructure,
                         bool calibrateVolatility)
    : CalibrationHelper(volatility,termStructure,calibrateVolatility) {

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
                               index->businessDayConvention(), false, false);
        Leg floatingLeg = IborLeg(nominals,
                                  floatSchedule,
                                  index,
                                  DayCounter(),
                                  index->businessDayConvention(),
                                  std::vector<Natural>(1,0));
        boost::shared_ptr<IborCouponPricer> fictitiousPricer(new
            BlackIborCouponPricer(Handle<CapletVolatilityStructure>()));
        setCouponPricer(floatingLeg, fictitiousPricer);

        Schedule fixedSchedule(startDate, maturity, Period(fixedLegFrequency),
                               index->fixingCalendar(),
                               Unadjusted, Unadjusted,
                               false, false);
        Leg fixedLeg = FixedRateLeg(nominals,
                                    fixedSchedule,
                                    std::vector<Rate>(1, fixedRate),
                                    fixedLegDayCounter,
                                    index->businessDayConvention());

        boost::shared_ptr<Swap> swap(
            new Swap(termStructure, floatingLeg, fixedLeg));
        Rate fairRate = fixedRate - swap->NPV()/(swap->legBPS(1)/1.0e-4);
        engine_  = boost::shared_ptr<PricingEngine>();
        cap_ = boost::shared_ptr<Cap>(new Cap(floatingLeg,
                                              std::vector<Rate>(1, fairRate),
                                              termStructure, engine_));
        marketValue_ = blackPrice(volatility_->value());
    }

    void CapHelper::addTimesTo(std::list<Time>& times) const {
        CapFloor::arguments args;
        cap_->setupArguments(&args);
        std::vector<Time> capTimes =
            DiscretizedCapFloor(args).mandatoryTimes();
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
                                 new BlackCapFloorEngine(Handle<Quote>(vol)));
        cap_->setPricingEngine(black);
        Real value = cap_->NPV();
        cap_->setPricingEngine(engine_);
        return value;
    }

}
