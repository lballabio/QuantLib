
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
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file bisection.cpp
    \brief bisection 1-D solver

    $Source$
    $Log$
    Revision 1.15  2001/04/02 10:59:49  lballabio
    Changed ObjectiveFunction::value to ObjectiveFunction::operator() - also in Python module

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


#include "bisection.h"

namespace QuantLib {

    namespace Solvers1D {

        double Bisection::solve_(const ObjectiveFunction& f,
                                 double xAccuracy) const {

            double dx, xMid, fMid;

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
                xMid=root_+dx;
                fMid=f(xMid);
                evaluationNumber_++;
                    if (fMid <= 0.0)
                        root_=xMid;
                    if (QL_FABS(dx) < xAccuracy || fMid == 0.0) {
                        return root_;
                    }
            }
            throw Error("Bisection: maximum number of function evaluations (" +
                            IntegerFormatter::toString(maxEvaluations_) +
                            ") exceeded");

            QL_DUMMY_RETURN(0.0);
        }

    }

}
