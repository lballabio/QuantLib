
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

/*! \file solver1d.cpp
    \brief Abstract 1-D solver class

    $Source$
    $Log$
    Revision 1.21  2001/05/16 09:57:27  lballabio
    Added indexes and piecewise flat forward curve

    Revision 1.20  2001/05/09 11:06:19  nando
    A few comments modified/removed

    Revision 1.19  2001/04/09 14:13:33  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.18  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.17  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

*/

#include "ql/solver1d.hpp"

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
                // check whether we really want to pass epsilon
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





