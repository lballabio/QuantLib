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

#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/models/shortrate/calibrationhelpers/caphelper.hpp>
#include <ql/pricingengines/capfloor/bacheliercapfloorengine.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/pricingengines/capfloor/discretizedcapfloor.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/schedule.hpp>
#include <utility>

namespace QuantLib {

    CapHelper::CapHelper(const Period& length,
                         const Handle<Quote>& volatility,
                         std::shared_ptr<IborIndex> index,
                         Frequency fixedLegFrequency,
                         DayCounter fixedLegDayCounter,
                         bool includeFirstSwaplet,
                         Handle<YieldTermStructure> termStructure,
                         BlackCalibrationHelper::CalibrationErrorType errorType,
                         const VolatilityType type,
                         const Real shift)
    : BlackCalibrationHelper(volatility, errorType, type, shift), length_(length),
      index_(std::move(index)), termStructure_(std::move(termStructure)),
      fixedLegFrequency_(fixedLegFrequency), fixedLegDayCounter_(std::move(fixedLegDayCounter)),
      includeFirstSwaplet_(includeFirstSwaplet) {
        registerWith(index_);
        registerWith(termStructure_);
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
        Handle<Quote> vol(std::shared_ptr<Quote>(new SimpleQuote(sigma)));
        std::shared_ptr<PricingEngine> engine;
        switch(volatilityType_) {
          case ShiftedLognormal:
            engine = std::make_shared<BlackCapFloorEngine>(
                termStructure_, vol, Actual365Fixed(), shift_);
            break;
          case Normal:
            engine = std::make_shared<BachelierCapFloorEngine>(
                termStructure_, vol, Actual365Fixed());
            break;
          default:
            QL_FAIL("unknown volatility type: " << volatilityType_);
        }
        cap_->setPricingEngine(engine);
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
        std::shared_ptr<IborIndex> dummyIndex(new
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
        swap.setPricingEngine(std::shared_ptr<PricingEngine>(
                            new DiscountingSwapEngine(termStructure_, false)));
        Rate fairRate = fixedRate - swap.NPV()/(swap.legBPS(1)/1.0e-4);
        cap_ = std::make_shared<Cap>(floatingLeg,
                                              std::vector<Rate>(1, fairRate));

        BlackCalibrationHelper::performCalculations();

    }


}
