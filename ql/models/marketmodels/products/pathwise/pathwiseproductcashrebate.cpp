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

#include <ql/auto_ptr.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/products/pathwise/pathwiseproductcashrebate.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <utility>

namespace QuantLib 
{


    bool MarketModelPathwiseCashRebate::alreadyDeflated() const
    {
        return false;
    }

    MarketModelPathwiseCashRebate::MarketModelPathwiseCashRebate(
        EvolutionDescription evolution,
        const std::vector<Time>& paymentTimes,
        Matrix amounts,
        Size numberOfProducts)
    : evolution_(std::move(evolution)), paymentTimes_(paymentTimes), amounts_(std::move(amounts)),
      numberOfProducts_(numberOfProducts) {

        checkIncreasingTimes(paymentTimes);

        QL_REQUIRE(amounts_.rows() == numberOfProducts_,
                   "the number of rows in the matrix must equal "
                   "the number of products");
        QL_REQUIRE(amounts_.columns() == paymentTimes_.size(),
                   "the number of columns in the matrix must equal "
                   "the number of payment times");
        QL_REQUIRE(evolution_.evolutionTimes().size() == paymentTimes_.size(),
                   "the number of evolution times must equal "
                   "the number of payment times");
    }


    std::vector<Time>
    MarketModelPathwiseCashRebate::possibleCashFlowTimes() const 
    {
        return paymentTimes_;
    }

    Size MarketModelPathwiseCashRebate::numberOfProducts() const 
    {
        return numberOfProducts_;
    }

    Size MarketModelPathwiseCashRebate::maxNumberOfCashFlowsPerProductPerStep() const 
    {
        return 1;
    }

    void MarketModelPathwiseCashRebate::reset() 
    {
       currentIndex_=0;
    }

    std::vector<Size>
    MarketModelPathwiseCashRebate::suggestedNumeraires() const 
    {
        QL_FAIL("not implemented (yet?)");
    }

    const EvolutionDescription& MarketModelPathwiseCashRebate::evolution() const
    {
        return evolution_;
    }


    bool MarketModelPathwiseCashRebate::nextTimeStep(
            const CurveState&,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >& cashFlowsGenerated) 
    {
        for (Size i=0; i<numberOfProducts_; ++i) 
        {
            numberCashFlowsThisStep[i] = 1;
            cashFlowsGenerated[i][0].timeIndex = currentIndex_;
            cashFlowsGenerated[i][0].amount[0] = amounts_[i][currentIndex_];

            for (Size k=1; k <= evolution_.numberOfRates(); ++k)
                 cashFlowsGenerated[i][0].amount[k] = 0.0;
 
        }
        ++currentIndex_;
        return true;
    }

    QL_UNIQUE_OR_AUTO_PTR<MarketModelPathwiseMultiProduct>
    MarketModelPathwiseCashRebate::clone() const 
    {
        return QL_UNIQUE_OR_AUTO_PTR<MarketModelPathwiseMultiProduct>(
                                    new MarketModelPathwiseCashRebate(*this));
    }

}
