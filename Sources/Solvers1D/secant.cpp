
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

/*! \file secant.cpp
    \brief secant 1-D solver

    $Source$
    $Name$
    $Log$
    Revision 1.14  2001/01/17 13:54:02  nando
    80 columns enforced
    tabs removed
    private data member now have trailing underscore

    Revision 1.13  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

/* The implementation of the algorithm was inspired by
 * "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
 * Chapter 9
 */


#include "secant.h"

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
                froot=f.value(root_);
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
