/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud

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

#include <ql/models/marketmodels/piecewiseconstantcorrelations/tapcorrelations.hpp>
#include <cmath>

namespace QuantLib {

    Disposable<Matrix> triangularAnglesParametrization(const Array& angles) {
            Matrix m(angles.size()+1, angles.size()+1);
            for (Size i=0; i<m.rows(); ++i) {
                Real cosPhi, sinPhi;
                if (i>0) {
                    cosPhi = std::cos(angles[i-1]);
                    sinPhi = std::sin(angles[i-1]);
                } 
                else {
                    cosPhi = 1;
                    sinPhi = 0;
                }

                for (Size j=0; j<i; ++j)
                    m[i][j] = sinPhi * m[i-1][j];

                m[i][i] = cosPhi;

                for (Size j=i+1; j<m.rows(); ++j)
                    m[i][j] = 0;
            }
            return m;
    }

    Disposable<Matrix> triangularAnglesParametrizationUnconstrained (
                                                        const Array& x) {
        Array angles(x.size()); 
        //we convert the unconstrained parameters in angles
        for(Size i = 0; i < x.size(); ++i)
            angles[i] = M_PI*.5 - std::atan(x[i]);
        return triangularAnglesParametrization(angles);
    }
    
    Disposable<Matrix> triangularAnglesParametrizationRankThree(Real alpha, Real t0, 
                                                           Real epsilon, Size nbRows) {
            Matrix m(nbRows, 3);
            for (Size i=0; i<m.rows(); ++i) {
                Real t = t0 * (1 - std::exp(epsilon*Real(i)));
                Real phi = std::atan(alpha * t);
                m[i][0] = std::cos(t)*std::cos(phi);
                m[i][1] = std::sin(t)*std::cos(phi);
                m[i][2] = -std::sin(phi);
            }
        return m;
    }

    Disposable<Matrix> triangularAnglesParametrizationRankThreeVectorial(
        const Array& paramters, Size nbRows){
        QL_REQUIRE(paramters.size() == 3, 
            "the parameter array must contain exactly 3 values" );
        return  triangularAnglesParametrizationRankThree(paramters[0], 
            paramters[1], paramters[2], nbRows);
        
    }

    Real FrobeniusCostFunction::value(const Array& x) const{
        Array temp = values(x);
        return DotProduct(temp, temp);
    }

    Disposable<Array> FrobeniusCostFunction::values(const Array& x) const{
        Array result(target_.rows()*target_.columns());
        // refresh parameterizedMatrix_ with values implied by the new set of
        // parameters
        Matrix pseudoRoot = f_(x);
        Matrix differences = pseudoRoot * transpose(pseudoRoot) - target_;
        // then we store the elementwise differences in a vector.
        for (Size i=0; i<target_.rows(); ++i)
            for (Size j=0; j<target_.columns(); ++j)
                result[i*target_.rows()+j] 
                            = differences[i][j]*differences[i][j];
        return result;
    }
}
