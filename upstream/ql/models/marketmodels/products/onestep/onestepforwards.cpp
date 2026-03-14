/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/products/onestep/onestepforwards.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <utility>

namespace QuantLib {

    OneStepForwards::OneStepForwards(const std::vector<Time>& rateTimes,
                                     std::vector<Real> accruals,
                                     const std::vector<Time>& paymentTimes,
                                     std::vector<Rate> strikes)
    : MultiProductOneStep(rateTimes), accruals_(std::move(accruals)), paymentTimes_(paymentTimes),
      strikes_(std::move(strikes)) {
        checkIncreasingTimes(paymentTimes);
    }

    bool OneStepForwards::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&
                                                               genCashFlows) {
        for (Size i=0; i<strikes_.size(); ++i) {
            Rate liborRate = currentState.forwardRate(i);
            genCashFlows[i][0].timeIndex = i;
            genCashFlows[i][0].amount =
                (liborRate-strikes_[i])*accruals_[i];
        }

        std::fill(numberCashFlowsThisStep.begin(),
                  numberCashFlowsThisStep.end(), 1);
        return true;
    }

    std::unique_ptr<MarketModelMultiProduct>
    OneStepForwards::clone() const {
        return std::unique_ptr<MarketModelMultiProduct>(new OneStepForwards(*this));
    }

}

