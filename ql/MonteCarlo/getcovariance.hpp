
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file getcovariance.hpp
    \brief Covariance matrix calculation
*/

#ifndef quantlib_montecarlo_get_covariance_h
#define quantlib_montecarlo_get_covariance_h

#include <ql/Math/matrix.hpp>

namespace QuantLib {

    /*! Combines the correlation matrix and the vector of volatilities
        to return the covariance matrix.

        Note that only the symmetric part of the correlation matrix is
        used. Also it is assumed that the diagonal member of the
        correlation matrix equals one.

        \pre The correlation matrix must be symmetric with the diagonal
             members equal to one.
    */
    template<class DataIterator>
    Disposable<Matrix> getCovariance(DataIterator volBegin,
                                     DataIterator volEnd,
                                     const Matrix& corr){
        Size size = std::distance(volBegin, volEnd);
        QL_REQUIRE(corr.rows() == size,
                   "volatilities and correlations "
                   "have different size");
        QL_REQUIRE(corr.columns() == size,
                   "correlation matrix is not square");

        Matrix covariance(size,size);
        Size i, j;
        DataIterator iIt, jIt;
        for (i=0, iIt=volBegin; i<size; i++, iIt++){
            for (j=0, jIt=volBegin; j<i; j++, jIt++){
                QL_REQUIRE(QL_FABS(corr[i][j]-corr[j][i]) <= 1.0e-12,
                           "invalid correlation matrix:"
                           "\nc[" + SizeFormatter::toString(i) +
                           ", "   + SizeFormatter::toString(j) +
                           "] = " + DoubleFormatter::toString(corr[i][j]) +
                           "\nc[" + SizeFormatter::toString(j) +
                           ", "   + SizeFormatter::toString(i) +
                           "] = " + DoubleFormatter::toString(corr[j][i]));
                covariance[i][i] = (*iIt) * (*iIt);
                covariance[i][j] = (*iIt) * (*jIt) *
                    0.5 * (corr[i][j] + corr[j][i]);
                covariance[j][i] = covariance[i][j];
            }
            QL_REQUIRE(QL_FABS(corr[i][i]-1.0) <= 1.0e-12,
                       "invalid correlation matrix, diagonal element of the "
                       + SizeFormatter::toOrdinal(i) +
                       " row is "
                       + DoubleFormatter::toString(corr[i][i]) +
                       " instead of 1.0");
            covariance[i][i] = (*iIt) * (*iIt);
        }
        return covariance;
    }

}


#endif
