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

#include <ql/auto_ptr.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/products/multistep/multistepforwards.hpp>
#include <ql/models/marketmodels/products/pathwise/pathwiseproductcaplet.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <utility>

namespace QuantLib {


    bool MarketModelPathwiseMultiCaplet::alreadyDeflated() const
    {
        return false;
    }

    MarketModelPathwiseMultiCaplet::MarketModelPathwiseMultiCaplet(const std::vector<Time>& rateTimes,
        const std::vector<Real>& accruals,
        const std::vector<Time>& paymentTimes,
        const std::vector<Rate>& strikes)
        : rateTimes_(rateTimes), 
        accruals_(accruals),
        paymentTimes_(paymentTimes), 
        strikes_(strikes) ,
        numberRates_(accruals_.size())
    {
        checkIncreasingTimes(rateTimes);
        checkIncreasingTimes(paymentTimes);
        std::vector<Time> evolTimes(rateTimes_);
        evolTimes.pop_back();

        QL_REQUIRE(evolTimes.size()==numberRates_,
            "rateTimes.size()<> numberOfRates+1");

        QL_REQUIRE(paymentTimes.size()==numberRates_,
            "paymentTimes.size()<> numberOfRates");

        QL_REQUIRE(accruals.size()==numberRates_,
            "accruals.size()<> numberOfRates");

        QL_REQUIRE(strikes.size()==numberRates_,
            "strikes.size()<> numberOfRates");


        evolution_ = EvolutionDescription(rateTimes,evolTimes);

    }

   


    bool MarketModelPathwiseMultiCaplet::nextTimeStep(
        const CurveState& currentState,
        std::vector<Size>& numberCashFlowsThisStep,
        std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >& cashFlowsGenerated) 
    {
        Rate liborRate = currentState.forwardRate(currentIndex_);
        cashFlowsGenerated[currentIndex_][0].timeIndex = currentIndex_;
        cashFlowsGenerated[currentIndex_][0].amount[0] =
            (liborRate-strikes_[currentIndex_])*accruals_[currentIndex_];

        std::fill(numberCashFlowsThisStep.begin(),
            numberCashFlowsThisStep.end(),0);

        if (  cashFlowsGenerated[currentIndex_][0].amount[0]  >0)
        {
            numberCashFlowsThisStep[currentIndex_] = 1;
            for (Size i=1; i <= numberRates_; ++i)
                cashFlowsGenerated[currentIndex_][0].amount[i] =0;

            cashFlowsGenerated[currentIndex_][0].amount[currentIndex_+1]  = accruals_[currentIndex_];
        }
        ++currentIndex_;
        return (currentIndex_ == strikes_.size());
    }

    QL_UNIQUE_OR_AUTO_PTR<MarketModelPathwiseMultiProduct>
    MarketModelPathwiseMultiCaplet::clone() const 
    {
        return QL_UNIQUE_OR_AUTO_PTR<MarketModelPathwiseMultiProduct>(
                                   new MarketModelPathwiseMultiCaplet(*this));
    }

    std::vector<Size> MarketModelPathwiseMultiCaplet::suggestedNumeraires() const
    {
        std::vector<Size> numeraires(numberRates_);
        for (Size i=0; i < numberRates_; ++i)
            numeraires[i] = i+1;

        return numeraires;
    }

    const EvolutionDescription& MarketModelPathwiseMultiCaplet::evolution() const
    {
        return evolution_;
    }

    std::vector<Time> MarketModelPathwiseMultiCaplet::possibleCashFlowTimes() const
    {
        return paymentTimes_;
    }

    Size MarketModelPathwiseMultiCaplet::numberOfProducts() const
    {
        return numberRates_;
    }

    Size MarketModelPathwiseMultiCaplet::maxNumberOfCashFlowsPerProductPerStep() const
    {
        return 1;

    }

    void MarketModelPathwiseMultiCaplet::reset()
    {
        currentIndex_=0;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool MarketModelPathwiseMultiDeflatedCaplet::alreadyDeflated() const
    {
        return true;
    }


    MarketModelPathwiseMultiDeflatedCaplet::MarketModelPathwiseMultiDeflatedCaplet(const std::vector<Time>& rateTimes,
        const std::vector<Real>& accruals,
        const std::vector<Time>& paymentTimes,
        const std::vector<Rate>& strikes)
        : rateTimes_(rateTimes), 
        accruals_(accruals),
        paymentTimes_(paymentTimes), 
        strikes_(strikes) ,
        numberRates_(accruals_.size())
    {
        checkIncreasingTimes(rateTimes);
        checkIncreasingTimes(paymentTimes);
        std::vector<Time> evolTimes(rateTimes_);
        evolTimes.pop_back();

        QL_REQUIRE(evolTimes.size()==numberRates_,
            "rateTimes.size()<> numberOfRates+1");

        QL_REQUIRE(paymentTimes.size()==numberRates_,
            "paymentTimes.size()<> numberOfRates");

        QL_REQUIRE(accruals.size()==numberRates_,
            "accruals.size()<> numberOfRates");

        QL_REQUIRE(strikes.size()==numberRates_,
            "strikes.size()<> numberOfRates");


        evolution_ = EvolutionDescription(rateTimes,evolTimes);

    }

       MarketModelPathwiseMultiDeflatedCaplet::MarketModelPathwiseMultiDeflatedCaplet(const std::vector<Time>& rateTimes,
        const std::vector<Real>& accruals,
        const std::vector<Time>& paymentTimes,
        Rate strike)
        : rateTimes_(rateTimes), 
        accruals_(accruals),
        paymentTimes_(paymentTimes), 
        strikes_(accruals.size()) ,
        numberRates_(accruals_.size())
    {
        checkIncreasingTimes(rateTimes);
        checkIncreasingTimes(paymentTimes);
        std::vector<Time> evolTimes(rateTimes_);
        evolTimes.pop_back();

        QL_REQUIRE(evolTimes.size()==numberRates_,
            "rateTimes.size()<> numberOfRates+1");

        QL_REQUIRE(paymentTimes.size()==numberRates_,
            "paymentTimes.size()<> numberOfRates");

        QL_REQUIRE(accruals.size()==numberRates_,
            "accruals.size()<> numberOfRates");

        
        std::fill(strikes_.begin(), strikes_.end(),strike);


        evolution_ = EvolutionDescription(rateTimes,evolTimes);

    }


    bool MarketModelPathwiseMultiDeflatedCaplet::nextTimeStep(
        const CurveState& currentState,
        std::vector<Size>& numberCashFlowsThisStep,
        std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >& cashFlowsGenerated) 
    {
        Rate liborRate = currentState.forwardRate(currentIndex_);
        cashFlowsGenerated[currentIndex_][0].timeIndex = currentIndex_;
        cashFlowsGenerated[currentIndex_][0].amount[0] =
            (liborRate-strikes_[currentIndex_])*accruals_[currentIndex_]*currentState.discountRatio(currentIndex_+1,0);

        std::fill(numberCashFlowsThisStep.begin(),
            numberCashFlowsThisStep.end(),0);

        if (  cashFlowsGenerated[currentIndex_][0].amount[0]  >0)
        {
            numberCashFlowsThisStep[currentIndex_] = 1;
            for (Size i=1; i <= numberRates_; ++i)
                cashFlowsGenerated[currentIndex_][0].amount[i] =0;

            cashFlowsGenerated[currentIndex_][0].amount[currentIndex_+1]  = accruals_[currentIndex_]*currentState.discountRatio(currentIndex_+1,0);

            for (Size i=0; i <= currentIndex_; ++i)
            {
                Real stepDF = currentState.discountRatio(i+1,i);
                cashFlowsGenerated[currentIndex_][0].amount[i+1] -=   accruals_[i]*stepDF
                    *cashFlowsGenerated[currentIndex_][0].amount[0];
            }
        }
        ++currentIndex_;
        return (currentIndex_ == strikes_.size());
    }

    QL_UNIQUE_OR_AUTO_PTR<MarketModelPathwiseMultiProduct>
    MarketModelPathwiseMultiDeflatedCaplet::clone() const 
    {
        return QL_UNIQUE_OR_AUTO_PTR<MarketModelPathwiseMultiProduct>(
                           new MarketModelPathwiseMultiDeflatedCaplet(*this));
    }

    std::vector<Size> MarketModelPathwiseMultiDeflatedCaplet::suggestedNumeraires() const
    {
        std::vector<Size> numeraires(numberRates_);
        for (Size i=0; i < numberRates_; ++i)
            numeraires[i] = i;

        return numeraires;
    }

    const EvolutionDescription& MarketModelPathwiseMultiDeflatedCaplet::evolution() const
    {
        return evolution_;
    }

    std::vector<Time> MarketModelPathwiseMultiDeflatedCaplet::possibleCashFlowTimes() const
    {
        return paymentTimes_;
    }

    Size MarketModelPathwiseMultiDeflatedCaplet::numberOfProducts() const
    {
        return numberRates_;
    }

    Size MarketModelPathwiseMultiDeflatedCaplet::maxNumberOfCashFlowsPerProductPerStep() const
    {
        return 1;

    }

    void MarketModelPathwiseMultiDeflatedCaplet::reset()
    {
        currentIndex_=0;
    }

    MarketModelPathwiseMultiDeflatedCap::MarketModelPathwiseMultiDeflatedCap(
        const std::vector<Time>& rateTimes,
        const std::vector<Real>& accruals,
        const std::vector<Time>& paymentTimes,
        Rate strike,
        std::vector<std::pair<Size, Size> > startsAndEnds)
    : underlyingCaplets_(rateTimes, accruals, paymentTimes, strike), numberRates_(accruals.size()),
      startsAndEnds_(std::move(startsAndEnds)) {
        for (Size j=0; j < startsAndEnds_.size(); ++j)
        {
            QL_REQUIRE(startsAndEnds_[j].first < startsAndEnds_[j].second,"a cap must start before it ends: " << j << startsAndEnds_[j].first << startsAndEnds_[j].second );
            QL_REQUIRE(startsAndEnds_[j].second <= accruals.size() ,"a cap must end when the underlying caplets: " << j << startsAndEnds_[j].first << startsAndEnds_[j].second );
           
        }

        innerCashFlowSizes_.resize(accruals.size());
        innerCashFlowsGenerated_.resize(accruals.size());
        for (auto& i : innerCashFlowsGenerated_) {
            i.resize(underlyingCaplets_.maxNumberOfCashFlowsPerProductPerStep());
            for (Size j=0; j < underlyingCaplets_.maxNumberOfCashFlowsPerProductPerStep(); ++j)
                i[j].amount.resize(accruals.size() + 1);
        }
    }


    std::vector<Size> MarketModelPathwiseMultiDeflatedCap::suggestedNumeraires() const
    {
        return underlyingCaplets_.suggestedNumeraires();
    }


    const EvolutionDescription& MarketModelPathwiseMultiDeflatedCap::evolution() const
    {
        return underlyingCaplets_.evolution();
    }

    std::vector<Time> MarketModelPathwiseMultiDeflatedCap::possibleCashFlowTimes() const
    {
        return underlyingCaplets_.possibleCashFlowTimes();
    }

    Size MarketModelPathwiseMultiDeflatedCap::numberOfProducts() const
    {
        return startsAndEnds_.size();
    }
    Size MarketModelPathwiseMultiDeflatedCap::maxNumberOfCashFlowsPerProductPerStep() const
    {
        return underlyingCaplets_.maxNumberOfCashFlowsPerProductPerStep();
    }

    // has division by the numeraire already been done?
    bool MarketModelPathwiseMultiDeflatedCap::alreadyDeflated() const
    {
        return underlyingCaplets_.alreadyDeflated();
    }


    void MarketModelPathwiseMultiDeflatedCap::reset()
    {
        underlyingCaplets_.reset();
        currentIndex_=0;
    }


    bool MarketModelPathwiseMultiDeflatedCap::nextTimeStep(
        const CurveState& currentState,
        std::vector<Size>& numberCashFlowsThisStep,
        std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >& cashFlowsGenerated)
    {

        bool done = underlyingCaplets_.nextTimeStep(currentState, innerCashFlowSizes_, innerCashFlowsGenerated_);
        
        for (Size k=0; k < startsAndEnds_.size(); ++k)
            numberCashFlowsThisStep[k]=0;

        for (Size j=0; j < numberRates_; ++j)
        {
            if (innerCashFlowSizes_[j]>0)
            {
                for (Size k=0; k < startsAndEnds_.size(); ++k)
                {
                    if (startsAndEnds_[k].first <= j && j < startsAndEnds_[k].second)
                    {
                        for (Size l=0; l < innerCashFlowSizes_[j]; ++l)
                            cashFlowsGenerated[k][numberCashFlowsThisStep[k]++] = innerCashFlowsGenerated_[j][l];
                    }
                }
            }
        }

        return done;
    }

    QL_UNIQUE_OR_AUTO_PTR<MarketModelPathwiseMultiProduct>
    MarketModelPathwiseMultiDeflatedCap::clone() const
    {
        return QL_UNIQUE_OR_AUTO_PTR<MarketModelPathwiseMultiProduct>(
                              new MarketModelPathwiseMultiDeflatedCap(*this));
    }

}

