
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file matrix.cpp
    \brief matrix used in linear algebra.
*/

#include <ql/Math/symmetricschurdecomposition.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    namespace Math {

        Disposable<Matrix> pseudoSqrt(const Matrix &realSymmMatrix, 
                                      SalvagingAlgorithm sa) {

            Size size = realSymmMatrix.rows();
            SymmetricSchurDecomposition jd(realSymmMatrix);

            double maxEv=0;
            Size i, j;
            for (i = 0; i < size;i++) {
                if (jd.eigenvalues()[i] >= maxEv)
                    maxEv = jd.eigenvalues()[i];
            }

            Matrix diagonal(size,size,0);
            for (i = 0; i < size; i++) {
                // eigenvalues smaller than relative tolerance
                // are considered zero
                double tolerance = 1e-15;
                if (QL_FABS(jd.eigenvalues()[i]) <= tolerance*maxEv) {
                    diagonal[i][i] = 0.0;
                    continue;
                }
                switch (sa) {
                  case None:
                    QL_REQUIRE(jd.eigenvalues()[i] >=0,
                          "pseudoSqrt: eigenvalues("+
                          IntegerFormatter::toString(i) + ") = " +
                          DoubleFormatter::toString(jd.eigenvalues()[i]) +
                          " is negative!");
                    diagonal[i][i] = QL_SQRT(jd.eigenvalues()[i]);
                    break;
                /*  see "The most general methodology to create a valid
                    correlation matrix for risk management and option
                    pricing purposes", by R. Rebonato and P. Jäckel.
                    The Journal of Risk, 2(2), Winter 1999/2000
                    http://www.rebonato.com/correlationmatrix.pdf

                    Revised and extended in "Monte Carlo Methods in Finance",
                    by Peter Jäckel, Chapter 6
                */
                  case Spectral:
                  case Hypersphere:
                    if (jd.eigenvalues()[i] >=0)
                      diagonal[i][i] = QL_SQRT(jd.eigenvalues()[i]);
                    else
                      diagonal[i][i] = 0.0;
                    break;
                  default:
                    throw Error("unknown matrix pseudo square root "
                            "salvaging algorithm");
                }
            }

            Matrix result(jd.eigenvectors() * diagonal *
                                transpose(jd.eigenvectors()));

            switch (sa) {
              case Spectral:
              case Hypersphere:
                // need to normalize the rows
                for (i = 0; i < size;i++) {
                    double norm = 0.0;
                    for (j = 0; j < size; j++) {
                        norm += result[i][j]*result[i][j];
                    }
                    norm = QL_SQRT(norm);
                    for(j = 0; j < size; j++) {
                        result[i][j] /= norm;
                    }
                }
              default:
                break;
            }

            switch (sa) {
              case Hypersphere:
                throw Error("not implemented yet");
              default:
                return result;
            }
        }

    }

}

