/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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
/*! \file simulatedannealing.cpp
    \brief Simulated Annealing optimization method

    \fullpath
    ql/Optimization/%simulatedannealing.cpp
*/

#include "ql/Optimization/simulatedannealing.hpp"

namespace QuantLib {

    namespace Optimization {

        double SimulatedAnnealing::extrapolate(
            OptimizationProblem& P,
            Size iHighest,
            double& highestValue,
            double factor) {

            Size dimensions = values_.size() - 1;
            double factor1 = (1.0 - factor)/dimensions;
            double factor2 = factor1 - factor;
            Array test = sum_*factor1 - vertices_[iHighest]*factor2;
            double testValue = P.value(test);
            if (testValue < bestValue_) {
                best_ = test;
                bestValue_ = testValue;
            }
            double fluValue = testValue - fluctuation();
            if (fluValue < highestValue) {
                values_[iHighest] = testValue;
                highestValue = fluValue;
                sum_ += test - vertices_[iHighest];
                vertices_[iHighest] = test;
            }
            return fluValue;

        }
        void SimulatedAnnealing::minimize(OptimizationProblem& P) {
            bool EndCriteria = false;

            Array& X = x();
            Size n = X.size(), i;

            vertices_ = std::vector<Array>(n+1, X);
            for (i=0; i<n; i++) {
                vertices_[i+1][i] += lambda_;
            }
            values_ = Array(n+1, 0.0);
            for (i=0; i<=n; i++)
                values_[i] = P.value(vertices_[i]);

            best_ = X;
            bestValue_ = values_[0];

            do {
                sum_ = Array(n, 0.0);
				Size i;
                for (i=0; i<=n; i++)
                    sum_ += vertices_[i];

                //Determine best, worst and 2nd worst vertices
                Size iLowest = 0;
                Size iHighest = 1;
                double lowestValue = values_[0] + fluctuation();
                double nextHighestValue = lowestValue;
                double highestValue = values_[1] + fluctuation();
 
                if (lowestValue > highestValue) {
                    iHighest = 0;
                    iLowest = 1;
                    nextHighestValue = highestValue;
                    highestValue = lowestValue;
                    lowestValue = nextHighestValue;
                }
                for (i=2;i<=n; i++) {
                    double tempValue = values_[i] + fluctuation();
                    if (tempValue <= lowestValue) {
                        iLowest = i;
                        lowestValue = tempValue;
                    } 
                    if (tempValue > highestValue) {
                        nextHighestValue = highestValue;
                        iHighest = i;
                        highestValue = tempValue;
                    } else {
                        if (tempValue > nextHighestValue)
                            nextHighestValue = tempValue;
                    }
                }
                double rtol = 2.0*QL_FABS(highestValue - lowestValue)/
                    (QL_FABS(highestValue) + QL_FABS(lowestValue) + QL_EPSILON);
                if (rtol < tol_) {
                    X = vertices_[iLowest];
                    return;
                }

                double tryValue = extrapolate(P, iHighest, highestValue, -1.0);
                if (tryValue <= lowestValue) {
                    extrapolate(P, iHighest, highestValue, 2.0);
                } else {
                    if (tryValue >= nextHighestValue) {
                        double saveValue = highestValue;
                        tryValue = extrapolate(P, iHighest, highestValue, 0.5);
                        if (tryValue >= saveValue) {
                            for (Size i=0; i<=n; i++) {
                                if (i!=iLowest) {
                                    vertices_[i] = 
                                        0.5*(vertices_[i] + vertices_[iLowest]);
                                    values_[i] = P.value(vertices_[i]);
                                }
                            }
                        }

                    }
                }
                iterationNumber()++;
                if ((iterationNumber()%40)==0)
                    temperature_ *= 0.5;
            } while (EndCriteria == false);

        }

    }

}
