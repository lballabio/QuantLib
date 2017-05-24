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

/*! \file bisection.hpp
    \brief bisection 1-D solver
*/

#ifndef quantlib_solver1d_bisection_h
#define quantlib_solver1d_bisection_h

#include <ql/math/solver1d.hpp>

namespace QuantLib {

    //! %Bisection 1-D solver
    /*! \test the correctness of the returned values is tested by
              checking them against known good results.

        \ingroup solvers
    */
    class Bisection : public Solver1D<Bisection> {
      public:
        template <class F>
        Real solveImpl(const F& f,
                       Real xAccuracy) const {

            /* The implementation of the algorithm was inspired by
               Press, Teukolsky, Vetterling, and Flannery,
               "Numerical Recipes in C", 2nd edition, Cambridge
               University Press
            */

            Real dx, xMid, fMid;

            // Orient the search so that f>0 lies at root_+dx
            if (fxMin_ < 0.0) {
                dx = xMax_-xMin_;
                root_ = xMin_;
            } else {
                dx = xMin_-xMax_;
                root_ = xMax_;
            }

            while (evaluationNumber_<=maxEvaluations_) {
                dx /= 2.0;
                xMid = root_+dx;
                fMid = f(xMid);
                ++evaluationNumber_;
                if (fMid <= 0.0)
                    root_ = xMid;
                if (std::fabs(dx) < xAccuracy || (close(fMid, 0.0))) {
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
