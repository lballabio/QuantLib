
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file ridder.hpp
    \brief Ridder 1-D solver
*/

#ifndef quantlib_solver1d_ridder_h
#define quantlib_solver1d_ridder_h

#include <ql/solver1d.hpp>

namespace QuantLib {

    //! %Ridder 1-D solver
    class Ridder : public Solver1D<Ridder> {
      public:
        template <class F>
        double solveImpl(const F& f, double xAcc) const {

            /* The implementation of the algorithm was inspired by
               Press, Teukolsky, Vetterling, and Flannery,
               "Numerical Recipes in C", 2nd edition, 
               Cambridge University Press
            */

            double fxMid, froot, s, xMid, nextRoot;

            // test on Black-Scholes implied volatility show that
            // Ridder solver algorithm actually provides an
            // accuracy 100 times below promised
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
                if (sign(fxMid,froot) != fxMid) {
                    xMin_=xMid;
                    fxMin_=fxMid;
                    xMax_=root_;
                    fxMax_=froot;
                } else if (sign(fxMin_,froot) != fxMin_) {
                    xMax_=root_;
                    fxMax_=froot;
                } else if (sign(fxMax_,froot) != fxMax_) {
                    xMin_=root_;
                    fxMin_=froot;
                } else {
                    QL_FAIL("Ridder: never get here.");
                }

                if (QL_FABS(xMax_-xMin_) <= xAccuracy) return root_;
            }

            QL_FAIL("Ridder::solveImpl: "
                    "maximum number of function evaluations (" +
                    SizeFormatter::toString(maxEvaluations_) + 
                    ") exceeded");

            QL_DUMMY_RETURN(0.0);
        }
      private:
        double sign(double a, double b) const {
            return b >= 0.0 ? QL_FABS(a) : -QL_FABS(a);
        }
    };

}


#endif
