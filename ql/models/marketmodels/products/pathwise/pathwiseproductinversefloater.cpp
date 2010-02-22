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

#include <ql/models/marketmodels/products/pathwise/pathwiseproductinversefloater.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/utilities.hpp>

namespace QuantLib 
{


    bool MarketModelPathwiseInverseFloater::alreadyDeflated() const
    {
        return false;
    }

    MarketModelPathwiseInverseFloater::MarketModelPathwiseInverseFloater(const std::vector<Time>& rateTimes,
        const std::vector<Real>& fixedAccruals,
        const std::vector<Real>& floatingAccruals,
        const std::vector<Real>& fixedStrikes,
        const std::vector<Real>& fixedMultipliers, 
        const std::vector<Real>& floatingSpreads,
        const std::vector<Time>& paymentTimes,
        bool payer)
        : rateTimes_(rateTimes),
        fixedAccruals_(fixedAccruals), 
        floatingAccruals_(floatingAccruals),
        fixedStrikes_(fixedStrikes),
        fixedMultipliers_(fixedMultipliers),
        floatingSpreads_(floatingSpreads),
        paymentTimes_(paymentTimes),
        payer_(payer),
        multiplier_(payer ? -1.0 : 1.0), 
        lastIndex_(rateTimes.size()-1)
    {
        checkIncreasingTimes(paymentTimes);
        QL_REQUIRE(fixedAccruals_.size() == lastIndex_," Incorrect number of fixedAccruals given, should be " <<  lastIndex_ << " not " << fixedAccruals_.size() );
        QL_REQUIRE(floatingAccruals.size() == lastIndex_," Incorrect number of floatingAccruals given, should be " <<  lastIndex_ << " not " << floatingAccruals.size() );
        QL_REQUIRE(fixedStrikes.size() == lastIndex_," Incorrect number of fixedStrikes given, should be " <<  lastIndex_ << " not " << fixedStrikes.size() );
        QL_REQUIRE(fixedMultipliers.size() == lastIndex_," Incorrect number of fixedMultipliers given, should be " <<  lastIndex_ << " not " << fixedMultipliers.size() );
        QL_REQUIRE(floatingSpreads.size() == lastIndex_," Incorrect number of floatingSpreads given, should be " <<  lastIndex_ << " not " << floatingSpreads.size() );
        QL_REQUIRE(paymentTimes.size() == lastIndex_," Incorrect number of paymentTimes given, should be " <<  lastIndex_ << " not " << paymentTimes.size() );

        std::vector<Time> evolTimes(rateTimes);
        evolTimes.pop_back();


        evolution_ = EvolutionDescription(rateTimes,evolTimes);

    }

    bool MarketModelPathwiseInverseFloater::nextTimeStep(
        const CurveState& currentState,
        std::vector<Size>& numberCashFlowsThisStep,
        std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >& cashFlowsGenerated) 
    {
        numberCashFlowsThisStep[0] =1 ;
        for (Size i=1; i <= lastIndex_; ++i)
            cashFlowsGenerated[0][0].amount[i] =0;

        Rate liborRate = currentState.forwardRate(currentIndex_);
        Real inverseFloatingCoupon = std::max((fixedStrikes_[currentIndex_] - fixedMultipliers_[currentIndex_]*liborRate),0.0)*fixedAccruals_[currentIndex_] ;
        Real floatingCoupon = (liborRate+floatingSpreads_[currentIndex_])*floatingAccruals_[currentIndex_];
        cashFlowsGenerated[0][0].timeIndex = currentIndex_;
        cashFlowsGenerated[0][0].amount[0] =multiplier_*(inverseFloatingCoupon - floatingCoupon);
 
 

        if (inverseFloatingCoupon > 0.0)
        {

            cashFlowsGenerated[0][0].amount[currentIndex_+1] =multiplier_*( - fixedMultipliers_[currentIndex_]*fixedAccruals_[currentIndex_]  - floatingAccruals_[currentIndex_]);


        }
        else
        {

            cashFlowsGenerated[0][0].amount[currentIndex_+1] =-multiplier_*floatingAccruals_[currentIndex_];

        }

        ++currentIndex_;

        return (currentIndex_ == lastIndex_);


    }

    std::auto_ptr<MarketModelPathwiseMultiProduct> MarketModelPathwiseInverseFloater::clone() const 
    {
        return std::auto_ptr<MarketModelPathwiseMultiProduct>(
            new MarketModelPathwiseInverseFloater(*this));
    }

    std::vector<Size> MarketModelPathwiseInverseFloater::suggestedNumeraires() const
    {
        std::vector<Size> numeraires(lastIndex_);
        for (Size i=0; i < lastIndex_; ++i)
            numeraires[i] = i;

        return numeraires;
    }

    const EvolutionDescription& MarketModelPathwiseInverseFloater::evolution() const
    {
        return evolution_;
    }

    std::vector<Time> MarketModelPathwiseInverseFloater::possibleCashFlowTimes() const
    {
        return paymentTimes_; 
    }

    Size MarketModelPathwiseInverseFloater::numberOfProducts() const
    {
        return 1;
    }

    Size MarketModelPathwiseInverseFloater::maxNumberOfCashFlowsPerProductPerStep() const
    {
        return 1;

    }

    void MarketModelPathwiseInverseFloater::reset()
    {
        currentIndex_=0;
    }

}

