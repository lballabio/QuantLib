/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013, 2018 Peter Caspers

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

#include <ql/exercise.hpp>
#include <ql/instruments/floatfloatswaption.hpp>
#include <utility>

namespace QuantLib {

    FloatFloatSwaption::FloatFloatSwaption(ext::shared_ptr<FloatFloatSwap> swap,
                                           const ext::shared_ptr<Exercise>& exercise,
                                           Settlement::Type delivery,
                                           Settlement::Method settlementMethod)
    : Option(ext::shared_ptr<Payoff>(), exercise), swap_(std::move(swap)),
      settlementType_(delivery), settlementMethod_(settlementMethod) {
        registerWith(swap_);
        // When we ask for the NPV of an expired swaption, the
        // swap is not recalculated and thus wouldn't forward
        // later notifications according to the default behavior of
        // LazyObject instances.  This means that even if the
        // evaluation date changes so that the swaption is no longer
        // expired, the instrument wouldn't be notified and thus it
        // wouldn't recalculate.  To avoid this, we override the
        // default behavior of the underlying swap.
        swap_->alwaysForwardNotifications();
    }

    bool FloatFloatSwaption::isExpired() const {
        return detail::simple_event(exercise_->dates().back()).hasOccurred();
    }

    void
    FloatFloatSwaption::setupArguments(PricingEngine::arguments *args) const {

        swap_->setupArguments(args);

        auto* arguments = dynamic_cast<FloatFloatSwaption::arguments*>(args);

        QL_REQUIRE(arguments != nullptr, "wrong argument type");

        arguments->swap = swap_;
        arguments->exercise = exercise_;
        arguments->settlementType = settlementType_;
        arguments->settlementMethod = settlementMethod_;
    }

    void FloatFloatSwaption::arguments::validate() const {
        FloatFloatSwap::arguments::validate();
        QL_REQUIRE(swap, "underlying cms swap not set");
        QL_REQUIRE(exercise, "exercise not set");
        Settlement::checkTypeAndMethodConsistency(settlementType,
                                                  settlementMethod);
    }

    std::vector<ext::shared_ptr<BlackCalibrationHelper>>
    FloatFloatSwaption::calibrationBasket(
        const ext::shared_ptr<SwapIndex>& standardSwapBase,
        const ext::shared_ptr<SwaptionVolatilityStructure>& swaptionVolatility,
        const BasketGeneratingEngine::CalibrationBasketType basketType) const {

        ext::shared_ptr<BasketGeneratingEngine> engine =
            ext::dynamic_pointer_cast<BasketGeneratingEngine>(engine_);
        QL_REQUIRE(engine, "engine is not a basket generating engine");
        engine_->reset();
        setupArguments(engine_->getArguments());
        engine_->getArguments()->validate();
        return engine->calibrationBasket(exercise_, standardSwapBase,
                                         swaptionVolatility, basketType);
    }

}
