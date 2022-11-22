/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi

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

#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/products/multistep/multistepoptionlets.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <ql/payoff.hpp>
#include <utility>

namespace QuantLib {

    MultiStepOptionlets::MultiStepOptionlets(const std::vector<Time>& rateTimes,
                                             std::vector<Real> accruals,
                                             const std::vector<Time>& paymentTimes,
                                             std::vector<ext::shared_ptr<Payoff> > payoffs)
    : MultiProductMultiStep(rateTimes), accruals_(std::move(accruals)), paymentTimes_(paymentTimes),
      payoffs_(std::move(payoffs)) {
        checkIncreasingTimes(paymentTimes);
    }

    bool MultiStepOptionlets::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&
                                                               genCashFlows) {
        Rate liborRate = currentState.forwardRate(currentIndex_);
        genCashFlows[currentIndex_][0].timeIndex = currentIndex_;
        genCashFlows[currentIndex_][0].amount =
            (*payoffs_[currentIndex_])(liborRate) *
            accruals_[currentIndex_];
        std::fill(numberCashFlowsThisStep.begin(),
                  numberCashFlowsThisStep.end(), 0);
        numberCashFlowsThisStep[currentIndex_] = 1;
        ++currentIndex_;
        return (currentIndex_ == payoffs_.size());
    }

    std::unique_ptr<MarketModelMultiProduct>
    MultiStepOptionlets::clone() const {
        return std::unique_ptr<MarketModelMultiProduct>(new MultiStepOptionlets(*this));
    }

}
