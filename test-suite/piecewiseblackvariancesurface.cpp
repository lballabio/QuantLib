/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Rich Amaya

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
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/matrix.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/volatility/equityfx/piecewiseblackvariancesurface.hpp>
#include <ql/termstructures/volatility/flatsmilesection.hpp>
#include <ql/termstructures/volatility/interpolatedsmilesection.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(PiecewiseBlackVarianceSurfaceSuite)

BOOST_AUTO_TEST_CASE(testExactRepricing) {
    BOOST_TEST_MESSAGE(
        "Testing exact repricing at input tenors...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Date d1 = today + 3*Months;
    Date d2 = today + 6*Months;
    Date d3 = today + 1*Years;

    Volatility vol1 = 0.20, vol2 = 0.25, vol3 = 0.30;

    std::vector<Date> dates = {d1, d2, d3};
    std::vector<ext::shared_ptr<SmileSection>> sections = {
        ext::make_shared<FlatSmileSection>(d1, vol1, dc, today),
        ext::make_shared<FlatSmileSection>(d2, vol2, dc, today),
        ext::make_shared<FlatSmileSection>(d3, vol3, dc, today)
    };

    PiecewiseBlackVarianceSurface surface(today, dates, sections, dc);

    Real tol = 1.0e-12;
    Real strike = 100.0;

    for (Size i = 0; i < dates.size(); ++i) {
        Real expected = sections[i]->variance(strike);
        Real calculated = surface.blackVariance(dates[i], strike);
        Real diff = std::fabs(calculated - expected);

        if (diff > tol)
            BOOST_FAIL("failed to reprice at tenor " << i
                       << "\n    date:       " << dates[i]
                       << "\n    calculated: " << std::setprecision(16) << std::scientific << calculated
                       << "\n    expected:   " << expected
                       << "\n    difference: " << diff
                       << "\n    tolerance:  " << tol);
    }
}

BOOST_AUTO_TEST_CASE(testInterpolation) {
    BOOST_TEST_MESSAGE(
        "Testing linear variance interpolation between tenors...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Date d1 = today + 6*Months;
    Date d2 = today + 1*Years;

    Volatility vol1 = 0.20, vol2 = 0.30;

    std::vector<Date> dates = {d1, d2};
    std::vector<ext::shared_ptr<SmileSection>> sections = {
        ext::make_shared<FlatSmileSection>(d1, vol1, dc, today),
        ext::make_shared<FlatSmileSection>(d2, vol2, dc, today)
    };

    PiecewiseBlackVarianceSurface surface(today, dates, sections, dc);
    surface.enableExtrapolation();

    Real strike = 100.0;
    Time t1 = dc.yearFraction(today, d1);
    Time t2 = dc.yearFraction(today, d2);
    Real var1 = vol1 * vol1 * t1;
    Real var2 = vol2 * vol2 * t2;

    // test at midpoint between the two tenors
    Time tMid = 0.5 * (t1 + t2);
    Date dMid = today + Integer(tMid * 365 + 0.5);
    Time tMidActual = dc.yearFraction(today, dMid);
    Real alpha = (tMidActual - t1) / (t2 - t1);
    Real expectedVar = var1 + (var2 - var1) * alpha;
    Real calculated = surface.blackVariance(dMid, strike);
    Real diff = std::fabs(calculated - expectedVar);
    Real tol = 1.0e-12;

    if (diff > tol)
        BOOST_FAIL("failed to interpolate at midpoint"
                   << "\n    date:       " << dMid
                   << "\n    time:       " << tMidActual
                   << "\n    calculated: " << std::setprecision(16) << std::scientific << calculated
                   << "\n    expected:   " << expectedVar
                   << "\n    difference: " << diff
                   << "\n    tolerance:  " << tol);

    // test before the first tenor (interpolation from (0,0))
    Date dEarly = today + 1*Months;
    Time tEarly = dc.yearFraction(today, dEarly);
    Real expectedEarly = var1 * tEarly / t1;
    Real calculatedEarly = surface.blackVariance(dEarly, strike);
    diff = std::fabs(calculatedEarly - expectedEarly);

    if (diff > tol)
        BOOST_FAIL("failed to interpolate before first tenor"
                   << "\n    date:       " << dEarly
                   << "\n    time:       " << tEarly
                   << "\n    calculated: " << std::setprecision(16) << std::scientific << calculatedEarly
                   << "\n    expected:   " << expectedEarly
                   << "\n    difference: " << diff
                   << "\n    tolerance:  " << tol);
}

BOOST_AUTO_TEST_CASE(testBlackVolDerivation) {
    BOOST_TEST_MESSAGE(
        "Testing blackVol derivation from blackVariance...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Date d1 = today + 6*Months;
    Volatility vol1 = 0.25;

    std::vector<Date> dates = {d1};
    std::vector<ext::shared_ptr<SmileSection>> sections = {
        ext::make_shared<FlatSmileSection>(d1, vol1, dc, today)
    };

    PiecewiseBlackVarianceSurface surface(today, dates, sections, dc);

    Real strike = 100.0;
    Real var = surface.blackVariance(d1, strike);
    Time t = dc.yearFraction(today, d1);
    Real expectedVol = std::sqrt(var / t);
    Real calculatedVol = surface.blackVol(d1, strike);
    Real diff = std::fabs(calculatedVol - expectedVol);
    Real tol = 1.0e-12;

    if (diff > tol)
        BOOST_FAIL("blackVol inconsistent with blackVariance"
                   << "\n    blackVariance: " << std::setprecision(16) << std::scientific << var
                   << "\n    time:          " << t
                   << "\n    expected vol:  " << expectedVol
                   << "\n    calculated:    " << calculatedVol
                   << "\n    difference:    " << diff
                   << "\n    tolerance:     " << tol);
}

BOOST_AUTO_TEST_CASE(testExtrapolation) {
    BOOST_TEST_MESSAGE(
        "Testing flat-vol extrapolation beyond last tenor...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Date d1 = today + 6*Months;
    Date d2 = today + 1*Years;
    Volatility vol1 = 0.20, vol2 = 0.30;

    std::vector<Date> dates = {d1, d2};
    std::vector<ext::shared_ptr<SmileSection>> sections = {
        ext::make_shared<FlatSmileSection>(d1, vol1, dc, today),
        ext::make_shared<FlatSmileSection>(d2, vol2, dc, today)
    };

    PiecewiseBlackVarianceSurface surface(today, dates, sections, dc);
    surface.enableExtrapolation();

    Real strike = 100.0;
    Time t2 = dc.yearFraction(today, d2);
    Real var2 = vol2 * vol2 * t2;

    // query beyond last tenor: flat variance rate extrapolation
    Date dBeyond = today + 2*Years;
    Time tBeyond = dc.yearFraction(today, dBeyond);
    Real expectedVar = var2 * tBeyond / t2;
    Real calculated = surface.blackVariance(dBeyond, strike, true);
    Real diff = std::fabs(calculated - expectedVar);
    Real tol = 1.0e-12;

    if (diff > tol)
        BOOST_FAIL("flat-vol extrapolation failed"
                   << "\n    date:       " << dBeyond
                   << "\n    time:       " << tBeyond
                   << "\n    calculated: " << std::setprecision(16) << std::scientific << calculated
                   << "\n    expected:   " << expectedVar
                   << "\n    difference: " << diff
                   << "\n    tolerance:  " << tol);

    // vol should be constant beyond last tenor
    Real expectedVol = vol2;
    Real calculatedVol = surface.blackVol(dBeyond, strike, true);
    diff = std::fabs(calculatedVol - expectedVol);

    if (diff > tol)
        BOOST_FAIL("flat-vol extrapolation: vol not constant"
                   << "\n    date:       " << dBeyond
                   << "\n    calculated: " << std::setprecision(16) << std::scientific << calculatedVol
                   << "\n    expected:   " << expectedVol
                   << "\n    difference: " << diff
                   << "\n    tolerance:  " << tol);
}

BOOST_AUTO_TEST_CASE(testObserver) {
    BOOST_TEST_MESSAGE(
        "Testing observer notification from SmileSections...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Date d1 = today + 6*Months;
    Time t1 = dc.yearFraction(today, d1);

    std::vector<Rate> strikes = {80.0, 100.0, 120.0};
    Volatility vol = 0.25;
    Real sqrtT = std::sqrt(t1);
    std::vector<ext::shared_ptr<SimpleQuote>> quotes = {
        ext::make_shared<SimpleQuote>(vol * sqrtT),
        ext::make_shared<SimpleQuote>(vol * sqrtT),
        ext::make_shared<SimpleQuote>(vol * sqrtT)
    };
    std::vector<Handle<Quote>> handles = {
        Handle<Quote>(quotes[0]),
        Handle<Quote>(quotes[1]),
        Handle<Quote>(quotes[2])
    };

    auto section = ext::make_shared<InterpolatedSmileSection<Linear>>(
        t1, strikes, handles,
        Handle<Quote>(ext::make_shared<SimpleQuote>(100.0)),
        Linear(), dc);

    std::vector<Date> dates = {d1};
    std::vector<ext::shared_ptr<SmileSection>> sections = {section};

    auto surface = ext::make_shared<PiecewiseBlackVarianceSurface>(
        today, dates, sections, dc);

    // trigger initial calculation so LazyObject forwards notifications
    surface->blackVariance(d1, 100.0);

    Flag flag;
    flag.registerWith(surface);

    // modify an underlying quote
    quotes[1]->setValue(0.30 * sqrtT);

    if (!flag.isUp())
        BOOST_FAIL("observer not notified after SmileSection quote change");
}

BOOST_AUTO_TEST_CASE(testStrikeDependence) {
    BOOST_TEST_MESSAGE(
        "Testing strike-dependent smile interpolation...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Date d1 = today + 1*Years;
    Time t1 = dc.yearFraction(today, d1);

    // build a skewed smile: lower vol at high strikes
    std::vector<Rate> strikes = {80.0, 100.0, 120.0};
    Volatility vol80 = 0.30, vol100 = 0.25, vol120 = 0.20;
    Real sqrtT = std::sqrt(t1);
    std::vector<Real> stdDevs = {
        vol80 * sqrtT, vol100 * sqrtT, vol120 * sqrtT
    };

    auto section = ext::make_shared<InterpolatedSmileSection<Linear>>(
        d1, strikes, stdDevs, 100.0, dc, Linear(), today);

    std::vector<Date> dates = {d1};
    std::vector<ext::shared_ptr<SmileSection>> sections = {section};

    PiecewiseBlackVarianceSurface surface(today, dates, sections, dc);

    Real tol = 1.0e-12;

    // check that variance matches at each strike point
    for (double strike : strikes) {
        Real expected = section->variance(strike);
        Real calculated = surface.blackVariance(d1, strike);
        Real diff = std::fabs(calculated - expected);

        if (diff > tol)
            BOOST_FAIL("strike-dependent repricing failed at strike " << strike
                       << "\n    calculated: " << std::setprecision(16) << std::scientific << calculated
                       << "\n    expected:   " << expected
                       << "\n    difference: " << diff
                       << "\n    tolerance:  " << tol);
    }

    // check different vol at different strikes
    Real var80 = surface.blackVariance(d1, 80.0);
    Real var120 = surface.blackVariance(d1, 120.0);

    if (var80 <= var120)
        BOOST_FAIL("expected higher variance at lower strike (skew)"
                   << "\n    var(80):  " << std::setprecision(16) << std::scientific << var80
                   << "\n    var(120): " << var120);
}

BOOST_AUTO_TEST_CASE(testMultiTenorSmileInterpolation) {
    BOOST_TEST_MESSAGE(
        "Testing interpolation between tenors with different smiles...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Date d1 = today + 6*Months;
    Date d2 = today + 1*Years;
    Time t1 = dc.yearFraction(today, d1);
    Time t2 = dc.yearFraction(today, d2);

    // tenor 1: moderate skew
    std::vector<Rate> strikes = {80.0, 100.0, 120.0};
    Real sqrtT1 = std::sqrt(t1);
    std::vector<Real> stdDevs1 = {
        0.25 * sqrtT1, 0.20 * sqrtT1, 0.18 * sqrtT1
    };
    auto section1 = ext::make_shared<InterpolatedSmileSection<Linear>>(
        d1, strikes, stdDevs1, 100.0, dc, Linear(), today);

    // tenor 2: steeper skew, higher overall level
    Real sqrtT2 = std::sqrt(t2);
    std::vector<Real> stdDevs2 = {
        0.35 * sqrtT2, 0.25 * sqrtT2, 0.20 * sqrtT2
    };
    auto section2 = ext::make_shared<InterpolatedSmileSection<Linear>>(
        d2, strikes, stdDevs2, 100.0, dc, Linear(), today);

    std::vector<Date> dates = {d1, d2};
    std::vector<ext::shared_ptr<SmileSection>> sections = {section1, section2};

    PiecewiseBlackVarianceSurface surface(today, dates, sections, dc);

    // query at midpoint time, at each strike
    Date dMid = today + 9*Months;
    Time tMid = dc.yearFraction(today, dMid);
    Real alpha = (tMid - t1) / (t2 - t1);
    Real tol = 1.0e-12;

    for (double strike : strikes) {
        Real var1 = section1->variance(strike);
        Real var2 = section2->variance(strike);
        Real expected = var1 + (var2 - var1) * alpha;
        Real calculated = surface.blackVariance(dMid, strike);
        Real diff = std::fabs(calculated - expected);

        if (diff > tol)
            BOOST_FAIL("multi-tenor smile interpolation failed at strike " << strike
                       << "\n    time:       " << tMid
                       << "\n    calculated: " << std::setprecision(16) << std::scientific << calculated
                       << "\n    expected:   " << expected
                       << "\n    difference: " << diff
                       << "\n    tolerance:  " << tol);
    }

    // verify the skew shape is preserved at the midpoint:
    // lower strikes should have higher variance
    Real varMid80 = surface.blackVariance(dMid, 80.0);
    Real varMid100 = surface.blackVariance(dMid, 100.0);
    Real varMid120 = surface.blackVariance(dMid, 120.0);

    if (varMid80 <= varMid100 || varMid100 <= varMid120)
        BOOST_FAIL("skew not preserved at interpolated tenor"
                   << "\n    var(80):  " << std::setprecision(16) << std::scientific << varMid80
                   << "\n    var(100): " << varMid100
                   << "\n    var(120): " << varMid120);

    // calendar arbitrage check: total variance must be non-decreasing in time
    for (double strike : strikes) {
        Real var_d1 = surface.blackVariance(d1, strike);
        Real var_dMid = surface.blackVariance(dMid, strike);
        Real var_d2 = surface.blackVariance(d2, strike);

        if (var_d1 > var_dMid + tol)
            BOOST_FAIL("calendar arbitrage: variance decreased from d1 to dMid"
                       << " at strike " << strike
                       << "\n    var(d1):   " << std::setprecision(16)
                       << std::scientific << var_d1
                       << "\n    var(dMid): " << var_dMid);

        if (var_dMid > var_d2 + tol)
            BOOST_FAIL("calendar arbitrage: variance decreased from dMid to d2"
                       << " at strike " << strike
                       << "\n    var(dMid): " << std::setprecision(16)
                       << std::scientific << var_dMid
                       << "\n    var(d2):   " << var_d2);
    }

    // butterfly arbitrage check: d^2w/dK^2 >= 0 at the interpolated tenor
    Real dK = 1.0;
    std::vector<Real> butterflyStrikes = {85.0, 90.0, 95.0, 100.0,
                                          105.0, 110.0, 115.0};

    for (double K : butterflyStrikes) {
        Real w   = surface.blackVariance(dMid, K);
        Real w_p = surface.blackVariance(dMid, K + dK);
        Real w_m = surface.blackVariance(dMid, K - dK);

        Real d2wdK2 = (w_p + w_m - 2.0 * w) / (dK * dK);

        if (d2wdK2 < -1.0e-10)
            BOOST_FAIL("butterfly arbitrage: d^2w/dK^2 < 0 at midpoint"
                       << " strike " << K
                       << "\n    w(K-dK): " << std::setprecision(16)
                       << std::scientific << w_m
                       << "\n    w(K):    " << w
                       << "\n    w(K+dK): " << w_p
                       << "\n    d2w/dK2: " << d2wdK2);
    }
}

BOOST_AUTO_TEST_CASE(testMakeFromGrid) {
    BOOST_TEST_MESSAGE(
        "Testing makeFromGrid factory method...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Date d1 = today + 6*Months;
    Date d2 = today + 1*Years;

    // strikes x dates matrix (rows = strikes, columns = dates)
    std::vector<Real> strikes = {80.0, 100.0, 120.0};
    Matrix blackVols(3, 2);
    blackVols[0][0] = 0.25; blackVols[0][1] = 0.30;   // K=80
    blackVols[1][0] = 0.20; blackVols[1][1] = 0.25;   // K=100
    blackVols[2][0] = 0.18; blackVols[2][1] = 0.20;   // K=120

    std::vector<Date> dates = {d1, d2};

    auto surface = PiecewiseBlackVarianceSurface::makeFromGrid(
        today, dates, strikes, blackVols, dc);

    Real tol = 1.0e-12;

    // check that each input vol is exactly repriced
    for (Size j = 0; j < dates.size(); ++j) {
        Time t = dc.yearFraction(today, dates[j]);
        for (Size i = 0; i < strikes.size(); ++i) {
            Real expectedVar = blackVols[i][j] * blackVols[i][j] * t;
            Real calculated = surface->blackVariance(dates[j], strikes[i]);
            Real diff = std::fabs(calculated - expectedVar);

            if (diff > tol)
                BOOST_FAIL("makeFromGrid failed to reprice"
                           << "\n    date:       " << dates[j]
                           << "\n    strike:     " << strikes[i]
                           << "\n    input vol:  " << blackVols[i][j]
                           << "\n    calculated: " << std::setprecision(16) << std::scientific << calculated
                           << "\n    expected:   " << expectedVar
                           << "\n    difference: " << diff
                           << "\n    tolerance:  " << tol);
        }
    }

    // verify skew is preserved (higher var at lower strike)
    Real var80 = surface->blackVariance(d1, 80.0);
    Real var120 = surface->blackVariance(d1, 120.0);

    if (var80 <= var120)
        BOOST_FAIL("makeFromGrid: skew not preserved"
                   << "\n    var(80):  " << std::setprecision(16) << std::scientific << var80
                   << "\n    var(120): " << var120);

    // verify interpolation between strikes (K=90, between K=80 and K=100)
    Time t1 = dc.yearFraction(today, d1);
    Real vol90 = 0.5 * (blackVols[0][0] + blackVols[1][0]);  // linear interp
    Real expectedVar90 = vol90 * vol90 * t1;
    Real calculated90 = surface->blackVariance(d1, 90.0);
    Real diff90 = std::fabs(calculated90 - expectedVar90);

    if (diff90 > tol)
        BOOST_FAIL("makeFromGrid: between-strike interpolation failed"
                   << "\n    strike:     90.0"
                   << "\n    calculated: " << std::setprecision(16) << std::scientific << calculated90
                   << "\n    expected:   " << expectedVar90
                   << "\n    difference: " << diff90
                   << "\n    tolerance:  " << tol);

    // verify interpolation between tenors at a grid strike
    Time t2 = dc.yearFraction(today, d2);
    Date dMid = today + 9*Months;
    Time tMid = dc.yearFraction(today, dMid);
    Real alpha = (tMid - t1) / (t2 - t1);
    Real var1_100 = blackVols[1][0] * blackVols[1][0] * t1;
    Real var2_100 = blackVols[1][1] * blackVols[1][1] * t2;
    Real expectedVarMid = var1_100 + (var2_100 - var1_100) * alpha;
    Real calculatedMid = surface->blackVariance(dMid, 100.0);
    Real diffMid = std::fabs(calculatedMid - expectedVarMid);

    if (diffMid > tol)
        BOOST_FAIL("makeFromGrid: between-tenor interpolation failed"
                   << "\n    date:       " << dMid
                   << "\n    strike:     100.0"
                   << "\n    calculated: " << std::setprecision(16) << std::scientific << calculatedMid
                   << "\n    expected:   " << expectedVarMid
                   << "\n    difference: " << diffMid
                   << "\n    tolerance:  " << tol);
}

BOOST_AUTO_TEST_CASE(testConstructorValidation) {
    BOOST_TEST_MESSAGE(
        "Testing constructor validation checks...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Date d1 = today + 6*Months;
    Date d2 = today + 1*Years;
    Volatility vol = 0.25;
    auto section1 = ext::make_shared<FlatSmileSection>(d1, vol, dc, today);
    auto section2 = ext::make_shared<FlatSmileSection>(d2, vol, dc, today);

    // empty dates
    BOOST_CHECK_EXCEPTION(
        PiecewiseBlackVarianceSurface(today, {}, {}, dc),
        Error,
        ExpectedErrorMessage("at least one date"));

    // mismatched sizes (2 dates, 1 section)
    BOOST_CHECK_EXCEPTION(
        PiecewiseBlackVarianceSurface(
            today, {d1, d2}, {section1}, dc),
        Error,
        ExpectedErrorMessage("mismatch"));

    // first date on reference date
    BOOST_CHECK_EXCEPTION(
        PiecewiseBlackVarianceSurface(
            today, {today},
            {ext::make_shared<FlatSmileSection>(today, vol, dc, today)},
            dc),
        Error,
        ExpectedErrorMessage("must be after reference"));

    // unsorted dates
    BOOST_CHECK_EXCEPTION(
        PiecewiseBlackVarianceSurface(
            today, {d2, d1}, {section2, section1}, dc),
        Error,
        ExpectedErrorMessage("sorted and unique"));

    // duplicate dates
    BOOST_CHECK_EXCEPTION(
        PiecewiseBlackVarianceSurface(
            today, {d1, d1}, {section1, section1}, dc),
        Error,
        ExpectedErrorMessage("sorted and unique"));

    // null smile section
    BOOST_CHECK_EXCEPTION(
        PiecewiseBlackVarianceSurface(
            today, {d1},
            {ext::shared_ptr<SmileSection>()},
            dc),
        Error,
        ExpectedErrorMessage("null smile section"));
}

BOOST_AUTO_TEST_CASE(testMakeFromGridValidation) {
    BOOST_TEST_MESSAGE(
        "Testing makeFromGrid validation checks...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Date d1 = today + 6*Months;
    std::vector<Date> dates = {d1};
    std::vector<Real> strikes = {80.0, 100.0, 120.0};

    // row mismatch (3 strikes but 2-row matrix)
    Matrix wrongRows(2, 1);
    wrongRows[0][0] = 0.20;
    wrongRows[1][0] = 0.25;

    BOOST_CHECK_EXCEPTION(
        PiecewiseBlackVarianceSurface::makeFromGrid(
            today, dates, strikes, wrongRows, dc),
        Error,
        ExpectedErrorMessage("strikes"));

    // column mismatch (1 date but 2-column matrix)
    Matrix wrongCols(3, 2);
    for (Size i = 0; i < 3; ++i)
        for (Size j = 0; j < 2; ++j)
            wrongCols[i][j] = 0.20;

    BOOST_CHECK_EXCEPTION(
        PiecewiseBlackVarianceSurface::makeFromGrid(
            today, dates, strikes, wrongCols, dc),
        Error,
        ExpectedErrorMessage("dates"));
}

BOOST_AUTO_TEST_CASE(testAccessors) {
    BOOST_TEST_MESSAGE(
        "Testing dayCounter, maxDate, minStrike, maxStrike...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Date d1 = today + 6*Months;
    Date d2 = today + 1*Years;
    Volatility vol = 0.25;

    std::vector<Date> dates = {d1, d2};
    std::vector<ext::shared_ptr<SmileSection>> sections = {
        ext::make_shared<FlatSmileSection>(d1, vol, dc, today),
        ext::make_shared<FlatSmileSection>(d2, vol, dc, today)
    };

    PiecewiseBlackVarianceSurface surface(today, dates, sections, dc);

    if (surface.dayCounter().name() != dc.name())
        BOOST_FAIL("dayCounter mismatch"
                   << "\n    expected: " << dc.name()
                   << "\n    got:      " << surface.dayCounter().name());

    if (surface.maxDate() != d2)
        BOOST_FAIL("maxDate mismatch"
                   << "\n    expected: " << d2
                   << "\n    got:      " << surface.maxDate());

    if (surface.minStrike() != QL_MIN_REAL)
        BOOST_FAIL("minStrike not QL_MIN_REAL");

    if (surface.maxStrike() != QL_MAX_REAL)
        BOOST_FAIL("maxStrike not QL_MAX_REAL");
}

BOOST_AUTO_TEST_CASE(testZeroTimeVariance) {
    BOOST_TEST_MESSAGE(
        "Testing blackVariance at reference date (t=0)...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Date d1 = today + 6*Months;

    std::vector<Date> dates = {d1};
    std::vector<ext::shared_ptr<SmileSection>> sections = {
        ext::make_shared<FlatSmileSection>(d1, 0.25, dc, today)
    };

    PiecewiseBlackVarianceSurface surface(today, dates, sections, dc);

    Real calculated = surface.blackVariance(today, 100.0);

    if (calculated != 0.0)
        BOOST_FAIL("blackVariance at t=0 should be exactly 0.0"
                   << "\n    calculated: " << std::setprecision(16)
                   << std::scientific << calculated);
}

BOOST_AUTO_TEST_CASE(testSingleTenorSurface) {
    BOOST_TEST_MESSAGE(
        "Testing surface with a single tenor...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Date d1 = today + 1*Years;
    Volatility vol = 0.25;
    Time t1 = dc.yearFraction(today, d1);

    std::vector<Date> dates = {d1};
    std::vector<ext::shared_ptr<SmileSection>> sections = {
        ext::make_shared<FlatSmileSection>(d1, vol, dc, today)
    };

    PiecewiseBlackVarianceSurface surface(today, dates, sections, dc);
    surface.enableExtrapolation();

    Real strike = 100.0;
    Real var1 = vol * vol * t1;
    Real tol = 1.0e-12;

    // exact at tenor
    Real calculated = surface.blackVariance(d1, strike);
    Real diff = std::fabs(calculated - var1);
    if (diff > tol)
        BOOST_FAIL("single tenor: failed at exact tenor"
                   << "\n    calculated: " << std::setprecision(16)
                   << std::scientific << calculated
                   << "\n    expected:   " << var1);

    // before tenor: linear from (0,0)
    Date dEarly = today + 3*Months;
    Time tEarly = dc.yearFraction(today, dEarly);
    Real expectedEarly = var1 * tEarly / t1;
    Real calculatedEarly = surface.blackVariance(dEarly, strike);
    diff = std::fabs(calculatedEarly - expectedEarly);
    if (diff > tol)
        BOOST_FAIL("single tenor: failed before tenor"
                   << "\n    calculated: " << std::setprecision(16)
                   << std::scientific << calculatedEarly
                   << "\n    expected:   " << expectedEarly);

    // after tenor: flat vol extrapolation
    Date dLate = today + 2*Years;
    Time tLate = dc.yearFraction(today, dLate);
    Real expectedLate = var1 * tLate / t1;
    Real calculatedLate = surface.blackVariance(dLate, strike, true);
    diff = std::fabs(calculatedLate - expectedLate);
    if (diff > tol)
        BOOST_FAIL("single tenor: failed after tenor"
                   << "\n    calculated: " << std::setprecision(16)
                   << std::scientific << calculatedLate
                   << "\n    expected:   " << expectedLate);
}

BOOST_AUTO_TEST_CASE(testRaggedStrikeGrids) {
    BOOST_TEST_MESSAGE(
        "Testing surface with ragged strike grids across tenors...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    Date d1 = today + 6*Months;
    Date d2 = today + 1*Years;
    Time t1 = dc.yearFraction(today, d1);
    Time t2 = dc.yearFraction(today, d2);

    // tenor 1: strikes [80, 100, 120] — skewed
    std::vector<Rate> strikes1 = {80.0, 100.0, 120.0};
    Real sqrtT1 = std::sqrt(t1);
    std::vector<Real> stdDevs1 = {
        0.30 * sqrtT1, 0.20 * sqrtT1, 0.18 * sqrtT1
    };
    auto section1 = ext::make_shared<InterpolatedSmileSection<Linear>>(
        d1, strikes1, stdDevs1, 100.0, dc, Linear(), today);

    // tenor 2: strikes [70, 90, 110, 130] — different grid, wider
    std::vector<Rate> strikes2 = {70.0, 90.0, 110.0, 130.0};
    Real sqrtT2 = std::sqrt(t2);
    std::vector<Real> stdDevs2 = {
        0.35 * sqrtT2, 0.25 * sqrtT2, 0.22 * sqrtT2, 0.20 * sqrtT2
    };
    auto section2 = ext::make_shared<InterpolatedSmileSection<Linear>>(
        d2, strikes2, stdDevs2, 100.0, dc, Linear(), today);

    std::vector<Date> dates = {d1, d2};
    std::vector<ext::shared_ptr<SmileSection>> sections = {section1, section2};

    PiecewiseBlackVarianceSurface surface(today, dates, sections, dc);

    Real tol = 1.0e-12;

    // exact repricing at each tenor's own strike grid
    for (double i : strikes1) {
        Real expected = section1->variance(i);
        Real calculated = surface.blackVariance(d1, i);
        Real diff = std::fabs(calculated - expected);
        if (diff > tol)
            BOOST_FAIL("ragged grid: failed to reprice at tenor 1 strike "
                       << i
                       << "\n    calculated: " << std::setprecision(16)
                       << std::scientific << calculated
                       << "\n    expected:   " << expected);
    }
    for (double i : strikes2) {
        Real expected = section2->variance(i);
        Real calculated = surface.blackVariance(d2, i);
        Real diff = std::fabs(calculated - expected);
        if (diff > tol)
            BOOST_FAIL("ragged grid: failed to reprice at tenor 2 strike "
                       << i
                       << "\n    calculated: " << std::setprecision(16)
                       << std::scientific << calculated
                       << "\n    expected:   " << expected);
    }

    // strike 75 is inside section 2's grid [70..130] but outside
    // section 1's grid [80..120] — must throw without extrapolation
    Real strike = 75.0;
    Date dMid = today + 9*Months;
    BOOST_CHECK_EXCEPTION(surface.blackVariance(dMid, strike),
                          Error,
                          ExpectedErrorMessage("outside the range"));

    // enable extrapolation — now the query should succeed
    surface.enableExtrapolation();

    Time tMid = dc.yearFraction(today, dMid);
    Real alpha = (tMid - t1) / (t2 - t1);
    Real var1 = section1->variance(strike);
    Real var2 = section2->variance(strike);
    Real expected = var1 + (var2 - var1) * alpha;
    Real calculated = surface.blackVariance(dMid, strike);
    Real diff = std::fabs(calculated - expected);
    if (diff > tol)
        BOOST_FAIL("ragged grid: interpolation at off-grid strike " << strike
                   << "\n    calculated: " << std::setprecision(16)
                   << std::scientific << calculated
                   << "\n    expected:   " << expected);

    // calendar arbitrage: variance non-decreasing across tenors
    std::vector<Real> testStrikes = {70.0, 80.0, 90.0, 100.0, 110.0, 120.0, 130.0};
    for (double testStrike : testStrikes) {
        Real var_d1 = surface.blackVariance(d1, testStrike, true);
        Real var_dMid = surface.blackVariance(dMid, testStrike, true);
        Real var_d2 = surface.blackVariance(d2, testStrike, true);

        if (var_d1 > var_dMid + tol)
            BOOST_FAIL("ragged grid: calendar arbitrage d1->dMid at strike "
                       << testStrike
                       << "\n    var(d1):   " << std::setprecision(16)
                       << std::scientific << var_d1
                       << "\n    var(dMid): " << var_dMid);

        if (var_dMid > var_d2 + tol)
            BOOST_FAIL("ragged grid: calendar arbitrage dMid->d2 at strike "
                       << testStrike
                       << "\n    var(dMid): " << std::setprecision(16)
                       << std::scientific << var_dMid
                       << "\n    var(d2):   " << var_d2);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
