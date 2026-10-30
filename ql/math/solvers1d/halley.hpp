/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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

/*! \file halley.hpp
    \brief Halley 1-D solver
*/

#ifndef quantlib_solver1d_halley_hpp
#define quantlib_solver1d_halley_hpp

#include <ql/math/solvers1d/newtonsafe.hpp>

namespace QuantLib {

    //! %Halley 1-D solver
    /*! \note This solver requires that the passed function object
              implement a method <tt>Real derivative(Real)</tt>
              and <tt> Real secondDerivative(Real></tt>

        \test the correctness of the returned values is tested by
              checking them against known good results.

        \ingroup solvers
    */
    class Halley : public Solver1D<Halley> {
      public:
        template <class F>
        Real solveImpl(const F& f,
                       Real xAccuracy) const {

            while (++evaluationNumber_ <= maxEvaluations_) {
                const Real fx = f(root_);
                const Real fPrime = f.derivative(root_);
                const Real lf = fx*f.secondDerivative(root_)/(fPrime*fPrime);
                const Real step = 1.0/(1.0 - 0.5*lf)*fx/fPrime;
                root_ -= step;

                // jumped out of brackets, switch to NewtonSafe
                if ((xMin_-root_)*(root_-xMax_) < 0.0) {
                    NewtonSafe s;
                    s.setMaxEvaluations(maxEvaluations_-evaluationNumber_);
                    return s.solve(f, xAccuracy, root_+step, xMin_, xMax_);
                }

                if (std::abs(step) < xAccuracy) {
                    f(root_);
                    ++evaluationNumber_;
                    return root_;
                }

            }

            QL_FAIL("maximum number of function evaluations ("
                    << maxEvaluations_ << ") exceeded");
        }
    };
}

#endif
