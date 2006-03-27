/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/ShortRateModels/CalibrationHelpers/swaptionhelper.hpp>
#include <ql/PricingEngines/Swaption/blackswaptionengine.hpp>
#include <ql/PricingEngines/Swaption/discretizedswaption.hpp>
#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/Instruments/payoffs.hpp>

namespace QuantLib {

    SwaptionHelper::SwaptionHelper(
                              const Period& maturity,
                              const Period& length,
                              const Handle<Quote>& volatility,
                              const boost::shared_ptr<Xibor>& index,
                              Frequency fixedLegFrequency,
                              const DayCounter& fixedLegDayCounter,
                              const DayCounter& floatingLegDayCounter,
                              const Handle<YieldTermStructure>& termStructure,
                              bool calibrateVolatility)
    : CalibrationHelper(volatility,termStructure, calibrateVolatility) {

        Calendar calendar = index->calendar();
        Period indexTenor = index->tenor();
        Integer fixingDays = index->settlementDays();
        Date exerciseDate = calendar.advance(termStructure->referenceDate(),
                                             maturity,
                                             index->businessDayConvention());
        Date startDate = calendar.advance(exerciseDate,
                                          fixingDays, Days,
                                          index->businessDayConvention());
        Date endDate = calendar.advance(startDate, length,
                                        index->businessDayConvention());
        Schedule fixedSchedule(calendar, startDate, endDate,
                               fixedLegFrequency,
                               index->businessDayConvention());
        Schedule floatSchedule(calendar, startDate, endDate,
                               index->frequency(),
                               index->businessDayConvention());

        Rate fixedRate = 0.0;  //dummy value
        swap_ = boost::shared_ptr<VanillaSwap>(
                      new VanillaSwap(false, 1.0, fixedSchedule, fixedRate,
                                      fixedLegDayCounter, floatSchedule,
                                      index, 0, 0.0, floatingLegDayCounter,
                                      termStructure));
        Rate fairFixedRate = swap_->fairRate();
        swap_ = boost::shared_ptr<VanillaSwap>(
                      new VanillaSwap(false, 1.0, fixedSchedule, fairFixedRate,
                                      fixedLegDayCounter, floatSchedule,
                                      index, 0, 0.0, floatingLegDayCounter,
                                      termStructure));

        exerciseRate_ = fairFixedRate;
        engine_  = boost::shared_ptr<PricingEngine>();
        swaption_ = boost::shared_ptr<Swaption>(new Swaption(
            swap_,
            boost::shared_ptr<Exercise>(new EuropeanExercise(exerciseDate)),
            termStructure,
            engine_));
        marketValue_ = blackPrice(volatility_->value());
    }

    void SwaptionHelper::addTimesTo(std::list<Time>& times) const {
        Swaption::arguments args;
        swaption_->setupArguments(&args);
        std::vector<Time> swaptionTimes =
            DiscretizedSwaption(args).mandatoryTimes();
        std::copy(swaptionTimes.begin(), swaptionTimes.end(),
                  std::back_inserter(times));
    }

    Real SwaptionHelper::modelValue() const {
        swaption_->setPricingEngine(engine_);
        return swaption_->NPV();
    }

    Real SwaptionHelper::blackPrice(Volatility sigma) const {
        boost::shared_ptr<Quote> vol(new SimpleQuote(sigma));
        boost::shared_ptr<BlackModel> blackModel(
                         new BlackModel(Handle<Quote>(vol), termStructure_));
        boost::shared_ptr<PricingEngine> black(
                                        new BlackSwaptionEngine(blackModel));
        swaption_->setPricingEngine(black);
        Real value = swaption_->NPV();
        swaption_->setPricingEngine(engine_);
        return value;
    }

}
