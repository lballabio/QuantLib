/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Klaus Spanderen

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

#include <ql/math/matrix.hpp>
#include <ql/math/factorial.hpp>
#include <ql/methods/finitedifferences/operators/numericaldifferentiation.hpp>
#include <cmath>
#include <algorithm>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {
    bool isTheSame(Real a, Real b) {
        constexpr double eps = 500 * QL_EPSILON;

        if (std::fabs(b) < QL_EPSILON)
            return std::fabs(a) < eps;
        else
            return std::fabs((a - b)/b) < eps;
    }

    void checkTwoArraysAreTheSame(const Array& calculated,
                                  const Array& expected) {
        bool correct = (calculated.size() == expected.size())
            && std::equal(calculated.begin(), calculated.end(),
                          expected.begin(), isTheSame);

        if (!correct) {
            BOOST_FAIL("Failed to reproduce expected array"
                        << "\n    calculated: " << calculated
                        << "\n    expected:   " << expected
                        << "\n    difference: " << expected - calculated);
        }
    }

    void singleValueTest(const std::string& comment,
                         Real calculated, Real expected, Real tol) {
        if (std::fabs(calculated - expected) > tol)
            BOOST_FAIL("Failed to reproduce " << comment
                        << " order derivative"
                        << "\n    calculated: " << calculated
                        << "\n      expected: " << expected
                        << "\n     tolerance: " << tol
                        << "\n    difference: "
                        << expected - calculated);
    }
}

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(NumericalDifferentiationTest)

BOOST_AUTO_TEST_CASE(testTabulatedCentralScheme) {
    BOOST_TEST_MESSAGE("Testing numerical differentiation "
                       "using the central scheme...");
    const ext::function<Real(Real)> f;

    const NumericalDifferentiation::Scheme central
        = NumericalDifferentiation::Central;

    // see http://en.wikipedia.org/wiki/Finite_difference_coefficient
    checkTwoArraysAreTheSame(
        NumericalDifferentiation(f, 1, 1.0, 3, central).weights(),
        {-0.5, 0.0, 0.5});

    checkTwoArraysAreTheSame(
        NumericalDifferentiation(f, 1, 0.5, 3, central).weights(),
        {-1.0, 0.0, 1.0});

    checkTwoArraysAreTheSame(
        NumericalDifferentiation(f, 1, 0.25, 7, central).weights(),
        {-4/60.0, 12/20.0, -12/4.0, 0.0, 12/4.0, -12/20.0, 4/60.0});

    checkTwoArraysAreTheSame(
        NumericalDifferentiation(f, 4, std::pow(0.5, 0.25), 9, central).weights(),
        {14/240.0, -4/5.0, 338/60.0, -244/15.0, 182/8.0, -244/15.0, 338/60.0, -4/5.0, 14/240.0});

    checkTwoArraysAreTheSame(
        NumericalDifferentiation(f, 1, 0.5, 7, central).offsets(),
        {-1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5});
}

BOOST_AUTO_TEST_CASE(testTabulatedBackwardScheme) {
    BOOST_TEST_MESSAGE("Testing numerical differentiation "
                       "using the backward scheme...");
    const ext::function<Real(Real)> f;

    const NumericalDifferentiation::Scheme backward
        = NumericalDifferentiation::Backward;

    // see http://en.wikipedia.org/wiki/Finite_difference_coefficient
    checkTwoArraysAreTheSame(
        NumericalDifferentiation(f, 1, 1.0, 2, backward).weights(),
        {1.0, -1.0});

    checkTwoArraysAreTheSame(
        NumericalDifferentiation(f, 2, 2.0, 4, backward).weights(),
        {2/4.0, -5/4.0, 4/4.0, -1.0/4.0});

    checkTwoArraysAreTheSame(
        NumericalDifferentiation(f, 4, 1.0, 6, backward).weights(),
        {3.0, -14.0, 26.0, -24.0, 11.0, -2.0});

    checkTwoArraysAreTheSame(
        NumericalDifferentiation(f, 2, 0.5, 4, backward).offsets(),
        {0.0, -0.5, -1.0, -1.5});
}

BOOST_AUTO_TEST_CASE(testTabulatedForwardScheme) {
    BOOST_TEST_MESSAGE("Testing numerical differentiation "
                       "using the Forward scheme...");
    const ext::function<Real(Real)> f;

    const NumericalDifferentiation::Scheme forward
        = NumericalDifferentiation::Forward;

    // see http://en.wikipedia.org/wiki/Finite_difference_coefficient
    checkTwoArraysAreTheSame(
        NumericalDifferentiation(f, 1, 1.0, 2, forward).weights(),
        {-1.0, 1.0});

    checkTwoArraysAreTheSame(
        NumericalDifferentiation(f, 1, 0.5, 3, forward).weights(),
        {-6/2.0, 4.0, -2/2.0});

    checkTwoArraysAreTheSame(
        NumericalDifferentiation(f, 1, 0.5, 7, forward).weights(),
        {-98/20.0, 12.0, -30/2.0, 40/3.0, -30/4.0, 12/5.0, -2/6.0});

    checkTwoArraysAreTheSame(
        NumericalDifferentiation(f, 2, 0.5, 4, forward).offsets(),
        {0.0, 0.5, 1.0, 1.5});
}

BOOST_AUTO_TEST_CASE(testIrregularSchemeFirstOrder) {
    BOOST_TEST_MESSAGE("Testing numerical differentiation "
                       "of first order using an irregular scheme...");
    const ext::function<Real(Real)> f;

    const Real h1 = 5e-7;
    const Real h2 = 3e-6;

    const Real alpha = -h2/(h1*(h1+h2));
    const Real gamma =  h1/(h2*(h1+h2));
    const Real beta = -alpha - gamma;

    Array offsets = { -h1, 0.0, h2 };

    checkTwoArraysAreTheSame(
        NumericalDifferentiation(f, 1, offsets).weights(),
        { alpha, beta, gamma });
}

BOOST_AUTO_TEST_CASE(testIrregularSchemeSecondOrder) {
    BOOST_TEST_MESSAGE("Testing numerical differentiation "
                       "of second order using an irregular scheme...");
    const ext::function<Real(Real)> f;

    const Real h1 = 2e-7;
    const Real h2 = 8e-8;

    const Real alpha = 2/(h1*(h1+h2));
    const Real gamma = 2/(h2*(h1+h2));
    const Real beta = -alpha - gamma;

    Array offsets = { -h1, 0.0, h2 };

    checkTwoArraysAreTheSame(
        NumericalDifferentiation(f, 2, offsets).weights(),
        {alpha, beta, gamma});
}

BOOST_AUTO_TEST_CASE(testDerivativesOfSineFunction) {
    BOOST_TEST_MESSAGE("Testing numerical differentiation"
                       " of sin function...");

    const ext::function<Real(Real)> f = [](Real x) -> Real { return std::sin(x); };

    const ext::function<Real(Real)> df_central
        = NumericalDifferentiation(f, 1, std::sqrt(QL_EPSILON), 3,
                                   NumericalDifferentiation::Central);

    const ext::function<Real(Real)> df_backward
        = NumericalDifferentiation(f, 1, std::sqrt(QL_EPSILON), 3,
                                   NumericalDifferentiation::Backward);

    const ext::function<Real(Real)> df_forward
        = NumericalDifferentiation(f, 1, std::sqrt(QL_EPSILON), 3,
                                   NumericalDifferentiation::Forward);

    for (Real x=0.0; x < 5.0; x+=0.1) {
        const Real calculatedCentral = df_central(x);
        const Real calculatedBackward = df_backward(x);
        const Real calculatedForward = df_forward(x);
        const Real expected = std::cos(x);

        singleValueTest("central first", calculatedCentral, expected, 1e-8);
        singleValueTest("backward first", calculatedBackward, expected, 1e-6);
        singleValueTest("forward first", calculatedForward, expected, 1e-6);
    }

    const ext::function<Real(Real)> df4_central
        = NumericalDifferentiation(f, 4, 1e-2, 7,
                                   NumericalDifferentiation::Central);
    const ext::function<Real(Real)> df4_backward
        = NumericalDifferentiation(f, 4, 1e-2, 7,
                                   NumericalDifferentiation::Backward);
    const ext::function<Real(Real)> df4_forward
        = NumericalDifferentiation(f, 4, 1e-2, 7,
                                   NumericalDifferentiation::Forward);

    for (Real x=0.0; x < 5.0; x+=0.1) {
        const Real calculatedCentral = df4_central(x);
        const Real calculatedBackward = df4_backward(x);
        const Real calculatedForward = df4_forward(x);
        const Real expected = std::sin(x);

        singleValueTest("central 4th", calculatedCentral, expected, 1e-4);
        singleValueTest("backward 4th", calculatedBackward, expected, 1e-4);
        singleValueTest("forward 4th", calculatedForward, expected, 1e-4);
    }

    const Array offsets = {-0.01, -0.02, 0.03, 0.014, 0.041};
    NumericalDifferentiation df3_irregular(f, 3, offsets);

    checkTwoArraysAreTheSame(df3_irregular.offsets(), offsets);

    for (Real x=0.0; x < 5.0; x+=0.1) {
        const Real calculatedIrregular = df3_irregular(x);
        const Real expected = -std::cos(x);

        singleValueTest("irregular 3th", calculatedIrregular, expected, 5e-5);
    }
}

namespace {
    Array vandermondeCoefficients(
        Size order, Real x, const Array& gridPoints) {

        const Array q = gridPoints - x;
        const Size n = gridPoints.size();

        Matrix m(n, n, 1.0);
        for (Size i=1; i < n; ++i) {
            const Real fact = Factorial::get(i);
            for (Size j=0; j < n; ++j)
                m[i][j] = std::pow(q[j], Integer(i)) / fact;
        }

        Array b(n, 0.0);
        b[order] = 1.0;
        return inverse(m)*b;
    }
}

BOOST_AUTO_TEST_CASE(testCoefficientBasedOnVandermonde) {
    BOOST_TEST_MESSAGE("Testing coefficients from numerical differentiation"
                       " by comparison with results from"
                       " Vandermonde matrix inversion...");
    const ext::function<Real(Real)> f;

    for (Natural order=0; order < 5; ++order) {
        for (Natural nGridPoints = order + 1;
            nGridPoints < order + 3; ++nGridPoints) {

            Array gridPoints(nGridPoints);
            for (Natural i=0; i < nGridPoints; ++i) {
                const Real p = Real(i);
                gridPoints[i] = std::sin(p) + std::cos(p); // strange points
            }

            const Real x = 0.3902842; // strange points
            const Array weightsVandermonde
                = vandermondeCoefficients(order, x, gridPoints);
            const NumericalDifferentiation nd(f, order, gridPoints-x);

            checkTwoArraysAreTheSame(gridPoints, nd.offsets() + x);
            checkTwoArraysAreTheSame(weightsVandermonde, nd.weights());
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()