
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file ridder.cpp
    \brief Ridder 1-D solver
*/

/* The implementation of the algorithm was inspired by
   Press, Teukolsky, Vetterling, and Flannery,
   "Numerical Recipes in C", 2nd edition, 
   Cambridge University Press
*/


#include <ql/Solvers1D/ridder.hpp>

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
