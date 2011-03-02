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

#include <ql/models/shortrate/calibrationhelpers/swaptionhelper.hpp>
#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
#include <ql/pricingengines/swaption/discretizedswaption.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/time/schedule.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    SwaptionHelper::SwaptionHelper(
                              const Period& maturity,
                              const Period& length,
                              const Handle<Quote>& volatility,
                              const boost::shared_ptr<IborIndex>& index,
                              const Period& fixedLegTenor,
                              const DayCounter& fixedLegDayCounter,
                              const DayCounter& floatingLegDayCounter,
                              const Handle<YieldTermStructure>& termStructure,
                              CalibrationHelper::CalibrationErrorType errorType)
    : CalibrationHelper(volatility,termStructure, errorType) {

        Calendar calendar = index->fixingCalendar();
        Period indexTenor = index->tenor();
        Natural fixingDays = index->fixingDays();
        Date exerciseDate = calendar.advance(termStructure->referenceDate(),
                                             maturity,
                                             index->businessDayConvention());
        Date startDate = calendar.advance(exerciseDate,
                                          fixingDays, Days,
                                          index->businessDayConvention());
        Date endDate = calendar.advance(startDate, length,
                                        index->businessDayConvention());

        Schedule fixedSchedule(startDate, endDate, fixedLegTenor, calendar,
                               index->businessDayConvention(),
                               index->businessDayConvention(),
                               DateGeneration::Forward, false);
        Schedule floatSchedule(startDate, endDate, index->tenor(), calendar,
                               index->businessDayConvention(),
                               index->businessDayConvention(),
                               DateGeneration::Forward, false);

        boost::shared_ptr<PricingEngine> swapEngine(
                             new DiscountingSwapEngine(termStructure, false));

        VanillaSwap temp(VanillaSwap::Receiver, 1.0,
                         fixedSchedule, 0.0, fixedLegDayCounter,
                         floatSchedule, index, 0.0, floatingLegDayCounter);
        temp.setPricingEngine(swapEngine);
        exerciseRate_ = temp.fairRate();
        swap_ = boost::shared_ptr<VanillaSwap>(
            new VanillaSwap(VanillaSwap::Receiver, 1.0,
                            fixedSchedule, exerciseRate_, fixedLegDayCounter,
                            floatSchedule, index, 0.0, floatingLegDayCounter));
        swap_->setPricingEngine(swapEngine);

        engine_  = boost::shared_ptr<PricingEngine>();
        boost::shared_ptr<Exercise> exercise(
                                          new EuropeanExercise(exerciseDate));
        swaption_ = boost::shared_ptr<Swaption>(new Swaption(swap_, exercise));
        marketValue_ = blackPrice(volatility_->value());
    }

    void SwaptionHelper::addTimesTo(std::list<Time>& times) const {
        Swaption::arguments args;
        swaption_->setupArguments(&args);
        std::vector<Time> swaptionTimes =
            DiscretizedSwaption(args,
                                termStructure_->referenceDate(),
                                termStructure_->dayCounter()).mandatoryTimes();
        times.insert(times.end(),
                     swaptionTimes.begin(), swaptionTimes.end());
    }

    Real SwaptionHelper::modelValue() const {
        swaption_->setPricingEngine(engine_);
        return swaption_->NPV();
    }

    Real SwaptionHelper::blackPrice(Volatility sigma) const {
        Handle<Quote> vol(boost::shared_ptr<Quote>(new SimpleQuote(sigma)));
        boost::shared_ptr<PricingEngine> black(new
                                    BlackSwaptionEngine(termStructure_, vol));
        swaption_->setPricingEngine(black);
        Real value = swaption_->NPV();
        swaption_->setPricingEngine(engine_);
        return value;
    }

}
