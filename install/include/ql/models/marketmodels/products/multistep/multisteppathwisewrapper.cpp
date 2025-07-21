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

#include <ql/models/marketmodels/products/multistep/multisteppathwisewrapper.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>

namespace QuantLib 
{
    MultiProductPathwiseWrapper::MultiProductPathwiseWrapper(const MarketModelPathwiseMultiProduct& innerProduct)
        :  innerProduct_(innerProduct), cashFlowsGenerated_ (innerProduct.numberOfProducts(), std::vector<MarketModelPathwiseMultiProduct::CashFlow>(innerProduct.maxNumberOfCashFlowsPerProductPerStep())),
        numberOfProducts_(innerProduct.numberOfProducts())
    {

        for (auto& i : cashFlowsGenerated_)
            for (auto& j : i)
                j.amount.resize(1 + innerProduct.evolution().numberOfRates());
    }

        std::vector<Time>  MultiProductPathwiseWrapper::possibleCashFlowTimes() const
        {
            return innerProduct_->possibleCashFlowTimes();
        }
        
        Size  MultiProductPathwiseWrapper::numberOfProducts() const
        {
            return innerProduct_->numberOfProducts();
        }
        
        Size  MultiProductPathwiseWrapper::maxNumberOfCashFlowsPerProductPerStep() const
        {
               return innerProduct_->maxNumberOfCashFlowsPerProductPerStep();
        }
        
        void  MultiProductPathwiseWrapper::reset()
        {
            innerProduct_->reset();
        }
        
        bool  MultiProductPathwiseWrapper::nextTimeStep(
                     const CurveState& currentState,
                     std::vector<Size>& numberCashFlowsThisStep,
                     std::vector<std::vector<CashFlow> >& cashFlowsGenerated)
        {
            bool done = innerProduct_->nextTimeStep(currentState, numberCashFlowsThisStep,cashFlowsGenerated_);

            // tranform the data
            for (Size i=0; i < numberOfProducts_; ++i)
                for (Size j=0; j< numberCashFlowsThisStep[i]; ++j)
                {
                    cashFlowsGenerated[i][j].timeIndex =cashFlowsGenerated_[i][j].timeIndex;
                    cashFlowsGenerated[i][j].amount =cashFlowsGenerated_[i][j].amount[0];
                }


            return done;
        }

        std::vector<Size> MultiProductPathwiseWrapper::suggestedNumeraires() const
        {
            return innerProduct_->suggestedNumeraires();
        }

        const EvolutionDescription& MultiProductPathwiseWrapper::evolution() const
        {
            return innerProduct_->evolution();
        }

        std::unique_ptr<MarketModelMultiProduct>
        MultiProductPathwiseWrapper::clone() const
        {
            return std::unique_ptr<MarketModelMultiProduct>(new MultiProductPathwiseWrapper(*this));
        }

      

}

