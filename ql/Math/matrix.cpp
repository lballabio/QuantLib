
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
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

/*! \file matrix.cpp
    \brief matrix used in linear algebra.
*/

#include <ql/Math/symmetricschurdecomposition.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {


    /*  see "The most general methodology to create a valid correlation matrix
        for risk management and option pricing purposes",
        by R. Rebonato and P. Jäckel.
        The Journal of Risk, 2(2), Winter 1999/2000
        http://www.rebonato.com/correlationmatrix.pdf

        Revised and extended in "Monte Carlo Methods in Finance",
        by Peter Jäckel, Chapter 6
    */
    const Disposable<Matrix> pseudoSqrt(const Matrix& matrix,
                                        SalvagingAlgorithm::Type sa) {

        QL_REQUIRE(matrix.rows() == matrix.columns(),
                   "pseudoSqrt: matrix not square");
        Size i, j;
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        for (i=0; i<matrix.rows(); i++)
            for (j=0; j<i; j++)
                QL_REQUIRE(matrix[i][j] == matrix[j][i],
                           "pseudoSqrt: matrix not symmetric");
        #endif

        // spectral (a.k.a Principal Component) analysis
        SymmetricSchurDecomposition jd(matrix);
        Size size = matrix.rows();
        Matrix diagonal(size, size, 0.0);


        // salvaging algorithm
        Matrix result(size, size);
        //Disposable<Matrix> result();
        switch (sa) {
          case SalvagingAlgorithm::None:
            // eigenvalues are sorted in decreasing order
            QL_REQUIRE(jd.eigenvalues()[size-1]>=-1e-16,
                "Matrix pseudoSqrt: negative eigenvalue(s) ("
                + DoubleFormatter::toExponential(jd.eigenvalues()[size-1]) +
                ")");
            result = CholeskyDecomposition(matrix, true);
            break;
          default:
            // salvaging algorithm:
            // negative eigenvalues set to zero
            for (i=0; i<size; i++)
                diagonal[i][i] = QL_SQRT(QL_MAX(jd.eigenvalues()[i], 0.0));

            result = jd.eigenvectors() * diagonal;
            // row normalization
            for (i = 0; i < size;i++) {
                double norm = 0.0;
                for (j = 0; j < size; j++)
                    norm += result[i][j]*result[i][j];
                norm = QL_SQRT(norm);
                for(j = 0; j < size; j++)
                    result[i][j] /= norm;
            }
            break;
        }

        switch (sa) {
          case SalvagingAlgorithm::None:
          case SalvagingAlgorithm::Spectral:
            return result;
            // optimization step
          case SalvagingAlgorithm::Hypersphere:
            result = result * transpose(result);
            result = CholeskyDecomposition(result, true);
            // will use spectral result as starting guess

            // optimization (general or LMM specific)
            QL_FAIL("not implemented yet");
          default:
            QL_FAIL("unknown salvaging algorithm");
        }

    }


    const Disposable<Matrix> rankReducedSqrt(
                                         const Matrix& matrix,
                                         Size maxRank, 
                                         double componentRetainedPercentage,
                                         SalvagingAlgorithm::Type sa) {

        QL_REQUIRE(matrix.rows() == matrix.columns(),
                   "pseudoSqrt: matrix not square");
        Size i, j;
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        for (i=0; i<matrix.rows(); i++)
            for (j=0; j<i; j++)
                QL_REQUIRE(matrix[i][j] == matrix[j][i],
                           "pseudoSqrt: matrix not symmetric");
        #endif

        QL_REQUIRE(componentRetainedPercentage>0.0,
                   "rankReducedSqrt: no eigenvalues retained");

        QL_REQUIRE(componentRetainedPercentage<=1.0,
                   "rankReducedSqrt: percentage to be retained > 100%");

        QL_REQUIRE(maxRank>=1,
                   "rankReducedSqrt: max rank required < 1");

        // spectral (a.k.a Principal Component) analysis
        SymmetricSchurDecomposition jd(matrix);
        Size size = matrix.rows();
        Matrix diagonal(size, size, 0.0);
        Matrix result(size, size);

        // require a positive semi-definite matrix
        // (eigenvalues are sorted in decreasing order)
        QL_REQUIRE(jd.eigenvalues()[size-1]>=0.0,
                   "rankReducedSqrt: negative eigenvalue(s)");

        // rank reduction:
        // output is granted to have a rank<=maxRank
        // if maxRank>=size, then the required percentage of eigenvalues
        // is retained
        double enough = componentRetainedPercentage * size;
        double components = 0.0;
        for (i=0; i<QL_MIN(size, maxRank); i++) {
            diagonal[i][i] =
                (components<enough ? QL_SQRT(jd.eigenvalues()[i]) : 0.0);
            components += jd.eigenvalues()[i];
        }

        result = jd.eigenvectors() * diagonal;

        // row normalization
        for (i = 0; i < size;i++) {
            double norm = 0.0;
            for (j = 0; j < size; j++)
                norm += result[i][j]*result[i][j];
            norm = QL_SQRT(norm);
            for(j = 0; j < size; j++)
                result[i][j] /= norm;
        }

        switch (sa) {
          case SalvagingAlgorithm::Spectral:
            return result;
          case SalvagingAlgorithm::Hypersphere:
            result = result * transpose(result);
            result = CholeskyDecomposition(result, true);
            // will use spectral result as starting guess

            // optimization (general or LMM specific)
            QL_FAIL("rankReducedSqrt: not implemented yet");
          default:
            QL_FAIL("unknown salvaging algorithm");
        }

    }


    const Disposable<Matrix> CholeskyDecomposition(const Matrix &S,
                                                   bool flexible) {
        Size i, j, size = S.rows();
        // should check for simmetry too: the algorithm will only use
        // the upper triangular part of S anyway...
        QL_REQUIRE(size == S.columns(),
                   "CholeskyDecomposition : "
                   "input matrix is not a square matrix");
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        for (i=0; i<matrix.rows(); i++)
            for (j=0; j<i; j++)
                QL_REQUIRE(S[i][j] == S[j][i],
                           "CholeskyDecomposition : "
                           "input matrix is not symmetric");
        #endif

        Matrix result(size, size, 0.0);
        double sum;
        for (i=0; i<size; i++) {
            for (j=i; j<size; j++) {
                sum = S[i][j];
                for (int k=0; k<=int(i)-1; k++) {
                    sum -= result[i][k]*result[j][k];
                }
                if (i == j) {
                    QL_REQUIRE(flexible || sum > 0.0,
                        "CholeskyDecomposition : "
                        "input matrix is not positive definite");
                    // To handle positive semi-definite matrices take the
                    // square root of sum if positive, else zero.
                    result[i][i] = QL_SQRT(QL_MAX(sum, 0.0));
                } else {
                    // With positive semi-definite matrices is possible
                    // to have result[i][i]==0.0
                    // In this case sum happens to be zero as well
                    result[j][i] =
                        (sum==0.0 ? 0.0 : sum/result[i][i]);
                }
            }
        }

        return result;
    }

 
}

