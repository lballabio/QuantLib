
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

/*! \file ridder.cpp
    \fullpath Sources/Solvers1D/%ridder.cpp
    \brief Ridder 1-D solver

*/

// $Id$
// $Log$
// Revision 1.28  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.27  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.26  2001/08/07 11:25:56  sigmud
// copyright header maintenance
//
// Revision 1.25  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.24  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

/* The implementation of the algorithm was inspired by
 * "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
 * Chapter 9
 */


#include "ql/Solvers1D/ridder.hpp"

namespace QuantLib {

    namespace Solvers1D {

        #define SIGN(a,b) ((b) >= 0.0 ? QL_FABS(a) : -QL_FABS(a))

        double Ridder::solve_(const ObjectiveFunction& f, double xAcc) const {
            double fxMid, froot, s, xMid, nextRoot;

            // test on black scholes implied vol show that Ridder solver
            // algorythm actually provides an accuracy 100 times below promised
            double xAccuracy = xAcc/100.0;

            // Any highly unlikely value, to simplify logic below
            root_=QL_MIN_DOUBLE;

            while (evaluationNumber_<=maxEvaluations_) {
                xMid=0.5*(xMin_+xMax_);
                // First of two function evaluations per iteraton
                fxMid=f(xMid);
                evaluationNumber_++;
                s=QL_SQRT(fxMid*fxMid-fxMin_*fxMax_);
                if (s == 0.0)
                    return root_;
                // Updating formula
                nextRoot = xMid + (xMid - xMin_) *
                            ((fxMin_ >= fxMax_ ? 1.0 : -1.0) * fxMid / s);
                if (QL_FABS(nextRoot-root_) <= xAccuracy)
                    return root_;

                root_=nextRoot;
                // Second of two function evaluations per iteration
                froot=f(root_);
                evaluationNumber_++;
                if (froot == 0.0)
                    return root_;

                // Bookkeeping to keep the root bracketed on next iteration
                if (SIGN(fxMid,froot) != fxMid) {
                    xMin_=xMid;
                    fxMin_=fxMid;
                    xMax_=root_;
                    fxMax_=froot;
                } else if (SIGN(fxMin_,froot) != fxMin_) {
                    xMax_=root_;
                    fxMax_=froot;
                } else if (SIGN(fxMax_,froot) != fxMax_) {
                    xMin_=root_;
                    fxMin_=froot;
                } else
                    throw Error("Ridder: never get here.");

                if (QL_FABS(xMax_-xMin_) <= xAccuracy) return root_;
            }
            throw Error("Ridder: maximum number of function evaluations (" +
             IntegerFormatter::toString(maxEvaluations_) + ") exceeded");

            QL_DUMMY_RETURN(0.0);
        }

    }

}
