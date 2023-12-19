/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

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
#include <ql/math/matrixutilities/tqreigendecomposition.hpp>

using namespace QuantLib;
using boost::unit_test_framework::test_suite;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(TqrEigenDecompositionTests)

BOOST_AUTO_TEST_CASE(testEigenValueDecomposition) {
    BOOST_TEST_MESSAGE("Testing TQR eigenvalue decomposition...");

    Array diag(5);
    Array sub(4,1);
    diag[0]=11; diag[1]=7; diag[2]=6; diag[3]=2; diag[4]=0;
    Real ev[5] = {11.2467832217139119,
                  7.4854967362908535,
                  5.5251516080277518,
                  2.1811760273123308,
                  -0.4386075933448487};

    TqrEigenDecomposition tqre(diag, sub,
                               TqrEigenDecomposition::WithoutEigenVector);
    for (Size i=0; i < diag.size(); ++i) {
        const Real expected(ev[i]);
        const Real calculated(tqre.eigenvalues()[i]);
        Real tolerance = 1.0e-10;
        if (std::fabs(expected-calculated) > tolerance) {
            BOOST_FAIL(std::string("wrong eigenvalue \n")
                       << "calculated: "
                       << calculated
                       <<" expected  : "
                       << expected);
        }
    }
}

BOOST_AUTO_TEST_CASE(testZeroOffDiagEigenValues) {
    BOOST_TEST_MESSAGE("Testing TQR zero-off-diagonal eigenvalues...");

    Array diag(5);
    Array sub(4,1);
    sub[0] =sub[2]=0;
    diag[0]=12; diag[1]=9; diag[2]=6; diag[3]=3; diag[4]=0;

    TqrEigenDecomposition tqre1(diag, sub);

    sub[0]=sub[2]=1e-14;
    TqrEigenDecomposition tqre2(diag, sub);

    for (Size i=0; i < diag.size(); ++i) {
        const Real expected(tqre2.eigenvalues()[i]);
        const Real calculated(tqre1.eigenvalues()[i]);
        Real tolerance = 1.0e-10;
        if (std::fabs(expected-calculated) > tolerance) {
            BOOST_FAIL(std::string("wrong eigenvalue \n")
                       << "calculated: "
                       << calculated
                       << " expected  : "
                       << expected);
        }
    }
}

BOOST_AUTO_TEST_CASE(testEigenVectorDecomposition) {
    BOOST_TEST_MESSAGE("Testing TQR eigenvector decomposition...");

    Array diag(2,1);
    Array sub(1,1);

    TqrEigenDecomposition tqre(diag, sub);
    Real tolerance = 1.0e-10;

    if (std::fabs(0.25 + tqre.eigenvectors()[0][0]
                       * tqre.eigenvectors()[0][1]
                       * tqre.eigenvectors()[1][0]
                       * tqre.eigenvectors()[1][1]) > tolerance) {
        BOOST_FAIL("wrong eigenvector");
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
