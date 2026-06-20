/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Colin Alberts

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

namespace QuantLib {

    //! Limited-memory BFGS for bound-constrained optimization (L-BFGS-B)
    /*! This is a faithful implementation of the limited-memory BFGS
        algorithm for box-constrained optimization of Byrd, Lu, Nocedal
        and Zhu (1995), the method wrapped by the canonical Fortran
        routine ACM TOMS Algorithm 778 and by
        <tt>scipy.optimize.minimize(method='L-BFGS-B')</tt>.

        Each iteration builds the compact limited-memory representation
        \f$ B_k = \theta I - W M W^T \f$ of the BFGS Hessian
        approximation, computes the generalized Cauchy point along the
        projected steepest-descent path (which determines the active set
        of variables held at a bound), minimizes the quadratic model over
        the remaining free variables by the direct primal method, and
        finally performs a line search satisfying the strong Wolfe
        conditions along the resulting direction.

        Box bounds are taken from the optimization Problem's constraint
        through <tt>lowerBound()</tt> / <tt>upperBound()</tt>; coordinates
        whose bound is \f$ \pm\infty \f$ (i.e.
        \f$ \pm \mathrm{DBL\_MAX} \f$, as returned by the default
        Constraint and by NoConstraint) are treated as unbounded, so with
        no active bounds the method reduces to plain limited-memory BFGS.
        Explicit bounds may also be supplied to the constructor, in which
        case they override those of the constraint.

        References:
        - R. H. Byrd, P. Lu, J. Nocedal and C. Zhu, "A Limited Memory
          Algorithm for Bound Constrained Optimization", SIAM J. Sci.
          Comput. 16(5):1190-1208, 1995.
        - R. H. Byrd, J. Nocedal and R. B. Schnabel, "Representations of
          quasi-Newton matrices and their use in limited memory methods",
          Math. Programming 63:129-156, 1994.

        \ingroup optimizers
    */
    class LBFGSB : public OptimizationMethod {
      public:
        /*! \param memory number of stored correction pairs (the "m" of
                          L-BFGS-B)
            \param pgTol  convergence tolerance on the infinity norm of
                          the projected gradient (mirrors SciPy's
                          \c pgtol)
            \param factr  the iteration stops when the relative reduction
                          of the objective falls below
                          <tt>factr</tt> times machine precision (mirrors
                          SciPy's \c factr)
        */
        explicit LBFGSB(Size memory = 10, Real pgTol = 1e-8, Real factr = 1e7);

        /*! Convenience constructor taking explicit box bounds; these
            override the bounds of the Problem's constraint. Use
            \f$ \pm \mathrm{DBL\_MAX} \f$ for unbounded coordinates.
        */
        LBFGSB(Array lowerBound,
               Array upperBound,
               Size memory = 10,
               Real pgTol = 1e-8,
               Real factr = 1e7);

        EndCriteria::Type minimize(Problem& P,
                                   const EndCriteria& endCriteria) override;

      private:
        Size m_;
        Real pgTol_, factr_;
        Array lowerBound_, upperBound_;
    };

}

#endif
