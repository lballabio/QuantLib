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

#include <ql/Optimization/steepestdescent.hpp>
#include <ql/Optimization/problem.hpp>
#include <ql/Optimization/linesearch.hpp>

namespace QuantLib {

    EndCriteria::Type SteepestDescent::minimize(Problem& P,
                                                const EndCriteria& endCriteria) {
        P.reset();
        Array x_ = P.currentValue();
        Integer iterationNumber_=0;
        lineSearch_->searchDirection() = Array(x_.size());

        bool end;

        // function and squared norm of gradient values;
        Real normdiff;
        // classical initial value for line-search step
        Real t = 1.0;

        // Set gold at the size of the optimization problem search direction
        Array gold(lineSearch_->searchDirection().size());
        Array gdiff(lineSearch_->searchDirection().size());

        P.setFunctionValue(P.valueAndGradient(gold, x_));
        lineSearch_->searchDirection() = -gold;
        P.setGradientNormValue(DotProduct(gold, gold));
        normdiff = std::sqrt(P.gradientNormValue());

        do {
            // Linesearch
            t = (*lineSearch_)(P, endCriteria, t);

            QL_REQUIRE(lineSearch_->succeed(), "line-search failed!");
            // End criteria
            end = endCriteria(iterationNumber_, P.functionValue(),
                              std::sqrt(P.gradientNormValue()),
                              lineSearch_->lastFunctionValue(),
                              std::sqrt(lineSearch_->lastGradientNorm2())
                              // FIXME: it's never been used!
                              //, normdiff
                              );

            // Updates
            // New point
            x_ = lineSearch_->lastX();
            // New function value
            P.setFunctionValue(lineSearch_->lastFunctionValue());
            // New gradient and search direction vectors
            gdiff = gold - lineSearch_->lastGradient();
            normdiff = std::sqrt(DotProduct (gdiff, gdiff));
            gold = lineSearch_->lastGradient();
            lineSearch_->searchDirection() = -gold;
            // New gradient squared norm
            P.setGradientNormValue(lineSearch_->lastGradientNorm2());

            // Increase interation number
            ++iterationNumber_;
        } while (end == false);

        P.setCurrentValue(x_);
        return endCriteria.type();

    }

}
