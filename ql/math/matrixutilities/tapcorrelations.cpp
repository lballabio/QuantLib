/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud

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

#include <ql/math/matrixutilities/tapcorrelations.hpp>
#include <cmath>

namespace QuantLib {

    Matrix triangularAnglesParametrization(const Array& angles,
                                           Size matrixSize,
                                           Size rank) {

        // what if rank == 1?
        QL_REQUIRE((rank-1) * (2*matrixSize - rank) == 2*angles.size(),
                    "rank-1) * (matrixSize - rank/2) == angles.size()");
        Matrix m(matrixSize, matrixSize);

        // first row filling
        m[0][0] = 1.0;
        for (Size j=1; j<matrixSize; ++j)
            m[0][j] = 0.0;

        // next ones...
        Size k = 0; //angles index
        for (Size i=1; i<m.rows(); ++i) {
            Real sinProduct = 1.0;
            Size bound = std::min(i,rank-1);
            for (Size j=0; j<bound; ++j) {
                m[i][j] = std::cos(angles[k]);
                m[i][j] *= sinProduct;
                sinProduct *= std::sin(angles[k]);
                ++k;
            }
            m[i][bound] = sinProduct;
            for (Size j=bound+1; j<m.rows(); ++j)
                m[i][j] = 0;
        }
        return m;
    }

    Matrix lmmTriangularAnglesParametrization(const Array& angles,
                                              Size matrixSize,
                                              Size) {
        Matrix m(matrixSize, matrixSize);
        for (Size i=0; i<m.rows(); ++i) {
            Real cosPhi, sinPhi;
            if (i>0) {
                cosPhi = std::cos(angles[i-1]);
                sinPhi = std::sin(angles[i-1]);
            } else {
                cosPhi = 1.0;
                sinPhi = 0.0;
            }

            for (Size j=0; j<i; ++j)
                m[i][j] = sinPhi * m[i-1][j];

            m[i][i] = cosPhi;

            for (Size j=i+1; j<m.rows(); ++j)
                m[i][j] = 0.0;
        }
        return m;
    }

    Matrix triangularAnglesParametrizationUnconstrained(const Array& x,
                                                        Size matrixSize,
                                                        Size rank) {
        Array angles(x.size());
        //we convert the unconstrained parameters in angles
        for (Size i = 0; i < x.size(); ++i)
            angles[i] = M_PI*.5 - std::atan(x[i]);
        return triangularAnglesParametrization(angles, matrixSize, rank);
    }

    Matrix lmmTriangularAnglesParametrizationUnconstrained(const Array& x,
                                                           Size matrixSize,
                                                           Size rank) {
        Array angles(x.size());
        //we convert the unconstrained parameters in angles
        for (Size i = 0; i < x.size(); ++i)
            angles[i] = M_PI*.5 - std::atan(x[i]);
        return lmmTriangularAnglesParametrization(angles, matrixSize, rank);
    }

    Matrix triangularAnglesParametrizationRankThree(Real alpha, Real t0,
                                                    Real epsilon, Size matrixSize) {
        Matrix m(matrixSize, 3);
        for (Size i=0; i<m.rows(); ++i) {
            Real t = t0 * (1 - std::exp(epsilon*Real(i)));
            Real phi = std::atan(alpha * t);
            m[i][0] = std::cos(t)*std::cos(phi);
            m[i][1] = std::sin(t)*std::cos(phi);
            m[i][2] = -std::sin(phi);
        }
        return m;
    }

    Matrix triangularAnglesParametrizationRankThreeVectorial(const Array& parameters,
                                                             Size nbRows) {
        QL_REQUIRE(parameters.size() == 3,
                   "the parameter array must contain exactly 3 values" );
        return triangularAnglesParametrizationRankThree(parameters[0],
                                                        parameters[1],
                                                        parameters[2],
                                                        nbRows);

    }

    Real FrobeniusCostFunction::value(const Array& x) const {
        Array temp = values(x);
        return DotProduct(temp, temp);
    }

    Array FrobeniusCostFunction::values(const Array& x) const {
        Array result((target_.rows()*(target_.columns()-1))/2);
        Matrix pseudoRoot = f_(x, matrixSize_, rank_);
        Matrix differences = pseudoRoot * transpose(pseudoRoot) - target_;
        Size k = 0;
        // then we store the elementwise differences in a vector.
        for (Size i=0; i<target_.rows(); ++i) {
            for (Size j=0; j<i; ++j){
                result[k] = differences[i][j];
                ++k;
            }
        }
        return result;
    }
}
