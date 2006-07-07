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

#include <ql/MarketModels/Products/marketmodelcapletsonestep.hpp>

namespace QuantLib {

    MarketModelCapletsOneStep::MarketModelCapletsOneStep(
                                                   const Array& rateTimes,
                                                   const Array& accruals,
                                                   const Array& paymentTimes,
                                                   const Array& strikes)
        : rateTimes_(rateTimes), accruals_(accruals), 
          paymentTimes_(paymentTimes), strikes_(strikes)
    {
    }

    EvolutionDescription MarketModelCapletsOneStep::suggestedEvolution() const
    {
         Array evolutionTimes(1,rateTimes_[rateTimes_.size()-2]);
         std::vector<Size> numeraires(1,rateTimes_.size()-1);
    
         std::vector<std::pair<Size,Size> > relevanceRates(1);
         relevanceRates[0] = std::make_pair(0,rateTimes_.size()-1);

         return EvolutionDescription(rateTimes_, evolutionTimes,
                                     numeraires, relevanceRates);
    }    

    Array MarketModelCapletsOneStep::possibleCashFlowTimes() const 
    {
      return paymentTimes_;
    }

    Size MarketModelCapletsOneStep::numberOfProducts() const
    {
        return strikes_.size();    
    }

    Size MarketModelCapletsOneStep::maxNumberOfCashFlowsPerProductPerStep() const
    {
        return 1;
    }

    void MarketModelCapletsOneStep::reset() {}

    bool MarketModelCapletsOneStep::nextTimeStep(
        const CurveState& currentState, 
        std::vector<Size>& numberCashFlowsThisStep, 
        std::vector<std::vector<MarketModelProduct::CashFlow> >& genCashFlows)
    {
        std::fill(numberCashFlowsThisStep.begin(),
                  numberCashFlowsThisStep.end(), 0);
        for (Size i=0; i<strikes_.size(); ++i) {
            Rate liborRate = currentState.forwardRate(i);
            if (liborRate > strikes_[i]) {
                numberCashFlowsThisStep[i] = 1;
                genCashFlows[i][0].timeIndex = i;
                genCashFlows[i][0].amount =
                    (liborRate-strikes_[i])*accruals_[i];
            }
        }

        return true;
    }

}
