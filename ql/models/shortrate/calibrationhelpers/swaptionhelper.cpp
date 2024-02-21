/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2015 Peter Caspers

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

#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/overnightindexedswap.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/models/shortrate/calibrationhelpers/swaptionhelper.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
#include <ql/pricingengines/swaption/discretizedswaption.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/schedule.hpp>
#include <utility>

namespace QuantLib {

    SwaptionHelper::SwaptionHelper(const Period& maturity,
                                   const Period& length,
                                   const Handle<Quote>& volatility,
                                   ext::shared_ptr<IborIndex> index,
                                   const Period& fixedLegTenor,
                                   DayCounter fixedLegDayCounter,
                                   DayCounter floatingLegDayCounter,
                                   Handle<YieldTermStructure> termStructure,
                                   BlackCalibrationHelper::CalibrationErrorType errorType,
                                   const Real strike,
                                   const Real nominal,
                                   const VolatilityType type,
                                   const Real shift,
                                   Natural settlementDays,
                                   RateAveraging::Type averagingMethod)
    : BlackCalibrationHelper(volatility, errorType, type, shift), exerciseDate_(Null<Date>()),
      endDate_(Null<Date>()), maturity_(maturity), length_(length), fixedLegTenor_(fixedLegTenor),
      index_(std::move(index)), termStructure_(std::move(termStructure)),
      fixedLegDayCounter_(std::move(fixedLegDayCounter)),
      floatingLegDayCounter_(std::move(floatingLegDayCounter)), strike_(strike), nominal_(nominal),
      settlementDays_(settlementDays), averagingMethod_(averagingMethod) {
        registerWith(index_);
        registerWith(termStructure_);
    }

    SwaptionHelper::SwaptionHelper(const Date& exerciseDate,
                                   const Period& length,
                                   const Handle<Quote>& volatility,
                                   ext::shared_ptr<IborIndex> index,
                                   const Period& fixedLegTenor,
                                   DayCounter fixedLegDayCounter,
                                   DayCounter floatingLegDayCounter,
                                   Handle<YieldTermStructure> termStructure,
                                   BlackCalibrationHelper::CalibrationErrorType errorType,
                                   const Real strike,
                                   const Real nominal,
                                   const VolatilityType type,
                                   const Real shift,
                                   Natural settlementDays,
                                   RateAveraging::Type averagingMethod)
    : BlackCalibrationHelper(volatility, errorType, type, shift), exerciseDate_(exerciseDate),
      endDate_(Null<Date>()), maturity_(0 * Days), length_(length), fixedLegTenor_(fixedLegTenor),
      index_(std::move(index)), termStructure_(std::move(termStructure)),
      fixedLegDayCounter_(std::move(fixedLegDayCounter)),
      floatingLegDayCounter_(std::move(floatingLegDayCounter)), strike_(strike), nominal_(nominal),
      settlementDays_(settlementDays), averagingMethod_(averagingMethod) {
        registerWith(index_);
        registerWith(termStructure_);
    }

    SwaptionHelper::SwaptionHelper(const Date& exerciseDate,
                                   const Date& endDate,
                                   const Handle<Quote>& volatility,
                                   ext::shared_ptr<IborIndex> index,
                                   const Period& fixedLegTenor,
                                   DayCounter fixedLegDayCounter,
                                   DayCounter floatingLegDayCounter,
                                   Handle<YieldTermStructure> termStructure,
                                   BlackCalibrationHelper::CalibrationErrorType errorType,
                                   const Real strike,
                                   const Real nominal,
                                   const VolatilityType type,
                                   const Real shift,
                                   Natural settlementDays,
                                   RateAveraging::Type averagingMethod)
    : BlackCalibrationHelper(volatility, errorType, type, shift), exerciseDate_(exerciseDate),
      endDate_(endDate), maturity_(0 * Days), length_(0 * Days), fixedLegTenor_(fixedLegTenor),
      index_(std::move(index)), termStructure_(std::move(termStructure)),
      fixedLegDayCounter_(std::move(fixedLegDayCounter)),
      floatingLegDayCounter_(std::move(floatingLegDayCounter)), strike_(strike), nominal_(nominal),
      settlementDays_(settlementDays), averagingMethod_(averagingMethod) {
        registerWith(index_);
        registerWith(termStructure_);
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
        Handle<Quote> vol(ext::shared_ptr<Quote>(new SimpleQuote(sigma)));
        ext::shared_ptr<PricingEngine> engine;
        switch(volatilityType_) {
        case ShiftedLognormal:
            engine = ext::make_shared<BlackSwaptionEngine>(
                termStructure_, vol, Actual365Fixed(), shift_);
            break;
        case Normal:
            engine = ext::make_shared<BachelierSwaptionEngine>(
                termStructure_, vol, Actual365Fixed());
            break;
        default:
            QL_FAIL("can not construct engine: " << volatilityType_);
            break;
        }
        swaption_->setPricingEngine(engine);
        Real value = swaption_->NPV();
        swaption_->setPricingEngine(engine_);
        return value;
    }

    void SwaptionHelper::performCalculations() const {

        Calendar calendar = index_->fixingCalendar();
        Date exerciseDate = exerciseDate_;
        if (exerciseDate == Null<Date>())
            exerciseDate = calendar.advance(termStructure_->referenceDate(),
                                            maturity_,
                                            index_->businessDayConvention());
        Date startDate;
        if (settlementDays_ == Null<Size>()) {
            startDate = index_->valueDate(index_->fixingCalendar().adjust(exerciseDate));
        } else {
            startDate = calendar.advance(exerciseDate,
                                         index_->fixingDays(), Days,
                                         index_->businessDayConvention());
        }

        Date endDate = endDate_;
        if (endDate == Null<Date>())
            endDate = calendar.advance(startDate, length_,
                                       index_->businessDayConvention());
        auto onIndex = ext::dynamic_pointer_cast<OvernightIndex>(index_);
        Schedule fixedSchedule(startDate, endDate, fixedLegTenor_, calendar,
                               index_->businessDayConvention(),
                               index_->businessDayConvention(),
                               DateGeneration::Forward, false);
        Schedule floatSchedule(startDate, endDate, index_->tenor(), calendar,
                               index_->businessDayConvention(),
                               index_->businessDayConvention(),
                               DateGeneration::Forward, false);

        ext::shared_ptr<PricingEngine> swapEngine(
                             new DiscountingSwapEngine(termStructure_, false));

        Swap::Type type = Swap::Receiver;
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));
        std::unique_ptr<FixedVsFloatingSwap> temp;
        if (onIndex) {
            // construct ois swap
            temp = std::make_unique<OvernightIndexedSwap>(
                Swap::Receiver, nominal_, fixedSchedule, 0.0,
                fixedLegDayCounter_, onIndex, 0.0, 0, Following, Calendar(),
                true, averagingMethod_
            );
        } else {
            temp = std::make_unique<VanillaSwap>(
                 Swap::Receiver, nominal_, fixedSchedule, 0.0, fixedLegDayCounter_,
                 floatSchedule, index_, 0.0, floatingLegDayCounter_
            );
        }
        temp->setPricingEngine(swapEngine);
        Real forward = temp->fairRate();
        if (strike_ == Null<Real>()) {
            exerciseRate_ = forward;
        } else {
            exerciseRate_ = strike_;
            type = strike_ <= forward ? Swap::Receiver : Swap::Payer;
        }
        if (onIndex) {
            swap_ = ext::make_shared<OvernightIndexedSwap>(
                type, nominal_, fixedSchedule, 0.0, fixedLegDayCounter_, onIndex, 0.0, 0, Following,
                Calendar(), true, averagingMethod_);
        } else {
            swap_ = ext::make_shared<VanillaSwap>(type, nominal_, fixedSchedule, exerciseRate_,
                                                  fixedLegDayCounter_, floatSchedule, index_, 0.0,
                                                  floatingLegDayCounter_);
        }
        swap_->setPricingEngine(swapEngine);
        swaption_ = ext::make_shared<Swaption>(swap_, exercise);
        BlackCalibrationHelper::performCalculations();
    }
}
