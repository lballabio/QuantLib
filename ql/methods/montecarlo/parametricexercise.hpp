/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_parametric_exercise_hpp
#define quantlib_parametric_exercise_hpp

#include <ql/methods/montecarlo/nodedata.hpp>
#include <ql/math/optimization/method.hpp>

namespace QuantLib {

    //! Abstract interface for a parametric early-exercise strategy.
    /*! Derived classes encode a family of exercise boundaries
        parameterized by a set of real-valued parameters.  The
        optimal parameters are found by numerical optimization
        (see genericEarlyExerciseOptimization()).
    */
    class ParametricExercise {
      public:
        virtual ~ParametricExercise() = default;
        //! number of state variables used at each exercise date
        virtual std::vector<Size> numberOfVariables() const = 0;
        //! number of free parameters at each exercise date
        virtual std::vector<Size> numberOfParameters() const = 0;
        /*! returns true if exercise is optimal given \p parameters
        and the state \p variables at \p exerciseNumber */
        virtual bool exercise(Size exerciseNumber,
                              const std::vector<Real>& parameters,
                              const std::vector<Real>& variables) const = 0;
        //! provides an initial guess for the parameters
        virtual void guess(Size exerciseNumber,
                           std::vector<Real>& parameters) const = 0;
    };
    

    /*! Estimates the value of early-exercise rights
        by optimizing a parametric exercise strategy.

        Returns a biased estimate of the option value
        by maximizing the strategy value over parameters at each exercise date.

        \param simulationData Node data for all paths and exercise dates.
                              Contains n+1 elements for n exercise dates
                              - simulationData[0][j]: accumulated cash flows on path j
                              - simulationData[i+1][j]: data at exercise date i on path j
        \param exercise Parametric exercise strategy to optimize
        \param parameters Output: optimal parameters for each exercise date (size = n)
        \param endCriteria Convergence criteria for the optimizer
        \param method Optimization algorithm
    */
    Real genericEarlyExerciseOptimization(
        std::vector<std::vector<NodeData> >& simulationData,
        const ParametricExercise& exercise,
        std::vector<std::vector<Real> >& parameters,
        const EndCriteria& endCriteria,
        OptimizationMethod& method);

}


#endif

