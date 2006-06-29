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

#include <ql/MarketModels/Products/marketmodelratchet.hpp>

namespace QuantLib {

    MarketModelRatchet::MarketModelRatchet(
        const Array& rateTimes,
        const Array& fixedAccruals,
        const Array& floatingAccruals,
        const Array& floatingSpreads,
        const Array& paymentTimes,
        double initialCoupon)
    : rateTimes_(rateTimes), fixedAccruals_(fixedAccruals),
      floatingAccruals_(floatingAccruals), floatingSpreads_(floatingSpreads),
      paymentTimes_(paymentTimes), initialCoupon_(initialCoupon)
    {
        // data checks
        lastIndex_ = rateTimes.size()-1;
    }

    EvolutionDescription MarketModelRatchet::suggestedEvolution() const
    {
        Array evolutionTimes(rateTimes_.size()-1);
        for (Size i = 0; i<evolutionTimes.size(); ++i)
            evolutionTimes[i]=rateTimes_[i];

        // terminal measure
        std::vector<Size> numeraires(evolutionTimes.size(),
                                     rateTimes_.size()-1);

        std::vector<std::pair<Size,Size> > relevanceRates(
            evolutionTimes.size());
        for (Size i =0; i < evolutionTimes.size(); ++i)
            relevanceRates[i] = std::make_pair(i,i+1);


         return EvolutionDescription(rateTimes_, evolutionTimes,
                                     numeraires, relevanceRates);
    }


    Array MarketModelRatchet::possibleCashFlowTimes() const
    {
        return paymentTimes_;
    }

    Size MarketModelRatchet::numberOfProducts() const
    {
        return 1;    
    }

    Size MarketModelRatchet::maxNumberOfCashFlowsPerProductPerStep() const
    {
        return 2;
    }

    void MarketModelRatchet::reset()
    {
       currentIndex_=0;
       currentCoupon_=initialCoupon_;
    }
       
    bool MarketModelRatchet::nextTimeStep(
        const CurveState& currentState, 
        std::vector<Size>& numberCashFlowsThisStep, 
        std::vector<std::vector<MarketModelProduct::CashFlow> >& genCashFlows)
    {
        double liborRate = currentState.forwardRate(currentIndex_);
        currentCoupon_= std::max(liborRate,currentCoupon_);

        genCashFlows[0][0].timeIndex = currentIndex_;
        genCashFlows[0][0].amount = currentCoupon_*fixedAccruals_[currentIndex_];

  
        genCashFlows[0][1].timeIndex = currentIndex_;
        genCashFlows[0][1].amount = (liborRate+floatingSpreads_[currentIndex_])
                                    *floatingAccruals_[currentIndex_];

        numberCashFlowsThisStep[0] = 2;

       ++currentIndex_;

       return (currentIndex_ == lastIndex_);
    }

}
