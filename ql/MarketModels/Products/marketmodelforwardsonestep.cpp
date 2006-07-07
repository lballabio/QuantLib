/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

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

#include <ql/MarketModels/Products/marketmodelforwardsonestep.hpp>

namespace QuantLib {

    MarketModelForwardsOneStep::MarketModelForwardsOneStep(
                                                   const Array& rateTimes,
                                                   const Array& accruals,
                                                   const Array& paymentTimes,
                                                   const Array& strikes)
        : rateTimes_(rateTimes), accruals_(accruals), 
          paymentTimes_(paymentTimes), strikes_(strikes)
    {
    }

    EvolutionDescription MarketModelForwardsOneStep::suggestedEvolution() const
    {
         Array evolutionTimes(1,rateTimes_[rateTimes_.size()-2]);
         std::vector<Size> numeraires(1,rateTimes_.size()-1);
    
         std::vector<std::pair<Size,Size> > relevanceRates(1);
         relevanceRates[0] = std::make_pair(0,rateTimes_.size()-1);

         return EvolutionDescription(rateTimes_, evolutionTimes,
                                     numeraires, relevanceRates);
    }    

    Array MarketModelForwardsOneStep::possibleCashFlowTimes() const 
    {
      return paymentTimes_;
    }

    Size MarketModelForwardsOneStep::numberOfProducts() const
    {
        return strikes_.size();    
    }

    Size MarketModelForwardsOneStep::maxNumberOfCashFlowsPerProductPerStep() const
    {
        return 1;
    }

    void MarketModelForwardsOneStep::reset() {}

    bool MarketModelForwardsOneStep::nextTimeStep(
        const CurveState& currentState, 
        std::vector<Size>& numberCashFlowsThisStep, 
        std::vector<std::vector<MarketModelProduct::CashFlow> >& genCashFlows)
    {
        for (Size i=0; i<strikes_.size(); ++i) {
            double liborRate = currentState.forwardRate(i);
            genCashFlows[i][0].timeIndex = i;
            genCashFlows[i][0].amount =
                (liborRate-strikes_[i])*accruals_[i];
        }

        std::fill(numberCashFlowsThisStep.begin(),
                  numberCashFlowsThisStep.end(), 1);
        return true;
    }

}
