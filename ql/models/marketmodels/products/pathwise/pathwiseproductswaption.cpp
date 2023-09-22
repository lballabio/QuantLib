/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Mark Joshi

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

#include <ql/errors.hpp>
#include <ql/models/marketmodels/products/pathwise/pathwiseproductswaption.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/utilities.hpp>

namespace QuantLib {


    bool MarketModelPathwiseCoterminalSwaptionsDeflated::alreadyDeflated() const
    {
        return false;
    }

    MarketModelPathwiseCoterminalSwaptionsDeflated::MarketModelPathwiseCoterminalSwaptionsDeflated(const std::vector<Time>& rateTimes,
                        const std::vector<Rate>& strikes)
    : rateTimes_(rateTimes), 
      strikes_(strikes) ,
      numberRates_(rateTimes.size()-1)
    {
        checkIncreasingTimes(rateTimes);
        std::vector<Time> evolTimes(rateTimes_);
        evolTimes.pop_back();

        QL_REQUIRE(evolTimes.size()==numberRates_,
                   "rateTimes.size()<> numberOfRates+1");

        QL_REQUIRE(strikes.size()==numberRates_,
                   "strikes.size()<> numberOfRates");


        evolution_ = EvolutionDescription(rateTimes,evolTimes);

    }

    bool MarketModelPathwiseCoterminalSwaptionsDeflated::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >& cashFlowsGenerated) 
    {
        Rate swapRate = currentState.coterminalSwapRate(currentIndex_);
        cashFlowsGenerated[currentIndex_][0].timeIndex = currentIndex_;

        Real annuity = currentState.coterminalSwapAnnuity(currentIndex_,currentIndex_);
        cashFlowsGenerated[currentIndex_][0].amount[0] =
            (swapRate-strikes_[currentIndex_])*annuity;
        
        std::fill(numberCashFlowsThisStep.begin(),
                  numberCashFlowsThisStep.end(),0);

        if (  cashFlowsGenerated[currentIndex_][0].amount[0]  >0)
        {
            numberCashFlowsThisStep[currentIndex_] = 1;
            for (Size i=1; i <= numberRates_; ++i)
                cashFlowsGenerated[currentIndex_][0].amount[i] =0;
            
            for (Size k=currentIndex_; k < numberRates_; ++k)
            {
              cashFlowsGenerated[currentIndex_][0].amount[k+1]  = (rateTimes_[k+1]-rateTimes_[k])*currentState.discountRatio(k+1,currentIndex_); 

              Real multiplier = - (rateTimes_[k+1]-rateTimes_[k])*currentState.discountRatio(k+1,k);

              for (Size l=k; l < numberRates_; ++l)
                 cashFlowsGenerated[currentIndex_][0].amount[k+1]  +=(currentState.forwardRate(l)-strikes_[currentIndex_])*(rateTimes_[l+1]-rateTimes_[l])
                                                                    * multiplier*currentState.discountRatio(l+1,currentIndex_);
            }
        }
        ++currentIndex_;
        return (currentIndex_ == strikes_.size());
    }

    std::unique_ptr<MarketModelPathwiseMultiProduct>
    MarketModelPathwiseCoterminalSwaptionsDeflated::clone() const 
    {
        return std::unique_ptr<MarketModelPathwiseMultiProduct>(new MarketModelPathwiseCoterminalSwaptionsDeflated(*this));
    }

    std::vector<Size> MarketModelPathwiseCoterminalSwaptionsDeflated::suggestedNumeraires() const
    {
            std::vector<Size> numeraires(numberRates_);
            for (Size i=0; i < numberRates_; ++i)
                numeraires[i] = i;

            return numeraires;
    }

    const EvolutionDescription& MarketModelPathwiseCoterminalSwaptionsDeflated::evolution() const
    {
        return evolution_;
    }

    std::vector<Time> MarketModelPathwiseCoterminalSwaptionsDeflated::possibleCashFlowTimes() const
    {
        return rateTimes_;
    }

    Size MarketModelPathwiseCoterminalSwaptionsDeflated::numberOfProducts() const
    {
           return numberRates_;
    }
    
    Size MarketModelPathwiseCoterminalSwaptionsDeflated::maxNumberOfCashFlowsPerProductPerStep() const
    {
        return 1;

    }
    
    void MarketModelPathwiseCoterminalSwaptionsDeflated::reset()
    {
        currentIndex_=0;
    }


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    bool MarketModelPathwiseCoterminalSwaptionsNumericalDeflated::alreadyDeflated() const
    {
        return false;
    }

    MarketModelPathwiseCoterminalSwaptionsNumericalDeflated::MarketModelPathwiseCoterminalSwaptionsNumericalDeflated(const std::vector<Time>& rateTimes,
                                                                const std::vector<Rate>& strikes, 
                                                                Real bumpSize)
    : rateTimes_(rateTimes), 
      strikes_(strikes) ,
      numberRates_(rateTimes.size()-1), bumpSize_(bumpSize), up_(rateTimes), down_(rateTimes), forwards_(numberRates_)
    {
        checkIncreasingTimes(rateTimes);
        std::vector<Time> evolTimes(rateTimes_);
        evolTimes.pop_back();

        QL_REQUIRE(evolTimes.size()==numberRates_,
                   "rateTimes.size()<> numberOfRates+1");

        QL_REQUIRE(strikes.size()==numberRates_,
                   "strikes.size()<> numberOfRates");


        evolution_ = EvolutionDescription(rateTimes,evolTimes);

    }

    bool MarketModelPathwiseCoterminalSwaptionsNumericalDeflated::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >& cashFlowsGenerated) 
    {
        Rate swapRate = currentState.coterminalSwapRate(currentIndex_);
        cashFlowsGenerated[currentIndex_][0].timeIndex = currentIndex_;

        Real annuity = currentState.coterminalSwapAnnuity(currentIndex_,currentIndex_);
        cashFlowsGenerated[currentIndex_][0].amount[0] =
            (swapRate-strikes_[currentIndex_])*annuity;
        
        std::fill(numberCashFlowsThisStep.begin(),
                  numberCashFlowsThisStep.end(),0);

        if (  cashFlowsGenerated[currentIndex_][0].amount[0]  >0)
        {
            numberCashFlowsThisStep[currentIndex_] = 1;
            for (Size i=1; i <= numberRates_; ++i)
                cashFlowsGenerated[currentIndex_][0].amount[i] =0;
            
            for (Size k=currentIndex_; k < numberRates_; ++k)
            {
                forwards_ = currentState.forwardRates();
                forwards_[k] += bumpSize_;
                up_.setOnForwardRates(forwards_);
    
                forwards_[k] -= bumpSize_;
                forwards_[k] -= bumpSize_;
                down_.setOnForwardRates(forwards_);

                Real upSR = up_.coterminalSwapRate(currentIndex_);
                Real upAnnuity = up_.coterminalSwapAnnuity(currentIndex_,currentIndex_);
                Real upValue = (upSR -  strikes_[currentIndex_])*  upAnnuity;

                Real downSR = down_.coterminalSwapRate(currentIndex_);
                Real downAnnuity = down_.coterminalSwapAnnuity(currentIndex_,currentIndex_);
                Real downValue = (downSR -  strikes_[currentIndex_])*  downAnnuity;

                Real deriv = (upValue - downValue)/(2.0*bumpSize_);

                cashFlowsGenerated[currentIndex_][0].amount[k+1]  = deriv; 
            
            }
        }
        ++currentIndex_;
        return (currentIndex_ == strikes_.size());
    }

    std::unique_ptr<MarketModelPathwiseMultiProduct>
    MarketModelPathwiseCoterminalSwaptionsNumericalDeflated::clone() const 
    {
        return std::unique_ptr<MarketModelPathwiseMultiProduct>(
          new MarketModelPathwiseCoterminalSwaptionsNumericalDeflated(*this));
    }

    std::vector<Size> MarketModelPathwiseCoterminalSwaptionsNumericalDeflated::suggestedNumeraires() const
    {
            std::vector<Size> numeraires(numberRates_);
            for (Size i=0; i < numberRates_; ++i)
                numeraires[i] = i;

            return numeraires;
    }

    const EvolutionDescription& MarketModelPathwiseCoterminalSwaptionsNumericalDeflated::evolution() const
    {
        return evolution_;
    }

    std::vector<Time> MarketModelPathwiseCoterminalSwaptionsNumericalDeflated::possibleCashFlowTimes() const
    {
        return rateTimes_;
    }

    Size MarketModelPathwiseCoterminalSwaptionsNumericalDeflated::numberOfProducts() const
    {
           return numberRates_;
    }
    
    Size MarketModelPathwiseCoterminalSwaptionsNumericalDeflated::maxNumberOfCashFlowsPerProductPerStep() const
    {
        return 1;

    }
    
    void MarketModelPathwiseCoterminalSwaptionsNumericalDeflated::reset()
    {
        currentIndex_=0;
    }


}

