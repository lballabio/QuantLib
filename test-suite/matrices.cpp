
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
#include <ql/Math/svd.hpp>
#include <ql/Math/symmetricschurdecomposition.hpp>
#include <ql/dataformatters.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

using namespace QuantLib;

namespace {

    Size N;
    Matrix M1, M2, M3, M4, I;

    double norm(const Array& v) {
        return QL_SQRT(DotProduct(v,v));
    }

    double norm(const Matrix& m) {
        double sum = 0.0;
        for (Size i=0; i<m.rows(); i++)
            for (Size j=0; j<m.columns(); j++)
                sum += m[i][j]*m[i][j];
        return QL_SQRT(sum);
    }

}

void MatricesTest::setUp() {

    N = 3;
    M1 = M2 = I = Matrix(N,N);
    M3 = Matrix(3,4);
    M4 = Matrix(4,3);

    M1[0][0] = 1.0;  M1[0][1] = 0.9;  M1[0][2] = 0.7;
    M1[1][0] = 0.9;  M1[1][1] = 1.0;  M1[1][2] = 0.4;
    M1[2][0] = 0.7;  M1[2][1] = 0.4;  M1[2][2] = 1.0;

    M2[0][0] = 1.0;  M2[0][1] = 0.9;  M2[0][2] = 0.7;
    M2[1][0] = 0.9;  M2[1][1] = 1.0;  M2[1][2] = 0.3;
    M2[2][0] = 0.7;  M2[2][1] = 0.3;  M2[2][2] = 1.0;

    I[0][0] = 1.0;  I[0][1] = 0.0;  I[0][2] = 0.0;
    I[1][0] = 0.0;  I[1][1] = 1.0;  I[1][2] = 0.0;
    I[2][0] = 0.0;  I[2][1] = 0.0;  I[2][2] = 1.0;

    M3[0][0] = 1; M3[0][1] = 2; M3[0][2] = 3; M3[0][3] = 4;
    M3[1][0] = 2; M3[1][1] = 0; M3[1][2] = 2; M3[1][3] = 1;
    M3[2][0] = 0; M3[2][1] = 1; M3[2][2] = 0; M3[2][3] = 0;

    M4[0][0] = 1; M4[0][1] = 2; M4[0][2] = 400; 
    M4[1][0] = 2; M4[1][1] = 0; M4[1][2] = 1; 
    M4[2][0] = 30; M4[2][1] = 2; M4[2][2] = 0; 
    M4[3][0] = 2; M4[3][1] = 0; M4[3][2] = 1.05;
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
        if (norm(m-I) > 1.0e-15)
            CPPUNIT_FAIL("Eigenvector not normalized");
    }
}

void MatricesTest::testSqrt() {

    Matrix m = pseudoSqrt(M1, SalvagingAlgorithm::None);
    Matrix temp = m*m;
    double error = norm(temp - M1);
    double tolerance = 1.0e-12;
    if (error>tolerance) {
        CPPUNIT_FAIL("Matrix square root calculation failed"
        "\noriginal matrix:\n" + ArrayFormatter::toString(M1.begin(), M1.end(), 6, 0, M1.columns()) +
        "\npseudoSqrt:\n" + ArrayFormatter::toString(m.begin(), m.end(), 6, 0, M1.columns()) +
        "\npseudoSqrt*pseudoSqrt:\n" + ArrayFormatter::toString(temp.begin(), temp.end(), 6, 0, M1.columns()) +
        "\nerror:     " + DoubleFormatter::toString(error) +
        "\ntolerance: " + DoubleFormatter::toString(tolerance)
        );
    }

}

void MatricesTest::testSVD() {      
    
    double tol = 1.0e-12;
    Matrix testMatrices[] = { M1, M2, M3, M4 };

    for (Size j = 0; j < LENGTH(testMatrices); j++) {
        // m >= n required (rows >= columns)        
        Matrix& A = testMatrices[j];        
        SVD svd(A);    
        // U is m x n
        Matrix U = svd.U();
        // s is n long
        Array s = svd.singularValues();
        // S is n x n
        Matrix S = svd.S();
        // V is n x n 
        Matrix V = svd.V();

        for (Size i=0; i < S.rows(); i++) {
            if (S[i][i] != s[i])
                CPPUNIT_FAIL("S not consistent with s");
        }
        
        // tests
        Matrix U_Utranspose = transpose(U)*U;
        if (norm(U_Utranspose-I) > tol)
            CPPUNIT_FAIL("U not orthogonal (norm of U*U^T-I = " +
                         DoubleFormatter::toExponential(norm(U_Utranspose-I))
                         + ")");

        Matrix V_Vtranspose = transpose(V)*V;
        if (norm(V_Vtranspose-I) > tol)
            CPPUNIT_FAIL("V not orthogonal (norm of V*V^T-I = " +
                         DoubleFormatter::toExponential(norm(V_Vtranspose-I))
                         + ")");

        Matrix A_reconstructed = U * S * transpose(V);        
        if (norm(A_reconstructed-A) > tol)
            CPPUNIT_FAIL("Product does not recover A: (norm of A'-A = " +
                         DoubleFormatter::toExponential(norm(A_reconstructed-A))
                         + ")");
    }
}


CppUnit::Test* MatricesTest::suite() {
    CppUnit::TestSuite* tests = new CppUnit::TestSuite("Matrix tests");
    tests->addTest(new CppUnit::TestCaller<MatricesTest>
                   ("Testing eigenvalues and eigenvectors calculation",
                    &MatricesTest::testEigenvectors));
    tests->addTest(new CppUnit::TestCaller<MatricesTest>
                   ("Testing matricial square root",
                    &MatricesTest::testSqrt));
    tests->addTest(new CppUnit::TestCaller<MatricesTest>
                   ("Testing singular value decomposition",
                    &MatricesTest::testSVD));
    return tests;
}

