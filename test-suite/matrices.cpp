/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2007, 2008 Klaus Spanderen
 Copyright (C) 2007 Neil Firth
 Copyright (C) 2016 Peter Caspers

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/experimental/math/moorepenroseinverse.hpp>
#include <ql/math/matrix.hpp>
#include <ql/math/matrixutilities/basisincompleteordered.hpp>
#include <ql/math/matrixutilities/bicgstab.hpp>
#include <ql/math/matrixutilities/choleskydecomposition.hpp>
#include <ql/math/matrixutilities/gmres.hpp>
#include <ql/math/matrixutilities/householder.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/math/matrixutilities/qrdecomposition.hpp>
#include <ql/math/matrixutilities/svd.hpp>
#include <ql/math/matrixutilities/symmetricschurdecomposition.hpp>
#include <ql/math/matrixutilities/sparsematrix.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <cmath>
#include <iterator>
#include <utility>
#include <numeric>

using namespace QuantLib;
using namespace boost::unit_test_framework;

using std::fabs;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(MatricesTests)

#ifdef __cpp_concepts
static_assert(std::random_access_iterator<Matrix::column_iterator>);
static_assert(std::random_access_iterator<Matrix::const_column_iterator>);
static_assert(std::random_access_iterator<Matrix::reverse_column_iterator>);
static_assert(std::random_access_iterator<Matrix::const_reverse_column_iterator>);
#endif

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

class MatrixMult {
  public:
    explicit MatrixMult(Matrix m) : m_(std::move(m)) {}
    Array operator()(const Array& x) const {
        return m_ * x;
    }

  private:
    const Matrix m_;
};

Real norm2(const Array& x) {
    return std::sqrt(DotProduct(x,x));
}


BOOST_AUTO_TEST_CASE(testEigenvectors) {

    BOOST_TEST_MESSAGE("Testing eigenvalues and eigenvectors calculation...");

    setup();

    Matrix testMatrices[] = { M1, M2 };

    for (auto& M : testMatrices) {

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

BOOST_AUTO_TEST_CASE(testSqrt) {

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

BOOST_AUTO_TEST_CASE(testHighamSqrt) {
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

BOOST_AUTO_TEST_CASE(testSVD) {

    BOOST_TEST_MESSAGE("Testing singular value decomposition...");

    setup();

    Real tol = 1.0e-12;
    Matrix testMatrices[] = { M1, M2, M3, M4 };

    for (auto& A : testMatrices) {
        // m >= n required (rows >= columns)
        SVD svd(A);
        // U is m x n
        const Matrix& U = svd.U();
        // s is n long
        Array s = svd.singularValues();
        // S is n x n
        Matrix S = svd.S();
        // V is n x n
        const Matrix& V = svd.V();

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

BOOST_AUTO_TEST_CASE(testQRDecomposition) {

    BOOST_TEST_MESSAGE("Testing QR decomposition...");

    setup();

    Real tol = 1.0e-12;
    Matrix testMatrices[] = { M1, M2, I,
                              M3, transpose(M3), M4, transpose(M4), M5 };

    for (const auto& A : testMatrices) {
        Matrix Q, R;
        bool pivot = true;
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

BOOST_AUTO_TEST_CASE(testQRSolve) {

    BOOST_TEST_MESSAGE("Testing QR solve...");

    setup();

    Real tol = 1.0e-12;
    MersenneTwisterUniformRng rng(1234);
    Matrix bigM(50, 100, 0.0);
    for (Size i=0; i < std::min(bigM.rows(), bigM.columns()); ++i) {
        bigM[i][i] = i+1.0;
    }

    Matrix randM(50, 200);
    for (Size i=0; i < randM.rows(); ++i)
        for (Size j=0; j < randM.columns(); ++j)
            randM[i][j] = rng.next().value;

    Matrix testMatrices[] = {M1, M2, M3, transpose(M3),
                              M4, transpose(M4), M5, I, M7,
                              bigM, transpose(bigM),
                              randM, transpose(randM) };

    for (const auto& A : testMatrices) {
        Array b(A.rows());

        for (Size k=0; k < 10; ++k) {
            for (Real& iter : b) {
                iter = rng.next().value;
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
                                                          b.begin(), Real(0.0))/w[i];

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

BOOST_AUTO_TEST_CASE(testInverse) {

    BOOST_TEST_MESSAGE("Testing LU inverse calculation...");

    setup();

    Real tol = 1.0e-12;
    Matrix testMatrices[] = { M1, M2, I, M5 };

    for (const auto& A : testMatrices) {
        const Matrix invA = inverse(A);

        const Matrix I1 = invA*A;
        const Matrix I2 = A*invA;

        Matrix identity(A.rows(), A.rows(), 0.0);
        for (Size i=0; i < A.rows(); ++i) identity[i][i] = 1.0;

        if (norm(I1 - identity) > tol)
            BOOST_FAIL("inverse(A)*A does not recover unit matrix (norm = "
                       << norm(I1-identity) << ")");

        if (norm(I2 - identity) > tol)
            BOOST_FAIL("A*inverse(A) does not recover unit matrix (norm = "
                       << norm(I1-identity) << ")");
    }
}

BOOST_AUTO_TEST_CASE(testDeterminant) {

    BOOST_TEST_MESSAGE("Testing LU determinant calculation...");

    setup();
    Real tol = 1e-10;

    Matrix testMatrices[] = {M1, M2, M5, M6, I};
    // expected results calculated with octave
    Real expected[] = { 0.044, -0.012, 5.0, 5.7621e-11, 1.0};

    for (Size j=0; j<std::size(testMatrices); ++j) {
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
        for (Real& iter : m)
            iter = rng.next().value;

        if ((j % 3) == 0U) {
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

BOOST_AUTO_TEST_CASE(testOrthogonalProjection) {
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

BOOST_AUTO_TEST_CASE(testCholeskyDecomposition) {

    BOOST_TEST_MESSAGE("Testing Cholesky Decomposition...");

    // This test case fails prior to release 1.8

    // The eigenvalues of this matrix are
    // 0.0438523; 0.0187376; 0.000245617; 0.000127656; 8.35899e-05; 6.14215e-05;
    // 1.94241e-05; 1.14417e-06; 9.79481e-18; 1.31141e-18; 5.81155e-19

    Real tmp[11][11] = {
        {6.4e-05, 5.28e-05, 2.28e-05, 0.00032, 0.00036, 6.4e-05,
         6.3968010664e-06, 7.2e-05, 7.19460269899e-06, 1.2e-05,
         1.19970004999e-06},
        {5.28e-05, 0.000121, 1.045e-05, 0.00044, 0.000165, 2.2e-05,
         2.19890036657e-06, 1.65e-05, 1.64876311852e-06, 1.1e-05,
         1.09972504583e-06},
        {2.28e-05, 1.045e-05, 9.025e-05, 0, 0.0001425, 9.5e-06,
         9.49525158294e-07, 2.85e-05, 2.84786356835e-06, 4.75e-06,
         4.74881269789e-07},
        {0.00032, 0.00044, 0, 0.04, 0.009, 0.0008, 7.996001333e-05, 0.0006,
         5.99550224916e-05, 0.0001, 9.99750041661e-06},
        {0.00036, 0.000165, 0.0001425, 0.009, 0.0225, 0.0003, 2.99850049987e-05,
         0.001125, 0.000112415667172, 0.000225, 2.24943759374e-05},
        {6.4e-05, 2.2e-05, 9.5e-06, 0.0008, 0.0003, 0.0001, 9.99500166625e-06,
         7.5e-05, 7.49437781145e-06, 2e-05, 1.99950008332e-06},
        {6.3968010664e-06, 2.19890036657e-06, 9.49525158294e-07,
         7.996001333e-05, 2.99850049987e-05, 9.99500166625e-06,
         9.99000583083e-07, 7.49625124969e-06, 7.49063187129e-07,
         1.99900033325e-06, 1.99850066645e-07},
        {7.2e-05, 1.65e-05, 2.85e-05, 0.0006, 0.001125, 7.5e-05,
         7.49625124969e-06, 0.000225, 2.24831334343e-05, 1.5e-05,
         1.49962506249e-06},
        {7.19460269899e-06, 1.64876311852e-06, 2.84786356835e-06,
         5.99550224916e-05, 0.000112415667172, 7.49437781145e-06,
         7.49063187129e-07, 2.24831334343e-05, 2.24662795123e-06,
         1.49887556229e-06, 1.49850090584e-07},
        {1.2e-05, 1.1e-05, 4.75e-06, 0.0001, 0.000225, 2e-05, 1.99900033325e-06,
         1.5e-05, 1.49887556229e-06, 2.5e-05, 2.49937510415e-06},
        {1.19970004999e-06, 1.09972504583e-06, 4.74881269789e-07,
         9.99750041661e-06, 2.24943759374e-05, 1.99950008332e-06,
         1.99850066645e-07, 1.49962506249e-06, 1.49850090584e-07,
         2.49937510415e-06, 2.49875036451e-07}};

    Matrix m(11,11);
    for(Size i=0;i<11;++i) {
        for(Size j=0;j<11;++j) {
            m[i][j] = tmp[i][j];
        }
    }

    Matrix c = CholeskyDecomposition(m,true);
    Matrix m2 = c * transpose(c);

    Real tol = 1.0E-12;
    for(Size i=0;i<11;++i) {
        for(Size j=0;j<11;++j) {
            if(std::isnan(m2[i][j])) {
                BOOST_FAIL("Faield to verify Cholesky decomposition at (i,j)=("
                           << i << "," << j << "), replicated value is nan");
            }
            // this does not detect nan values
            if(std::abs(m[i][j]-m2[i][j]) > tol) {
                BOOST_FAIL("Failed to verify Cholesky decomposition at (i,j)=("
                           << i << "," << j << "), original value is "
                           << m[i][j] << ", replicated value is " << m2[i][j]);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testMoorePenroseInverse) {

    BOOST_TEST_MESSAGE("Testing Moore-Penrose inverse...");

    // this is taken from
    // http://de.mathworks.com/help/matlab/ref/pinv.html
    Real tmp[8][6] = {{64, 2, 3, 61, 60, 6},    {9, 55, 54, 12, 13, 51},
                      {17, 47, 46, 20, 21, 43}, {40, 26, 27, 37, 36, 30},
                      {32, 34, 35, 29, 28, 38}, {41, 23, 22, 44, 45, 19},
                      {49, 15, 14, 52, 53, 11}, {8, 58, 59, 5, 4, 62}};
    Matrix A(8, 6);
    for (Size i = 0; i < 8; ++i) {
        for (Size j = 0; j < 6; ++j) {
            A(i, j) = tmp[i][j];
        }
    }

    Matrix P = moorePenroseInverse(A);
    Array b(8, 260.0);
    Array x = P*b;

    Real cached[6] = {1.153846153846152, 1.461538461538463, 1.384615384615384,
                      1.384615384615385, 1.461538461538462, 1.153846153846152};
    constexpr double tol = 500.0 * QL_EPSILON;

    for (Size i = 0; i < 6; ++i) {
        if (std::abs(x[i] - cached[i]) > tol) {
            BOOST_FAIL("Failed to verify minimal norm solution obtained from "
                       "Moore-Penrose-Inverse against cached results, component "
                       << i << " is " << x[i] << ", expected " << cached[i]
                       << ", difference " << x[i] - cached[i] << ", tolerance "
                       << tol);
        }
    }

    Array y = A*x;
    constexpr double tol2 = 2000.0 * QL_EPSILON;
    for (Size i = 0; i < 6; ++i) {
        if (std::abs(y[i] - 260.0) > tol2) {
            BOOST_FAIL(
                "Failed to verify minimal norm solution obtained from "
                "Moore-Penrose-Inverse when back-substituting, rhs component "
                << i << " is " << y[i] << ", expected 260.0, difference "
                << y[i] - 260.0 << ", tolerance " << tol2);
        }
    }

}

BOOST_AUTO_TEST_CASE(testIterativeSolvers) {
    BOOST_TEST_MESSAGE("Testing iterative solvers...");

    setup();

    Array b(3);
    b[0] = 1.0; b[1] = 0.5; b[2] = 3.0;

    constexpr double relTol = 1e4 * QL_EPSILON;

    const Array x = BiCGstab(MatrixMult(M1), 3, relTol).solve(b).x;
    if (norm2(M1*x-b)/norm2(b) > relTol) {
        BOOST_FAIL("Failed to calculate inverse using BiCGstab"
                << "\n  rel error     : " << norm2(M1*x-b)/norm2(b)
                << "\n  rel tolerance : " << relTol);
    }

    const GMRESResult u = GMRES(MatrixMult(M1), 3, relTol).solve(b, b);
    if (norm2(M1*u.x-b)/norm2(b) > relTol) {
        BOOST_FAIL("Failed to calculate inverse using gmres"
                << "\n  rel error     : " << norm2(M1*u.x-b)/norm2(b)
                << "\n  rel tolerance : " << relTol);
    }
    const Array errors = Array(u.errors.begin(), u.errors.end());
    for (Real error : errors) {
        const Array x = GMRES(MatrixMult(M1), 10, 1.01 * error).solve(b, b).x;

        const Real calculated = norm2(M1*x-b)/norm2(b);
        const Real expected = error;

        if (std::fabs(calculated - expected) > relTol) {
            BOOST_FAIL("Failed to calculate solution error"
                    << "\n  calculated error: " << calculated
                    << "\n  expected error  : " << expected);
        }
    }

    const Array v = GMRES(MatrixMult(M1), 1, relTol,
        MatrixMult(inverse(M1))).solve(b, b).x;

    if (norm2(M1*v-b)/norm2(b) > relTol) {
        BOOST_FAIL("Failed to calculate inverse using gmres "
                   "with exact preconditioning"
                << "\n  rel error     : " << norm2(M1*v-b)/norm2(b)
                << "\n  rel tolerance : " << relTol);
    }

    const Array w = GMRES(MatrixMult(M1), 3, relTol,
        MatrixMult(M1)).solve(b, b).x;
    if (norm2(M1*w-b)/norm2(b) > relTol) {
        BOOST_FAIL("Failed to calculate inverse using gmres "
                   "with nonsense preconditioning"
                << "\n  rel error     : " << norm2(M1*w-b)/norm2(b)
                << "\n  rel tolerance : " << relTol);
    }
}

BOOST_AUTO_TEST_CASE(testInitializers) {
    BOOST_TEST_MESSAGE("Testing matrix initializers...");

    Matrix m1 = {};
    BOOST_REQUIRE(m1.rows() == 0);
    BOOST_REQUIRE(m1.columns() == 0);

    Matrix m2 = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0}
    };
    BOOST_REQUIRE(m2.rows() == 2);
    BOOST_REQUIRE(m2.columns() == 3);
    BOOST_CHECK_EQUAL(m2(0, 0), 1.0);
    BOOST_CHECK_EQUAL(m2(0, 1), 2.0);
    BOOST_CHECK_EQUAL(m2(0, 2), 3.0);
    BOOST_CHECK_EQUAL(m2(1, 0), 4.0);
    BOOST_CHECK_EQUAL(m2(1, 1), 5.0);
    BOOST_CHECK_EQUAL(m2(1, 2), 6.0);
}


typedef std::pair< std::pair< std::vector<Size>, std::vector<Size> >,
                   std::vector<Real> > coordinate_tuple;

coordinate_tuple sparseMatrixToCoordinateTuple(const SparseMatrix& m) {
    std::vector<Size> row_idx, col_idx;
    std::vector<Real> data;
    for (auto iter1 = m.begin1(); iter1 != m.end1(); ++iter1)
        for (auto iter2 = iter1.begin(); iter2 != iter1.end(); ++iter2) {
            row_idx.push_back(iter1.index1());
            col_idx.push_back(iter2.index2());
            data.push_back(*iter2);
        }

    return std::make_pair(std::make_pair(row_idx, col_idx), data);
}


BOOST_AUTO_TEST_CASE(testSparseMatrixMemory) {

    BOOST_TEST_MESSAGE("Testing sparse matrix memory layout...");

    SparseMatrix m(8, 4);
    BOOST_CHECK_EQUAL(m.filled1(), 1);
    BOOST_CHECK_EQUAL(m.size1(), 8);
    BOOST_CHECK_EQUAL(m.size2(), 4);
    BOOST_CHECK_EQUAL(std::distance(m.begin1(), m.end1()), m.size1());

    auto coords = sparseMatrixToCoordinateTuple(m);
    BOOST_CHECK_EQUAL(coords.first.first.size(), 0);

    m(3, 1) = 42;
    coords = sparseMatrixToCoordinateTuple(m);
    BOOST_CHECK_EQUAL(std::distance(m.begin1(), m.end1()), m.size1());
    BOOST_CHECK_EQUAL(coords.first.first.size(), 1);
    BOOST_CHECK_EQUAL(coords.first.first[0], 3);
    BOOST_CHECK_EQUAL(coords.first.second[0], 1);
    BOOST_CHECK_EQUAL(coords.second[0], 42);

    m(1, 2) = 6;
    coords = sparseMatrixToCoordinateTuple(m);
    BOOST_CHECK_EQUAL(coords.first.first.size(), 2);
    BOOST_CHECK_EQUAL(coords.first.first[0], 1);
    BOOST_CHECK_EQUAL(coords.first.second[0], 2);
    BOOST_CHECK_EQUAL(coords.second[0], 6);

    Array x{1, 2, 3, 4};
    Array y = prod(m, x);
    BOOST_CHECK_EQUAL(y, Array({0, 18, 0, 84}));

    m(3, 2) = 43;
    coords = sparseMatrixToCoordinateTuple(m);
    BOOST_CHECK_EQUAL(coords.first.first.size(), 3);
    BOOST_CHECK_EQUAL(coords.first.first[2], 3);
    BOOST_CHECK_EQUAL(coords.first.second[2], 2);
    BOOST_CHECK_EQUAL(coords.second[2], 43);

    m(7, 3) = 44;
    coords = sparseMatrixToCoordinateTuple(m);
    BOOST_CHECK_EQUAL(coords.first.first.size(), 4);
    BOOST_CHECK_EQUAL(coords.first.first[3], 7);
    BOOST_CHECK_EQUAL(coords.first.second[3], 3);
    BOOST_CHECK_EQUAL(coords.second[3], 44);

    Size entries(0);
    for (auto iter1 = m.begin1(); iter1 != m.end1(); ++iter1)
        entries+=std::distance(iter1.begin(), iter1.end());

    BOOST_CHECK_EQUAL(entries, 4);

}

#define QL_CHECK_CLOSE_MATRIX_TOL(actual, expected, tol)                    \
    BOOST_REQUIRE(actual.rows() == expected.rows() &&                       \
                  actual.columns() == expected.columns());                  \
    for (auto i = 0u; i < actual.rows(); i++) {                             \
        for (auto j = 0u; j < actual.columns(); j++) {                      \
            QL_CHECK_CLOSE(actual(i, j), expected(i, j), tol);              \
        }                                                                   \
    }                                                                       \


#define QL_CHECK_CLOSE_MATRIX(actual, expected)                             \
        QL_CHECK_CLOSE_MATRIX_TOL(actual, expected, 100 * QL_EPSILON)       \


BOOST_AUTO_TEST_CASE(testOperators) {

    BOOST_TEST_MESSAGE("Testing matrix operators...");

    auto get_matrix = []() {
        return Matrix(2, 3, 4.0);
    };

    const auto m = get_matrix();

    const auto negative = Matrix(2, 3, -4.0);
    const auto lvalue_negative = -m;
    const auto rvalue_negative = -get_matrix();

    QL_CHECK_CLOSE_MATRIX(lvalue_negative, negative);
    QL_CHECK_CLOSE_MATRIX(rvalue_negative, negative);

    const auto matrix_sum = Matrix(2, 3, 8.0);
    const auto lvalue_lvalue_sum = m + m;
    const auto lvalue_rvalue_sum = m + get_matrix();
    const auto rvalue_lvalue_sum = get_matrix() + m;
    const auto rvalue_rvalue_sum = get_matrix() + get_matrix();

    QL_CHECK_CLOSE_MATRIX(lvalue_lvalue_sum, matrix_sum);
    QL_CHECK_CLOSE_MATRIX(lvalue_rvalue_sum, matrix_sum);
    QL_CHECK_CLOSE_MATRIX(rvalue_lvalue_sum, matrix_sum);
    QL_CHECK_CLOSE_MATRIX(rvalue_rvalue_sum, matrix_sum);

    const auto matrix_difference = Matrix(2, 3, 0.0);
    const auto lvalue_lvalue_difference = m - m;  // NOLINT(misc-redundant-expression)
    const auto lvalue_rvalue_difference = m - get_matrix();
    const auto rvalue_lvalue_difference = get_matrix() - m;
    const auto rvalue_rvalue_difference = get_matrix() - get_matrix();

    QL_CHECK_CLOSE_MATRIX(lvalue_lvalue_difference, matrix_difference);
    QL_CHECK_CLOSE_MATRIX(lvalue_rvalue_difference, matrix_difference);
    QL_CHECK_CLOSE_MATRIX(rvalue_lvalue_difference, matrix_difference);
    QL_CHECK_CLOSE_MATRIX(rvalue_rvalue_difference, matrix_difference);

    const auto scalar_product = Matrix(2, 3, 6.0);
    const auto lvalue_real_product = m * 1.5;
    const auto rvalue_real_product = get_matrix() * 1.5;
    const auto real_lvalue_product = 1.5 * m;
    const auto real_rvalue_product = 1.5 * get_matrix();

    QL_CHECK_CLOSE_MATRIX(lvalue_real_product, scalar_product);
    QL_CHECK_CLOSE_MATRIX(rvalue_real_product, scalar_product);
    QL_CHECK_CLOSE_MATRIX(real_lvalue_product, scalar_product);
    QL_CHECK_CLOSE_MATRIX(real_rvalue_product, scalar_product);

    const auto scalar_quotient = Matrix(2, 3, 2.0);
    const auto lvalue_real_quotient = m / 2.0;
    const auto rvalue_real_quotient = get_matrix() / 2.0;

    QL_CHECK_CLOSE_MATRIX(lvalue_real_quotient, scalar_quotient);
    QL_CHECK_CLOSE_MATRIX(rvalue_real_quotient, scalar_quotient);
}

namespace MatrixTests {
    Matrix createTestCorrelationMatrix(Size n) {
        Matrix rho(n, n);
        for (Size i=0; i < n; ++i)
            for (Size j=i; j < n; ++j)
                rho[i][j] = rho[j][i] =
                    std::exp(-0.1*std::abs(Real(i)-Real(j)) - ((i!=j) ? 0.02*(i+j): 0.0));

        return rho;
    }
}

BOOST_AUTO_TEST_CASE(testPrincipalMatrixSqrt) {
    BOOST_TEST_MESSAGE("Testing principal matrix pseudo sqrt...");

    std::vector<Real> dims = {1, 4, 10, 40};
    for (auto n: dims) {
        const Matrix rho = MatrixTests::createTestCorrelationMatrix(n);
        const Matrix sqrtRho = pseudoSqrt(rho, SalvagingAlgorithm::Principal);

        // matrix is symmetric
        QL_CHECK_CLOSE_MATRIX_TOL(sqrtRho, transpose(sqrtRho), 1e3*QL_EPSILON);

        // matrix is square root of original matrix
        QL_CHECK_CLOSE_MATRIX_TOL((sqrtRho*sqrtRho), rho, 1e5*QL_EPSILON);
    }
}


BOOST_AUTO_TEST_CASE(testCholeskySolverFor) {
    BOOST_TEST_MESSAGE("Testing CholeskySolverFor...");

    MersenneTwisterUniformRng rng(1234);

    std::vector<Real> dims = {1, 4, 10, 25, 50};
    for (auto n: dims) {

        Array b(n);
        for (Size i=0; i < n; ++i)
            b[i] = rng.nextReal();

        const Matrix rho = MatrixTests::createTestCorrelationMatrix(n);
        const Array x = CholeskySolveFor(CholeskyDecomposition(rho), b);

        const Array diff = Abs(rho*x - b);

        BOOST_CHECK_SMALL(std::sqrt(DotProduct(diff, diff)), 20*std::sqrt(n)*QL_EPSILON);
    }
}


BOOST_AUTO_TEST_CASE(testCholeskySolverForIncomplete) {
    BOOST_TEST_MESSAGE("Testing CholeskySolverFor with incomplete matrix...");

    const Size n = 4;

    Matrix rho(n, n, 0.0);
    rho[0][0] = rho[1][1] = Real(1);
    rho[0][1] = rho[1][0] = 0.9;

    const Matrix L = CholeskyDecomposition(rho, true);
    QL_CHECK_CLOSE_MATRIX((L*transpose(L)), rho);
}

namespace {
	void QL_CHECK_CLOSE_ARRAY_TOL(
		const Array& actual, const Array& expected, Real tol) {
		BOOST_REQUIRE(actual.size() == expected.size());
		for (auto i = 0u; i < actual.size(); i++) {
			BOOST_CHECK_SMALL(actual[i] - expected[i], tol);
		}
	}
}

BOOST_AUTO_TEST_CASE(testHouseholderTransformation) {
    BOOST_TEST_MESSAGE("Testing Householder Transformation...");

    MersenneTwisterUniformRng rng(1234);

    const auto I = [](Size i) -> Matrix {
    	Matrix id(i, i, 0.0);
    	for (Size j=0; j < i; ++j)
    		id[j][j] = 1.0;

    	return id;
    };

    for (Size i=1; i < 10; ++i) {
    	Array v(i), x(i);
    	for (Size j=0; j < i; ++j) {
    		v[j] = rng.nextReal()-0.5;
    		x[j] = rng.nextReal()-0.5;
    	}

    	const Array expected = (I(i)- 2.0*outerProduct(v, v))*x;
    	const Array calculated = HouseholderTransformation(v)(x);
    	QL_CHECK_CLOSE_ARRAY_TOL(calculated, expected, 1e4*QL_EPSILON);
    }
}

BOOST_AUTO_TEST_CASE(testHouseholderReflection) {
    BOOST_TEST_MESSAGE("Testing Householder Reflection...");

    const Real tol=1e4*QL_EPSILON;

    const auto e = [](Size n, Size m=0) -> Array {
    	Array e(n, 0.0);
    	e[m] = 1.0;
    	return e;
    };

    for (Size i=0; i < 5; ++i) {
		QL_CHECK_CLOSE_ARRAY_TOL(
			HouseholderReflection(e(5))(e(5, i)), e(5), tol);
		QL_CHECK_CLOSE_ARRAY_TOL(
			HouseholderReflection(e(5))(M_PI*e(5, i)), M_PI*e(5), tol);
		QL_CHECK_CLOSE_ARRAY_TOL(
			HouseholderReflection(e(5))(
				e(5, i) + e(5)),
				((i==0)? 2.0 : M_SQRT2)*e(5), tol);
    }

    // limits
	for (Real x=10; x > 1e-50; x*=0.1) {
        QL_CHECK_CLOSE_ARRAY_TOL(
            HouseholderReflection(e(3))(
                Array({10.0, x, 0})),
                std::sqrt(10.0*10.0+x*x)*e(3), tol
        );

        QL_CHECK_CLOSE_ARRAY_TOL(
            HouseholderReflection(e(3))(
                Array({10.0, x, 1e-3})),
                std::sqrt(10.0*10.0+x*x+1e-3*1e-3)*e(3), tol
        );
	}

    MersenneTwisterUniformRng rng(1234);

    for (Size i=0; i < 100; ++i) {
        const Array v = Array({rng.nextReal(), rng.nextReal(), rng.nextReal()}) - 0.5;
        const Matrix u = HouseholderTransformation(v / Norm2(v)).getMatrix();

        const Array eu = u*e(3, i%3);
        const Array a = Array({rng.nextReal(), rng.nextReal(), rng.nextReal()}) - 0.5;

        const Matrix H = HouseholderTransformation(
            HouseholderReflection(eu).reflectionVector(a)).getMatrix();

        QL_CHECK_CLOSE_ARRAY_TOL(u*H*a, Norm2(a)*e(3, i%3), tol);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
