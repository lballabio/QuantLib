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

/*! \file secant.hpp
    \brief secant 1-D solver
*/

#ifndef quantlib_solver1d_secant_h
#define quantlib_solver1d_secant_h

#include <ql/math/solver1d.hpp>

namespace QuantLib {

    //! %Secant 1-D solver
    /*! \test the correctness of the returned values is tested by
              checking them against known good results.
    */
    class Secant : public Solver1D<Secant> {
      public:
        template <class F>
        Real solveImpl(const F& f,
                       Real xAccuracy) const {

            /* The implementation of the algorithm was inspired by
               Press, Teukolsky, Vetterling, and Flannery,
               "Numerical Recipes in C", 2nd edition,
               Cambridge University Press
            */

            Real fl, froot, dx, xl;

            // Pick the bound with the smaller function value
            // as the most recent guess
            if (std::fabs(fxMin_) < std::fabs(fxMax_)) {
                root_ = xMin_;
                froot = fxMin_;
                xl = xMax_;
                fl = fxMax_;
            } else {
                root_ = xMax_;
                froot = fxMax_;
                xl = xMin_;
                fl = fxMin_;
            }
            while (evaluationNumber_<=maxEvaluations_) {
                dx = (xl-root_)*froot/(froot-fl);
                xl = root_;
                fl = froot;
                root_ += dx;
                froot = f(root_);
                ++evaluationNumber_;
                if (std::fabs(dx) < xAccuracy || (close(froot, 0.0)))
                    return root_;
            }
            QL_FAIL("maximum number of function evaluations ("
                    << maxEvaluations_ << ") exceeded");
        }
    };

}

#endif
