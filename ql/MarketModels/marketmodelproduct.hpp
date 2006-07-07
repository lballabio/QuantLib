/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_marketmodelproduct_hpp
#define quantlib_marketmodelproduct_hpp

#include <ql/MarketModels/evolutiondescription.hpp>
#include <ql/MarketModels/curvestate.hpp>

namespace QuantLib {

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

    class MarketModelProduct
    {
    public:
  
        struct CashFlow
        {
            Size timeIndex;
            Real amount;
        }; 

        virtual ~MarketModelProduct() {}
       
        //! for initializing other objects
        virtual EvolutionDescription suggestedEvolution() const=0;
        virtual Array possibleCashFlowTimes() const=0;
        virtual Size numberOfProducts() const=0;
        virtual Size maxNumberOfCashFlowsPerProductPerStep() const=0;

        //!during simulation
        //!put product at start of path
        virtual void reset()=0; 
        //! bool return indicates whether path is finished, true means done
        virtual bool nextTimeStep(
            const CurveState& currentState, 
            std::vector<Size>& numberCashFlowsThisStep, //! one int for each product 
            std::vector<std::vector<CashFlow> >& cashFlowsGenerated)=0 ; //! the cash flows

    };

}

#endif
