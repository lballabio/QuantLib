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

#include <ql/MarketModels/Products/MultiStep/multistepcoterminalswaptions.hpp>

namespace QuantLib {

    MultiStepCoterminalSwaptions::MultiStepCoterminalSwaptions(
                                        const std::vector<Time>& rateTimes,
                                        const std::vector<Real>& accruals,
                                        const std::vector<Time>& paymentTimes,
                                        const std::vector<Rate>& strikes)
    : MultiProductMultiStep(rateTimes), accruals_(accruals),
      paymentTimes_(paymentTimes), strikes_(strikes) {  
        // data checks
        lastIndex_ = rateTimes.size()-1;
    }

    bool MultiStepCoterminalSwaptions::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&
                                                               genCashFlows) 
    {
        Rate swapRate = currentState.coterminalSwapRate(currentIndex_);
        genCashFlows[currentIndex_][0].timeIndex = currentIndex_;
        genCashFlows[currentIndex_][0].amount =
            std::max(swapRate-strikes_[currentIndex_], 0.0) *
            accruals_[currentIndex_];
        std::fill(numberCashFlowsThisStep.begin(),
                  numberCashFlowsThisStep.end(),0);
        numberCashFlowsThisStep[currentIndex_] = 1;
        ++currentIndex_;
        return (currentIndex_ == lastIndex_);
    }

    std::auto_ptr<MarketModelMultiProduct>
    MultiStepCoterminalSwaptions::clone() const {
        return std::auto_ptr<MarketModelMultiProduct>(
                                         new MultiStepCoterminalSwaptions(*this));
    }

}
