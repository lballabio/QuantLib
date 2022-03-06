/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Ferdinando Ametrano

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

/*! \file finitedifferencenewtonsafe.hpp
    \brief Safe (bracketed) Newton 1-D solver with finite difference derivatives
*/

#ifndef quantlib_solver1d_finitedifferencenewtonsafe_h
#define quantlib_solver1d_finitedifferencenewtonsafe_h

#include <ql/math/solver1d.hpp>

namespace QuantLib {

    //! safe %Newton 1-D solver with finite difference derivatives
    /*!
        \test the correctness of the returned values is tested by
              checking them against known good results.

        \ingroup solvers
    */
    class FiniteDifferenceNewtonSafe : public Solver1D<FiniteDifferenceNewtonSafe> {
      public:
        template <class F>
        Real solveImpl(const F& f,
                       Real xAccuracy) const {

            // Orient the search so that f(xl) < 0
            Real xh, xl;
            if (fxMin_ < 0.0) {
                xl = xMin_;
                xh = xMax_;
            } else {
                xh = xMin_;
                xl = xMax_;
            }

            Real froot = f(root_);
            ++evaluationNumber_;
            // first order finite difference derivative
            Real dfroot = xMax_-root_ < root_-xMin_ ?
                (fxMax_-froot)/(xMax_-root_) :
                (fxMin_-froot)/(xMin_-root_) ;

            // xMax_-xMin_>0 is verified in the constructor
            Real dx = xMax_-xMin_;
            while (evaluationNumber_<=maxEvaluations_) {
                Real frootold = froot;
                Real rootold = root_;
                Real dxold = dx;
                // Bisect if (out of range || not decreasing fast enough)
                if ((((root_-xh)*dfroot-froot)*
                     ((root_-xl)*dfroot-froot) > 0.0)
                    || (std::fabs(2.0*froot) > std::fabs(dxold*dfroot))) {
                    dx = (xh-xl)/2.0;
                    root_ = xl+dx;
                    // if the root estimate just computed is close to the
                    // previous one, we should calculate dfroot at root and
                    // xh rather than root and rootold (xl instead of xh would
                    // be just as good)
                    if (close(root_, rootold, 2500)) {
                        rootold = xh;
                        frootold = f(xh);
                    }
                } else { // Newton
                    dx = froot/dfroot;
                    root_ -= dx;
                }

                // Convergence criterion
                if (std::fabs(dx) < xAccuracy)
                    return root_;

                froot = f(root_);
                ++evaluationNumber_;
                dfroot = (frootold-froot)/(rootold-root_);

                if (froot < 0.0)
                    xl=root_;
                else
                    xh=root_;
            }

            QL_FAIL("maximum number of function evaluations ("
                    << maxEvaluations_ << ") exceeded");
        }
    };

}

#endif


#ifndef id_7b18ca8ef82bb5059fd02c52646a6042
#define id_7b18ca8ef82bb5059fd02c52646a6042
inline bool test_7b18ca8ef82bb5059fd02c52646a6042(const int* i) {
    return i != nullptr;
}
#endif
