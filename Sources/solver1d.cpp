
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

/*! \file solver1d.cpp
    \brief Abstract 1-D solver class

    $Source$
    $Log$
    Revision 1.16  2001/04/02 10:59:48  lballabio
    Changed ObjectiveFunction::value to ObjectiveFunction::operator() - also in Python module

    Revision 1.15  2001/03/15 15:31:26  aleppo
    Minimum at boudaries compatible with accuracy required.

    Revision 1.14  2001/03/02 14:01:36  aleppo
    Bug Fixed

    Revision 1.13  2001/02/16 07:57:41  marmar
    The condition "fxMax_ == 0.0" became "if (QL_FABS(fxMax_) <= xAccuracy",
    and other minor changes

    Revision 1.12  2001/01/17 13:53:47  nando
    80 columns enforced
    tabs removed
    private data member now have trailing underscore

    Revision 1.11  2000/12/27 14:53:50  lballabio
    using QL_EPSILON macro

    Revision 1.10  2000/12/27 14:05:57  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

    Revision 1.9  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#include "solver1d.h"

namespace QuantLib {

    const double growthFactor = 1.6;

    double Solver1D::solve(const ObjectiveFunction& f,
                           double xAccuracy,
                           double guess,
                           double step) const {

        int flipflop = -1;

        root_ = guess;
        fxMax_ = f(root_);

        // monotonically crescent bias, as in optionValue(volatility)
        if (QL_FABS(fxMax_) <= xAccuracy)
            return root_;
        else if (fxMax_ > 0.0) {
            xMin_ = enforceBounds_(root_ - step);
            fxMin_ = f(xMin_);
            xMax_ = root_;
        } else {
            xMin_ = root_;
            fxMin_ = fxMax_;
            xMax_ = enforceBounds_(root_+step);
            fxMax_ = f(xMax_);
        }

        evaluationNumber_ = 2;
        while (evaluationNumber_ <= maxEvaluations_) {
            if (fxMin_*fxMax_ <= 0.0) {
                if (fxMin_ == 0.0)    return xMin_;
                if (fxMax_ == 0.0)    return xMax_;
                root_ = (xMax_+xMin_)/2.0;
                return solve_(f, QL_MAX(QL_FABS(xAccuracy), QL_EPSILON));
            }
            if (QL_FABS(fxMin_) < QL_FABS(fxMax_)) {
                xMin_ = enforceBounds_(xMin_+growthFactor*(xMin_ - xMax_));
                fxMin_= f(xMin_);
            } else if (QL_FABS(fxMin_) > QL_FABS(fxMax_)) {
                xMax_ = enforceBounds_(xMax_+growthFactor*(xMax_ - xMin_));
                fxMax_= f(xMax_);
            } else if (flipflop == -1) {
                xMin_ = enforceBounds_(xMin_+growthFactor*(xMin_ - xMax_));
                fxMin_= f(xMin_);
                evaluationNumber_++;
                flipflop = 1;
            } else if (flipflop == 1) {
                xMax_ = enforceBounds_(xMax_+growthFactor*(xMax_ - xMin_));
                fxMax_= f(xMax_);
                flipflop = -1;
            }
            evaluationNumber_++;
        }

        throw Error("unable to bracket root in " +
                    IntegerFormatter::toString(maxEvaluations_) +
                    " function evaluations (last bracket attempt: f[" +
                    DoubleFormatter::toString(xMin_) +
                    "," + DoubleFormatter::toString(xMax_) + "] -> [" +
                    DoubleFormatter::toString(fxMin_) + "," +
                    DoubleFormatter::toString(fxMax_) + "])");
    }



    double Solver1D::solve(const ObjectiveFunction& f,
                           double xAccuracy,
                           double guess,
                           double xMin,
                           double xMax) const {

        xMin_ = xMin;
        xMax_ = xMax;
        QL_REQUIRE(xMin_ < xMax_, "invalid range: xMin_ (" +
                DoubleFormatter::toString(xMin_) +
                ") >= xMax_ (" + DoubleFormatter::toString(xMax_) + ")");

        QL_REQUIRE(!lowBoundEnforced_ || xMin_ >= lowBound_, "xMin_ (" +
                DoubleFormatter::toString(xMin_) + ") < enforced low bound (" +
                DoubleFormatter::toString(lowBound_) + ")");

        QL_REQUIRE(!hiBoundEnforced_ || xMax_ <= hiBound_, "xMax_ (" +
                DoubleFormatter::toString(xMax_) +
                ") > enforced hi bound (" +
                DoubleFormatter::toString(hiBound_) + ")");

        fxMin_ = f(xMin_);
        if (QL_FABS(fxMin_) < xAccuracy)
            return xMin_;

        fxMax_ = f(xMax_);
        if (QL_FABS(fxMax_) < xAccuracy)
            return xMax_;

        evaluationNumber_ = 2;

        QL_REQUIRE((fxMin_*fxMax_ < 0.0),  "root not bracketed: f[" +
                    DoubleFormatter::toString(xMin_) + "," +
                    DoubleFormatter::toString(xMax_) + "] -> [" +
                    DoubleFormatter::toString(fxMin_,20) + "," +
                    DoubleFormatter::toString(fxMax_,20) + "]");


        QL_REQUIRE(guess > xMin_, "Solver1D: guess (" +
                    DoubleFormatter::toString(guess) + ") < xMin_ (" +
                    DoubleFormatter::toString(xMin_) + ")");

        QL_REQUIRE(guess < xMax_, "Solver1D: guess (" +
                    DoubleFormatter::toString(guess) + ") > xMax_ (" +
                    DoubleFormatter::toString(xMax_) + ")");

        root_ = guess;

        return solve_(f, QL_MAX(QL_FABS(xAccuracy), QL_EPSILON));
    }

}





