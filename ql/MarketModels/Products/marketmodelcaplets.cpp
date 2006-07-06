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

#include <ql/MarketModels/Products/marketmodelcaplets.hpp>

namespace QuantLib {

    MarketModelCaplets::MarketModelCaplets(const Array& rateTimes,
                        const Array& accruals,
                        const Array& paymentTimes,
                        const Array& strikes)
                        : rateTimes_(rateTimes), accruals_(accruals), 
                        paymentTimes_(paymentTimes), strikes_(strikes)
    {
    }

    MarketModelCaplets::~MarketModelCaplets(){};
    EvolutionDescription MarketModelCaplets::suggestedEvolution() const
    {

         Array evolutionTimes(rateTimes_.size()-1);
            // was terminal measure
        std::vector<Size> numeraires(evolutionTimes.size()
                                       // rateTimes_.size()-1
                                     );

         for (Size i = 0; i<evolutionTimes.size(); ++i)
         {
                evolutionTimes[i]=rateTimes_[i];
                numeraires[i]=i;
         }
         
    
        std::vector<std::pair<Size,Size> > relevanceRates(
            evolutionTimes.size());
        for (Size i =0; i < evolutionTimes.size(); ++i)
            relevanceRates[i] = std::make_pair(i,i+1);


         return EvolutionDescription(rateTimes_, evolutionTimes,
                                     numeraires, relevanceRates);
    }    

    Array MarketModelCaplets::possibleCashFlowTimes() const 
    {
      return paymentTimes_;
    }
      Size MarketModelCaplets::numberOfProducts() const
    {
        return strikes_.size();    
    }
       Size MarketModelCaplets::maxNumberOfCashFlowsPerProductPerStep() const
    {
        return 1;
    }
      void MarketModelCaplets::reset()
    {
       currentIndex_=0;
    }

    bool MarketModelCaplets::nextTimeStep(
        const CurveState& currentState, 
        std::vector<Size>& numberCashFlowsThisStep, 
        std::vector<std::vector<MarketModelProduct::CashFlow> >& genCashFlows)
    {
        double liborRate = currentState.forwardRate(currentIndex_);
        genCashFlows[currentIndex_][0].timeIndex = currentIndex_;
        genCashFlows[currentIndex_][0].amount =
            std::max(liborRate-strikes_[currentIndex_], 0.0) *
                                            accruals_[currentIndex_];
        std::fill(numberCashFlowsThisStep.begin(),numberCashFlowsThisStep.end(),0);
        numberCashFlowsThisStep[currentIndex_] = 1;
       ++currentIndex_;
       return (currentIndex_ == strikes_.size());
    }

}
