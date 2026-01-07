/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl
 Copyright (C) 2011 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/methods/finitedifferences/dzero.hpp>
#include <ql/methods/finitedifferences/dplusdminus.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(OperatorTests)

BOOST_AUTO_TEST_CASE(testTridiagonal) {

    BOOST_TEST_MESSAGE("Testing tridiagonal operator...");

    Size n = 8; // can use 3 for easier debugging

    TridiagonalOperator T(n);
    T.setFirstRow(1.0, 2.0);
    T.setMidRows( 0.0, 2.0, 0.0);
    T.setLastRow(      2.0, 1.0);

    Array original(n, 1.0);

    Array intermediate = T.applyTo(original);

    Array final(intermediate);
    T.solveFor(final, final);
    for (Size i=0; i<n; ++i) {
        if (final[i]!=original[i])
            BOOST_FAIL("\n applyTo + solveFor does not equal identity:"
                       "\n            original vector: " << original <<
                       "\n         transformed vector: " << intermediate <<
                       "\n inverse transformed vector: " << final);
    }

    final = Array(n, 0.0);
    Array temp(intermediate);
    T.solveFor(temp, final);
    for (Size i=0; i<n; ++i) {
        if (temp[i]!=intermediate[i])
            BOOST_FAIL("\n solveFor altered rhs:"
                       "\n            original vector: " << original <<
                       "\n         transformed vector: " << intermediate <<
                       "\n altered transformed vector: " << temp <<
                       "\n inverse transformed vector: " << final);
    }
    for (Size i=0; i<n; ++i) {
        if (final[i]!=original[i])
            BOOST_FAIL("\n applyTo + solveFor does not equal identity:"
                       "\n            original vector: " << original <<
                       "\n         transformed vector: " << intermediate <<
                       "\n inverse transformed vector: " << final);
    }

    final = T.solveFor(temp);
    for (Size i=0; i<n; ++i) {
        if (temp[i]!=intermediate[i])
            BOOST_FAIL("\n solveFor altered rhs:"
                       "\n            original vector: " << original <<
                       "\n         transformed vector: " << intermediate <<
                       "\n altered transformed vector: " << temp <<
                       "\n inverse transformed vector: " << final);
    }
    for (Size i=0; i<n; ++i) {
        if (final[i]!=original[i])
            BOOST_FAIL("\n applyTo + solveFor does not equal identity:"
                       "\n            original vector: " << original <<
                       "\n         transformed vector: " << intermediate <<
                       "\n inverse transformed vector: " << final);
    }

    Real delta, error = 0.0, tolerance = 1e-9;
    final = T.SOR(temp, tolerance);
    for (Size i=0; i<n; ++i) {
        delta = final[i]-original[i];
        error += delta * delta;
        if (temp[i]!=intermediate[i])
            BOOST_FAIL("\n SOR altered rhs:"
                       "\n            original vector: " << original <<
                       "\n         transformed vector: " << intermediate <<
                       "\n altered transformed vector: " << temp <<
                       "\n inverse transformed vector: " << final);
    }
    if (error>tolerance)
        BOOST_FAIL("\n applyTo + SOR does not equal identity:"
                   "\n            original vector: " << original <<
                   "\n         transformed vector: " << intermediate <<
                   "\n inverse transformed vector: " << final <<
                   "\n                      error: " << error <<
                   "\n                  tolerance: " << tolerance);
}

BOOST_AUTO_TEST_CASE(testConsistency) {

    BOOST_TEST_MESSAGE("Testing differential operators...");

    Real average = 0.0, sigma = 1.0;

    NormalDistribution normal(average,sigma);
    CumulativeNormalDistribution cum(average,sigma);

    Real xMin = average - 4*sigma,
         xMax = average + 4*sigma;
    Size N = 10001;
    Real h = (xMax-xMin)/(N-1);

    Array x(N), y(N), yi(N), yd(N), temp(N), diff(N);

    Size i;
    for (i=0; i<N; i++)
        x[i] = xMin+h*i;
    std::transform(x.begin(),x.end(),y.begin(),normal);
    std::transform(x.begin(),x.end(),yi.begin(),cum);
    for (i=0; i<x.size(); i++)
        yd[i] = normal.derivative(x[i]);

    // define the differential operators
    DZero D(N,h);
    DPlusDMinus D2(N,h);

    // check that the derivative of cum is Gaussian
    temp = D.applyTo(yi);
    std::transform(y.begin(), y.end(), temp.begin(), diff.begin(), std::minus<>());
    Real e = norm(diff.begin(), diff.end(), h);
    if (e > 1.0e-6) {
        BOOST_FAIL("norm of 1st derivative of cum minus Gaussian: " << e
                   << "\ntolerance exceeded");
    }

    // check that the second derivative of cum is normal.derivative
    temp = D2.applyTo(yi);
    std::transform(yd.begin(), yd.end(), temp.begin(), diff.begin(), std::minus<>());
    e = norm(diff.begin(), diff.end(), h);
    if (e > 1.0e-4) {
        BOOST_FAIL("norm of 2nd derivative of cum minus Gaussian derivative: "
                   << e << "\ntolerance exceeded");
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
