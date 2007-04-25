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
#include <ql/math/matrix.hpp>
#include <cmath>
#include <iostream>

namespace QuantLib {

    void setTriangularAnglesParametrization(const std::vector<Real>& angles, 
                                                                Matrix& m) {
        QL_REQUIRE(m.rows() == m.columns(), "the matrix m must be square!");
        QL_REQUIRE(m.rows() == angles.size()+1, 
                          "angles vector size must be equal to m.rows()-1");  
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
    }

    void setTriangularAnglesParametrizationRankThree(Real alpha, Real t0, 
                                                Real epsilon, Matrix& m) {
        QL_REQUIRE(3 == m.columns(), 
                            "the matrix m must contain exactly 3 columns!");
        for (Size i=0; i<m.rows(); ++i) {
            Real t = t0 * (1 - std::exp(epsilon*Real(i)));
            Real phi = std::atan(alpha * t);
            std::cout << i << "\t" <<  t << "\t" << phi << std::endl;
            m[i][0] = std::cos(t)*std::cos(phi);
            m[i][1] = std::sin(t)*std::cos(phi);
            m[i][2] = -std::sin(phi);
        }
    }
}
