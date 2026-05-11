/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Junjie Guo

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
#include <ql/processes/g2process.hpp>
#include <ql/models/shortrate/twofactormodel.hpp>
#include <ql/models/shortrate/twofactormodels/g2.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/methods/montecarlo/multipathgenerator.hpp>
#include <ql/methods/montecarlo/mctraits.hpp>
#include <ql/timegrid.hpp>
#include <cmath>
#include <vector>

using namespace QuantLib;
using boost::unit_test_framework::test_suite;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(G2ProcessTests)

namespace {

    // Reference implementation of the G2++ deterministic offset phi(t),
    // copied verbatim from G2::FittingParameter::Impl::value in
    // ql/models/shortrate/twofactormodels/g2.hpp. The tests check that
    // G2Process / G2ForwardProcess match this formula.
    Real referencePhi(const Handle<YieldTermStructure>& h, Time t,
                      Real a, Real sigma, Real b, Real eta, Real rho) {
        Rate forward = h->forwardRate(t, t, Continuous, NoFrequency);
        Real temp1 = sigma*(1.0-std::exp(-a*t))/a;
        Real temp2 = eta *(1.0-std::exp(-b*t))/b;
        return 0.5*temp1*temp1 + 0.5*temp2*temp2 + rho*temp1*temp2 + forward;
    }

    Handle<YieldTermStructure> makeFlatCurve(Rate rate) {
        return Handle<YieldTermStructure>(
            ext::make_shared<FlatForward>(0, TARGET(), rate, Actual365Fixed()));
    }

}

BOOST_AUTO_TEST_CASE(testG2ProcessDriftIncludesTermStructure) {

    BOOST_TEST_MESSAGE(
        "Testing that G2Process drift includes the term-structure shift...");

    const Real a = 0.1, sigma = 0.01, b = 0.2, eta = 0.013, rho = -0.5;

    G2Process paramOnly(a, sigma, b, eta, rho);
    G2Process withCurve(a, sigma, b, eta, rho, makeFlatCurve(0.04));

    const Time t = 1.5;
    Array z(2);
    z[0] = 0.002;
    z[1] = -0.003;

    Array d1 = paramOnly.drift(t, z);
    Array d2 = withCurve.drift(t, z);

    // The y-component must not depend on the term structure.
    const Real tol = 1e-12;
    if (std::fabs(d1[1] - d2[1]) > tol) {
        BOOST_ERROR("y-drift should not depend on the term structure: "
                    << d1[1] << " vs " << d2[1]);
    }

    // The x-component must differ by exactly a*phi(t) + phi'(t)
    // (computed with the same numerical derivative as the implementation).
    const Real h = 1.0e-4;
    Real phi_t  = withCurve.phi(t);
    Real phi_th = withCurve.phi(t + h);
    Real expectedDelta = a * phi_t + (phi_th - phi_t) / h;
    Real actualDelta = d2[0] - d1[0];
    if (std::fabs(actualDelta - expectedDelta) > 1e-10) {
        BOOST_ERROR("x-drift shift mismatch: expected " << expectedDelta
                    << ", got " << actualDelta);
    }

    // Diffusion is purely deterministic and curve-independent.
    Matrix diff1 = paramOnly.diffusion(t, z);
    Matrix diff2 = withCurve.diffusion(t, z);
    for (Size i=0; i<2; ++i)
        for (Size j=0; j<2; ++j)
            if (std::fabs(diff1[i][j] - diff2[i][j]) > 1e-14)
                BOOST_ERROR("G2Process diffusion changed when term structure "
                            "was provided at (" << i << "," << j << "): "
                            << diff1[i][j] << " vs " << diff2[i][j]);
}

BOOST_AUTO_TEST_CASE(testG2ProcessPhiAndShortRate) {

    BOOST_TEST_MESSAGE(
        "Testing G2Process phi(t), shortRate(t, z1, z2), and initialValues...");

    const Real a = 0.1, sigma = 0.01, b = 0.2, eta = 0.013, rho = -0.5;
    const Rate flatRate = 0.03;
    Handle<YieldTermStructure> curve = makeFlatCurve(flatRate);

    G2Process process(a, sigma, b, eta, rho, curve);

    const Time times[] = { 0.25, 1.0, 5.0, 10.0 };

    const Real tol = 1e-12;
    for (Time t : times) {
        Real expectedPhi =
            referencePhi(curve, t, a, sigma, b, eta, rho);
        Real actualPhi = process.phi(t);
        if (std::fabs(actualPhi - expectedPhi) > tol) {
            BOOST_ERROR("G2Process::phi mismatch at t=" << t
                        << ": expected " << expectedPhi
                        << ", got " << actualPhi);
        }
    }

    // shortRate is just the sum of the simulated components: the term-structure
    // offset phi(t) is already baked into the first component.
    const Real zs1[] = { -0.01, 0.0, 0.005 };
    const Real zs2[] = { -0.002, 0.0, 0.004 };
    for (Real z1 : zs1)
        for (Real z2 : zs2) {
            Rate expected = z1 + z2;
            Rate actual = process.shortRate(1.0, z1, z2);
            if (std::fabs(actual - expected) > tol) {
                BOOST_ERROR("G2Process::shortRate(t, z1, z2) should equal z1+z2: "
                            << "got " << actual << ", expected " << expected);
            }
        }

    // Initial state must satisfy z1(0) + z2(0) = phi(0) = f(0,0).
    Array iv = process.initialValues();
    Real expectedR0 = referencePhi(curve, 0.0, a, sigma, b, eta, rho);
    if (std::fabs((iv[0] + iv[1]) - expectedR0) > 1e-12) {
        BOOST_ERROR("initialValues do not sum to phi(0): "
                    << iv[0] + iv[1] << " vs " << expectedR0);
    }
    if (std::fabs(process.x0() - expectedR0) > 1e-12) {
        BOOST_ERROR("x0() should equal phi(0) when a curve is set: "
                    << process.x0() << " vs " << expectedR0);
    }
    if (std::fabs(process.y0()) > 1e-14) {
        BOOST_ERROR("y0() should be zero, got " << process.y0());
    }
}

BOOST_AUTO_TEST_CASE(testG2ProcessPhiMatchesG2Model) {

    BOOST_TEST_MESSAGE(
        "Testing that G2Process::phi matches the G2 model fitting parameter...");

    const Real a = 0.12, sigma = 0.011, b = 0.17, eta = 0.009, rho = -0.3;
    Handle<YieldTermStructure> curve = makeFlatCurve(0.025);

    G2Process process(a, sigma, b, eta, rho, curve);
    G2 model(curve, a, sigma, b, eta, rho);
    ext::shared_ptr<TwoFactorModel::ShortRateDynamics> dyn = model.dynamics();
    BOOST_REQUIRE(dyn);

    const Real tol = 1e-12;
    for (Time t : {0.1, 0.5, 2.0, 7.5, 20.0}) {
        // dyn->shortRate(t, 0, 0) collapses to fitting_(t) = phi(t).
        Rate fromModel = dyn->shortRate(t, 0.0, 0.0);
        Rate fromProcess = process.phi(t);
        if (std::fabs(fromProcess - fromModel) > tol) {
            BOOST_ERROR("G2Process::phi disagrees with G2 model at t="
                        << t << ": process=" << fromProcess
                        << ", model=" << fromModel);
        }
    }
}

BOOST_AUTO_TEST_CASE(testG2ProcessExpectationConsistentWithCurve) {

    BOOST_TEST_MESSAGE(
        "Testing that G2Process expectation reproduces phi(t)...");

    const Real a = 0.1, sigma = 0.01, b = 0.2, eta = 0.013, rho = -0.4;
    Handle<YieldTermStructure> curve = makeFlatCurve(0.035);

    G2Process process(a, sigma, b, eta, rho, curve);
    Array iv = process.initialValues();

    const Real tol = 1e-12;
    for (Time t : {0.1, 0.5, 2.0, 5.0, 10.0}) {
        // Expectation starting from t=0 in the initial state must satisfy
        // E[z1(t) + z2(t)] = phi(t), since z2(0) = 0 and y is zero-mean OU.
        Array exp_t = process.expectation(0.0, iv, t);
        Real expectedR = process.phi(t);
        Real actualR = exp_t[0] + exp_t[1];
        if (std::fabs(actualR - expectedR) > tol) {
            BOOST_ERROR("E[r(t)] mismatch at t=" << t
                        << ": expected " << expectedR
                        << ", got " << actualR);
        }
    }
}

BOOST_AUTO_TEST_CASE(testG2ProcessPhiRequiresTermStructure) {

    BOOST_TEST_MESSAGE(
        "Testing that G2Process::phi throws without a term structure...");

    G2Process process(0.1, 0.01, 0.2, 0.013, -0.5);
    BOOST_CHECK(process.termStructure().empty());
    BOOST_CHECK_THROW(process.phi(1.0), Error);

    // shortRate no longer touches the curve: it just sums the two components.
    BOOST_CHECK_NO_THROW(process.shortRate(1.0, 0.01, 0.01));
    if (std::fabs(process.shortRate(1.0, 0.01, 0.02) - 0.03) > 1e-14) {
        BOOST_ERROR("shortRate(t, z1, z2) should equal z1+z2 with no curve");
    }

    // Without a curve, the process degenerates to two zero-mean OU factors.
    Array iv = process.initialValues();
    if (std::fabs(iv[0]) > 1e-14 || std::fabs(iv[1]) > 1e-14) {
        BOOST_ERROR("initialValues should be (0,0) without a curve, got ("
                    << iv[0] << ", " << iv[1] << ")");
    }
}

BOOST_AUTO_TEST_CASE(testG2ProcessObservesTermStructure) {

    BOOST_TEST_MESSAGE(
        "Testing that G2Process phi(t) updates when the curve moves...");

    const Real a = 0.1, sigma = 0.01, b = 0.2, eta = 0.013, rho = -0.5;

    ext::shared_ptr<SimpleQuote> rate = ext::make_shared<SimpleQuote>(0.02);
    Handle<YieldTermStructure> curve(ext::make_shared<FlatForward>(
        0, TARGET(), Handle<Quote>(rate), Actual365Fixed()));

    G2Process process(a, sigma, b, eta, rho, curve);

    const Time t = 2.0;
    Real phiBefore = process.phi(t);
    rate->setValue(0.05);
    Real phiAfter = process.phi(t);

    // Under a flat curve, increasing the instantaneous forward by 300bp
    // must raise phi by approximately the same amount.
    const Real expectedDelta = 0.03;
    if (std::fabs((phiAfter - phiBefore) - expectedDelta) > 1e-10) {
        BOOST_ERROR("G2Process did not respond to term-structure change: "
                    << "delta=" << (phiAfter - phiBefore)
                    << " (expected " << expectedDelta << ")");
    }
}

BOOST_AUTO_TEST_CASE(testG2ProcessPathGeneratorMatchesCurve) {

    BOOST_TEST_MESSAGE(
        "Testing that MultiPathGenerator on G2Process reproduces "
        "the curve-implied short-rate expectation...");

    const Real a = 0.1, sigma = 0.01, b = 0.2, eta = 0.013, rho = -0.3;
    Handle<YieldTermStructure> curve = makeFlatCurve(0.03);

    auto process = ext::make_shared<G2Process>(a, sigma, b, eta, rho, curve);

    const Time horizon = 5.0;
    const Size steps = 50;
    TimeGrid grid(horizon, steps);

    typedef PseudoRandom::rsg_type rsg_type;
    rsg_type rsg = PseudoRandom::make_sequence_generator(
        process->factors() * steps, 42);
    MultiPathGenerator<rsg_type> generator(process, grid, rsg, false);

    const Size nPaths = 20000;
    std::vector<Real> sumR(steps + 1, 0.0);
    for (Size n = 0; n < nPaths; ++n) {
        const MultiPath& mp = generator.next().value;
        for (Size i = 0; i <= steps; ++i)
            sumR[i] += mp[0][i] + mp[1][i];
    }

    // Empirical mean of r(t_i) should converge to phi(t_i). With 20k
    // antithetic-free paths the MC standard error is comfortably below 5e-4
    // for the parameters above.
    for (Size i = 0; i <= steps; ++i) {
        Real meanR = sumR[i] / nPaths;
        Real expected = process->phi(grid[i]);
        if (std::fabs(meanR - expected) > 5e-4) {
            BOOST_ERROR("MC mean of r at t=" << grid[i]
                        << " is " << meanR
                        << ", expected " << expected
                        << " (diff " << (meanR - expected) << ")");
        }
    }
}

BOOST_AUTO_TEST_CASE(testG2ForwardProcessPhiAndShortRate) {

    BOOST_TEST_MESSAGE(
        "Testing G2ForwardProcess phi(t) and shortRate(t, z1, z2)...");

    const Real a = 0.1, sigma = 0.01, b = 0.2, eta = 0.013, rho = -0.5;
    Handle<YieldTermStructure> curve = makeFlatCurve(0.035);

    G2ForwardProcess fwd(a, sigma, b, eta, rho, curve);

    const Real tol = 1e-12;
    for (Time t : {0.25, 1.0, 5.0}) {
        Real expected =
            referencePhi(curve, t, a, sigma, b, eta, rho);
        Real actual = fwd.phi(t);
        if (std::fabs(actual - expected) > tol) {
            BOOST_ERROR("G2ForwardProcess::phi mismatch at t=" << t
                        << ": expected " << expected << ", got " << actual);
        }

        // shortRate sums the simulated components.
        Rate sr = fwd.shortRate(t, 0.002, -0.001);
        if (std::fabs(sr - 0.001) > tol) {
            BOOST_ERROR("G2ForwardProcess::shortRate mismatch at t=" << t
                        << ": expected 0.001, got " << sr);
        }
    }

    // Without a curve, phi() throws but shortRate() still works.
    G2ForwardProcess paramOnly(a, sigma, b, eta, rho);
    BOOST_CHECK(paramOnly.termStructure().empty());
    BOOST_CHECK_THROW(paramOnly.phi(1.0), Error);
    BOOST_CHECK_NO_THROW(paramOnly.shortRate(1.0, 0.01, 0.01));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
