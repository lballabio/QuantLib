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


#ifndef quantlib_MarketModelForwards_hpp
#define quantlib_MarketModelForwards_hpp

#ifndef QL_EXTRA_SAFETY_CHECKS
#   define QL_EXTRA_SAFETY_CHECKS
#endif

#include <ql/MarketModels/marketmodelproduct.hpp>

namespace QuantLib {
    class MarketModelForwards : public MarketModelProduct
    {
    public:
  
        MarketModelForwards(const Array& rateTimes,
                            const Array& accruals,
                            const Array& paymentTimes,
                            const Array& strikes);
      
        virtual ~MarketModelForwards();
       
        //! for initializing other objects
        virtual EvolutionDescription suggestedEvolution() const;
        virtual Array possibleCashFlowTimes() const;
        virtual Size numberOfProducts() const;
        virtual Size maxNumberOfCashFlowsPerProductPerStep() const;

        //!during simulation
        //!put product at start of path
        virtual void reset(); 
        //! bool return indicates whether path is finished, true means done
        virtual bool nextTimeStep(const CurveState& currentState, 
            std::vector<Size>& numberCashFlowsThisStep, //! one int for each product 
            std::vector<std::vector<CashFlow> >& cashFlowsGenerated); //! the cash flows

    private:
            Array rateTimes_;
            Array accruals_;
            Array paymentTimes_;
            Array strikes_;
            // things that vary in a path
            Size currentIndex_;
    };

}


#endif
