/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Giorgio Facchinetti

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/MarketModels/Products/OneStep/onestepcoinitialswaps.hpp>

namespace QuantLib {

    OneStepCoinitialSwaps::OneStepCoinitialSwaps(
        const std::vector<Time>& rateTimes,
        const std::vector<Real>& fixedAccruals,
        const std::vector<Real>& floatingAccruals,
        const std::vector<Time>& paymentTimes,
        double fixedRate)
    : MultiProductOneStep(rateTimes),
      fixedAccruals_(fixedAccruals), floatingAccruals_(floatingAccruals), 
      paymentTimes_(paymentTimes), fixedRate_(fixedRate) {
        // data checks
        lastIndex_ = rateTimes.size()-1;
    }

    bool OneStepCoinitialSwaps::nextTimeStep(
        const CurveState& currentState, 
        std::vector<Size>& numberCashFlowsThisStep, 
        std::vector<std::vector<MarketModelMultiProduct::CashFlow> >& genCashFlows)
    //{
    //    //double liborRate = currentState.forwardRate(currentIndex_);
    //    //std::fill(numberCashFlowsThisStep.begin(),numberCashFlowsThisStep.end(),0);
    //    //for(Size i=currentIndex_;i<lastIndex_;i++){
    //    //    genCashFlows[i][0].timeIndex = currentIndex_;
    //    //    genCashFlows[i][0].amount = -fixedRate_*fixedAccruals_[currentIndex_];

    //    //    genCashFlows[i][1].timeIndex = currentIndex_;
    //    //    genCashFlows[i][1].amount = liborRate*floatingAccruals_[currentIndex_];

    //    //    numberCashFlowsThisStep[i] = 2;
    //    //}
    //    //++currentIndex_;

    //    return true;//(currentIndex_ == lastIndex_);
    //}
        {
        std::fill(numberCashFlowsThisStep.begin(),numberCashFlowsThisStep.end(),0);

        for(Size indexOfTime=0;indexOfTime<lastIndex_;indexOfTime++){
            double liborRate = currentState.forwardRate(indexOfTime);
            for(Size i=indexOfTime;i<lastIndex_;i++){
                 genCashFlows[i][(indexOfTime)*2].timeIndex = indexOfTime;
                genCashFlows[i][(indexOfTime)*2].amount = -fixedRate_*fixedAccruals_[indexOfTime];

                genCashFlows[i][(indexOfTime)*2+1].timeIndex = indexOfTime;
                genCashFlows[i][(indexOfTime)*2+1].amount = liborRate*floatingAccruals_[indexOfTime];  

                numberCashFlowsThisStep[i] += 2;
            }
        }
        return true ;
    }

}
