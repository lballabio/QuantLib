
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

#include "matrices.hpp"
#include "utilities.hpp"
#include <ql/Math/matrix.hpp>
#include <ql/Math/symmetricschurdecomposition.hpp>
#include <ql/dataformatters.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

using namespace QuantLib;

namespace {

    Size N;
    Matrix M1, M2, I;

    double norm(const Array& v) {
        return QL_SQRT(DotProduct(v,v));
    }

    double norm(const Matrix& m) {
        double sum = 0.0;
        for (Size i=0; i<m.rows(); i++)
            for (Size j=0; j<m.columns(); j++)
                sum += m[i][j];
        return QL_SQRT(sum);
    }

}

void MatricesTest::setUp() {

    N = 3;
    M1 = M2 = I = Matrix(N,N);

    M1[0][0] = 1.0;  M1[0][1] = 0.9;  M1[0][2] = 0.7;
    M1[1][0] = 0.9;  M1[1][1] = 1.0;  M1[1][2] = 0.4;
    M1[2][0] = 0.7;  M1[2][1] = 0.4;  M1[2][2] = 1.0;

    M2[0][0] = 1.0;  M2[0][1] = 0.9;  M2[0][2] = 0.7;
    M2[1][0] = 0.9;  M2[1][1] = 1.0;  M2[1][2] = 0.3;
    M2[2][0] = 0.7;  M2[2][1] = 0.3;  M2[2][2] = 1.0;

    I[0][0] = 1.0;  I[0][1] = 0.0;  I[0][2] = 0.0;
    I[1][0] = 0.0;  I[1][1] = 1.0;  I[1][2] = 0.0;
    I[2][0] = 0.0;  I[2][1] = 0.0;  I[2][2] = 1.0;
}

void MatricesTest::testEigenvectors() {

    Matrix testMatrices[] = { M1, M2 };

    for (Size k=0; k<LENGTH(testMatrices); k++) {

        Matrix& M = testMatrices[k];
        SymmetricSchurDecomposition dec(M);
        Array eigenValues = dec.eigenvalues();
        Matrix eigenVectors = dec.eigenvectors();

        for (Size i=0; i<N; i++) {
            Array v(N);
            for (Size j=0; j<N; j++)
                v[j] = eigenVectors[j][i];
            // check definition
            Array a = M*v;
            Array b = eigenValues[i]*v;
            if (norm(a-b) > 1.0e-15)
                CPPUNIT_FAIL("Eigenvector definition not satisfied");
        }

        // check normalization
        Matrix m = eigenVectors * transpose(eigenVectors);
        if (norm(m-I) > 1.0e-7)
            CPPUNIT_FAIL("Eigenvector not normalized");
    }
}

void MatricesTest::testSqrt() {

    Matrix m = pseudoSqrt(M1, SalvagingAlgorithm::None);
    if (norm(m*m - M1) > 1.0e-20)
        CPPUNIT_FAIL("Matrix square root calculation failed");

}


CppUnit::Test* MatricesTest::suite() {
    CppUnit::TestSuite* tests = new CppUnit::TestSuite("Matrix tests");
    tests->addTest(new CppUnit::TestCaller<MatricesTest>
                   ("Testing eigenvalues and eigenvectors calculation",
                    &MatricesTest::testEigenvectors));
    tests->addTest(new CppUnit::TestCaller<MatricesTest>
                   ("Testing matricial square root",
                    &MatricesTest::testSqrt));
    return tests;
}

