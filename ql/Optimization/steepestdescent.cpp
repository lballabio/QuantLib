
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
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
