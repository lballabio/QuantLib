/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

 /*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 Francois Du Vignaud
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/optimization/conjugategradient.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/linesearch.hpp>

namespace QuantLib {

    EndCriteria::Type ConjugateGradient::minimize(Problem &P,
                                                  const EndCriteria& endCriteria) {
        // Initializations
        Real ftol = endCriteria.functionEpsilon();
        Size maxStationaryStateIterations_
            = endCriteria.maxStationaryStateIterations();
        EndCriteria::Type ecType = EndCriteria::None;       // reset end criteria
        P.reset();                                          // reset problem
        Array x_ = P.currentValue();                        // store the starting point
        Size iterationNumber_=0 /*, stationaryStateIterationNumber_=0 */;
        lineSearch_->searchDirection() = Array(x_.size());  // dimension line search
        bool done = false;

        // function and squared norm of gradient values;
        Real fnew, fold, gold2;
        Real c;
        Real fdiff, normdiff;
        // classical initial value for line-search step
        Real t = 1.0;
        // Set gradient g at the size of the optimization problem search direction
        Size sz = lineSearch_->searchDirection().size();
        Array g(sz), d(sz), sddiff(sz);
        // Initialize cost function, gradient g and search direction
        P.setFunctionValue(P.valueAndGradient(g, x_));
        P.setGradientNormValue(DotProduct(g, g));
        lineSearch_->searchDirection() = -g;
        // Loop over iterations
        do {
            // Linesearch
            t = (*lineSearch_)(P, ecType, endCriteria, t);
            // don't throw: it can fail just because maxIterations exceeded
            //QL_REQUIRE(lineSearch_->succeed(), "line-search failed!");
            if (lineSearch_->succeed())
            {
                // Updates
                d = lineSearch_->searchDirection();
                // New point
                x_ = lineSearch_->lastX();
                // New function value
                fold = P.functionValue();
                P.setFunctionValue(lineSearch_->lastFunctionValue());
                // New gradient and search direction vectors
                g = lineSearch_->lastGradient();
                // orthogonalization coef
                gold2 = P.gradientNormValue();
                P.setGradientNormValue(lineSearch_->lastGradientNorm2());
                c = P.gradientNormValue() / gold2;
                // conjugate gradient search direction
                sddiff = (-g + c * d) - lineSearch_->searchDirection();
                normdiff = std::sqrt(DotProduct(sddiff, sddiff));
                lineSearch_->searchDirection() = -g + c * d;
                // Now compute accuracy and check end criteria
                // Numerical Recipes exit strategy on fx (see NR in C++, p.423)
                fnew = P.functionValue();
                fdiff = 2.0*std::fabs(fnew-fold) /
                        (std::fabs(fnew) + std::fabs(fold) + QL_EPSILON);
                if (fdiff < ftol ||
                    endCriteria.checkMaxIterations(iterationNumber_, ecType)) {
                    endCriteria.checkStationaryFunctionValue(0.0, 0.0,
                        maxStationaryStateIterations_, ecType);
                    endCriteria.checkMaxIterations(iterationNumber_, ecType);
                    return ecType;
                }
                //done = endCriteria(iterationNumber_,
                //                   stationaryStateIterationNumber_,
                //                   true,  //FIXME: it should be in the problem
                //                   fold,
                //                   std::sqrt(gold2),
                //                   P.functionValue(),
                //                   std::sqrt(P.gradientNormValue()),
                //                   ecType);
                P.setCurrentValue(x_);      // update problem current value
                ++iterationNumber_;         // Increase iteration number
            } else {
                done=true;
            }
        } while (!done);
        P.setCurrentValue(x_);
        return ecType;
    }

}
