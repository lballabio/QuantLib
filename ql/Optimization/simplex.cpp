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
/*! \file simplex.cpp
    \brief Simplex optimization method

    \fullpath
    ql/Optimization/%simplex.cpp
*/

#include "ql/Optimization/simplex.hpp"

namespace QuantLib {

    namespace Optimization {

        double Simplex::extrapolate(
            OptimizationProblem& P,
            Size iHighest,
            double factor) {

            Size dimensions = values_.size() - 1;
            double factor1 = (1.0 - factor)/dimensions;
            double factor2 = factor1 - factor;
            Array pTry = sum_*factor1 - vertices_[iHighest]*factor2;
            double vTry = P.value(pTry);
            if (vTry < values_[iHighest]) {
               values_[iHighest] = vTry;
               sum_ += pTry - vertices_[iHighest];
               vertices_[iHighest] = pTry;
            }
            return vTry;

        }
        void Simplex::minimize(OptimizationProblem& P) {
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


            do {
                sum_ = Array(n, 0.0);
				Size i;
                for (i=0; i<=n; i++)
                    sum_ += vertices_[i];

                //Determine best, worst and 2nd worst vertices
                Size iLowest = 0;
                Size iHighest, iNextHighest;
                if (values_[0]<values_[1]) {
                    iHighest = 1;
                    iNextHighest = 0;
                } else {
                    iHighest = 0;
                    iNextHighest = 1;
                }
                for (i=1;i<=n; i++) {
                    if (values_[i]>values_[iHighest]) {
                        iNextHighest = iHighest;
                        iHighest = i;
                    } else {
                        if ((values_[i]>values_[iNextHighest]) && i!=iHighest)
                            iNextHighest = i;
                    }
                    if (values_[i]<values_[iLowest])
                        iLowest = i;
                }
                double low = values_[iLowest], high = values_[iHighest];

                double rtol = 2.0*QL_FABS(high - low)/
                    (QL_FABS(high) + QL_FABS(low) + QL_EPSILON);
                if (rtol < tol_) {
                    X = vertices_[iLowest];
                    return;
                }

                double vTry = extrapolate(P, iHighest, -1.0);
                if (vTry <= values_[iLowest]) {
                    extrapolate(P, iHighest, 2.0);
                } else {
                    if (vTry >= values_[iNextHighest]) {
                        double vSave = values_[iHighest];
                        vTry = extrapolate(P, iHighest, 0.5);
                        if (vTry >= vSave) {
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

            } while (EndCriteria == false);

        }

    }

}
