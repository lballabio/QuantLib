/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti

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

#include <ql/instruments/makeswaption.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/exercise.hpp>
#include <ql/settings.hpp>

namespace QuantLib {

    MakeSwaption::MakeSwaption(const boost::shared_ptr<SwapIndex>& swapIndex,
                               const Period& optionTenor,
                               Rate strike)
    : swapIndex_(swapIndex),
      delivery_(Settlement::Physical),
      optionTenor_(optionTenor),
      optionConvention_(ModifiedFollowing),
      fixingDate_(Null<Date>()),
      strike_(strike),
      underlyingType_(VanillaSwap::Payer) {}

    MakeSwaption::MakeSwaption(const boost::shared_ptr<SwapIndex>& swapIndex,
                               const Date& fixingDate,
                               Rate strike)
    : swapIndex_(swapIndex),
      delivery_(Settlement::Physical),
      optionConvention_(ModifiedFollowing),
      fixingDate_(fixingDate),
      strike_(strike),
      underlyingType_(VanillaSwap::Payer) {}

    MakeSwaption::operator Swaption() const {
        boost::shared_ptr<Swaption> swaption = *this;
        return *swaption;
    }

    MakeSwaption::operator boost::shared_ptr<Swaption>() const {

        const Date& evaluationDate = Settings::instance().evaluationDate();
        const Calendar& fixingCalendar = swapIndex_->fixingCalendar();
        if(fixingDate_ == Null<Date>())
            fixingDate_ = fixingCalendar.advance(evaluationDate, optionTenor_,
                                             optionConvention_);
        if (exerciseDate_ == Null<Date>()) {
            exercise_ = boost::shared_ptr<Exercise>(new
                EuropeanExercise(fixingDate_));
        } else {
            QL_REQUIRE(exerciseDate_ <= fixingDate_,
                       "exercise date (" << exerciseDate_ << ") must be less "
                       "than or equal to fixing date (" << fixingDate_ << ")");
            exercise_ = boost::shared_ptr<Exercise>(new
                EuropeanExercise(exerciseDate_));
        }

        Rate usedStrike = strike_;
        if (strike_ == Null<Rate>()) {
            // ATM on the forecasting curve
            QL_REQUIRE(!swapIndex_->forwardingTermStructure().empty(),
                       "null term structure set to this instance of " <<
                       swapIndex_->name());
            boost::shared_ptr<VanillaSwap> temp =
                swapIndex_->underlyingSwap(fixingDate_);
            temp->setPricingEngine(boost::shared_ptr<PricingEngine>(
                new DiscountingSwapEngine(
                                        swapIndex_->forwardingTermStructure(),
                                        false)));
            usedStrike = temp->fairRate();
        }

        BusinessDayConvention bdc = swapIndex_->fixedLegConvention();
        underlyingSwap_ =
            MakeVanillaSwap(swapIndex_->tenor(),
                            swapIndex_->iborIndex(), usedStrike)
            .withEffectiveDate(swapIndex_->valueDate(fixingDate_))
            .withFixedLegCalendar(swapIndex_->fixingCalendar())
            .withFixedLegDayCount(swapIndex_->dayCounter())
            .withFixedLegTenor(swapIndex_->fixedLegTenor())
            .withFixedLegConvention(bdc)
            .withFixedLegTerminationDateConvention(bdc)
            .withType(underlyingType_);

        boost::shared_ptr<Swaption> swaption(new
            Swaption(underlyingSwap_, exercise_, delivery_));
        swaption->setPricingEngine(engine_);
        return swaption;
    }

    MakeSwaption& MakeSwaption::withSettlementType(Settlement::Type delivery) {
        delivery_ = delivery;
        return *this;
    }

    MakeSwaption&
    MakeSwaption::withOptionConvention(BusinessDayConvention bdc) {
        optionConvention_ = bdc;
        return *this;
    }

    MakeSwaption& MakeSwaption::withExerciseDate(const Date& date) {
        exerciseDate_ = date;
        return *this;
    }

    MakeSwaption& MakeSwaption::withUnderlyingType(const VanillaSwap::Type type) {
        underlyingType_ = type;
        return *this;
    }

    MakeSwaption& MakeSwaption::withPricingEngine(
                             const boost::shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

}
