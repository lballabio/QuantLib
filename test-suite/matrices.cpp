
/*
 Copyright (C) 2003 Ferdinando Ametrano

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
// $Id$

#include "matrices.hpp"
#include <ql/Math/matrix.hpp>
#include <ql/Math/symmetricschurdecomposition.hpp>
#include <vector>

using namespace QuantLib;
using namespace QuantLib::Math;

void MatricesTest::runTest() {

    // numerical example from "Monte Carlo Methods in Finance",
    // by Peter Jäckel, Section 6.4


    Matrix good(3,3,1.0);
    good[0][1] = good[1][0] = 0.9;
    good[0][2] = good[2][0] = 0.7;
    good[1][2] = good[2][1] = 0.4;
    Matrix bad(good);
    bad[1][2] = bad[2][1] = 0.3;
    Matrix identity(3, 3, 0.0);
    identity[0][0] = good[1][1] = good[2][2] = 1.0;

    Matrix eigenVectors, pseudoSqrtMatrix, null;
    std::vector<double> errorVector;
    double error = 0.0;

    eigenVectors = SymmetricSchurDecomposition(good).eigenvectors();
    null = (eigenVectors * transpose(eigenVectors)) - identity;
    errorVector = std::vector<double>(null.end()-null.begin());
    std::transform(null.begin(), null.end(), null.begin(), errorVector.begin(),
                   std::multiplies<double>());
    std::accumulate(errorVector.begin(), errorVector.end(), error);
    if (error > 1.0e-29) {
        char s[10];
        QL_SPRINTF(s,"%5.2e", error);
        CPPUNIT_FAIL("Matrices test failed at eigenvector test: "
            "\n error: " + std::string(s));
    }

    pseudoSqrtMatrix = pseudoSqrt(good);
    null = (pseudoSqrtMatrix * transpose(pseudoSqrtMatrix)) - good;
    errorVector = std::vector<double>(null.end()-null.begin());
    std::transform(null.begin(), null.end(), null.begin(), errorVector.begin(),
                   std::multiplies<double>());
    std::accumulate(errorVector.begin(), errorVector.end(), error);
    if (error > 1.0e-29) {
        char s[10];
        QL_SPRINTF(s,"%5.2e", error);
        CPPUNIT_FAIL("Matrices test failed at eigenvector test: "
            "\n error: " + std::string(s));
    }


    eigenVectors = SymmetricSchurDecomposition(bad).eigenvectors();
    null = (eigenVectors * transpose(eigenVectors)) - identity;
    errorVector = std::vector<double>(null.end()-null.begin());
    std::transform(null.begin(), null.end(), null.begin(), errorVector.begin(),
                   std::multiplies<double>());
    std::accumulate(errorVector.begin(), errorVector.end(), error);
    if (error > 1.0e-29) {
        char s[10];
        QL_SPRINTF(s,"%5.2e", error);
        CPPUNIT_FAIL("Matrices test failed at eigenvector test: "
            "\n error: " + std::string(s));
    }

    pseudoSqrtMatrix = pseudoSqrt(bad, Spectral);
    null = (pseudoSqrtMatrix * transpose(pseudoSqrtMatrix)) - bad;
    errorVector = std::vector<double>(null.end()-null.begin());
    std::transform(null.begin(), null.end(), null.begin(), errorVector.begin(),
                   std::multiplies<double>());
    std::accumulate(errorVector.begin(), errorVector.end(), error);
    if (error > 1.01e-4) {
        char s[10];
        QL_SPRINTF(s,"%5.2e", error);
        CPPUNIT_FAIL("Matrices test failed at eigenvector test: "
            "\n error: " + std::string(s));
    }

}

