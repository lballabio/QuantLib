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

#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/products/multistep/multistepcoterminalswaps.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <utility>

namespace QuantLib {

    MultiStepCoterminalSwaps::MultiStepCoterminalSwaps(const std::vector<Time>& rateTimes,
                                                       std::vector<Real> fixedAccruals,
                                                       std::vector<Real> floatingAccruals,
                                                       const std::vector<Time>& paymentTimes,
                                                       Real fixedRate)
    : MultiProductMultiStep(rateTimes), fixedAccruals_(std::move(fixedAccruals)),
      floatingAccruals_(std::move(floatingAccruals)), paymentTimes_(paymentTimes),
      fixedRate_(fixedRate) {
        checkIncreasingTimes(paymentTimes);

        lastIndex_ = rateTimes.size()-1;
    }

    bool MultiStepCoterminalSwaps::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&
                                                               genCashFlows) {
        Rate liborRate = currentState.forwardRate(currentIndex_);
        std::fill(numberCashFlowsThisStep.begin(),
                  numberCashFlowsThisStep.end(),0);
        for(Size i=0;i<=currentIndex_;i++){
            genCashFlows[i][0].timeIndex = currentIndex_;
            genCashFlows[i][0].amount =
                -fixedRate_*fixedAccruals_[currentIndex_];

            genCashFlows[i][1].timeIndex = currentIndex_;
            genCashFlows[i][1].amount =
                liborRate*floatingAccruals_[currentIndex_];

            numberCashFlowsThisStep[i] = 2;
        }
        ++currentIndex_;

        return (currentIndex_ == lastIndex_);
    }

    std::unique_ptr<MarketModelMultiProduct>
    MultiStepCoterminalSwaps::clone() const {
        return std::unique_ptr<MarketModelMultiProduct>(new MultiStepCoterminalSwaps(*this));
    }

}
