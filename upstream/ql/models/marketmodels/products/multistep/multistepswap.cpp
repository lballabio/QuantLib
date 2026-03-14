/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Giorgio Facchinetti

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
#include <ql/models/marketmodels/products/multistep/multistepswap.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <utility>

namespace QuantLib {

    MultiStepSwap::MultiStepSwap(const std::vector<Time>& rateTimes,
                                 std::vector<Real> fixedAccruals,
                                 std::vector<Real> floatingAccruals,
                                 const std::vector<Time>& paymentTimes,
                                 Real fixedRate,
                                 bool payer)
    : MultiProductMultiStep(rateTimes), fixedAccruals_(std::move(fixedAccruals)),
      floatingAccruals_(std::move(floatingAccruals)), paymentTimes_(paymentTimes),
      fixedRate_(fixedRate), multiplier_(payer ? 1.0 : -1.0), lastIndex_(rateTimes.size() - 1) {
        checkIncreasingTimes(paymentTimes);
    }

    bool MultiStepSwap::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&
                                                                 genCashFlows)
    {
        Rate liborRate = currentState.forwardRate(currentIndex_);

        genCashFlows[0][0].timeIndex = currentIndex_;
        genCashFlows[0][0].amount =
            -multiplier_*fixedRate_*fixedAccruals_[currentIndex_];

        genCashFlows[0][1].timeIndex = currentIndex_;
        genCashFlows[0][1].amount =
            multiplier_*liborRate*floatingAccruals_[currentIndex_];

        numberCashFlowsThisStep[0] = 2;

        ++currentIndex_;

        return (currentIndex_ == lastIndex_);
    }

    std::unique_ptr<MarketModelMultiProduct>
    MultiStepSwap::clone() const {
        return std::unique_ptr<MarketModelMultiProduct>(new MultiStepSwap(*this));
    }

}

