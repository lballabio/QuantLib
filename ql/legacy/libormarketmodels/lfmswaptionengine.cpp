/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen

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

#include <ql/legacy/libormarketmodels/lfmswaptionengine.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <utility>

namespace QuantLib {

    LfmSwaptionEngine::LfmSwaptionEngine(const ext::shared_ptr<LiborForwardModel>& model,
                                         Handle<YieldTermStructure> discountCurve)
    : GenericModelEngine<LiborForwardModel, Swaption::arguments, Swaption::results>(model),
      discountCurve_(std::move(discountCurve)) {
        registerWith(discountCurve_);
    }


    void LfmSwaptionEngine::calculate() const {

        QL_REQUIRE(arguments_.settlementMethod != Settlement::ParYieldCurve,
                   "cash settled (ParYieldCurve) swaptions not priced with Lfm engine");

        static const Spread basisPoint = 1.0e-4;

        VanillaSwap swap = *arguments_.swap;
        swap.setPricingEngine(ext::shared_ptr<PricingEngine>(
                           new DiscountingSwapEngine(discountCurve_, false)));

        Spread correction = swap.spread() *
            std::fabs(swap.floatingLegBPS()/swap.fixedLegBPS());
        Rate fixedRate = swap.fixedRate() - correction;
        Rate fairRate = swap.fairRate() - correction;

        ext::shared_ptr<SwaptionVolatilityMatrix> volatility =
            model_->getSwaptionVolatilityMatrix();

        Date referenceDate = volatility->referenceDate();
        DayCounter dayCounter = volatility->dayCounter();

        Time exercise = dayCounter.yearFraction(referenceDate,
                                                arguments_.exercise->date(0));
        Time swapLength =
            dayCounter.yearFraction(referenceDate,
                                    arguments_.fixedPayDates.back())
            - dayCounter.yearFraction(referenceDate,
                                      arguments_.fixedResetDates[0]);

        Option::Type w = arguments_.type==Swap::Payer ? Option::Call : Option::Put;
        Volatility vol = volatility->volatility(exercise, swapLength,
                                                fairRate, true);
        results_.value = (swap.fixedLegBPS()/basisPoint) *
            blackFormula(w, fixedRate, fairRate, vol*std::sqrt(exercise));
    }

}

