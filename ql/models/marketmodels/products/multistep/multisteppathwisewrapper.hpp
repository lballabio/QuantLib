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

#ifndef quantlib_multistep_pathwise_wrapper_hpp
#define quantlib_multistep_pathwise_wrapper_hpp


#include <ql/models/marketmodels/multiproduct.hpp>
#include <ql/models/marketmodels/pathwisemultiproduct.hpp>
#include <ql/utilities/clone.hpp>

namespace QuantLib 
{
/*! MultiStepPathwiseWrapper
    Pathwise products do everything that ordinary products do and more. This lets
    you treat a pathwise product as an ordinary product. So you only have to write the 
    product once. 

    Tested in MarketModels::testInverseFloater()

*/

    class MultiProductPathwiseWrapper : public MarketModelMultiProduct
    {
      public:
        MultiProductPathwiseWrapper(const MarketModelPathwiseMultiProduct& innerProduct_);
        std::vector<Time> possibleCashFlowTimes() const override;
        Size numberOfProducts() const override;
        Size maxNumberOfCashFlowsPerProductPerStep() const override;
        void reset() override;
        bool nextTimeStep(const CurveState& currentState,
                          std::vector<Size>& numberCashFlowsThisStep,
                          std::vector<std::vector<CashFlow> >& cashFlowsGenerated) override;
#if defined(QL_USE_STD_UNIQUE_PTR)
        std::unique_ptr<MarketModelMultiProduct> clone() const override;
#else
        std::auto_ptr<MarketModelMultiProduct> clone() const;
        #endif
        std::vector<Size> suggestedNumeraires() const override;
        const EvolutionDescription& evolution() const override;

      private:
          Clone<MarketModelPathwiseMultiProduct> innerProduct_;
          std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> > cashFlowsGenerated_;
          Size numberOfProducts_;
     
    };


}

#endif
