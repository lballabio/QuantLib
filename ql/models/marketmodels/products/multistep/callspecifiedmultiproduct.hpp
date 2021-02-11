/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

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


#ifndef quantlib_callspecified_multiproduct_hpp
#define quantlib_callspecified_multiproduct_hpp

#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/multiproduct.hpp>
#include <ql/methods/montecarlo/exercisestrategy.hpp>
#include <ql/utilities/clone.hpp>
#include <valarray>

namespace QuantLib {

    class CallSpecifiedMultiProduct : public MarketModelMultiProduct {
     public:
       CallSpecifiedMultiProduct(
           const Clone<MarketModelMultiProduct>& underlying,
           const Clone<ExerciseStrategy<CurveState> >&,
           Clone<MarketModelMultiProduct> rebate = Clone<MarketModelMultiProduct>());
       //! \name MarketModelMultiProduct interface
       //@{
       std::vector<Size> suggestedNumeraires() const override;
       const EvolutionDescription& evolution() const override;
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
        //@}
        const MarketModelMultiProduct& underlying() const;
        const ExerciseStrategy<CurveState>& strategy() const;
        const MarketModelMultiProduct& rebate() const;
        void enableCallability();
        void disableCallability();
      private:
        Clone<MarketModelMultiProduct> underlying_;
        Clone<ExerciseStrategy<CurveState> > strategy_;
        Clone<MarketModelMultiProduct> rebate_;
        EvolutionDescription evolution_;
        std::vector<std::valarray<bool> > isPresent_;
        std::vector<Time> cashFlowTimes_;
        Size rebateOffset_;
        bool wasCalled_;
        std::vector<Size> dummyCashFlowsThisStep_;
        std::vector<std::vector<CashFlow> > dummyCashFlowsGenerated_;
        Size currentIndex_;
        bool callable_;
    };

}

#endif
