/*
 Copyright (C) 2001, 2002 Nicolas Di Césaré

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
/*! \file powell.cpp
    \brief Powell optimization method

    \fullpath
    ql/Optimization/%powell.cpp
*/

#include "ql/Optimization/powell.hpp"

namespace QuantLib {

    namespace Optimization {

        void Powell::minimize(OptimizationProblem& P) {
            bool EndCriteria = false;

            // reference X as the optimization problem variable
            Array& X = x();
            Size n = X.size();
            double normdiff;
            // Set gold at the size of the optimization problem search direction
            Array gold(searchDirection().size());
            Array gdiff(searchDirection().size());
            functionValue() = P.valueAndGradient(gold, X);
            searchDirection() = -gold;
            gradientNormValue() = DotProduct(gold, gold);
            normdiff = QL_SQRT(gradientNormValue());


            std::vector<Array> direction(n, Array(n, 0.0));
            // initializing set of directions to eigenvectors
            for (Size i=0; i<n; i++)
                direction[i][i] = 1.0;

            Array distance(X);
            Array values(n+2);        // OF evaluations
            double residue = 0.0;

            Array initial(X);
            // beginning of the Powell's method
            do {
                Array model_init(n), model_end(n);// input to line search
                values[0] = P.value(initial);

                for (Size i=1; i<=n; i++) {
                    searchDirection() = direction[i-1];
                    (*lineSearch_)(P, 1.0);
                    if (!lineSearch_->succeed())
                        throw Error("Powell::Minimize(), line-search failed!");
                    EndCriteria =
                        endCriteria()(iterationNumber_, functionValue(), 
                            QL_SQRT(gradientNormValue()),
                            lineSearch_->lastFunctionValue(),
                            QL_SQRT(lineSearch_->lastGradientNorm2()),
                            normdiff);

                    // Updates
                    // New point
                    X = lineSearch_->lastX();
                    // New function value
                    functionValue() = lineSearch_->lastFunctionValue();
                    // New gradient and search direction vectors
                    gdiff = gold - lineSearch_->lastGradient();
                    normdiff = QL_SQRT(DotProduct (gdiff, gdiff));
                    gold = lineSearch_->lastGradient();
                    searchDirection() = -gold;
                    // New gradient squared norm
                    gradientNormValue() = lineSearch_->lastGradientNorm2();

                    // Increase interation number
                    iterationNumber()++;

                    values[i] = functionValue();
                }

                // model stores the last outcome of the line search
                model_end = 2.0*X - initial;
                values[n+1] = P.value(model_end);

                // computing differences in the OF evaluations
                Size index_max = 0;
                double deltak = -QL_MIN_DOUBLE;
                Array diffValues(n);
                for (Size i=0; i<n; i++) {
                    diffValues[i] = values[i] - values[i+1];
                    if (diffValues[i]>deltak) {
                        index_max = i;
                        deltak = diffValues[i];
                    }
                }

                // decide on new direction of search
                if (values[n+1] >= values[0] ||
                    ((values[0] - 2.0*values[n] + values[n+1]) *
                     (values[0] - values[n] - deltak) *
                     (values[0] - values[n] - deltak) >=
                     0.5*deltak*(values[0] - values[n+1])*
                     (values[0] - values[n+1]))) {        
                    // keep all search directions

                    // deciding the new starting point
                    if (values[n] >= values[n+1])
                        initial = model_end;
                    else
                        initial = X;
                } else {                
                    // just one search direction will be replaced
                
                    searchDirection() = X - initial;
                    (*lineSearch_)(P, 1.0);
                    if (!lineSearch_->succeed())
                        throw Error("Powell::Minimize(), line-search failed!");
                    EndCriteria = endCriteria()(
                        iterationNumber_, functionValue(), 
                        QL_SQRT(gradientNormValue()),
                        lineSearch_->lastFunctionValue(), 
                        QL_SQRT(lineSearch_->lastGradientNorm2()), normdiff);

                    // Updates
                    // New point
                    X = lineSearch_->lastX();
                    // New function value
                    functionValue() = lineSearch_->lastFunctionValue();
                    // New gradient and search direction vectors
                    gdiff = gold - lineSearch_->lastGradient();
                    normdiff = QL_SQRT(DotProduct (gdiff, gdiff));
                    gold = lineSearch_->lastGradient();
                    searchDirection() = -gold;
                    // New gradient squared norm
                    gradientNormValue() = lineSearch_->lastGradientNorm2();

                    // Increase interation number
                    iterationNumber()++;

                    // and replacing the row index_max in matrix directions
                    for (Size i = index_max + 1; i < n; i++)
                        direction[i-1] = direction[i];

                    // and adding the new search direction
                    direction[n-1] = searchDirection();
                }

                // checking the convergence of the method
                distance -= initial;
                double err = DotProduct(distance, distance);
                double temperr = DotProduct(initial, initial);
                residue += err/temperr;

            } while (EndCriteria == false);

        }

    }

}
