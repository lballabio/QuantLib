
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

/*! \file matrix.cpp
    \brief matrix used in linear algebra.
*/

#include <ql/Math/symmetricschurdecomposition.hpp>
#include <ql/Math/cholesky.hpp>
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
    Disposable<Matrix> pseudoSqrt(const Matrix &realSymmetricMatrix,
        SalvagingAlgorithm sa) {


        // spectral (a.k.a Principal Component) analysis
        SymmetricSchurDecomposition jd(realSymmetricMatrix);
        Size i, j, size = realSymmetricMatrix.rows();
        Matrix diagonal(size, size, 0.0);


        // salvaging algorithm
        Matrix result(size, size);
        switch (sa) {
          case None:
            // eigenvalues are sorted in decreasing order
            QL_REQUIRE(jd.eigenvalues()[size-1]>=-1e-16,
                "Matrix pseudoSqrt: negative eigenvalue(s) ("
                + DoubleFormatter::toExponential(jd.eigenvalues()[size-1]) +
                ")");
            result = Cholesky(realSymmetricMatrix, true).decomposition();
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
            case None:
            case Spectral:
                return result;
            // optimization step
            case Hypersphere:
                result = result * transpose(result);
                result = Cholesky(result, true).decomposition();
                // will use spectral result as starting guess

                // optimization (general or LMM specific)
                throw Error("not implemented yet");
            default:
                throw Error("unknown salvaging algorithm");
        }

    }


    Disposable<Matrix> rankReducedSqrt(const Matrix& realSymmetricMatrix,
                                       Size maxRank,
                                       double componentRetainedPercentage,
                                       SalvagingAlgorithm sa) {

        QL_REQUIRE(componentRetainedPercentage>0.0,
            "Matrix rankReducedSqrt: no eigenvalues retained");

        QL_REQUIRE(componentRetainedPercentage<=1.0,
            "Matrix rankReducedSqrt: percentage to be retained > 100%");

        QL_REQUIRE(maxRank>1,
            "Matrix rankReducedSqrt: max rank required < 1");

        // spectral (a.k.a Principal Component) analysis
        SymmetricSchurDecomposition jd(realSymmetricMatrix);
        Size i, j, size = realSymmetricMatrix.rows();
        Matrix diagonal(size, size, 0.0);
        Matrix result(size, size);

        // require a positive semi-definite matrix
        // (eigenvalues are sorted in decreasing order)
        QL_REQUIRE(jd.eigenvalues()[size-1]>=0.0,
            "Matrix rankReducedSqrt: negative eigenvalue(s)");

        // rank reduction:
        // output is granted to have a rank<=maxRank
        // if maxRank>=size, then the required percentage of eigenvalues
        // is retained
        double enough = componentRetainedPercentage * size;
        double components = 0.0;
        for (i=0; i<QL_MIN(size, maxRank); i++) {
            components += jd.eigenvalues()[i];
            diagonal[i][i] =
                (components<enough ? QL_SQRT(jd.eigenvalues()[i]) : 0.0);
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
        case Spectral:
            return result;
        case Hypersphere:
            result = result * transpose(result);
            result = Cholesky(result, true).decomposition();
            // will use spectral result as starting guess

            // optimization (general or LMM specific)
            throw Error("rankReducedSqrt: not implemented yet");
        default:
            throw Error("unknown salvaging algorithm");
        }

    }


 
}

