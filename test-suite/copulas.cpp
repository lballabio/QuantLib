/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 LW Osterbrink

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
#include <ql/experimental/math/claytoncopularng.hpp>
#include <ql/experimental/math/farliegumbelmorgensterncopularng.hpp>
#include <ql/experimental/math/frankcopularng.hpp>
#include <ql/math/copulas/claytoncopula.hpp>
#include <ql/math/copulas/farliegumbelmorgensterncopula.hpp>
#include <ql/math/copulas/frankcopula.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <cmath>
#include <string>
#include <vector>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CopulaTests)

namespace {

    constexpr unsigned long seed = 42;

    /*! Draws from a copula random-number generator and compares the empirical
        copula against the analytical one on a grid of the unit square. This
        also covers the marginals, which have to come out uniform.
    */
    template <class Generator, class Copula>
    void checkSamples(const std::string& name,
                      Real theta,
                      Generator generator,
                      const Copula& copula) {

        const Size samples = 50000;
        // the empirical CDF has a standard error sqrt(p(1-p)/n) < 0.5/sqrt(samples),
        // i.e. <0.0023 here. The tolerance is about four times that.
        const Real tolerance = 0.01;

        std::vector<Real> u(samples), v(samples);
        Real meanU = 0.0, meanV = 0.0;

        for (Size i = 0; i < samples; ++i) {
            const std::vector<Real>& s = generator.next().value;
            BOOST_REQUIRE_MESSAGE(s.size() == 2,
                                  name << ": expected a 2-dimensional sample, got "
                                       << s.size() << " dimensions");
            if (s[0] < 0.0 || s[0] > 1.0 || s[1] < 0.0 || s[1] > 1.0)
                BOOST_FAIL(name << " with theta = " << theta
                                << ": sample (" << s[0] << ", " << s[1]
                                << ") lies outside the unit square");
            u[i] = s[0];
            v[i] = s[1];
            meanU += s[0];
            meanV += s[1];
        }
        meanU /= samples;
        meanV /= samples;

        // marginals uniform on [0,1] => mean = 0.5
        if (std::fabs(meanU - 0.5) > tolerance || std::fabs(meanV - 0.5) > tolerance)
            BOOST_ERROR(name << " with theta = " << theta << ": non-uniform marginals\n"
                             << "    calculated means: " << meanU << ", " << meanV << "\n"
                             << "    expected means:   0.5, 0.5\n"
                             << "    tolerance:        " << tolerance);

        // interior grid points i/(gridPoints+1), i = 1..gridPoints, in each dimension
        const Size gridPoints = 4;
        const Real gridStep = 1.0 / static_cast<Real>(gridPoints + 1);

        // check copula on a grid of the unit square
        for (Size i = 1; i <= gridPoints; ++i) {
            for (Size j = 1; j <= gridPoints; ++j) {
                Real x = gridStep * static_cast<Real>(i), y = gridStep * static_cast<Real>(j);
                Size count = 0;
                for (Size k = 0; k < samples; ++k)
                    if (u[k] <= x && v[k] <= y)
                        ++count;
                Real empirical = Real(count) / samples;
                Real expected = copula(x, y);
                if (std::fabs(empirical - expected) > tolerance)
                    BOOST_ERROR(name << " with theta = " << theta << ": wrong copula at ("
                                     << x << ", " << y << ")\n"
                                     << "    calculated: " << empirical << "\n"
                                     << "    expected:   " << expected << "\n"
                                     << "    tolerance:  " << tolerance);
            }
        }
    }

}

BOOST_AUTO_TEST_CASE(testFarlieGumbelMorgensternCopulaRng) {
    BOOST_TEST_MESSAGE("Testing Farlie-Gumbel-Morgenstern copula random-number generator...");

    for (Real theta : {-1.0, -0.5, 0.0, 0.5, 1.0}) {
        FarlieGumbelMorgensternCopulaRng<MersenneTwisterUniformRng> rng(
            MersenneTwisterUniformRng(seed), theta);
        checkSamples("Farlie-Gumbel-Morgenstern copula", theta, rng,
                     FarlieGumbelMorgensternCopula(theta));
    }
}

BOOST_AUTO_TEST_CASE(testClaytonCopulaRng) {
    BOOST_TEST_MESSAGE("Testing Clayton copula random-number generator...");

    // the conditional-inversion algorithm is only valid for positive theta
    for (Real theta : {0.5, 2.0, 5.0}) {
        ClaytonCopulaRng<MersenneTwisterUniformRng> rng(MersenneTwisterUniformRng(seed), theta);
        checkSamples("Clayton copula", theta, rng, ClaytonCopula(theta));
    }
}

BOOST_AUTO_TEST_CASE(testFrankCopulaStrongDependence) {
    BOOST_TEST_MESSAGE("Testing Frank copula at strong finite dependence...");

    const FrankCopula moderate(40.0);
    QL_CHECK_SMALL(moderate(0.95, 0.95) - 0.9344229684900084, 1e-14);

    const FrankCopula positive(1000.0), negative(-1000.0);
    QL_CHECK_SMALL(positive(0.5, 0.5) - 0.49930685281944005, 1e-14);
    QL_CHECK_SMALL(negative(0.5, 0.5) - 0.0006931471805599453, 1e-14);
    QL_CHECK_SMALL(negative(0.75, 0.75) - 0.5, 1e-14);
    QL_CHECK_SMALL(negative(0.25, 0.5) / 2.6691902155412764e-112 - 1.0, 1e-12);
    QL_CHECK_SMALL(positive(1e-300, 0.25) / 1e-300 - 1.0, 1e-12);

    for (const Real theta : {-1000.0, -50.0, 50.0, 1000.0}) {
        const FrankCopula copula(theta);
        for (const Real x : {0.0, 0.25, 0.5, 0.75, 1.0}) {
            BOOST_CHECK_EQUAL(copula(x, 1.0), x);
            BOOST_CHECK_EQUAL(copula(1.0, x), x);
            BOOST_CHECK_EQUAL(copula(x, 0.0), 0.0);
            BOOST_CHECK_EQUAL(copula(0.0, x), 0.0);
            if (x > 0.0)
                for (const Real y : {0.25, 0.5, 0.75, 1.0}) {
                    const Real cell = copula(x, y) - copula(x - 0.25, y)
                                    - copula(x, y - 0.25) + copula(x - 0.25, y - 0.25);
                    BOOST_CHECK_GE(cell, -1e-14);
                }
        }
    }
}

BOOST_AUTO_TEST_CASE(testFrankCopulaRng) {
    BOOST_TEST_MESSAGE("Testing Frank copula random-number generator...");

    for (Real theta : {-5.0, -1.0, 1.0, 5.0}) {
        FrankCopulaRng<MersenneTwisterUniformRng> rng(MersenneTwisterUniformRng(seed), theta);
        checkSamples("Frank copula", theta, rng, FrankCopula(theta));
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
