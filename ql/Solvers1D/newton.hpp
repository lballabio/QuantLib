
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file newton.hpp
    \brief Newton 1-D solver
*/

#ifndef quantlib_solver1d_newton_h
#define quantlib_solver1d_newton_h

#include <ql/Solvers1D/newtonsafe.hpp>

namespace QuantLib {

    //! %Newton 1-D solver
    /*! \note This solver requires that the passed function object
              implement a method <tt>double derivative(double)</tt>.
    */
    class Newton : public Solver1D<Newton> {
      public:
        template <class F>
        double solveImpl(const F& f, double xAccuracy) const {

            /* The implementation of the algorithm was inspired by
               Press, Teukolsky, Vetterling, and Flannery,
               "Numerical Recipes in C", 2nd edition, 
               Cambridge University Press
            */

            double froot, dfroot, dx;

            froot = f(root_);
            dfroot = f.derivative(root_);
            QL_REQUIRE(dfroot != Null<double>(),
                       "Newton requires function's derivative");
            evaluationNumber_++;

            while (evaluationNumber_<=maxEvaluations_) {
                dx=froot/dfroot;
                root_ -= dx;
                // jumped out of brackets, switch to NewtonSafe
                if ((xMin_-root_)*(root_-xMax_) < 0.0) {
                    NewtonSafe s;
                    s.setMaxEvaluations(maxEvaluations_-evaluationNumber_);
                    return s.solve(f, xAccuracy, root_+dx, xMin_, xMax_);
                }
                if (QL_FABS(dx) < xAccuracy)
                    return root_;
                froot = f(root_);
                dfroot = f.derivative(root_);
                evaluationNumber_++;
            }

            QL_FAIL("maximum number of function evaluations (" +
                    SizeFormatter::toString(maxEvaluations_) +
                    ") exceeded");

            QL_DUMMY_RETURN(0.0);
        }
    };

}


#endif
