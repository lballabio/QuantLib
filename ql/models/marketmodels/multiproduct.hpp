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


#ifndef quantlib_market_model_multi_product_hpp
#define quantlib_market_model_multi_product_hpp

#include <ql/types.hpp>
#include <vector>
#include <memory>

namespace QuantLib {

    class EvolutionDescription;
    class CurveState;

    //! market-model product
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

    */

    class MarketModelMultiProduct {
     public:
        struct CashFlow {
            Size timeIndex;
            Real amount;
        };
        virtual ~MarketModelMultiProduct() = default;

        virtual std::vector<Size> suggestedNumeraires() const = 0;
        virtual const EvolutionDescription& evolution() const = 0;
        virtual std::vector<Time> possibleCashFlowTimes() const = 0;
        virtual Size numberOfProducts() const = 0;
        virtual Size maxNumberOfCashFlowsPerProductPerStep() const = 0;
        //! during simulation put product at start of path
        virtual void reset() = 0;
        //! return value indicates whether path is finished, TRUE means done
        virtual bool nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<CashFlow> >& cashFlowsGenerated) = 0;
        //! returns a newly-allocated copy of itself
        #if defined(QL_USE_STD_UNIQUE_PTR)
        virtual std::unique_ptr<MarketModelMultiProduct> clone() const = 0;
        #else
        virtual std::auto_ptr<MarketModelMultiProduct> clone() const = 0;
        #endif
    };

}


#endif


#ifndef id_f8d6bd6bf94d905b25023a1dd68c6517
#define id_f8d6bd6bf94d905b25023a1dd68c6517
inline bool test_f8d6bd6bf94d905b25023a1dd68c6517(int* i) { return i != 0; }
#endif
