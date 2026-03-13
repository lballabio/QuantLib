/*
 Copyright (C) 2019 Quaternion Risk Management Ltd
 Copyright (C) 2020 Skandinaviska Enskilda Banken AB (publ)
 Copyright (C) 2025 Paolo D'Elia

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
#include <boost/make_shared.hpp>
#include <boost/test/unit_test.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/termstructures/volatility/equityfx/blackvolsurfacedelta.hpp>

using namespace boost::unit_test_framework;
using namespace QuantLib;
using namespace std;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(BlackVolSurfaceDeltaTest)

BOOST_AUTO_TEST_CASE(testBlackVolSurfaceDeltaConstantVol) {

    BOOST_TEST_MESSAGE("Testing BlackVolatilitySurfaceDelta...");

    Date refDate(1, Jan, 2010);
    Settings::instance().evaluationDate() = refDate;

    Volatility constVol = 0.10; // 10%

    // Setup a 2x2
    vector<Date> dates = { Date(1, Jan, 2011), Date(1, Jan, 2012) };
    vector<Real> putDeltas = { -0.25 };
    vector<Real> callDeltas = { 0.25 };
    bool hasAtm = false;
    Matrix blackVolMatrix(2, 2, constVol);

    // dummy spot and zero yield curve
    Handle<Quote> spot(ext::make_shared<SimpleQuote>(1.0));
    Handle<YieldTermStructure> dts(ext::make_shared<FlatForward>(0, TARGET(), 0.011, ActualActual(ActualActual::ISDA)));
    Handle<YieldTermStructure> fts(ext::make_shared<FlatForward>(0, TARGET(), 0.012, ActualActual(ActualActual::ISDA)));

    // build a vol surface
    BlackVolatilitySurfaceDelta surface(refDate, dates, putDeltas, callDeltas, hasAtm, blackVolMatrix, ActualActual(ActualActual::ISDA),
                                        TARGET(), spot, dts, fts);

    // ask for volatility at lots of points, should be constVol at every point
    // make sure we ask for vols outside 25D and 2Y
    for (Time t : { 0.25, 0.5, 1.0, 1.5, 2.0, 2.5, 10.0 }) {
        for (Real k = 0.5; k < 2.0; k += 0.05) {
            Volatility vol = surface.blackVol(t, k);
            BOOST_CHECK_EQUAL(vol, constVol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testBlackVolSurfaceDeltaNonConstantVol) {

    BOOST_TEST_MESSAGE("Testing BlackVolatilitySurfaceDelta with non constant vol surface...");

    Date refDate(1, Jan, 2010);
    Settings::instance().evaluationDate() = refDate;

    Matrix vols = {
        {0.15, 0.13, 0.135}, // 1M
        {0.14, 0.11, 0.125}, // 6M
        {0.13, 0.10, 0.12}, // 1Y
        {0.125, 0.095, 0.115}, // 2Y
    };

    // Setup a 4x3
    vector<Date> dates = {
        refDate + Period(1, Months),
        refDate + Period(6, Months),
        refDate + Period(1, Years),
        refDate + Period(2, Years)
    };
    vector<Real> putDeltas = { -0.25 };
    vector<Real> callDeltas = { 0.25 };
    bool hasAtm = true;

    // dummy spot and zero yield curve
    Handle<Quote> spot(ext::make_shared<SimpleQuote>(1.18));
    Handle<YieldTermStructure> dts(ext::make_shared<FlatForward>(0, TARGET(), 0.02, ActualActual(ActualActual::ISDA)));
    Handle<YieldTermStructure> fts(ext::make_shared<FlatForward>(0, TARGET(), 0.035, ActualActual(ActualActual::ISDA)));

    // build a vol surface
    BlackVolatilitySurfaceDelta surface(refDate, dates, putDeltas, callDeltas, hasAtm, vols, ActualActual(ActualActual::ISDA),
                                        TARGET(), spot, dts, fts);

    // Testing ATM strike
    Real atmStrike = 1.18;

    // ask for volatility at 1M (present in the matrix)
    auto smile1M = surface.blackVolSmile(refDate + Period(1, Months));
    BOOST_CHECK_CLOSE(smile1M->volatility(atmStrike), 0.13010360399, 1e-8);

    // ask for volatility at 15D (using time interpolation) should be the same as the 1M
    auto smile15D = surface.blackVolSmile(refDate + Period(15, Days));
    BOOST_CHECK_CLOSE(smile15D->volatility(atmStrike), 0.13007226607, 1e-8);

    // ask for volatility at 3M (using time interpolation)
    auto smile3M = surface.blackVolSmile(refDate + Period(3, Months));
    BOOST_CHECK_CLOSE(smile3M->volatility(atmStrike), 0.115077252583, 1e-8);

    // ask for volatility at 6M for "extreme" strikes (using strike extrapolation)
    Real lowStrike = 1.10;
    Real highStrike = 1.30;

    auto smile6M = surface.blackVolSmile(refDate + Period(6, Months));
    BOOST_CHECK_CLOSE(smile6M->volatility(lowStrike), 0.1411379628132, 1e-8);
    BOOST_CHECK_CLOSE(smile6M->volatility(highStrike), 0.136291154962, 1e-8);
}


BOOST_AUTO_TEST_CASE(testTimeExtrapolation) {

    BOOST_TEST_MESSAGE("Testing time extrapolation of BlackVolatilitySurfaceDelta...");

    Date refDate(1, Jan, 2010);
    Settings::instance().evaluationDate() = refDate;

    Matrix vols = {
        {0.15, 0.13, 0.135}, // 1M
        {0.14, 0.11, 0.125}, // 6M
        {0.13, 0.10, 0.12}, // 1Y
        {0.125, 0.095, 0.115}, // 2Y
    };

    vector<Date> dates = {
        refDate + Period(1, Months),
        refDate + Period(6, Months),
        refDate + Period(1, Years),
        refDate + Period(2, Years)
    };
    vector<Real> putDeltas = { -0.25 };
    vector<Real> callDeltas = { 0.25 };
    bool hasAtm = true;
    Real atmStrike = 1.18;

    Handle<Quote> spot(ext::make_shared<SimpleQuote>(1.18));
    Handle<YieldTermStructure> dts(ext::make_shared<FlatForward>(0, TARGET(), 0.02, ActualActual(ActualActual::ISDA)));
    Handle<YieldTermStructure> fts(ext::make_shared<FlatForward>(0, TARGET(), 0.035, ActualActual(ActualActual::ISDA)));

    // flat volatility extrapolation

    BlackVolatilitySurfaceDelta surface1(refDate, dates, putDeltas, callDeltas, hasAtm, vols, ActualActual(ActualActual::ISDA),
                                         TARGET(), spot, dts, fts, DeltaVolQuote::DeltaType::Spot, DeltaVolQuote::AtmType::AtmSpot,
                                         ext::nullopt, BlackVolatilitySurfaceDelta::SmileInterpolationMethod::Linear, false,
                                         BlackVolTimeExtrapolation::FlatVolatility);

    BOOST_CHECK_CLOSE(surface1.blackVol(refDate + Period(2, Years), atmStrike), 0.095, 1e-8);
    BOOST_CHECK_CLOSE(surface1.blackVol(refDate + Period(2, Years), atmStrike - 0.1), 0.11684859871, 1e-8);
    BOOST_CHECK_CLOSE(surface1.blackVol(refDate + Period(2, Years), atmStrike + 0.1), 0.11438709864, 1e-8);
    BOOST_CHECK_CLOSE(surface1.blackVol(refDate + Period(3, Years), atmStrike), 0.095, 1e-8);
    BOOST_CHECK_CLOSE(surface1.blackVol(refDate + Period(3, Years), atmStrike - 0.1), 0.11684859871, 1e-8);
    BOOST_CHECK_CLOSE(surface1.blackVol(refDate + Period(3, Years), atmStrike + 0.1), 0.11438709864, 1e-8);

    // linear variance extrapolation

    BlackVolatilitySurfaceDelta surface2(refDate, dates, putDeltas, callDeltas, hasAtm, vols, ActualActual(ActualActual::ISDA),
                                         TARGET(), spot, dts, fts, DeltaVolQuote::DeltaType::Spot, DeltaVolQuote::AtmType::AtmSpot,
                                         ext::nullopt, BlackVolatilitySurfaceDelta::SmileInterpolationMethod::Linear, false,
                                         BlackVolTimeExtrapolation::LinearVariance);

    BOOST_CHECK_CLOSE(surface2.blackVol(refDate + Period(2, Years), atmStrike), 0.095, 1e-8);
    BOOST_CHECK_CLOSE(surface2.blackVol(refDate + Period(2, Years), atmStrike - 0.1), 0.11684859871, 1e-8);
    BOOST_CHECK_CLOSE(surface2.blackVol(refDate + Period(2, Years), atmStrike + 0.1), 0.11438709864, 1e-8);
    BOOST_CHECK_CLOSE(surface2.blackVol(refDate + Period(3, Years), atmStrike), 0.09327379053, 1e-8);
    BOOST_CHECK_CLOSE(surface2.blackVol(refDate + Period(3, Years), atmStrike - 0.1), 0.11174756764, 1e-8);
    BOOST_CHECK_CLOSE(surface2.blackVol(refDate + Period(3, Years), atmStrike + 0.1), 0.11128755593, 1e-8);

    // delegate to underlying interpolator (also linear variance)

    BlackVolatilitySurfaceDelta surface3(refDate, dates, putDeltas, callDeltas, hasAtm, vols, ActualActual(ActualActual::ISDA),
                                         TARGET(), spot, dts, fts, DeltaVolQuote::DeltaType::Spot, DeltaVolQuote::AtmType::AtmSpot,
                                         ext::nullopt, BlackVolatilitySurfaceDelta::SmileInterpolationMethod::Linear, false,
                                         BlackVolTimeExtrapolation::UseInterpolator);
    surface3.enableExtrapolation();

    BOOST_CHECK_CLOSE(surface3.blackVol(refDate + Period(2, Years), atmStrike), 0.095, 1e-8);
    BOOST_CHECK_CLOSE(surface3.blackVol(refDate + Period(2, Years), atmStrike - 0.1), 0.11684859871, 1e-8);
    BOOST_CHECK_CLOSE(surface3.blackVol(refDate + Period(2, Years), atmStrike + 0.1), 0.11438709864, 1e-8);
    BOOST_CHECK_CLOSE(surface3.blackVol(refDate + Period(3, Years), atmStrike), 0.09327379053, 1e-8);
    BOOST_CHECK_CLOSE(surface3.blackVol(refDate + Period(3, Years), atmStrike - 0.1), 0.11174756764, 1e-8);
    BOOST_CHECK_CLOSE(surface3.blackVol(refDate + Period(3, Years), atmStrike + 0.1), 0.11128755593, 1e-8);
}


BOOST_AUTO_TEST_CASE(testSmileInterpolation) {

    BOOST_TEST_MESSAGE("Testing smile interpolation of BlackVolatilitySurfaceDelta...");

    Date refDate(1, Jan, 2010);
    Settings::instance().evaluationDate() = refDate;

    Matrix vols = {
        {0.15, 0.13, 0.135}, // 1M
        {0.14, 0.11, 0.125}, // 6M
        {0.13, 0.10, 0.12}, // 1Y
        {0.125, 0.095, 0.115}, // 2Y
    };

    vector<Date> dates = {
        refDate + Period(1, Months),
        refDate + Period(6, Months),
        refDate + Period(1, Years),
        refDate + Period(2, Years)
    };
    vector<Real> putDeltas = { -0.25 };
    vector<Real> callDeltas = { 0.25 };
    bool hasAtm = true;
    Real atmStrike = 1.18;

    Handle<Quote> spot(ext::make_shared<SimpleQuote>(1.18));
    Handle<YieldTermStructure> dts(ext::make_shared<FlatForward>(0, TARGET(), 0.02, ActualActual(ActualActual::ISDA)));
    Handle<YieldTermStructure> fts(ext::make_shared<FlatForward>(0, TARGET(), 0.035, ActualActual(ActualActual::ISDA)));

    // just cache some values to check that they're not insane and that they don't change unexpectedly

    BlackVolatilitySurfaceDelta surface1(refDate, dates, putDeltas, callDeltas, hasAtm, vols, ActualActual(ActualActual::ISDA),
                                         TARGET(), spot, dts, fts, DeltaVolQuote::DeltaType::Spot, DeltaVolQuote::AtmType::AtmSpot,
                                         ext::nullopt, BlackVolatilitySurfaceDelta::SmileInterpolationMethod::Linear, false,
                                         BlackVolTimeExtrapolation::FlatVolatility);

    auto smile = surface1.blackVolSmile(refDate + Period(6, Months));
    BOOST_CHECK_CLOSE(smile->volatility(atmStrike), 0.11, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(atmStrike - 0.1), 0.14882625471, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(atmStrike + 0.1), 0.13265179475, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(smile->minStrike() - 0.1), 0.17882625471, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(smile->maxStrike() + 0.1), 0.14765179475, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(smile->minStrike() - 0.5), 0.33413127354, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(smile->maxStrike() + 0.5), 0.23825897375, 1e-8);

    BlackVolatilitySurfaceDelta surface2(refDate, dates, putDeltas, callDeltas, hasAtm, vols, ActualActual(ActualActual::ISDA),
                                         TARGET(), spot, dts, fts, DeltaVolQuote::DeltaType::Spot, DeltaVolQuote::AtmType::AtmSpot,
                                         ext::nullopt, BlackVolatilitySurfaceDelta::SmileInterpolationMethod::NaturalCubic, false,
                                         BlackVolTimeExtrapolation::FlatVolatility);

    smile = surface2.blackVolSmile(refDate + Period(6, Months));
    BOOST_CHECK_CLOSE(smile->volatility(atmStrike), 0.11, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(atmStrike - 0.1), 0.15285738778, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(atmStrike + 0.1), 0.13548210924, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(smile->minStrike() - 0.1), 0.16572286711, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(smile->maxStrike() + 0.1), 0.13314942082, 1e-8);
    // cubic being cubic
    BOOST_CHECK_CLOSE(smile->volatility(smile->minStrike() - 0.5), 0.0, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(smile->maxStrike() + 0.5), 0.0, 1e-8);

    BlackVolatilitySurfaceDelta surface3(refDate, dates, putDeltas, callDeltas, hasAtm, vols, ActualActual(ActualActual::ISDA),
                                         TARGET(), spot, dts, fts, DeltaVolQuote::DeltaType::Spot, DeltaVolQuote::AtmType::AtmSpot,
                                         ext::nullopt, BlackVolatilitySurfaceDelta::SmileInterpolationMethod::FinancialCubic, false,
                                         BlackVolTimeExtrapolation::FlatVolatility);

    smile = surface3.blackVolSmile(refDate + Period(6, Months));
    BOOST_CHECK_CLOSE(smile->volatility(atmStrike), 0.11, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(atmStrike - 0.1), 0.15285738778, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(atmStrike + 0.1), 0.13548210924, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(smile->minStrike() - 0.1), 0.16572286711, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(smile->maxStrike() + 0.1), 0.13314942082, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(smile->minStrike() - 0.5), 0.0, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(smile->maxStrike() + 0.5), 0.0, 1e-8);

    BlackVolatilitySurfaceDelta surface4(refDate, dates, putDeltas, callDeltas, hasAtm, vols, ActualActual(ActualActual::ISDA),
                                         TARGET(), spot, dts, fts, DeltaVolQuote::DeltaType::Spot, DeltaVolQuote::AtmType::AtmSpot,
                                         ext::nullopt, BlackVolatilitySurfaceDelta::SmileInterpolationMethod::CubicSpline, false,
                                         BlackVolTimeExtrapolation::FlatVolatility);

    smile = surface4.blackVolSmile(refDate + Period(6, Months));
    BOOST_CHECK_CLOSE(smile->volatility(atmStrike), 0.11, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(atmStrike - 0.1), 0.15226345029, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(atmStrike + 0.1), 0.13619688725, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(smile->minStrike() - 0.1), 0.16765348886, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(smile->maxStrike() + 0.1), 0.12948693808, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(smile->minStrike() - 0.5), 0.0, 1e-8);
    BOOST_CHECK_CLOSE(smile->volatility(smile->maxStrike() + 0.5), 0.0, 1e-8);

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
