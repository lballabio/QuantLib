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
#include <ql/models/marketmodels/products/onestep/onestepoptionlets.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <ql/payoff.hpp>
#include <utility>

namespace QuantLib {

    OneStepOptionlets::OneStepOptionlets(const std::vector<Time>& rateTimes,
                                         std::vector<Real> accruals,
                                         const std::vector<Time>& paymentTimes,
                                         std::vector<std::shared_ptr<Payoff> > payoffs)
    : MultiProductOneStep(rateTimes), accruals_(std::move(accruals)), paymentTimes_(paymentTimes),
      payoffs_(std::move(payoffs)) {
        checkIncreasingTimes(paymentTimes);
    }

    bool OneStepOptionlets::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&
                                                               genCashFlows) {
        std::fill(numberCashFlowsThisStep.begin(),
                  numberCashFlowsThisStep.end(), 0);
        for (Size i=0; i<payoffs_.size(); ++i) {
            Rate liborRate = currentState.forwardRate(i);
            Real payoff = (*payoffs_[i])(liborRate);
            if (payoff>0.0) {
                numberCashFlowsThisStep[i] = 1;
                genCashFlows[i][0].timeIndex = i;
                genCashFlows[i][0].amount = payoff*accruals_[i];
            }
        }

        return true;
    }

    std::unique_ptr<MarketModelMultiProduct>
    OneStepOptionlets::clone() const {
        return std::unique_ptr<MarketModelMultiProduct>(new OneStepOptionlets(*this));
    }

}
