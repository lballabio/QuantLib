
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file cholesky.cpp
    \brief Cholesky decomposition
*/

#include <ql/Math/cholesky.hpp>

namespace QuantLib {

    Cholesky::Cholesky(const Matrix &S, bool flexible)
    : decomposition_(S.rows(), S.rows(), 0.0) {
        Size i, j, size = S.rows();
        // should check for simmetry too: the algorithm will only use
        // the upper triangular part of S anyway...
        QL_REQUIRE(size == S.columns(),
            "Cholesky::Cholesky : "
            "input matrix is not a square matrix");

        double sum;
        for (i=0; i<size; i++) {
            for (j=i; j<size; j++) {
                sum = S[i][j];
                for (int k=0; k<=int(i)-1; k++) {
                    sum -= decomposition_[i][k]*decomposition_[j][k];
                }
                if (i == j) {
                    QL_REQUIRE(flexible || sum > 0.0,
                        "Cholesky::Cholesky : "
                        "input matrix is not positive definite");
                    // To handle positive semi-definite matrices take the
                    // square root of sum if positive, else zero.
                    decomposition_[i][i] = QL_SQRT(QL_MAX(sum, 0.0));
                } else {
                    // With positive semi-definite matrices is possible
                    // to have decomposition_[i][i]==0.0
                    // In this case sum happens to be zero as well
                    decomposition_[j][i] =
                        (sum==0.0 ? 0.0 : sum/decomposition_[i][i]);
                }
            }
        }
    }

}
