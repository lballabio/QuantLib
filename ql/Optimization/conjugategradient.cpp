
/*
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

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

#include <ql/Optimization/conjugategradient.hpp>

namespace QuantLib {

    void ConjugateGradient::minimize(const Problem &P) const {
        bool end;

        // function and squared norm of gradient values;
        Real fold, gold2;
        Real c;
        Real normdiff;
        // classical initial value for line-search step
        Real t = 1.0;

        // reference X as the optimization problem variable
        Array& X = x();
        Array& SearchDirection = searchDirection();
        // Set g at the size of the optimization problem search direction
        Size sz = searchDirection().size();
        Array g(sz), d(sz), sddiff(sz);

        functionValue() = P.valueAndGradient(g, X);
        SearchDirection = -g;
        gradientNormValue() = DotProduct(g, g);

        do {
            // Linesearch
            t = (*lineSearch_)(P, t);
            QL_REQUIRE(lineSearch_->succeed(), "line-search failed!");

            // Updates
            d = SearchDirection;
            // New point
            X = lineSearch_->lastX();
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
            sddiff = (-g + c * d) - SearchDirection;
            normdiff = std::sqrt(DotProduct(sddiff, sddiff));
            SearchDirection = -g + c * d;
            // End criteria
            end = endCriteria()(iterationNumber_,
                                fold, std::sqrt(gold2), functionValue(),
                                std::sqrt(gradientNormValue()), normdiff);

            // Increase interation number
            iterationNumber()++;
        } while (end == false);

    }

}
