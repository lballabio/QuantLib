/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Liquidnet Holdings, Inc.

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

#include "autocovariances.hpp"
#include "utilities.hpp"
#include <ql/math/autocovariance.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;
using namespace std;

void AutocovariancesTest::testConvolutions() {
    BOOST_TEST_MESSAGE("Testing convolutions...");
    Array x(10, 1, 1);
    Array conv(6);
    convolutions(x.begin(), x.end(), conv.begin(), 5);
    Real expected[] = { 385, 330, 276, 224, 175, 130 };
    Array delta = conv - Array(expected, expected+6);
    if (DotProduct(delta, delta) > 1.0e-6)
        BOOST_ERROR("Convolution: \n"
                    << std::setprecision(4) << std::scientific
                    << "    calculated:   " << conv << "\n"
                    << "    expected:     " << Array(expected, expected+6));
}

void AutocovariancesTest::testAutoCovariances() {
    BOOST_TEST_MESSAGE("Testing auto-covariances...");
    Array x(10, 1, 1);
    Array acovf(6);
    Real mean = autocovariances(x.begin(), x.end(), acovf.begin(), 5, false);
    Real expected[] = { 8.25, 6.416667, 4.25, 1.75, -1.08333, -4.25 };
    if (std::fabs(mean-5.5) > 1.0e-6) {
        BOOST_ERROR("Mean: \n"
                    << "    calculated:   " << mean << "\n"
                    << "    expected:     " << 5.5);
    }
    Array delta = acovf - Array(expected, expected+6);
    if (DotProduct(delta, delta) > 1.0e-6)
        BOOST_ERROR("Autocovariances: \n"
                    << std::setprecision(4) << std::scientific
                    << "    calculated:   " << acovf << "\n"
                    << "    expected:     " << Array(expected, expected+6));
}

void AutocovariancesTest::testAutoCorrelations() {
    BOOST_TEST_MESSAGE("Testing auto-correlations...");
    Array x(10, 1, 1);
    Array acorf(6);
    Real mean = autocorrelations(x.begin(), x.end(), acorf.begin(), 5, true);
    Real expected[] = { 9.166667, 0.77777778, 0.51515152,
                        0.21212121, -0.13131313, -0.51515152 };
    if (std::fabs(mean-5.5) > 1.0e-6) {
        BOOST_ERROR("Mean: \n"
                    << "    calculated:   " << mean << "\n"
                    << "    expected:     " << 5.5);
    }
    Array delta = acorf - Array(expected, expected+6);
    if (DotProduct(delta, delta) > 1.0e-6)
        BOOST_ERROR("Autocovariances: \n"
                    << std::setprecision(4) << std::scientific
                    << "    calculated:   " << acorf << "\n"
                    << "    expected:     " << Array(expected, expected+6));
    delta = x - Array(10, -4.5, 1);
    if (DotProduct(delta, delta) > 1.0e-6)
        BOOST_ERROR("Centering: \n"
                    << std::setprecision(4) << std::scientific
                    << "    calculated:   " << x << "\n"
                    << "    expected:     " << Array(10, -4.5, 1));
}

test_suite* AutocovariancesTest::suite() {
    auto* suite = BOOST_TEST_SUITE("auto-covariance tests");
    suite->add(QUANTLIB_TEST_CASE(&AutocovariancesTest::testConvolutions));
    suite->add(QUANTLIB_TEST_CASE(&AutocovariancesTest::testAutoCovariances));
    suite->add(QUANTLIB_TEST_CASE(&AutocovariancesTest::testAutoCorrelations));
    return suite;
}

