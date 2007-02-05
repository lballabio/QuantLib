/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

 /*
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Optimization/conjugategradient.hpp>
#include <ql/Optimization/problem.hpp>
#include <ql/Optimization/linesearch.hpp>

namespace QuantLib {

    void ConjugateGradient::minimize(const Problem &P,
                                     const EndCriteria& endCriteria) {
        bool done = false;

        // function and squared norm of gradient values;
        Real fold, gold2;
        Real c;
        Real normdiff;
        // classical initial value for line-search step
        Real t = 1.0;

        // Set g at the size of the optimization problem search direction
        Size sz = lineSearch_->searchDirection().size();
        Array g(sz), d(sz), sddiff(sz);

        functionValue() = P.valueAndGradient(g, x_);
        lineSearch_->searchDirection() = -g;
        gradientNormValue() = DotProduct(g, g);

        do {
            // Linesearch
            t = (*lineSearch_)(P, endCriteria, t);
            // don't throw: it can fail just because maxIterations exceeded
            //QL_REQUIRE(lineSearch_->succeed(), "line-search failed!");
            if (lineSearch_->succeed())
            {
                // Updates
                d = lineSearch_->searchDirection();
                // New point
                x_ = lineSearch_->lastX();
                // New function value
                fold = functionValue();
                functionValue() = lineSearch_->lastFunctionValue();
                // New gradient and search direction vectors
                g = lineSearch_->lastGradient();
                // orthogonalization coef
                gold2 = gradientNormValue();
                gradientNormValue() = lineSearch_->lastGradientNorm2();
                c = gradientNormValue() / gold2;
                // conjugate gradient search direction
                sddiff = (-g + c * d) - lineSearch_->searchDirection();
                normdiff = std::sqrt(DotProduct(sddiff, sddiff));
                lineSearch_->searchDirection() = -g + c * d;
                
                // End criteria
                done = endCriteria(iterationNumber_,
                                   fold, std::sqrt(gold2), functionValue(),
                                   std::sqrt(gradientNormValue()), normdiff);
			    // Increase interation number
                ++iterationNumber_;
            } else {
                done=true;
            }
        } while (!done);

	}

}
