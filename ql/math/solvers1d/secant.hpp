/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file secant.hpp
    \brief secant 1-D solver
*/

#ifndef quantlib_solver1d_secant_h
#define quantlib_solver1d_secant_h

#include <ql/math/solver1d.hpp>

namespace QuantLib {

    //! %Secant 1-D solver
    /*! The solver returns only with a certificate: either f is zero at
        the returned point, or the root is bracketed within the required
        accuracy by the returned point and an evaluated point of opposite
        sign.  A step shorter than the accuracy that does not provide
        such a bracket triggers a probe towards the root; if the
        certificate cannot be obtained, the solver fails through the
        evaluation limit.  The last function evaluation is always made at
        the returned point.

        \test the correctness of the returned values is tested by
              checking them against known good results.

        \ingroup solvers
    */
    class Secant : public Solver1D<Secant> {
      public:
        template <class F>
        Real solveImpl(const F& f,
                       Real xAccuracy) const {

            /* The implementation of the algorithm was inspired by
               Press, Teukolsky, Vetterling, and Flannery,
               "Numerical Recipes in C", 2nd edition,
               Cambridge University Press
            */

            Real fl, froot, dx, xl;

            // Pick the bound with the smaller function value
            // as the most recent guess
            if (std::fabs(fxMin_) < std::fabs(fxMax_)) {
                root_ = xMin_;
                froot = fxMin_;
                xl = xMax_;
                fl = fxMax_;
            } else {
                root_ = xMax_;
                froot = fxMax_;
                xl = xMin_;
                fl = fxMin_;
            }
            // the last evaluated point where f has the opposite sign
            // of f(root_); the bounds bracket the root, so xl qualifies
            Real xo = xl;

            while (evaluationNumber_ < maxEvaluations_) {
                // equal values give no slope (the step would be infinite
                // or NaN); fail through the evaluation-limit diagnostic
                if (froot == fl)
                    break;
                dx = (xl-root_)*froot/(froot-fl);
                xl = root_;
                fl = froot;
                root_ += dx;
                froot = f(root_);
                ++evaluationNumber_;
                if (froot == 0.0)
                    return root_;
                if ((fl < 0.0) != (froot < 0.0))
                    xo = xl;
                // the root lies between xo and root_
                if (std::fabs(xo-root_) < xAccuracy)
                    return root_;

                if (std::fabs(dx) < xAccuracy) {
                    // A small step alone does not establish convergence.
                    // Probe towards xo to certify a bracket within accuracy.
                    if (evaluationNumber_ >= maxEvaluations_)
                        break;
                    Real probeStep = 0.5*xAccuracy;
                    if (xo < root_)
                        probeStep = -probeStep;
                    Real probe = root_ + probeStep;
                    if (probe == root_)
                        probe = root_ + 2.0*probeStep;
                    // rounding must not make the certified interval too wide
                    if (probe == root_ || std::fabs(probe-root_) > xAccuracy)
                        break;
                    Real fprobe = f(probe);
                    ++evaluationNumber_;
                    if (fprobe == 0.0) {
                        root_ = probe;
                        return root_;
                    }
                    if ((froot < 0.0) != (fprobe < 0.0)) {
                        // both ends are certified; prefer the smaller
                        // residual, re-evaluating root_ so that the last
                        // call is made at the returned point
                        if (std::fabs(froot) < std::fabs(fprobe)
                            && evaluationNumber_ < maxEvaluations_) {
                            f(root_);
                            ++evaluationNumber_;
                            return root_;
                        }
                        root_ = probe;
                        return root_;
                    }
                    // the probe becomes the next iterate
                    xl = root_;
                    fl = froot;
                    root_ = probe;
                    froot = fprobe;
                }
            }
            QL_FAIL("maximum number of function evaluations ("
                    << maxEvaluations_ << ") exceeded");
        }
    };

}

#endif
