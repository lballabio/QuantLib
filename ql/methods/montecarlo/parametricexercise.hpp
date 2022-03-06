/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

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


#ifndef quantlib_parametric_exercise_hpp
#define quantlib_parametric_exercise_hpp

#include <ql/methods/montecarlo/nodedata.hpp>
#include <ql/math/optimization/method.hpp>

namespace QuantLib {

    class ParametricExercise {
      public:
        virtual ~ParametricExercise() = default;
        // possibly different for each exercise
        virtual std::vector<Size> numberOfVariables() const = 0;
        virtual std::vector<Size> numberOfParameters() const = 0;
        virtual bool exercise(Size exerciseNumber,
                              const std::vector<Real>& parameters,
                              const std::vector<Real>& variables) const = 0;
        virtual void guess(Size exerciseNumber,
                           std::vector<Real>& parameters) const = 0;
    };
    

    //! returns the biased estimate obtained while optimizing
    /* TODO document:
       n exercises, n+1 elements in simulationData
       simulationData[0][j] -> cashflows up to first exercise, j-th path
       simulationData[i+1][j] -> i-th exercise, j-th path

       simulationData[0][j].foo unused (unusable?) if foo != cumulatedCashFlows

       parameters.size() = n
    */
    Real genericEarlyExerciseOptimization(
        std::vector<std::vector<NodeData> >& simulationData,
        const ParametricExercise& exercise,
        std::vector<std::vector<Real> >& parameters,
        const EndCriteria& endCriteria,
        OptimizationMethod& method);

}


#endif



#ifndef id_801ac7e1a8799a8b1c98591c75d55873
#define id_801ac7e1a8799a8b1c98591c75d55873
inline bool test_801ac7e1a8799a8b1c98591c75d55873(int* i) { return i != 0; }
#endif
