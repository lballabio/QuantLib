/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Mark Joshi

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

#include <ql/models/marketmodels/products/pathwise/pathwiseproductswap.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/utilities.hpp>

namespace QuantLib 
{


    bool MarketModelPathwiseSwap::alreadyDeflated() const
    {
        return false;
    }

    MarketModelPathwiseSwap::MarketModelPathwiseSwap(const std::vector<Time>& rateTimes,
                         const std::vector<Real>& accruals,
                        const std::vector<Rate>& strikes, 
                        Real multiplier)
    : rateTimes_(rateTimes), 
      accruals_(accruals),
      strikes_(strikes) ,
      numberRates_(rateTimes.size()-1),
      multiplier_(multiplier)
    {
        checkIncreasingTimes(rateTimes);
        std::vector<Time> evolTimes(rateTimes_);
        evolTimes.pop_back();

        QL_REQUIRE(evolTimes.size()==numberRates_,
                   "rateTimes.size()<> numberOfRates+1");

        if (strikes_.size() == 1)
            strikes_ = std::vector<Rate>(numberRates_, strikes[0]);

        if (accruals_.size() == 1)
            accruals_ = std::vector<Rate>(numberRates_, accruals[0]);

        QL_REQUIRE(accruals_.size()==numberRates_,
                   "accruals.size() does not equal numberOfRates or 1");

        QL_REQUIRE(strikes.size()==numberRates_,
                   "strikes.size() does not equal numberOfRates or 1" );

        evolution_ = EvolutionDescription(rateTimes,evolTimes);

    }

    bool MarketModelPathwiseSwap::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >& cashFlowsGenerated) 
    {
        Rate liborRate = currentState.forwardRate(currentIndex_);
        cashFlowsGenerated[0][0].timeIndex = currentIndex_+1;

        cashFlowsGenerated[0][0].amount[0] =
                     (liborRate-strikes_[currentIndex_])*accruals_[currentIndex_]*multiplier_;

        numberCashFlowsThisStep[0] = 1;
        
        for (Size i=1; i <= numberRates_; ++i)
                cashFlowsGenerated[0][0].amount[i] =0;

        cashFlowsGenerated[0][0].amount[currentIndex_+1] = accruals_[currentIndex_]*multiplier_;
            
        ++currentIndex_;
        return (currentIndex_ == strikes_.size());
    }

    std::unique_ptr<MarketModelPathwiseMultiProduct>
    MarketModelPathwiseSwap::clone() const 
    {
        return std::unique_ptr<MarketModelPathwiseMultiProduct>(new MarketModelPathwiseSwap(*this));
    }

    std::vector<Size> MarketModelPathwiseSwap::suggestedNumeraires() const
    {
            std::vector<Size> numeraires(numberRates_);
            for (Size i=0; i < numberRates_; ++i)
                numeraires[i] = i;

            return numeraires;
    }

    const EvolutionDescription& MarketModelPathwiseSwap::evolution() const
    {
        return evolution_;
    }

    std::vector<Time> MarketModelPathwiseSwap::possibleCashFlowTimes() const
    {
        return rateTimes_; // note rateTimes_[0] is not used as a cash flow time but it is easier to keep track if we include it.
    }

    Size MarketModelPathwiseSwap::numberOfProducts() const
    {
           return 1;
    }
    
    Size MarketModelPathwiseSwap::maxNumberOfCashFlowsPerProductPerStep() const
    {
        return 1;

    }
    
    void MarketModelPathwiseSwap::reset()
    {
        currentIndex_=0;
    }

}

