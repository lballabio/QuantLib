

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
/*! \file steepestdescent.cpp
    \brief Steepest descent optimization method

    \fullpath
    ql/Optimization/%steepestdescent.cpp
*/

#include "ql/Optimization/steepestdescent.hpp"

namespace QuantLib {

    namespace Optimization {

        void SteepestDescent::Minimize(OptimizationProblem& P) {
            bool EndCriteria = false;

            // function and squared norm of gradient values;
            double fold, gold2, normdiff;
            // classical initial value for line-search step
            double t = 1.0;

            // reference X as the optimization problem variable
            Array& X = x();
            // Set gold at the size of the optimization problem search direction
            Array gold(searchDirection().size());
            Array gdiff(searchDirection().size());

            fold = P.valueAndGradient(gold, X);
            searchDirection() = -gold;
            gold2 = DotProduct(gold, gold);
            normdiff = QL_SQRT(gold2);

            do {
                // Linesearch
                t = (*lineSearch_)(P, t, fold, gold2);

                if (lineSearch_->succeed()) {
                    // End criteria
                    EndCriteria =
                        endCriteria()(iterationNumber_, fold, QL_SQRT(gold2),
                             lineSearch_->lastFunctionValue(),
                             QL_SQRT(lineSearch_->lastGradientNorm2()),
                             normdiff);

                    // Updates
                    // New point
                    X = lineSearch_->lastX();
                    // New function value
                    fold = lineSearch_->lastFunctionValue();
                    // New gradient and search direction vectors
                    gdiff = gold - lineSearch_->lastGradient();
                    normdiff = QL_SQRT(DotProduct (gdiff, gdiff));
                    gold = lineSearch_->lastGradient();
                    searchDirection() = -gold;
                    // New gradient squared norm
                    gold2 = lineSearch_->lastGradientNorm2();

                    // Increase interation number
                    iterationNumber()++;
                }
            } while ((EndCriteria == false) && (lineSearch_->succeed()));

            if (!lineSearch_->succeed())
                throw Error("SteepestDescent::Minimize(), line-search failed!");
        }

    }

}
