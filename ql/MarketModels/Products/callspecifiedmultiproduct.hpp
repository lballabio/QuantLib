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


#ifndef quantlib_callspecified_multiproduct_hpp
#define quantlib_callspecified_multiproduct_hpp

#include <ql/MarketModels/marketmodelproduct.hpp>
#include <ql/MonteCarlo/exercisestrategy.hpp>

namespace QuantLib {

    /*! 
    */
    class CallSpecifiedMultiProduct : public MarketModelMultiProduct {
     public:
        CallSpecifiedMultiProduct(
                  const boost::shared_ptr<MarketModelMultiProduct>& underlying,
                  const boost::shared_ptr<ExerciseStrategy<CurveState> >&,
                  const boost::shared_ptr<MarketModelMultiProduct>& rebate
                      = const boost::shared_ptr<MarketModelMultiProduct>());

        EvolutionDescription suggestedEvolution() const;
        std::vector<Time> possibleCashFlowTimes() const;
        Size numberOfProducts() const;
        Size maxNumberOfCashFlowsPerProductPerStep() const;

        void reset(); 

        bool nextTimeStep(
            const CurveState& currentState, 
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<CashFlow> >& cashFlowsGenerated);

      private:
        boost::shared_ptr<MarketModelMultiProduct> underlying_;
        boost::shared_ptr<ExerciseStrategy<CurveState> > strategy_;
        boost::shared_ptr<MarketModelMultiProduct> rebate_;
        EvolutionDescription evolution_;
        std::vector<bool> isProductTime_, isExerciseTime_;
        std::vector<Time> cashFlowTimes_;
        Size rebateOffset_;
    };

}

#endif
