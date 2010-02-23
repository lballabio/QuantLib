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


#ifndef quantlib_market_model_pathwise_call_specified_hpp
#define quantlib_market_model_pathwise_call_specified_hpp

#include <ql/types.hpp>
#include <ql/models/marketmodels/pathwisemultiproduct.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/curvestates/lmmcurvestate.hpp>
#include <ql/methods/montecarlo/exercisestrategy.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/utilities/clone.hpp>
#include <valarray>
#include <vector>
#include <memory>

namespace QuantLib
{
    class curveState;


    class CallSpecifiedPathwiseMultiProduct : public MarketModelPathwiseMultiProduct
    {
    public:
        CallSpecifiedPathwiseMultiProduct(
            const Clone<MarketModelPathwiseMultiProduct>& underlying,
            const Clone<ExerciseStrategy<CurveState> >&,
            const Clone<MarketModelPathwiseMultiProduct>& rebate
            = Clone<MarketModelPathwiseMultiProduct>());
        //! \name MarketModelPathwiseMultiProduct interface
        //@{
        virtual std::vector<Size> suggestedNumeraires() const;
        virtual const EvolutionDescription& evolution() const;
        virtual std::vector<Time> possibleCashFlowTimes() const;
        virtual Size numberOfProducts() const;
        virtual Size maxNumberOfCashFlowsPerProductPerStep() const;
        virtual void reset();
        virtual bool nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >& cashFlowsGenerated) ;

        virtual std::auto_ptr<MarketModelPathwiseMultiProduct> clone() const;
          
        virtual bool alreadyDeflated() const;
        //@}
        const MarketModelPathwiseMultiProduct& underlying() const;
        const ExerciseStrategy<CurveState>& strategy() const;
        const MarketModelPathwiseMultiProduct& rebate() const;
        void enableCallability();
        void disableCallability();
              

    private:
        Clone<MarketModelPathwiseMultiProduct> underlying_;
        Clone<ExerciseStrategy<CurveState> > strategy_;
        Clone<MarketModelPathwiseMultiProduct> rebate_;
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
