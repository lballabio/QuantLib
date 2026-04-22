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
#include <cmath>

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

BOOST_AUTO_TEST_CASE(testG2ProcessDriftUnchangedByTermStructure) {

    BOOST_TEST_MESSAGE(
        "Testing that G2Process drift/diffusion ignore the term structure...");

    const Real a = 0.1, sigma = 0.01, b = 0.2, eta = 0.013, rho = -0.5;

    G2Process paramOnly(Handle<YieldTermStructure>(), a, sigma, b, eta, rho);
    G2Process withCurve(makeFlatCurve(0.04), a, sigma, b, eta, rho);

    const Time t = 1.5;
    Array x(2);
    x[0] = 0.002;
    x[1] = -0.003;

    Array d1 = paramOnly.drift(t, x);
    Array d2 = withCurve.drift(t, x);

    const Real tol = 1e-14;
    if (std::fabs(d1[0] - d2[0]) > tol || std::fabs(d1[1] - d2[1]) > tol) {
        BOOST_ERROR("G2Process drift changed when term structure was provided:\n"
                    << "  without curve: [" << d1[0] << ", " << d1[1] << "]\n"
                    << "  with curve:    [" << d2[0] << ", " << d2[1] << "]");
    }

    // x_t and y_t are OU with zero mean-reversion level, so at x = 0 the
    // drift must be exactly zero regardless of the curve.
    Array zero(2, 0.0);
    Array dz = withCurve.drift(t, zero);
    if (std::fabs(dz[0]) > tol || std::fabs(dz[1]) > tol) {
        BOOST_ERROR("G2Process drift at x=y=0 is not zero: ["
                    << dz[0] << ", " << dz[1] << "]");
    }

    Matrix diff1 = paramOnly.diffusion(t, x);
    Matrix diff2 = withCurve.diffusion(t, x);
    for (Size i=0; i<2; ++i)
        for (Size j=0; j<2; ++j)
            if (std::fabs(diff1[i][j] - diff2[i][j]) > tol)
                BOOST_ERROR("G2Process diffusion changed when term structure "
                            "was provided at (" << i << "," << j << "): "
                            << diff1[i][j] << " vs " << diff2[i][j]);
}

BOOST_AUTO_TEST_CASE(testG2ProcessPhiAndShortRate) {

    BOOST_TEST_MESSAGE(
        "Testing G2Process phi(t) and shortRate(t, x, y)...");

    const Real a = 0.1, sigma = 0.01, b = 0.2, eta = 0.013, rho = -0.5;
    const Rate flatRate = 0.03;
    Handle<YieldTermStructure> curve = makeFlatCurve(flatRate);

    G2Process process(curve, a, sigma, b, eta, rho);

    const Time times[] = { 0.25, 1.0, 5.0, 10.0 };
    const Real xs[]    = { -0.01, 0.0, 0.005 };
    const Real ys[]    = { -0.002, 0.0, 0.004 };

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

        for (Real x : xs)
            for (Real y : ys) {
                Rate expected = x + y + expectedPhi;
                Rate actual = process.shortRate(t, x, y);
                if (std::fabs(actual - expected) > tol) {
                    BOOST_ERROR("G2Process::shortRate mismatch at t="
                                << t << ", x=" << x << ", y=" << y
                                << ": expected " << expected
                                << ", got " << actual);
                }
            }
    }
}

BOOST_AUTO_TEST_CASE(testG2ProcessPhiMatchesG2Model) {

    BOOST_TEST_MESSAGE(
        "Testing that G2Process::phi matches the G2 model fitting parameter...");

    const Real a = 0.12, sigma = 0.011, b = 0.17, eta = 0.009, rho = -0.3;
    Handle<YieldTermStructure> curve = makeFlatCurve(0.025);

    G2Process process(curve, a, sigma, b, eta, rho);
    G2 model(curve, a, sigma, b, eta, rho);
    ext::shared_ptr<TwoFactorModel::ShortRateDynamics> dyn = model.dynamics();
    BOOST_REQUIRE(dyn);

    const Real tol = 1e-12;
    for (Time t : {0.1, 0.5, 2.0, 7.5, 20.0}) {
        Rate fromProcess = process.shortRate(t, 0.0, 0.0);
        Rate fromModel = dyn->shortRate(t, 0.0, 0.0);
        if (std::fabs(fromProcess - fromModel) > tol) {
            BOOST_ERROR("G2Process::shortRate disagrees with G2 model at t="
                        << t << ": process=" << fromProcess
                        << ", model=" << fromModel);
        }
    }
}

BOOST_AUTO_TEST_CASE(testG2ProcessPhiRequiresTermStructure) {

    BOOST_TEST_MESSAGE(
        "Testing that G2Process::phi throws without a term structure...");

    G2Process process(Handle<YieldTermStructure>(), 0.1, 0.01, 0.2, 0.013, -0.5);
    BOOST_CHECK(process.termStructure().empty());
    BOOST_CHECK_THROW(process.phi(1.0), Error);
    BOOST_CHECK_THROW(process.shortRate(1.0, 0.01, 0.01), Error);
}

BOOST_AUTO_TEST_CASE(testG2ProcessObservesTermStructure) {

    BOOST_TEST_MESSAGE(
        "Testing that G2Process phi(t) updates when the curve moves...");

    const Real a = 0.1, sigma = 0.01, b = 0.2, eta = 0.013, rho = -0.5;

    ext::shared_ptr<SimpleQuote> rate = ext::make_shared<SimpleQuote>(0.02);
    Handle<YieldTermStructure> curve(ext::make_shared<FlatForward>(
        0, TARGET(), Handle<Quote>(rate), Actual365Fixed()));

    G2Process process(curve, a, sigma, b, eta, rho);

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

BOOST_AUTO_TEST_CASE(testG2ForwardProcessPhiAndShortRate) {

    BOOST_TEST_MESSAGE(
        "Testing G2ForwardProcess phi(t) and shortRate(t, x, y)...");

    const Real a = 0.1, sigma = 0.01, b = 0.2, eta = 0.013, rho = -0.5;
    Handle<YieldTermStructure> curve = makeFlatCurve(0.035);

    G2ForwardProcess fwd(curve, a, sigma, b, eta, rho);

    const Real tol = 1e-12;
    for (Time t : {0.25, 1.0, 5.0}) {
        Real expected =
            referencePhi(curve, t, a, sigma, b, eta, rho);
        Real actual = fwd.phi(t);
        if (std::fabs(actual - expected) > tol) {
            BOOST_ERROR("G2ForwardProcess::phi mismatch at t=" << t
                        << ": expected " << expected << ", got " << actual);
        }

        Rate sr = fwd.shortRate(t, 0.002, -0.001);
        if (std::fabs(sr - (0.002 - 0.001 + expected)) > tol) {
            BOOST_ERROR("G2ForwardProcess::shortRate mismatch at t=" << t);
        }
    }

    G2ForwardProcess paramOnly(Handle<YieldTermStructure>(), a, sigma, b, eta, rho);
    BOOST_CHECK(paramOnly.termStructure().empty());
    BOOST_CHECK_THROW(paramOnly.phi(1.0), Error);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
