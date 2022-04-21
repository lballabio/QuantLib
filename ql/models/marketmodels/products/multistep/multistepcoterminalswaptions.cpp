/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Giorgio Facchinetti

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

#include <ql/instruments/payoffs.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/products/multistep/multistepcoterminalswaptions.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <utility>

namespace QuantLib {

    MultiStepCoterminalSwaptions::MultiStepCoterminalSwaptions(
        const std::vector<Time>& rateTimes,
        const std::vector<Time>& paymentTimes,
        std::vector<ext::shared_ptr<StrikedTypePayoff> > payoffs)
    : MultiProductMultiStep(rateTimes), paymentTimes_(paymentTimes), payoffs_(std::move(payoffs)) {
        checkIncreasingTimes(paymentTimes);

        lastIndex_ = rateTimes.size()-1;
    }

    bool MultiStepCoterminalSwaptions::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&
                                                               genCashFlows)
    {
        genCashFlows[currentIndex_][0].timeIndex = currentIndex_;

        Rate swapRate = currentState.coterminalSwapRate(currentIndex_);
        Real annuity = currentState.coterminalSwapAnnuity(currentIndex_, currentIndex_);
        genCashFlows[currentIndex_][0].amount =
            (*payoffs_[currentIndex_])(swapRate) * annuity;
        std::fill(numberCashFlowsThisStep.begin(),
                  numberCashFlowsThisStep.end(),0);
        numberCashFlowsThisStep[currentIndex_] = 1;
        ++currentIndex_;
        return (currentIndex_ == lastIndex_);
    }

    std::unique_ptr<MarketModelMultiProduct>
    MultiStepCoterminalSwaptions::clone() const {
        return std::unique_ptr<MarketModelMultiProduct>(new MultiStepCoterminalSwaptions(*this));
    }

}
