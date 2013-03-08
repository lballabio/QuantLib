/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2007, 2008 Klaus Spanderen
 Copyright (C) 2007 Neil Firth

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

#include "matrices.hpp"
#include "utilities.hpp"
#include <ql/math/matrix.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/math/matrixutilities/svd.hpp>
#include <ql/math/matrixutilities/symmetricschurdecomposition.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/math/matrixutilities/qrdecomposition.hpp>
#include <ql/math/matrixutilities/basisincompleteordered.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    Size N;
    Matrix M1, M2, M3, M4, M5, M6, M7, I;

    Real norm(const Array& v) {
        return std::sqrt(DotProduct(v,v));
    }

    Real norm(const Matrix& m) {
        Real sum = 0.0;
        for (Size i=0; i<m.rows(); i++)
            for (Size j=0; j<m.columns(); j++)
                sum += m[i][j]*m[i][j];
        return std::sqrt(sum);
    }

    void setup() {

        N = 3;
        M1 = M2 = I = Matrix(N,N);
        M3 = Matrix(3,4);
        M4 = Matrix(4,3);
        M5 = Matrix(4, 4, 0.0);
        M6 = Matrix(4, 4, 0.0);

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

        M4[0][0] = 1;  M4[0][1] = 2;  M4[0][2] = 400;
        M4[1][0] = 2;  M4[1][1] = 0;  M4[1][2] = 1;
        M4[2][0] = 30; M4[2][1] = 2;  M4[2][2] = 0;
        M4[3][0] = 2;  M4[3][1] = 0;  M4[3][2] = 1.05;

        // from Higham - nearest correlation matrix
        M5[0][0] = 2;   M5[0][1] = -1;  M5[0][2] = 0.0; M5[0][3] = 0.0;
        M5[1][0] = M5[0][1];  M5[1][1] = 2;   M5[1][2] = -1;  M5[1][3] = 0.0;
        M5[2][0] = M5[0][2]; M5[2][1] = M5[1][2];  M5[2][2] = 2;   M5[2][3] = -1;
        M5[3][0] = M5[0][3]; M5[3][1] = M5[1][3]; M5[3][2] = M5[2][3];  M5[3][3] = 2;

        // from Higham - nearest correlation matrix to M5
        M6[0][0] = 1;        M6[0][1] = -0.8084124981;  M6[0][2] = 0.1915875019;   M6[0][3] = 0.106775049;
        M6[1][0] = M6[0][1]; M6[1][1] = 1;        M6[1][2] = -0.6562326948;  M6[1][3] = M6[0][2];
        M6[2][0] = M6[0][2]; M6[2][1] = M6[1][2]; M6[2][2] = 1;        M6[2][3] = M6[0][1];
        M6[3][0] = M6[0][3]; M6[3][1] = M6[1][3]; M6[3][2] = M6[2][3]; M6[3][3] = 1;

        M7 = M1;
        M7[0][1] = 0.3; M7[0][2] = 0.2; M7[2][1] = 1.2;
    }

}


void MatricesTest::testEigenvectors() {

    BOOST_TEST_MESSAGE("Testing eigenvalues and eigenvectors calculation...");

    setup();

    Matrix testMatrices[] = { M1, M2 };

    for (Size k=0; k<LENGTH(testMatrices); k++) {

        Matrix& M = testMatrices[k];
        SymmetricSchurDecomposition dec(M);
        Array eigenValues = dec.eigenvalues();
        Matrix eigenVectors = dec.eigenvectors();
        Real minHolder = QL_MAX_REAL;

        for (Size i=0; i<N; i++) {
            Array v(N);
            for (Size j=0; j<N; j++)
                v[j] = eigenVectors[j][i];
            // check definition
            Array a = M*v;
            Array b = eigenValues[i]*v;
            if (norm(a-b) > 1.0e-15)
                BOOST_FAIL("Eigenvector definition not satisfied");
            // check decreasing ordering
            if (eigenValues[i] >= minHolder) {
                BOOST_FAIL("Eigenvalues not ordered: " << eigenValues);
            } else
                minHolder = eigenValues[i];
        }

        // check normalization
        Matrix m = eigenVectors * transpose(eigenVectors);
        if (norm(m-I) > 1.0e-15)
            BOOST_FAIL("Eigenvector not normalized");
    }
}

void MatricesTest::testSqrt() {

    BOOST_TEST_MESSAGE("Testing matricial square root...");

    setup();

    Matrix m = pseudoSqrt(M1, SalvagingAlgorithm::None);
    Matrix temp = m*transpose(m);
    Real error = norm(temp - M1);
    Real tolerance = 1.0e-12;
    if (error>tolerance) {
        BOOST_FAIL("Matrix square root calculation failed\n"
                   << "original matrix:\n" << M1
                   << "pseudoSqrt:\n" << m
                   << "pseudoSqrt*pseudoSqrt:\n" << temp
                   << "\nerror:     " << error
                   << "\ntolerance: " << tolerance);
    }
}

void MatricesTest::testHighamSqrt() {
    BOOST_TEST_MESSAGE("Testing Higham matricial square root...");

    setup();

    Matrix tempSqrt = pseudoSqrt(M5, SalvagingAlgorithm::Higham);
    Matrix ansSqrt = pseudoSqrt(M6, SalvagingAlgorithm::None);
    Real error = norm(ansSqrt - tempSqrt);
    Real tolerance = 1.0e-4;
    if (error>tolerance) {
        BOOST_FAIL("Higham matrix correction failed\n"
                   << "original matrix:\n" << M5
                   << "pseudoSqrt:\n" << tempSqrt
                   << "should be:\n" << ansSqrt
                   << "\nerror:     " << error
                   << "\ntolerance: " << tolerance);
    }
}

void MatricesTest::testSVD() {

    BOOST_TEST_MESSAGE("Testing singular value decomposition...");

    setup();

    Real tol = 1.0e-12;
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
                BOOST_FAIL("S not consistent with s");
        }

        // tests
        Matrix U_Utranspose = transpose(U)*U;
        if (norm(U_Utranspose-I) > tol)
            BOOST_FAIL("U not orthogonal (norm of U^T*U-I = "
                       << norm(U_Utranspose-I) << ")");

        Matrix V_Vtranspose = transpose(V)*V;
        if (norm(V_Vtranspose-I) > tol)
            BOOST_FAIL("V not orthogonal (norm of V^T*V-I = "
                       << norm(V_Vtranspose-I) << ")");

        Matrix A_reconstructed = U * S * transpose(V);
        if (norm(A_reconstructed-A) > tol)
            BOOST_FAIL("Product does not recover A: (norm of U*S*V^T-A = "
                       << norm(A_reconstructed-A) << ")");
    }
}

void MatricesTest::testQRDecomposition() {

    BOOST_TEST_MESSAGE("Testing QR decomposition...");

    setup();

    Real tol = 1.0e-12;
    Matrix testMatrices[] = { M1, M2, I,
                              M3, transpose(M3), M4, transpose(M4), M5 };

    for (Size j = 0; j < LENGTH(testMatrices); j++) {
        Matrix Q, R;
        bool pivot = true;
        const Matrix& A = testMatrices[j];
        const std::vector<Size> ipvt = qrDecomposition(A, Q, R, pivot);

        Matrix P(A.columns(), A.columns(), 0.0);

        // reverse column pivoting
        for (Size i=0; i < P.columns(); ++i) {
            P[ipvt[i]][i] = 1.0;
        }

        if (norm(Q*R - A*P) > tol)
            BOOST_FAIL("Q*R does not match matrix A*P (norm = "
                       << norm(Q*R-A*P) << ")");

        pivot = false;
        qrDecomposition(A, Q, R, pivot);

        if (norm(Q*R - A) > tol)
            BOOST_FAIL("Q*R does not match matrix A (norm = "
                       << norm(Q*R-A) << ")");
    }
}

void MatricesTest::testQRSolve() {

    BOOST_TEST_MESSAGE("Testing QR solve...");

    setup();

    Real tol = 1.0e-12;
    MersenneTwisterUniformRng rng(1234);
    Matrix bigM(50, 100, 0.0);
    for (Size i=0; i < std::min(bigM.rows(), bigM.columns()); ++i) {
        bigM[i][i] = i+1.0;
    }
    Matrix testMatrices[] = { M1, M2, M3, transpose(M3),
                              M4, transpose(M4), M5, I, M7, bigM, transpose(bigM) };

    for (Size j = 0; j < LENGTH(testMatrices); j++) {
        const Matrix& A = testMatrices[j];
        Array b(A.rows());

        for (Size k=0; k < 10; ++k) {
            for (Array::iterator iter = b.begin(); iter != b.end(); ++iter) {
                *iter = rng.next().value;
            }
            const Array x = qrSolve(A, b, true);

            if (A.columns() >= A.rows()) {
                if (norm(A*x - b) > tol)
                    BOOST_FAIL("A*x does not match vector b (norm = "
                               << norm(A*x - b) << ")");
            }
            else {
                // use the SVD to calculate the reference values
                const Size n = A.columns();
                Array xr(n, 0.0);

                SVD svd(A);
                const Matrix& V = svd.V();
                const Matrix& U = svd.U();
                const Array&  w = svd.singularValues();
                const Real threshold = n*QL_EPSILON;

                for (Size i=0; i<n; ++i) {
                    if (w[i] > threshold) {
                        const Real u = std::inner_product(U.column_begin(i),
                                                          U.column_end(i),
                                                          b.begin(), 0.0)/w[i];

                        for (Size j=0; j<n; ++j) {
                            xr[j]  +=u*V[j][i];
                        }
                    }
                }

                if (norm(xr-x) > tol) {
                    BOOST_FAIL("least square solution does not match (norm = "
                               << norm(x - xr) << ")");

                }
            }
        }
    }
}

void MatricesTest::testInverse() {

    BOOST_TEST_MESSAGE("Testing LU inverse calculation...");

    setup();

    Real tol = 1.0e-12;
    Matrix testMatrices[] = { M1, M2, I, M5 };

    for (Size j = 0; j < LENGTH(testMatrices); j++) {
        const Matrix& A = testMatrices[j];
        const Matrix invA = inverse(A);

        const Matrix I1 = invA*A;
        const Matrix I2 = A*invA;

        Matrix eins(A.rows(), A.rows(), 0.0);
        for (Size i=0; i < A.rows(); ++i) eins[i][i] = 1.0;

        if (norm(I1 - eins) > tol)
            BOOST_FAIL("inverse(A)*A does not recover unit matrix (norm = "
                       << norm(I1-eins) << ")");

        if (norm(I2 - eins) > tol)
            BOOST_FAIL("A*inverse(A) does not recover unit matrix (norm = "
                       << norm(I1-eins) << ")");
    }
}

void MatricesTest::testDeterminant() {

    BOOST_TEST_MESSAGE("Testing LU determinant calculation...");

    setup();
    Real tol = 1e-10;

    Matrix testMatrices[] = {M1, M2, M5, M6, I};
    // expected results calculated with octave
    Real expected[] = { 0.044, -0.012, 5.0, 5.7621e-11, 1.0};

    for (Size j=0; j<LENGTH(testMatrices); ++j) {
        const Real calculated = determinant(testMatrices[j]);
        if (std::fabs(expected[j] - calculated) > tol)
            BOOST_FAIL("determinant calculation failed "
                       << "\n matrix     :\n" << testMatrices[j]
                       << "\n calculated : " << calculated
                       << "\n expected   : " << expected[j]);
    }

    MersenneTwisterUniformRng rng(1234);
    for (Size j=0; j<100; ++j) {
        Matrix m(3, 3, 0.0);
        for (Matrix::iterator iter = m.begin(); iter != m.end(); ++iter)
            *iter = rng.next().value;

        if (!(j%3)) {
            // every third matrix is a singular matrix
            Size row = Size(3*rng.next().value);
            std::fill(m.row_begin(row), m.row_end(row), 0.0);
        }

        Real a=m[0][0];
        Real b=m[0][1];
        Real c=m[0][2];
        Real d=m[1][0];
        Real e=m[1][1];
        Real f=m[1][2];
        Real g=m[2][0];
        Real h=m[2][1];
        Real i=m[2][2];

        const Real expected = a*e*i+b*f*g+c*d*h-(g*e*c+h*f*a+i*d*b);
        const Real calculated = determinant(m);

        if (std::fabs(expected-calculated) > tol)
            BOOST_FAIL("determinant calculation failed "
                       << "\n matrix     :\n" << m
                       << "\n calculated : " << calculated
                       << "\n expected   : " << expected);
    }
}

void MatricesTest::testOrthogonalProjection() {
    BOOST_TEST_MESSAGE("Testing orthogonal projections...");

    Size dimension = 1000;
    Size numberVectors = 50;
    Real multiplier = 100;
    Real tolerance = 1e-6;
    unsigned long seed = 1;

    Real errorAcceptable = 1E-11;

    Matrix test(numberVectors,dimension);

    MersenneTwisterUniformRng rng(seed);

    for (Size i=0; i < numberVectors; ++i)
        for (Size j=0; j < dimension; ++j)
            test[i][j] = rng.next().value;

    OrthogonalProjections projector(test,
                                    multiplier,
                                    tolerance  );

    Size numberFailures =0;
    Size failuresTwo=0;

    for (Size i=0; i < numberVectors; ++i)
    {
        // check output vector i is orthogonal to all other vectors

        if (projector.validVectors()[i])
        {
            for (Size j=0; j < numberVectors; ++j)
                  if (projector.validVectors()[j] && i != j)
                  {
                      Real dotProduct=0.0;
                      for (Size k=0; k < dimension; ++k)
                          dotProduct += test[j][k]*projector.GetVector(i)[k];

                      if (fabs(dotProduct) > errorAcceptable)
                          ++numberFailures;

                  }

           Real innerProductWithOriginal =0.0;
           Real normSq =0.0;

           for (Size j=0; j < dimension; ++j)
           {
                innerProductWithOriginal +=   projector.GetVector(i)[j]*test[i][j];
                normSq += test[i][j]*test[i][j];
           }

           if (fabs(innerProductWithOriginal-normSq) > errorAcceptable)
               ++failuresTwo;

        }

    }

    if (numberFailures > 0 || failuresTwo >0)
        BOOST_FAIL("OrthogonalProjections test failed with " << numberFailures << " failures  of orthogonality and "
                    << failuresTwo << " failures of projection size.");

}



test_suite* MatricesTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Matrix tests");

    suite->add(QUANTLIB_TEST_CASE(&MatricesTest::testOrthogonalProjection));
    suite->add(QUANTLIB_TEST_CASE(&MatricesTest::testEigenvectors));
    suite->add(QUANTLIB_TEST_CASE(&MatricesTest::testSqrt));
    suite->add(QUANTLIB_TEST_CASE(&MatricesTest::testSVD));
    suite->add(QUANTLIB_TEST_CASE(&MatricesTest::testHighamSqrt));
    suite->add(QUANTLIB_TEST_CASE(&MatricesTest::testQRDecomposition));
    suite->add(QUANTLIB_TEST_CASE(&MatricesTest::testQRSolve));
    #if !defined(QL_NO_UBLAS_SUPPORT)
    suite->add(QUANTLIB_TEST_CASE(&MatricesTest::testInverse));
    suite->add(QUANTLIB_TEST_CASE(&MatricesTest::testDeterminant));
    #endif
    return suite;
}

