
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file secant.cpp
    \brief secant 1-D solver

    \fullpath
    ql/Solvers1D/%secant.cpp
*/

// $Id$

/* The implementation of the algorithm was inspired by
   Press, Teukolsky, Vetterling, and Flannery,
   "Numerical Recipes in C", 2nd edition, 
   Cambridge University Press
*/


#include <ql/Solvers1D/secant.hpp>

namespace QuantLib {

    namespace Solvers1D {

        double Secant::solve_(const ObjectiveFunction& f,
                              double xAccuracy) const {

            double fl, froot, dx, xl;

            // Pick the bound with the smaller function value
            // as the most recent guess
            if (QL_FABS(fxMin_) < QL_FABS(fxMax_)) {
                root_=xMin_;
                froot=fxMin_;
                xl=xMax_;
                fl=fxMax_;
            } else {
                root_=xMax_;
                froot=fxMax_;
                xl=xMin_;
                fl=fxMin_;
            }
            while (evaluationNumber_<=maxEvaluations_) {
                dx=(xl-root_)*froot/(froot-fl);
                xl=root_;
                fl=froot;
                root_ += dx;
                froot=f(root_);
              evaluationNumber_++;
                if (QL_FABS(dx) < xAccuracy || froot == 0.0)  return root_;
            }
            throw Error("Secant: maximum number of function evaluations (" +
                            IntegerFormatter::toString(maxEvaluations_) +
                            ") exceeded");
            QL_DUMMY_RETURN(0.0);
        }

    }

}
