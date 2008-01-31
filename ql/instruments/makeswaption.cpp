/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
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
      optionTenor_(optionTenor),
      strike_(strike),
      delivery_(Settlement::Physical),
      swaptionConvention_(Following) {}

    MakeSwaption::operator Swaption() const {
        boost::shared_ptr<Swaption> swaption = *this;
        return *swaption;
    }

    MakeSwaption::operator boost::shared_ptr<Swaption>() const {
        Date evaluationDate = Settings::instance().evaluationDate();
        Date optionDate =
            swapIndex_->fixingCalendar().advance(evaluationDate,
                                                 optionTenor_,
                                                 swaptionConvention_);
        exercise_ = boost::shared_ptr<Exercise>(new
            EuropeanExercise(optionDate));

        Rate usedStrike = strike_;
        if (strike_ == Null<Rate>()) {
            // ATM on the forecasting curve
            QL_REQUIRE(!swapIndex_->termStructure().empty(),
                       "no forecasting term structure set to " <<
                       swapIndex_->name());
            boost::shared_ptr<VanillaSwap> temp =
                swapIndex_->underlyingSwap(optionDate);
            temp->setPricingEngine(boost::shared_ptr<PricingEngine>(new
                        DiscountingSwapEngine(swapIndex_->termStructure())));
            usedStrike = temp->fairRate();
        }

        BusinessDayConvention bdc = swapIndex_->fixedLegConvention();
        underlyingSwap_ =
            MakeVanillaSwap(swapIndex_->tenor(),
                            swapIndex_->iborIndex(), usedStrike)
            .withEffectiveDate(swapIndex_->valueDate(optionDate))
            .withFixedLegCalendar(swapIndex_->fixingCalendar())
            .withFixedLegDayCount(swapIndex_->dayCounter())
            .withFixedLegConvention(bdc)
            .withFixedLegTerminationDateConvention(bdc);

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
    MakeSwaption::withSwaptionConvention(BusinessDayConvention bdc) {
        swaptionConvention_ = bdc;
        return *this;
    }

    MakeSwaption& MakeSwaption::withPricingEngine(
                             const boost::shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

}
