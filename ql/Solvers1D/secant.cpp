
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file secant.cpp

    \fullpath
    Sources/Solvers1D/%secant.cpp
    \brief secant 1-D solver

*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 13:53:18  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.28  2001/08/31 15:23:47  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.27  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.26  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.25  2001/08/07 11:25:56  sigmud
// copyright header maintenance
//
// Revision 1.24  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.23  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

/* The implementation of the algorithm was inspired by
 * "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
 * Chapter 9
 */


#include "ql/Solvers1D/secant.hpp"

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
