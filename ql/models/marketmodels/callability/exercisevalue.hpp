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


#ifndef quantlib_market_model_exercise_value_hpp
#define quantlib_market_model_exercise_value_hpp

// to be removed using forward declaration
#include <ql/models/marketmodels/multiproduct.hpp>
#include <memory>
#include <valarray>

namespace QuantLib {

    class CurveState;
    class EvolutionDescription;
    //struct MarketModelMultiProduct::CashFlow;

    class MarketModelExerciseValue {
      public:
        virtual ~MarketModelExerciseValue() = default;
        virtual Size numberOfExercises() const = 0;
        // including any time at which state should be updated
        virtual const EvolutionDescription& evolution() const = 0;
        virtual std::vector<Time> possibleCashFlowTimes() const = 0;
        virtual void nextStep(const CurveState&) = 0;
        virtual void reset() = 0;
        // whether or not evolution times are exercise times
        virtual std::valarray<bool> isExerciseTime() const = 0;
        virtual MarketModelMultiProduct::CashFlow value(
                                               const CurveState&) const = 0;
        #if defined(QL_USE_STD_UNIQUE_PTR)
        virtual std::unique_ptr<MarketModelExerciseValue> clone() const = 0;
        #else
        virtual std::auto_ptr<MarketModelExerciseValue> clone() const = 0;
        #endif
    };

}


#endif


#ifndef id_4cddcb1b88b0544a78c022bc1d5ba67c
#define id_4cddcb1b88b0544a78c022bc1d5ba67c
inline bool test_4cddcb1b88b0544a78c022bc1d5ba67c(int* i) { return i != 0; }
#endif
