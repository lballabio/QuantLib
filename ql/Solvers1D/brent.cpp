
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

/*! \file brent.cpp
    \brief Brent 1-D solver
*/

// $Id$

/* The implementation of the algorithm was inspired by
   Press, Teukolsky, Vetterling, and Flannery,
   "Numerical Recipes in C", 2nd edition, 
   Cambridge University Press
*/


#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    namespace Solvers1D {

        #define SIGN(a,b) ((b) >= 0.0 ? QL_FABS(a) : -QL_FABS(a))

        double Brent::solve_(const ObjectiveFunction& f,
                             double xAccuracy) const {
            double min1, min2;
            double froot, p, q, r, s, xAcc1, xMid;
            // dummy assignements to avoid compiler warning
            double d = 0.0;
            double e = 0.0;

            root_ = xMax_;
            froot = fxMax_;
            while (evaluationNumber_<=maxEvaluations_) {
                if ((froot > 0.0 && fxMax_ > 0.0) ||
                                        (froot < 0.0 && fxMax_ < 0.0)) {
                // Rename xMin_, root_, xMax_ and adjust bounding interval d
                    xMax_=xMin_;
                    fxMax_=fxMin_;
                    e=d=root_-xMin_;
                }
                if (QL_FABS(fxMax_) < QL_FABS(froot)) {
                    xMin_=root_;
                    root_=xMax_;
                    xMax_=xMin_;
                    fxMin_=froot;
                    froot=fxMax_;
                    fxMax_=fxMin_;
                }
                // Convergence check
                xAcc1=2.0*QL_EPSILON*QL_FABS(root_)+0.5*xAccuracy;
                xMid=(xMax_-root_)/2.0;
                if (QL_FABS(xMid) <= xAcc1 || froot == 0.0)
                    return root_;
                if (QL_FABS(e) >= xAcc1 && QL_FABS(fxMin_) > QL_FABS(froot)) {
                    s=froot/fxMin_;  // Attempt inverse quadratic interpolation
                    if (xMin_ == xMax_) {
                        p=2.0*xMid*s;
                        q=1.0-s;
                    } else {
                            q=fxMin_/fxMax_;
                            r=froot/fxMax_;
                            p=s*(2.0*xMid*q*(q-r)-(root_-xMin_)*(r-1.0));
                            q=(q-1.0)*(r-1.0)*(s-1.0);
                    }
                    if (p > 0.0) q = -q;  // Check whether in bounds
                    p=QL_FABS(p);
                    min1=3.0*xMid*q-QL_FABS(xAcc1*q);
                    min2=QL_FABS(e*q);
                    if (2.0*p < (min1 < min2 ? min1 : min2)) {
                        e=d;                // Accept interpolation
                        d=p/q;
                    } else {
                        d=xMid;  // Interpolation failed, use bisection
                        e=d;
                    }
                } else {         // Bounds decreasing too slowly, use bisection
                    d=xMid;
                    e=d;
                }
                xMin_=root_;
                fxMin_=froot;
                if (QL_FABS(d) > xAcc1)
                    root_ += d;
                else
                    root_ += SIGN(xAcc1,xMid);
                froot=f(root_);
                evaluationNumber_++;
            }
            throw Error("Brent: maximum number of function evaluations ("
            + IntegerFormatter::toString(maxEvaluations_) + ") exceeded");
            QL_DUMMY_RETURN(0.0);
        }

    }

}
