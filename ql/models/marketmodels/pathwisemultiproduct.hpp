/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2006, 2008 Mark Joshi

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


#ifndef quantlib_market_model_pathwise_multi_product_hpp
#define quantlib_market_model_pathwise_multi_product_hpp

#include <ql/types.hpp>
#include <vector>
#include <memory>

namespace QuantLib {

    class EvolutionDescription;
    class CurveState;

    //! market-model pathwise product
    /*! This is the abstract base class that encapsulates the notion of a
    product: it contains the information that would be in the termsheet
    of the product.

    It's useful to have it be able to do several products simultaneously.
    The products would have to have the same underlying rate times of
    course. The class is therefore really encapsulating the notion of a
    multi-product.

    For each time evolved to, it generates the cash flows associated to
    that time for the state of the yield curve. If one was doing a
    callable product then this would encompass the product and its
    exercise strategy.

    This class differs from market-model multi-product in that it also returns the
    derivative of the pay-off with respect to each forward rate

    */

    class MarketModelPathwiseMultiProduct 
    {
    public:
        struct CashFlow {
            Size timeIndex;
            std::vector<Real > amount;
        };
        virtual ~MarketModelPathwiseMultiProduct() {}

        virtual std::vector<Size> suggestedNumeraires() const = 0;
        virtual const EvolutionDescription& evolution() const = 0;
        virtual std::vector<Time> possibleCashFlowTimes() const = 0;
        virtual Size numberOfProducts() const = 0;
        virtual Size maxNumberOfCashFlowsPerProductPerStep() const = 0;

        virtual bool alreadyDeflated() const =0;

        //! during simulation put product at start of path
        virtual void reset() = 0;
        //! return value indicates whether path is finished, TRUE means done
        virtual bool nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >& cashFlowsGenerated) = 0;
        //! returns a newly-allocated copy of itself
        virtual std::auto_ptr<MarketModelPathwiseMultiProduct> clone() const = 0;
    };

}


#endif
