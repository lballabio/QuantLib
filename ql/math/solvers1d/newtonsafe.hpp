/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file newtonsafe.hpp
    \brief Safe (bracketed) Newton 1-D solver
*/

#ifndef quantlib_solver1d_newtonsafe_h
#define quantlib_solver1d_newtonsafe_h

#include <ql/math/solver1d.hpp>

namespace QuantLib {

    //! safe %Newton 1-D solver
    /*! \note This solver requires that the passed function object
              implement a method <tt>Real derivative(Real)</tt>.

        \test the correctness of the returned values is tested by
              checking them against known good results.
    */
    class NewtonSafe : public Solver1D<NewtonSafe> {
      public:
        template <class F>
        Real solveImpl(const F& f,
                       Real xAccuracy) const {

            /* The implementation of the algorithm was inspired by
               Press, Teukolsky, Vetterling, and Flannery,
               "Numerical Recipes in C", 2nd edition,
               Cambridge University Press
            */

            Real froot, dfroot, dx, dxold;
            Real xh, xl;

            // Orient the search so that f(xl) < 0
            if (fxMin_ < 0.0) {
                xl = xMin_;
                xh = xMax_;
            } else {
                xh = xMin_;
                xl = xMax_;
            }

            // the "stepsize before last"
            dxold = xMax_-xMin_;
            // it was dxold=std::fabs(xMax_-xMin_); in Numerical Recipes
            // here (xMax_-xMin_ > 0) is verified in the constructor

            // and the last step
            dx = dxold;

            froot = f(root_);
            dfroot = f.derivative(root_);
            QL_REQUIRE(dfroot != Null<Real>(),
                       "NewtonSafe requires function's derivative");
            ++evaluationNumber_;

            while (evaluationNumber_<=maxEvaluations_) {
                // Bisect if (out of range || not decreasing fast enough)
                if ((((root_-xh)*dfroot-froot)*
                     ((root_-xl)*dfroot-froot) > 0.0)
                    || (std::fabs(2.0*froot) > std::fabs(dxold*dfroot))) {

                    dxold = dx;
                    dx = (xh-xl)/2.0;
                    root_=xl+dx;
                } else {
                    dxold = dx;
                    dx = froot/dfroot;
                    root_ -= dx;
                }
                // Convergence criterion
                if (std::fabs(dx) < xAccuracy) {
                    f(root_);
                    ++evaluationNumber_;
                    return root_;
                }
                froot = f(root_);
                dfroot = f.derivative(root_);
                ++evaluationNumber_;
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
