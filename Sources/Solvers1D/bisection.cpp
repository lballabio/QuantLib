
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file bisection.cpp
    \brief bisection 1-D solver

    $Source$
    $Log$
    Revision 1.21  2001/05/09 11:06:19  nando
    A few comments modified/removed

    Revision 1.20  2001/04/09 14:13:34  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.19  2001/04/06 18:46:22  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.18  2001/04/04 13:32:54  enri
    tons of typos fixed

    Revision 1.17  2001/04/04 12:13:24  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.16  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

*/

/* The implementation of the algorithm was inspired by
 * "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
 * Chapter 9
 */


#include "ql/Solvers1D/bisection.hpp"

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
