
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

/*! \file newton.cpp
    \brief Newton 1-D solver

    $Source$
    $Log$
    Revision 1.18  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.17  2001/04/02 10:59:49  lballabio
    Changed ObjectiveFunction::value to ObjectiveFunction::operator() - also in Python module

    Revision 1.16  2001/03/12 17:35:11  lballabio
    Removed global IsNull function - could have caused very vicious loops

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


#include "newton.hpp"
#include "newtonsafe.hpp"

namespace QuantLib {

    namespace Solvers1D {

        double Newton::solve_(const ObjectiveFunction& f,
                              double xAccuracy) const {
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
                    NewtonSafe helper;
                    helper.setMaxEvaluations(maxEvaluations_-evaluationNumber_);
                    return helper.solve(f, xAccuracy, root_+dx, xMin_, xMax_);
                }
                if (QL_FABS(dx) < xAccuracy)
                    return root_;
                froot = f(root_);
                dfroot = f.derivative(root_);
                evaluationNumber_++;
            }

            throw Error("Newton: maximum number of function evaluations (" +
                        IntegerFormatter::toString(maxEvaluations_) +
                        ") exceeded");

            QL_DUMMY_RETURN(0.0);
        }

    }

}
