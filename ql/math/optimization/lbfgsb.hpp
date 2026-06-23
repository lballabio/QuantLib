/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Colin Alberts

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

/*! \file lbfgsb.hpp
    \brief L-BFGS-B limited-memory bound-constrained optimization method
*/

#ifndef quantlib_optimization_lbfgsb_hpp
#define quantlib_optimization_lbfgsb_hpp

#include <ql/math/array.hpp>
#include <ql/math/optimization/method.hpp>
#include <vector>

namespace QuantLib {

    //! Limited-memory BFGS for bound-constrained optimization (L-BFGS-B)
    /*! Faithful implementation of the algorithm of Byrd, Lu, Nocedal and
        Zhu, "A Limited Memory Algorithm for Bound Constrained
        Optimization", SIAM J. Sci. Comput. 16(5):1190-1208, 1995.

        Coordinates whose bound is \f$ \pm \mathrm{DBL\_MAX} \f$ are treated as
        unbounded, so with no active bounds the method reduces to plain
        limited-memory BFGS. Explicit bounds may instead be passed to the
        constructor, overriding the constraint.

        Limited memory trades accuracy of the Hessian model for cost: it
        stores only the last \c memory correction pairs, needing
        \f$ O(\mathrm{memory}\cdot n) \f$ storage and work per step instead of
        the \f$ O(n^2) \f$ of dense BFGS, at the price of linear rather than
        superlinear convergence. This typically results in a modest increase
        in iteration count.

        \ingroup optimizers
    */
    class LBFGSB : public OptimizationMethod {
      public:
        /*! \param memory number of stored correction pairs
            \param pgTol  convergence tolerance on the infinity norm of
                          the projected gradient
            \param fTol   the iteration stops when the relative reduction
                          of the objective falls below <tt>fTol</tt>
        */
        explicit LBFGSB(Size memory = 10, Real pgTol = 1e-8, Real fTol = 1e7 * QL_EPSILON);

        /*! Convenience constructor taking explicit box bounds; these
            override the bounds of the problem's constraint. Use
            \f$ \pm \mathrm{DBL\_MAX} \f$ for unbounded coordinates.
        */
        LBFGSB(Array lowerBound,
               Array upperBound,
               Size memory = 10,
               Real pgTol = 1e-8,
               Real fTol = 1e7 * QL_EPSILON);

        EndCriteria::Type minimize(Problem& P, const EndCriteria& endCriteria) override;

      private:
        Size m_;
        Real pgTol_, factr_;
        Array lowerBound_, upperBound_;
    };

}

#endif
