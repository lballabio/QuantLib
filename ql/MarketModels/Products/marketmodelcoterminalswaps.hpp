/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Giorgio Facchinetti

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


#ifndef quantlib_MarketModelCoterminalSwaps_hpp
#define quantlib_MarketModelCoterminalSwaps_hpp

#include <ql/MarketModels/marketmodelproduct.hpp>

namespace QuantLib {
    class MarketModelCoterminalSwaps : public MarketModelProduct
    {
      public:
        MarketModelCoterminalSwaps(const std::vector<Time>& rateTimes,
                           const std::vector<Real>& fixedAccruals,
                           const std::vector<Real>& floatingAccruals,
                           const std::vector<Time>& paymentTimes,
                           double fixedRate);
        //! for initializing other objects
        virtual EvolutionDescription suggestedEvolution() const;
        virtual std::vector<Time> possibleCashFlowTimes() const;
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
        std::vector<Time> rateTimes_;
        std::vector<Real> fixedAccruals_, floatingAccruals_;
        std::vector<Time> paymentTimes_;
        double fixedRate_;

        Size lastIndex_;

        // things that vary in a path
        Size currentIndex_;
        
    };

    // inline 

    inline std::vector<Time>
    MarketModelCoterminalSwaps::possibleCashFlowTimes() const {
        return paymentTimes_;
    }

    inline Size MarketModelCoterminalSwaps::numberOfProducts() const {
        return lastIndex_;    
    }

    inline Size
    MarketModelCoterminalSwaps::maxNumberOfCashFlowsPerProductPerStep() const {
        return 2;
    }

    inline void MarketModelCoterminalSwaps::reset() {
       currentIndex_=0;
    }
       
}

#endif
