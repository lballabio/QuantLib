/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Ferdinando Ametrano

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

/*! \file finitedifferencenewtonsafe.hpp
    \brief Safe (bracketed) Newton 1-D solver with finite difference derivatives
*/

#ifndef quantlib_solver1d_finitedifferencenewtonsafe_h
#define quantlib_solver1d_finitedifferencenewtonsafe_h

#include <ql/math/solver1d.hpp>

namespace QuantLib {

    //! safe %Newton 1-D solver with finite difference derivatives
    /*!
        \test the correctness of the returned values is tested by
              checking them against known good results.

        \ingroup solvers
    */
    class FiniteDifferenceNewtonSafe : public Solver1D<FiniteDifferenceNewtonSafe> {
      public:
        template <class F>
        Real solveImpl(const F& f,
                       Real xAccuracy) const {

            QL_REQUIRE(evaluationNumber_ < maxEvaluations_,
                       "maximum number of function evaluations ("
                       << maxEvaluations_ << ") exceeded");

            // Orient the search so that f(xl) < 0
            Real xh, xl;
            if (fxMin_ < 0.0) {
                xl = xMin_;
                xh = xMax_;
            } else {
                xh = xMin_;
                xl = xMax_;
            }

            Real froot = f(root_);
            ++evaluationNumber_;
            if (froot == 0.0)
                return root_;
            // first order finite difference derivative
            Real dfroot = xMax_-root_ < root_-xMin_ ?
                (fxMax_-froot)/(xMax_-root_) :
                (fxMin_-froot)/(xMin_-root_) ;

            // xMax_-xMin_>0 is verified in the constructor
            Real dx = xMax_-xMin_;
            while (true) {
                if (froot < 0.0)
                    xl = root_;
                else
                    xh = root_;

                // The current point has already been evaluated.
                if (std::fabs(xh-xl) < xAccuracy)
                    return root_;
                if (evaluationNumber_ >= maxEvaluations_)
                    break;

                Real frootold = froot;
                Real rootold = root_;
                Real dxold = dx;
                // Bisect if (out of range || not decreasing fast enough)
                if ((((root_-xh)*dfroot-froot)*
                     ((root_-xl)*dfroot-froot) > 0.0)
                    || (std::fabs(2.0*froot) > std::fabs(dxold*dfroot))) {
                    dx = (xh-xl)/2.0;
                    root_ = xl+dx;
                    // if the root estimate just computed is close to the
                    // previous one, we should calculate dfroot at root and
                    // xh rather than root and rootold (xl instead of xh would
                    // be just as good)
                    if (close(root_, rootold, 2500)) {
                        rootold = xh;
                        frootold = f(xh);
                        ++evaluationNumber_;
                    }
                } else { // Newton
                    dx = froot/dfroot;
                    root_ -= dx;
                }

                if (evaluationNumber_ >= maxEvaluations_)
                    break;
                froot = f(root_);
                ++evaluationNumber_;
                if (froot == 0.0)
                    return root_;

                if (std::fabs(dx) < xAccuracy) {
                    Real other = froot < 0.0 ? Real(xh) : Real(xl);
                    if (std::fabs(other-root_) < xAccuracy)
                        return root_;

                    // A small step alone does not establish convergence.  Probe
                    // towards the opposite end to certify a bracket within accuracy.
                    if (evaluationNumber_ >= maxEvaluations_)
                        break;
                    // Leave some accuracy margin when returning the probe.
                    Real probeStep = 0.5*xAccuracy;
                    if (other < root_)
                        probeStep = -probeStep;
                    Real probe = root_ + probeStep;
                    if (probe == root_)
                        probe = root_ + 2.0*probeStep;
                    // Rounding must not make the certified interval too wide.
                    if (probe == root_ || std::fabs(probe-root_) > xAccuracy)
                        break;
                    Real fprobe = f(probe);
                    ++evaluationNumber_;
                    if (fprobe == 0.0) {
                        root_ = probe;
                        return root_;
                    }
                    bool bracketed = (froot < 0.0 && fprobe >= 0.0)
                                  || (froot > 0.0 && fprobe <= 0.0);
                    if (bracketed) {
                        // Prefer the certified endpoint with the smaller residual.
                        if (std::fabs(froot) < std::fabs(fprobe)
                            && evaluationNumber_ < maxEvaluations_) {
                            f(root_);
                            ++evaluationNumber_;
                            return root_;
                        }
                        // The function state already matches the probe.
                        root_ = probe;
                        return root_;
                    }
                    rootold = root_;
                    frootold = froot;
                    root_ = probe;
                    froot = fprobe;
                }
                dfroot = (frootold-froot)/(rootold-root_);
            }

            QL_FAIL("maximum number of function evaluations ("
                    << maxEvaluations_ << ") exceeded");
        }
    };

}

#endif
