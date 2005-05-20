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

    #ifndef QL_DISABLE_DEPRECATED
    SwaptionHelper::SwaptionHelper(
                              const Period& maturity,
                              const Period& length,
                              const Handle<Quote>& volatility,
                              const boost::shared_ptr<Xibor>& index,
                              const Handle<YieldTermStructure>& termStructure)
    : CalibrationHelper(volatility,termStructure) {

        Calendar calendar = index->calendar();
        Period indexTenor = index->tenor();
        Frequency frequency = index->frequency();
        Date startDate =
            calendar.advance(termStructure->referenceDate(),
                             maturity.length(), maturity.units());
        Date endDate =
            calendar.advance(startDate, length.length(), length.units(),
                             index->businessDayConvention());
        Schedule fixedSchedule(calendar, startDate, endDate,
                               frequency, Unadjusted);
        Schedule floatSchedule(calendar, startDate, endDate,
                               frequency, index->businessDayConvention());

        Rate fixedRate = 0.04;//dummy value
        swap_ = boost::shared_ptr<SimpleSwap>(
                      new SimpleSwap(false, 1.0, fixedSchedule, fixedRate,
                                     index->dayCounter(), floatSchedule,
                                     index, 0, 0.0, termStructure));
        Rate fairFixedRate = swap_->fairRate();
        swap_ = boost::shared_ptr<SimpleSwap>(
                      new SimpleSwap(false, 1.0, fixedSchedule, fairFixedRate,
                                     index->dayCounter(), floatSchedule,
                                     index, 0, 0.0, termStructure));
        exerciseRate_ = fairFixedRate;
        engine_  = boost::shared_ptr<PricingEngine>();
        Date exerciseDate = calendar.adjust(startDate,
                                            index->businessDayConvention());

        swaption_ = boost::shared_ptr<Swaption>(new Swaption(
            swap_,
            boost::shared_ptr<Exercise>(new EuropeanExercise(exerciseDate)),
            termStructure,
            engine_));
        marketValue_ = blackPrice(volatility_->value());
    }
    #endif

    SwaptionHelper::SwaptionHelper(
                              const Period& maturity,
                              const Period& length,
                              const Handle<Quote>& volatility,
                              const boost::shared_ptr<Xibor>& index,
                              Frequency fixedLegFrequency,
                              const DayCounter& fixedLegDayCounter,
                              const Handle<YieldTermStructure>& termStructure)
    : CalibrationHelper(volatility,termStructure) {

        Calendar calendar = index->calendar();
        Period indexTenor = index->tenor();
        Date startDate =
            calendar.advance(termStructure->referenceDate(),
                             maturity.length(), maturity.units(),
                             index->businessDayConvention());
        Date endDate =
            calendar.advance(startDate, length.length(), length.units(),
                             index->businessDayConvention());
        Schedule fixedSchedule(calendar, startDate, endDate,
                               fixedLegFrequency, Unadjusted);
        Schedule floatSchedule(calendar, startDate, endDate,
                               index->frequency(),
                               index->businessDayConvention());

        Rate fixedRate = 0.04;//dummy value
        swap_ = boost::shared_ptr<SimpleSwap>(
                      new SimpleSwap(false, 1.0, fixedSchedule, fixedRate,
                                     fixedLegDayCounter, floatSchedule,
                                     index, 0, 0.0, termStructure));
        Rate fairFixedRate = swap_->fairRate();
        swap_ = boost::shared_ptr<SimpleSwap>(
                      new SimpleSwap(false, 1.0, fixedSchedule, fairFixedRate,
                                     fixedLegDayCounter, floatSchedule,
                                     index, 0, 0.0, termStructure));

        exerciseRate_ = fairFixedRate;
        engine_  = boost::shared_ptr<PricingEngine>();
        swaption_ = boost::shared_ptr<Swaption>(new Swaption(
            swap_,
            boost::shared_ptr<Exercise>(new EuropeanExercise(startDate)),
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
