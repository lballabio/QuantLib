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

#include <ql/models/marketmodels/products/multistep/multistepswaption.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

   
    MultiStepSwaption::MultiStepSwaption(const std::vector<Time>& rateTimes,
                                     Size startIndex,
                                     Size endIndex,
                                     boost::shared_ptr<StrikedTypePayoff> & payOff)
                                     : MultiProductMultiStep(rateTimes),
     startIndex_(startIndex), endIndex_(endIndex), payoff_(payOff) 
    {
        QL_REQUIRE(startIndex_ < endIndex_," start index must be before end index");
   
        QL_REQUIRE(endIndex_ < rateTimes.size(), "end index be before the end of the rates.");
        paymentTimes_.push_back(rateTimes[startIndex_]);

    }

    bool MultiStepSwaption::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&
                                                               genCashFlows)
    {
        if (currentIndex_ == startIndex_)
        {
            genCashFlows[0][0].timeIndex = 0;


            Rate swapRate = currentState.cmSwapRate(startIndex_,endIndex_-startIndex_);
            Real annuity = currentState.cmSwapAnnuity(startIndex_,startIndex_,endIndex_-startIndex_);

            genCashFlows[0][0].amount =
                (*payoff_)(swapRate) * annuity;
             
            numberCashFlowsThisStep[0] =genCashFlows[0][0].amount != 0.0 ? 1 : 0 ;

            return true;
        }
        else
        {
            numberCashFlowsThisStep[0] =0;
            ++currentIndex_;
            return false;
        }
    }

    std::auto_ptr<MarketModelMultiProduct>
    MultiStepSwaption::clone() const {
        return std::auto_ptr<MarketModelMultiProduct>(
                                         new MultiStepSwaption(*this));
    }

}
