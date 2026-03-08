/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Yassine Idyiahia

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
#include <ql/termstructures/volatility/equityfx/smilesectionblackvolsurface.hpp>
#include <ql/termstructures/volatility/flatsmilesection.hpp>
#include <ql/termstructures/volatility/sabrsmilesection.hpp>
#include <ql/experimental/volatility/sabrvoltermstructure.hpp>
#include <ql/experimental/volatility/svismilesection.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(SmileSectionBlackVolSurfaceTests)

BOOST_AUTO_TEST_CASE(testSingleSmile) {
    BOOST_TEST_MESSAGE("Testing SmileSectionBlackVolSurface with single smile...");

    Date today(15, January, 2025);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Volatility vol = 0.20;
    Date expiry = today + 6*Months;

    auto smile = ext::make_shared<FlatSmileSection>(expiry, vol, dc, today);
    SmileSectionBlackVolSurface surface(today, smile, dc);

    Real tol = 1.0e-12;
    Real strike = 100.0;

    // Vol should be the same at any maturity (flat forward variance)
    std::vector<Period> tenors = { 1*Months, 3*Months, 6*Months, 1*Years, 2*Years };
    for (const auto& tenor : tenors) {
        Date d = today + tenor;
        Volatility calculated = surface.blackVol(d, strike, true);
        if (std::fabs(calculated - vol) > tol) {
            BOOST_FAIL("single smile vol mismatch at " << tenor
                       << std::fixed << std::setprecision(12)
                       << "\n    calculated: " << calculated
                       << "\n    expected:   " << vol
                       << "\n    tolerance:  " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testMultipleSmiles) {
    BOOST_TEST_MESSAGE("Testing SmileSectionBlackVolSurface with multiple smiles...");

    Date today(15, January, 2025);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Date d1 = today + 3*Months;
    Date d2 = today + 1*Years;
    Volatility vol1 = 0.20;
    Volatility vol2 = 0.30;

    std::vector<ext::shared_ptr<SmileSection>> smiles = {
        ext::make_shared<FlatSmileSection>(d1, vol1, dc, today),
        ext::make_shared<FlatSmileSection>(d2, vol2, dc, today)
    };

    SmileSectionBlackVolSurface surface(today, smiles, dc);

    Real strike = 100.0;
    Real tol = 1.0e-10;

    // At first expiry: should match vol1
    Volatility calcVol1 = surface.blackVol(d1, strike, true);
    if (std::fabs(calcVol1 - vol1) > tol) {
        BOOST_FAIL("vol mismatch at first expiry"
                   << std::fixed << std::setprecision(12)
                   << "\n    calculated: " << calcVol1
                   << "\n    expected:   " << vol1);
    }

    // At second expiry: should match vol2
    Volatility calcVol2 = surface.blackVol(d2, strike, true);
    if (std::fabs(calcVol2 - vol2) > tol) {
        BOOST_FAIL("vol mismatch at second expiry"
                   << std::fixed << std::setprecision(12)
                   << "\n    calculated: " << calcVol2
                   << "\n    expected:   " << vol2);
    }

    // At midpoint: check variance interpolation
    Date dMid = today + 6*Months;
    Time t1 = dc.yearFraction(today, d1);
    Time t2 = dc.yearFraction(today, d2);
    Time tMid = dc.yearFraction(today, dMid);

    Real w1 = vol1 * vol1 * t1;
    Real w2 = vol2 * vol2 * t2;
    Real alpha = (tMid - t1) / (t2 - t1);
    Real wMid = w1 + alpha * (w2 - w1);
    Volatility expectedMid = std::sqrt(wMid / tMid);

    Volatility calcMid = surface.blackVol(dMid, strike, true);
    if (std::fabs(calcMid - expectedMid) > tol) {
        BOOST_FAIL("vol mismatch at midpoint"
                   << std::fixed << std::setprecision(12)
                   << "\n    calculated: " << calcMid
                   << "\n    expected:   " << expectedMid);
    }
}

BOOST_AUTO_TEST_CASE(testVarianceMonotonicity) {
    BOOST_TEST_MESSAGE("Testing SmileSectionBlackVolSurface variance monotonicity...");

    Date today(15, January, 2025);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    // Increasing vols ensure monotonic total variance
    std::vector<ext::shared_ptr<SmileSection>> smiles = {
        ext::make_shared<FlatSmileSection>(today + 3*Months, 0.15, dc, today),
        ext::make_shared<FlatSmileSection>(today + 6*Months, 0.20, dc, today),
        ext::make_shared<FlatSmileSection>(today + 1*Years,  0.25, dc, today)
    };

    SmileSectionBlackVolSurface surface(today, smiles, dc);

    Real strike = 100.0;
    Real prevVar = 0.0;

    for (Size i = 1; i <= 12; ++i) {
        Date d = today + i*Months;
        Time t = dc.yearFraction(today, d);
        Real var = surface.blackVariance(d, strike, true);

        if (var < prevVar - 1.0e-12) {
            BOOST_FAIL("total variance is not monotonic"
                       << "\n    at month:      " << i
                       << "\n    variance:      " << var
                       << "\n    prev variance: " << prevVar);
        }
        prevVar = var;
    }
}

BOOST_AUTO_TEST_CASE(testSabrEquivalence) {
    BOOST_TEST_MESSAGE(
        "Testing SmileSectionBlackVolSurface equivalence with SABRVolTermStructure...");

    Date today(15, January, 2025);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    // SABR parameters
    Real alpha = 0.2;
    Real beta = 0.8;
    Real nu = 0.4;
    Real rho = -0.3;
    Real s0 = 100.0;
    Real r = 0.05;

    // Build SABRVolTermStructure
    SABRVolTermStructure sabrSurface(alpha, beta, nu, rho, s0, r, today, dc);

    // Build equivalent surface via adapter
    Real tol = 1.0e-10;
    std::vector<Real> strikes = { 80.0, 90.0, 100.0, 110.0, 120.0 };
    std::vector<Period> tenors = { 3*Months, 6*Months, 1*Years, 2*Years };

    for (const auto& tenor : tenors) {
        Date expiry = today + tenor;
        Time t = dc.yearFraction(today, expiry);
        Real fwd = s0 * std::exp(r * t);

        std::vector<Real> sabrParams = { alpha, beta, nu, rho };
        auto smile = ext::make_shared<SabrSmileSection>(
            t, fwd, sabrParams);

        SmileSectionBlackVolSurface adapterSurface(today, smile, dc);

        for (Real strike : strikes) {
            Volatility expected = sabrSurface.blackVol(expiry, strike, true);
            Volatility calculated = adapterSurface.blackVol(expiry, strike, true);

            if (std::fabs(calculated - expected) > tol) {
                BOOST_FAIL("SABR equivalence failed"
                           << "\n    tenor:      " << tenor
                           << "\n    strike:     " << strike
                           << std::fixed << std::setprecision(12)
                           << "\n    SABRVolTS:  " << expected
                           << "\n    adapter:    " << calculated
                           << "\n    difference: " << std::fabs(calculated - expected)
                           << "\n    tolerance:  " << tol);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testSviSmileAsVolSurface) {
    BOOST_TEST_MESSAGE(
        "Testing SmileSectionBlackVolSurface with SVI smile section...");

    Date today(15, January, 2025);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Time T = 0.5;
    Real forward = 100.0;

    // SVI parameters: a, b, sigma, rho, m
    std::vector<Real> sviParams = { 0.04, 0.1, 0.3, -0.4, 0.0 };

    auto sviSmile = ext::make_shared<SviSmileSection>(T, forward, sviParams);
    SmileSectionBlackVolSurface surface(today, sviSmile, dc);

    // Verify the adapter reproduces the smile at its expiry
    Real tol = 1.0e-10;
    std::vector<Real> strikes = { 80.0, 90.0, 100.0, 110.0, 120.0 };

    Date expiry = today + Period(static_cast<Integer>(T * 365 + 0.5), Days);

    for (Real strike : strikes) {
        Volatility fromSmile = sviSmile->volatility(strike);
        Volatility fromSurface = surface.blackVol(expiry, strike, true);

        if (std::fabs(fromSmile - fromSurface) > tol) {
            BOOST_FAIL("SVI smile/surface mismatch"
                       << "\n    strike:     " << strike
                       << std::fixed << std::setprecision(12)
                       << "\n    smile:      " << fromSmile
                       << "\n    surface:    " << fromSurface
                       << "\n    difference: " << std::fabs(fromSmile - fromSurface)
                       << "\n    tolerance:  " << tol);
        }
    }

    // Verify smile produces a non-flat surface (SVI has skew)
    Volatility volLow = surface.blackVol(expiry, 80.0, true);
    Volatility volAtm = surface.blackVol(expiry, 100.0, true);
    Volatility volHigh = surface.blackVol(expiry, 120.0, true);

    if (std::fabs(volLow - volAtm) < 1.0e-6 && std::fabs(volHigh - volAtm) < 1.0e-6) {
        BOOST_FAIL("SVI surface appears flat — expected a smile"
                   << std::fixed << std::setprecision(6)
                   << "\n    vol(80):  " << volLow
                   << "\n    vol(100): " << volAtm
                   << "\n    vol(120): " << volHigh);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
