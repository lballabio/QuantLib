

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file solver1d.cpp
    \brief Abstract 1-D solver class

    \fullpath
    ql/%solver1d.cpp
*/

// $Id$

#include <ql/solver1d.hpp>

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
                    DoubleFormatter::toString(xMin_,10) + "," +
                    DoubleFormatter::toString(xMax_,10) + "] -> [" +
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





