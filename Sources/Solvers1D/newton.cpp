
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

/*! \file newton.cpp
    \brief Newton 1-D solver

    $Source$
    $Log$
    Revision 1.24  2001/05/24 13:57:52  nando
    smoothing #include xx.hpp and cutting old Log messages

*/

/* The implementation of the algorithm was inspired by
 * "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
 * Chapter 9
 */


#include "ql/Solvers1D/newton.hpp"
#include "ql/Solvers1D/newtonsafe.hpp"

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
