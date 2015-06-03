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

    SwaptionHelper::SwaptionHelper(const Period& maturity,
                              const Period& length,
                              const Handle<Quote>& volatility,
                              const boost::shared_ptr<IborIndex>& index,
                              const Period& fixedLegTenor,
                              const DayCounter& fixedLegDayCounter,
                              const DayCounter& floatingLegDayCounter,
                              const Handle<YieldTermStructure>& termStructure,
                              CalibrationHelper::CalibrationErrorType errorType,
                              const Real strike, const Real nominal, const Real shift)
    : CalibrationHelper(volatility,termStructure, errorType),
        exerciseDate_(Null<Date>()), endDate_(Null<Date>()),
        maturity_(maturity), length_(length), fixedLegTenor_(fixedLegTenor), index_(index),
        fixedLegDayCounter_(fixedLegDayCounter), floatingLegDayCounter_(floatingLegDayCounter),
        strike_(strike), nominal_(nominal), shift_(shift)
    {

        registerWith(index_);

    }

    SwaptionHelper::SwaptionHelper(
                              const Date& exerciseDate,
                              const Period& length,
                              const Handle<Quote>& volatility,
                              const boost::shared_ptr<IborIndex>& index,
                              const Period& fixedLegTenor,
                              const DayCounter& fixedLegDayCounter,
                              const DayCounter& floatingLegDayCounter,
                              const Handle<YieldTermStructure>& termStructure,
                              CalibrationHelper::CalibrationErrorType errorType,
                              const Real strike, const Real nominal, const Real shift)
    : CalibrationHelper(volatility,termStructure, errorType),
        exerciseDate_(exerciseDate), endDate_(Null<Date>()),
        maturity_(0*Days), length_(length), fixedLegTenor_(fixedLegTenor), index_(index),
        fixedLegDayCounter_(fixedLegDayCounter), floatingLegDayCounter_(floatingLegDayCounter),
        strike_(strike), nominal_(nominal), shift_(shift)
    {

        registerWith(index_);

    }

    SwaptionHelper::SwaptionHelper(
                              const Date& exerciseDate,
                              const Date& endDate,
                              const Handle<Quote>& volatility,
                              const boost::shared_ptr<IborIndex>& index,
                              const Period& fixedLegTenor,
                              const DayCounter& fixedLegDayCounter,
                              const DayCounter& floatingLegDayCounter,
                              const Handle<YieldTermStructure>& termStructure,
                              CalibrationHelper::CalibrationErrorType errorType,
                              const Real strike, const Real nominal, const Real shift)
    : CalibrationHelper(volatility,termStructure, errorType),
        exerciseDate_(exerciseDate), endDate_(endDate),
        maturity_(0*Days), length_(0*Days), fixedLegTenor_(fixedLegTenor), index_(index),
        fixedLegDayCounter_(fixedLegDayCounter), floatingLegDayCounter_(floatingLegDayCounter),
        strike_(strike), nominal_(nominal), shift_(shift)
    {

        registerWith(index_);

    }

    void SwaptionHelper::addTimesTo(std::list<Time>& times) const {
        calculate();
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
        calculate();
        swaption_->setPricingEngine(engine_);
        return swaption_->NPV();
    }

    Real SwaptionHelper::blackPrice(Volatility sigma) const {
        calculate();
        Handle<Quote> vol(boost::shared_ptr<Quote>(new SimpleQuote(sigma)));
        boost::shared_ptr<PricingEngine> black(new
        BlackSwaptionEngine(termStructure_, vol, Actual365Fixed(), shift_));
        swaption_->setPricingEngine(black);
        Real value = swaption_->NPV();
        swaption_->setPricingEngine(engine_);
        return value;
    }

    void SwaptionHelper::performCalculations() const {

        Calendar calendar = index_->fixingCalendar();
        Natural fixingDays = index_->fixingDays();

        Date exerciseDate = exerciseDate_;
        if (exerciseDate == Null<Date>())
            exerciseDate = calendar.advance(termStructure_->referenceDate(),
                                            maturity_,
                                            index_->businessDayConvention());

        Date startDate = calendar.advance(exerciseDate,
                                    fixingDays, Days,
                                    index_->businessDayConvention());

        Date endDate = endDate_;
        if (endDate == Null<Date>())
            endDate = calendar.advance(startDate, length_,
                                       index_->businessDayConvention());

        Schedule fixedSchedule(startDate, endDate, fixedLegTenor_, calendar,
                               index_->businessDayConvention(),
                               index_->businessDayConvention(),
                               DateGeneration::Forward, false);
        Schedule floatSchedule(startDate, endDate, index_->tenor(), calendar,
                               index_->businessDayConvention(),
                               index_->businessDayConvention(),
                               DateGeneration::Forward, false);

        boost::shared_ptr<PricingEngine> swapEngine(
                             new DiscountingSwapEngine(termStructure_, false));

        VanillaSwap::Type type = VanillaSwap::Receiver;

        VanillaSwap temp(VanillaSwap::Receiver, nominal_,
                            fixedSchedule, 0.0, fixedLegDayCounter_,
                            floatSchedule, index_, 0.0, floatingLegDayCounter_);
        temp.setPricingEngine(swapEngine);
        Real forward = temp.fairRate();
        if(strike_ == Null<Real>()) {
            exerciseRate_ = forward;
        }
        else {
            exerciseRate_ = strike_;
            type = strike_ <= forward ? VanillaSwap::Receiver : VanillaSwap::Payer;
            // ensure that calibration instrument is out of the money
        }
        swap_ = boost::shared_ptr<VanillaSwap>(
            new VanillaSwap(type, nominal_,
                            fixedSchedule, exerciseRate_, fixedLegDayCounter_,
                            floatSchedule, index_, 0.0, floatingLegDayCounter_));
        swap_->setPricingEngine(swapEngine);

        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

        swaption_ = boost::shared_ptr<Swaption>(new Swaption(swap_, exercise));

        CalibrationHelper::performCalculations();

    }

}
