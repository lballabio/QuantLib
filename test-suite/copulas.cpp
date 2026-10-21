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
#include <ql/experimental/credit/onefactorcopula.hpp>
#include <ql/experimental/credit/onefactorstudentcopula.hpp>
#include <ql/handle.hpp>
#include <ql/quotes/simplequote.hpp>
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

BOOST_AUTO_TEST_CASE(testFrankCopulaRng) {
    BOOST_TEST_MESSAGE("Testing Frank copula random-number generator...");

    for (Real theta : {-5.0, -1.0, 1.0, 5.0}) {
        FrankCopulaRng<MersenneTwisterUniformRng> rng(MersenneTwisterUniformRng(seed), theta);
        checkSamples("Frank copula", theta, rng, FrankCopula(theta));
    }
}

BOOST_AUTO_TEST_CASE(testOneFactorStudentCopulaMoments) {
    BOOST_TEST_MESSAGE("Testing OneFactorStudentCopula moments with "
                       "quantile-based integration range...");

    // checkMoments(tol) triggers performCalculations(), integrates the
    // marginal density of M (Gaussian) and Y (marginal of the copula), and
    // raises QL_REQUIRE if mean or variance deviate from 0 / 1 by more than
    // tol.  This validates that the quantile-based integration range covers
    // enough tail mass for each combination of degrees of freedom.
    //
    // We use a loose tolerance (3%) because the coarse default grid (200
    // steps) is not intended for high accuracy; the test is checking that the
    // range is wide enough, not that the quadrature is precise.
    Real tol = 0.03;

    auto makeQuote = [](Real v) {
        return Handle<Quote>(ext::make_shared<SimpleQuote>(v));
    };

    // Full Student-t: M ~ t(nz), Z ~ t(nm)  (note: constructor order is nz, nm)
    for (int nz : {3, 5, 10}) {
        for (int nm : {3, 5, 10}) {
            OneFactorStudentCopula copula(makeQuote(0.5), nz, nm);
            BOOST_CHECK_NO_THROW(copula.checkMoments(tol));
        }
    }

    // Gaussian M, Student-t Z
    for (int nz : {3, 5, 10}) {
        OneFactorGaussianStudentCopula copula(makeQuote(0.3), nz);
        BOOST_CHECK_NO_THROW(copula.checkMoments(tol));
    }

    // Student-t M, Gaussian Z
    for (int nm : {3, 5, 10}) {
        OneFactorStudentGaussianCopula copula(makeQuote(0.3), nm);
        BOOST_CHECK_NO_THROW(copula.checkMoments(tol));
    }
}

BOOST_AUTO_TEST_CASE(testOneFactorStudentCopulaConditionalProbability) {
    BOOST_TEST_MESSAGE("Testing OneFactorStudentCopula conditional probability "
                       "is a proper CDF...");

    // conditionalProbability(p, y) should be:
    // - bounded in [0, 1]
    // - strictly increasing in p for fixed y
    // - equal to p when correlation = 0 (independence)
    Real tol = 1.0e-3;

    auto makeQuote = [](Real v) {
        return Handle<Quote>(ext::make_shared<SimpleQuote>(v));
    };

    // nz=5, nm=5 (constructor order: nz then nm)
    OneFactorStudentCopula copula(makeQuote(0.5), /*nz=*/5, /*nm=*/5);

    Real prev = -1.0;
    for (int i = 1; i <= 9; ++i) {
        Real p = i * 0.1;
        Real cp = copula.conditionalProbability(p, /*y=*/0.0);
        BOOST_CHECK_GE(cp, 0.0);
        BOOST_CHECK_LE(cp, 1.0);
        BOOST_CHECK_GT(cp, prev);
        prev = cp;
    }

    // At zero correlation the copula factors: conditionalProbability(p, y) = p
    OneFactorStudentCopula indep(makeQuote(0.0), /*nz=*/5, /*nm=*/5);
    for (Real p : {0.1, 0.3, 0.5, 0.7, 0.9}) {
        Real cp = indep.conditionalProbability(p, /*y=*/0.5);
        if (std::fabs(cp - p) > tol)
            BOOST_ERROR("zero-correlation: conditionalProbability(" << p
                << ", 0.5) = " << cp << ", expected " << p);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
