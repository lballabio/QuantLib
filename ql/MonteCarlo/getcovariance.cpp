
/*
 Copyright (C) 2004 Ferdinando Ametrano

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


#include <ql/MonteCarlo/getcovariance.hpp>

namespace QuantLib {

    CovarianceDecomposition::CovarianceDecomposition(const Matrix& cov,
                                                     Real tolerance)
    : variances_(Array(cov.rows())), standardDeviations_(Array(cov.rows())),
      correlationMatrix_(Matrix(cov.rows(), cov.rows())) {

        Size size = cov.rows();
        QL_REQUIRE(size==cov.columns(),
                   "input covariance matrix must be square");

        Size i, j;
        for (i=0; i<size; i++) {
            variances_[i]=cov[i][i];
            standardDeviations_[i] = QL_SQRT(cov[i][i]);
            correlationMatrix_[i][i] = 1.0;
            for (j=0; j<i; j++){
                QL_REQUIRE(QL_FABS(cov[i][j]-cov[j][i]) <= tolerance,
                           "invalid covariance matrix:"
                           "\nc[" + SizeFormatter::toString(i) +
                           ", "   + SizeFormatter::toString(j) +
                           "] = " + DecimalFormatter::toString(cov[i][j]) +
                           "\nc[" + SizeFormatter::toString(j) +
                           ", "   + SizeFormatter::toString(i) +
                           "] = " + DecimalFormatter::toString(cov[j][i]));
                correlationMatrix_[i][j] = cov[i][j]/
                    (standardDeviations_[i]*standardDeviations_[j]);
                correlationMatrix_[j][i] = correlationMatrix_[i][j];
            }
        }
    }
}
