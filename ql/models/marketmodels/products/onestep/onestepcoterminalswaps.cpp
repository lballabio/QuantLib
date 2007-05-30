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

#include <ql/models/marketmodels/products/onestep/onestepcoterminalswaps.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/utilities.hpp>

namespace QuantLib {

    OneStepCoterminalSwaps::OneStepCoterminalSwaps(
        const std::vector<Time>& rateTimes,
        const std::vector<Real>& fixedAccruals,
        const std::vector<Real>& floatingAccruals,
        const std::vector<Time>& paymentTimes,
        double fixedRate)
    : MultiProductOneStep(rateTimes),
      fixedAccruals_(fixedAccruals), floatingAccruals_(floatingAccruals),
      paymentTimes_(paymentTimes), fixedRate_(fixedRate) {
        checkIncreasingTimes(paymentTimes);

        lastIndex_ = rateTimes.size()-1;
    }

    bool OneStepCoterminalSwaps::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&
                                                               genCashFlows) {
        std::fill(numberCashFlowsThisStep.begin(),
                  numberCashFlowsThisStep.end(),0);

        for (Size indexOfTime=0;indexOfTime<lastIndex_;indexOfTime++) {
            Rate liborRate = currentState.forwardRate(indexOfTime);

            for (Size i=0;i<=indexOfTime;i++) {
                genCashFlows[i][(indexOfTime-i)*2].timeIndex = indexOfTime;
                genCashFlows[i][(indexOfTime-i)*2].amount =
                    -fixedRate_*fixedAccruals_[indexOfTime];

                genCashFlows[i][(indexOfTime-i)*2+1].timeIndex = indexOfTime;
                genCashFlows[i][(indexOfTime-i)*2+1].amount =
                    liborRate*floatingAccruals_[indexOfTime];

                numberCashFlowsThisStep[i] += 2;
            }
        }
        return true ;
    }

    std::auto_ptr<MarketModelMultiProduct>
    OneStepCoterminalSwaps::clone() const {
        return std::auto_ptr<MarketModelMultiProduct>(
                                           new OneStepCoterminalSwaps(*this));
    }

}
