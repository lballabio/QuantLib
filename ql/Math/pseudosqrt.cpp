/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Math/pseudosqrt.hpp>
#include <ql/Math/choleskydecomposition.hpp>
#include <ql/Math/symmetricschurdecomposition.hpp>

namespace QuantLib {

    const Disposable<Matrix> pseudoSqrt(const Matrix& matrix,
                                        SalvagingAlgorithm::Type sa) {

        QL_REQUIRE(matrix.rows() == matrix.columns(),
                   "matrix not square");
        Size i, j;
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        for (i=0; i<matrix.rows(); i++)
            for (j=0; j<i; j++)
                QL_REQUIRE(matrix[i][j] == matrix[j][i],
                           "matrix not symmetric");
        #endif

        // spectral (a.k.a Principal Component) analysis
        SymmetricSchurDecomposition jd(matrix);
        Size size = matrix.rows();
        Matrix diagonal(size, size, 0.0);


        // salvaging algorithm
        Matrix result(size, size);
        switch (sa) {
          case SalvagingAlgorithm::None:
            // eigenvalues are sorted in decreasing order
            QL_REQUIRE(jd.eigenvalues()[size-1]>=-1e-16,
                       "negative eigenvalue(s) ("
                       << std::scientific << jd.eigenvalues()[size-1]
                       << ")");
            result = CholeskyDecomposition(matrix, true);
            break;
          case SalvagingAlgorithm::Spectral:
            // negative eigenvalues set to zero
            for (i=0; i<size; i++)
                diagonal[i][i] =
                    std::sqrt(std::max<Real>(jd.eigenvalues()[i], 0.0));

            result = jd.eigenvectors() * diagonal;
            // row normalization
            for (i = 0; i < size;i++) {
                Real norm = 0.0;
                for (j = 0; j < size; j++)
                    norm += result[i][j]*result[i][j];
                norm = std::sqrt(matrix[i][i]/norm);
                for(j = 0; j < size; j++)
                    result[i][j] *= norm;
            }
            break;
          default:
            QL_FAIL("unknown salvaging algorithm");
        }

        return result;
    }


    const Disposable<Matrix> rankReducedSqrt(const Matrix& matrix,
                                             Size maxRank,
                                             Real componentRetainedPercentage,
                                             SalvagingAlgorithm::Type sa)
    {
        Size size = matrix.rows();
        QL_REQUIRE(size == matrix.columns(),
                   "matrix not square");
        Size i, j;
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        for (i=0; i<size; i++)
            for (j=0; j<i; j++)
                QL_REQUIRE(matrix[i][j] == matrix[j][i],
                           "matrix not symmetric");
        #endif

        QL_REQUIRE(componentRetainedPercentage>0.0,
                   "no eigenvalues retained");

        QL_REQUIRE(componentRetainedPercentage<=1.0,
                   "percentage to be retained > 100%");

        QL_REQUIRE(maxRank>=1,
                   "max rank required < 1");

        // spectral (a.k.a Principal Component) analysis
        SymmetricSchurDecomposition jd(matrix);
        Array eigenValues = jd.eigenvalues();

        // salvaging algorithm
        switch (sa) {
          case SalvagingAlgorithm::None:
            // eigenvalues are sorted in decreasing order
            QL_REQUIRE(eigenValues[size-1]>=-1e-16,
                       "negative eigenvalue(s) ("
                       << std::scientific << eigenValues[size-1]
                       << ")");
            break;
          case SalvagingAlgorithm::Spectral:
            // negative eigenvalues set to zero
            for (i=0; i<size; i++)
                eigenValues[i] =
                    std::sqrt(std::max<Real>(eigenValues[i], 0.0));
            break;
          default:
            QL_FAIL("unknown salvaging algorithm");
        }

        // rank reduction:
        // output is granted to have a rank<=maxRank
        // if maxRank>=size, then the required percentage of eigenvalues
        // is retained
        Real enough = componentRetainedPercentage *
                      std::accumulate(eigenValues.begin(),
                                      eigenValues.end(), 0.0);
        Real components = 0.0;
        Matrix diagonal(size, size, 0.0);
        for (i=0; i<std::min(size, maxRank) && components<enough; i++) {
            diagonal[i][i] = std::sqrt(eigenValues[i]);
            components += eigenValues[i];
        }
        Matrix result = jd.eigenvectors() * diagonal;

        // row normalization
        for (i = 0; i < size;i++) {
            Real norm = 0.0;
            for (j = 0; j < size; j++)
                norm += result[i][j]*result[i][j];
                norm = std::sqrt(matrix[i][i]/norm);
                for(j = 0; j < size; j++)
                    result[i][j] *= norm;
        }

        return result;
    }

}

