/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2016 Peter Caspers

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

#include <ql/math/matrixutilities/choleskydecomposition.hpp>
#include <ql/math/comparison.hpp>

namespace QuantLib {

    const Disposable<Matrix> CholeskyDecomposition(const Matrix &S,
                                                   bool flexible) {
        Size i, j, size = S.rows();

        QL_REQUIRE(size == S.columns(),
                   "input matrix is not a square matrix");
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        for (i=0; i<S.rows(); i++)
            for (j=0; j<i; j++)
                QL_REQUIRE(S[i][j] == S[j][i],
                           "input matrix is not symmetric");
        #endif

        Matrix result(size, size, 0.0);
        Real sum;
        for (i=0; i<size; i++) {
            for (j=i; j<size; j++) {
                sum = S[i][j];
                for (Integer k=0; k<=Integer(i)-1; k++) {
                    sum -= result[i][k]*result[j][k];
                }
                if (i == j) {
                    QL_REQUIRE(flexible || sum > 0.0,
                               "input matrix is not positive definite");
                    // To handle positive semi-definite matrices take the
                    // square root of sum if positive, else zero.
                    result[i][i] = std::sqrt(std::max<Real>(sum, 0.0));
                } else {
                    // With positive semi-definite matrices is possible
                    // to have result[i][i]==0.0
                    // In this case sum happens to be zero as well
                    result[j][i] = close_enough(result[i][i], 0.0)
                                       ? 0.0
                                       : sum / result[i][i];
                }
            }
        }
        return result;
    }

}
