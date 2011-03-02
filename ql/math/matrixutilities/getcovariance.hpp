/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2009 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file getcovariance.hpp
    \brief Covariance matrix calculation
*/

#ifndef quantlib_montecarlo_get_covariance_h
#define quantlib_montecarlo_get_covariance_h

#include <ql/math/matrix.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>

namespace QuantLib {

    //! Calculation of covariance from correlation and standard deviations
    /*! Combines the correlation matrix and the vector of standard deviations
        to return the covariance matrix.

        Note that only the symmetric part of the correlation matrix is
        used. Also it is assumed that the diagonal member of the
        correlation matrix equals one.

        \pre The correlation matrix must be symmetric with the diagonal
             members equal to one.

        \test tested on know values and cross checked with
              CovarianceDecomposition
    */
    template<class DataIterator>
    Disposable<Matrix> getCovariance(DataIterator stdDevBegin,
                                     DataIterator stdDevEnd,
                                     const Matrix& corr,
                                     Real tolerance = 1.0e-12){
        Size size = std::distance(stdDevBegin, stdDevEnd);
        QL_REQUIRE(corr.rows() == size,
                   "dimension mismatch between volatilities (" << size <<
                   ") and correlation rows (" << corr.rows() << ")");
        QL_REQUIRE(corr.columns() == size,
                   "correlation matrix is not square: " << size <<
                   " rows and " << corr.columns() << " columns");

        Matrix covariance(size,size);
        Size i, j;
        DataIterator iIt, jIt;
        for (i=0, iIt=stdDevBegin; i<size; ++i, ++iIt){
            for (j=0, jIt=stdDevBegin; j<i; ++j, ++jIt){
                QL_REQUIRE(std::fabs(corr[i][j]-corr[j][i]) <= tolerance,
                           "correlation matrix not symmetric:"
                           << "\nc[" << i << "," << j << "] = " << corr[i][j]
                           << "\nc[" << j << "," << i << "] = " << corr[j][i]);
                covariance[i][i] = (*iIt) * (*iIt);
                covariance[i][j] = (*iIt) * (*jIt) *
                    0.5 * (corr[i][j] + corr[j][i]);
                covariance[j][i] = covariance[i][j];
            }
            QL_REQUIRE(std::fabs(corr[i][i]-1.0) <= tolerance,
                       "invalid correlation matrix, "
                       << "diagonal element of the " << io::ordinal(i+1)
                       << " row is " << corr[i][i] << " instead of 1.0");
            covariance[i][i] = (*iIt) * (*iIt);
        }
        return covariance;
    }

    //! Covariance decomposition into correlation and variances
    /*! Extracts the correlation matrix and the vector of variances
        out of the input covariance matrix.

        Note that only the lower symmetric part of the covariance matrix is
        used.

        \pre The covariance matrix must be symmetric.

        \test cross checked with getCovariance
    */
    class CovarianceDecomposition {
      public:
        /*! \pre covarianceMatrix must be symmetric */
        CovarianceDecomposition(
            const Matrix& covarianceMatrix,
            Real tolerance = 1.0e-12);
        /*! returns the variances Array */
        const Array& variances() const { return variances_; }
        /*! returns the standard deviations Array */
        const Array& standardDeviations() const {return stdDevs_; }
        /*! returns the correlation matrix */
        const Matrix& correlationMatrix() const { return correlationMatrix_; }
      private:
        Array variances_, stdDevs_;
        Matrix correlationMatrix_;
    };

}


#endif
