
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file newtonsafe.cpp
    \brief safe Newton 1-D solver

    $Source$
    $Name$
    $Log$
    Revision 1.15  2001/01/17 13:54:02  nando
    80 columns enforced
    tabs removed
    private data member now have trailing underscore

    Revision 1.14  2000/12/27 14:05:57  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

    Revision 1.13  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

/* The implementation of the algorithm was inspired by
 * "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
 * Chapter 9
 */


#include "newtonsafe.h"

namespace QuantLib {

    namespace Solvers1D {

        double NewtonSafe::solve_(const ObjectiveFunction& f,
                                  double xAccuracy) const {

            double froot, dfroot, dx, dxold;
            double xh, xl;

            // Orient the search so that f(xl) < 0
            if (fxMin_ < 0.0) {
                xl=xMin_;
                xh=xMax_;
            } else {
                xh=xMin_;
                xl=xMax_;
            }

            // the "stepsize before last"
            dxold=xMax_-xMin_;
            // it was dxold=fabs(xMax_-xMin_); in Numerical Recipes
            // here (xMax_-xMin_ > 0) is verified in the constructor

            // and the last step
            dx=dxold;

            froot = f.value(root_);
            dfroot = f.derivative(root_);
            QL_REQUIRE(!IsNull(dfroot),
                    "NewtonSafe requires function's derivative");
            evaluationNumber_++;

            while (evaluationNumber_<=maxEvaluations_) {
                // Bisect if (out of range || not decreasing fast enough)
                if ((((root_-xh)*dfroot-froot)*((root_-xl)*dfroot-froot) > 0.0)
                        || (QL_FABS(2.0*froot) > QL_FABS(dxold*dfroot))) {
                    dxold = dx;
                    dx = (xh-xl)/2.0;
                    root_=xl+dx;
                } else {
                    dxold=dx;
                    dx=froot/dfroot;
                    root_ -= dx;
                }
                // Convergence criterion
                if (QL_FABS(dx) < xAccuracy)
                    return root_;
                froot = f.value(root_);
                dfroot = f.derivative(root_);
                evaluationNumber_++;
                if (froot < 0.0)
                    xl=root_;
                else
                    xh=root_;
            }

            throw Error("NewtonSafe: maximum number of function evaluations (" +
                        IntegerFormatter::toString(maxEvaluations_) +
                        ") exceeded");

            QL_DUMMY_RETURN(0.0);

        } // double NewtonSafe::solve_

    }     // namespace Solvers1D

}         // namespace QuantLib
