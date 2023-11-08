/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2005, 2007 StatPro Italia srl
 Copyright (C) 2006 Warren Chou

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

#include "speedlevel.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/instruments/lookbackoption.hpp>
#include <ql/pricingengines/lookback/analyticcontinuousfloatinglookback.hpp>
#include <ql/pricingengines/lookback/analyticcontinuousfixedlookback.hpp>
#include <ql/pricingengines/lookback/analyticcontinuouspartialfloatinglookback.hpp>
#include <ql/pricingengines/lookback/analyticcontinuouspartialfixedlookback.hpp>
#include <ql/pricingengines/lookback/mclookbackengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/processes/blackscholesprocess.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#undef REPORT_FAILURE_FLOATING
#define REPORT_FAILURE_FLOATING(greekName, minmax, payoff, exercise, \
                                s, q, r, today, v, \
                                expected, calculated, error, tolerance) \
    BOOST_ERROR( \
        exerciseTypeToString(exercise) \
        << payoff->optionType() << " lookback option with " \
        << payoffTypeToString(payoff) << " payoff:\n" \
        << "    underlying value: " << s << "\n" \
        << "    dividend yield:   " << io::rate(q) << "\n" \
        << "    risk-free rate:   " << io::rate(r) << "\n" \
        << "    reference date:   " << today << "\n" \
        << "    maturity:         " << exercise->lastDate() << "\n" \
        << "    volatility:       " << io::volatility(v) << "\n\n" \
        << "    expected   " << greekName << ": " << expected << "\n" \
        << "    calculated " << greekName << ": " << calculated << "\n"\
        << "    error:            " << error << "\n" \
        << "    tolerance:        " << tolerance);

#undef REPORT_FAILURE_FIXED
#define REPORT_FAILURE_FIXED(greekName, minmax, payoff, exercise, \
                             s, q, r, today, v, \
                             expected, calculated, error, tolerance) \
    BOOST_ERROR( \
        exerciseTypeToString(exercise) \
        << payoff->optionType() << " lookback option with " \
        << payoffTypeToString(payoff) << " payoff:\n" \
        << "    underlying value: " << s << "\n" \
        << "    strike:           " << payoff->strike() << "\n" \
        << "    dividend yield:   " << io::rate(q) << "\n" \
        << "    risk-free rate:   " << io::rate(r) << "\n" \
        << "    reference date:   " << today << "\n" \
        << "    maturity:         " << exercise->lastDate() << "\n" \
        << "    volatility:       " << io::volatility(v) << "\n\n" \
        << "    expected   " << greekName << ": " << expected << "\n" \
        << "    calculated " << greekName << ": " << calculated << "\n"\
        << "    error:            " << error << "\n" \
        << "    tolerance:        " << tolerance);

#undef REPORT_FAILURE_MC
#define REPORT_FAILURE_MC(lookbackType, optionType, analytical, monteCarlo, tolerance) \
    BOOST_ERROR( \
        "Analytical and MC " << lookbackType << " " << optionType << " values differed by more than tolerance" << "\n" \
        << "    Analytical:    " << analytical << "\n" \
        << "    Monte Carlo:   " << monteCarlo << "\n" \
        << "    tolerance:     " << tolerance << "\n" \
        << "    difference:    " << std::abs(analytical - monteCarlo));

namespace {

    struct LookbackOptionData {
        Option::Type type;
        Real strike;
        Real minmax;
        Real s;        // spot
        Rate q;        // dividend
        Rate r;        // risk-free rate
        Time t;        // time to maturity
        Volatility v;  // volatility

        //Partial-time lookback options:
        Real l;        // level above/below actual extremum
        Real t1;       // time to start of lookback period

        Real result;   // result
        Real tol;      // tolerance
    };

}

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(LookbackOptionTest)

BOOST_AUTO_TEST_CASE(testAnalyticContinuousFloatingLookback) {

    BOOST_TEST_MESSAGE(
           "Testing analytic continuous floating-strike lookback options...");


    LookbackOptionData values[] = {

        // data from "Option Pricing Formulas", Haug, 1998, pg.61-62

        // type,             strike, minmax, s,     q,    r,    t,    v,    l, t1, result,  tol
        {  Option::Call,     0,      100,    120.0, 0.06, 0.10, 0.50, 0.30, 0, 0,  25.3533, 1.0e-4},

        // data from "Connecting discrete and continuous path-dependent options",
        // Broadie, Glasserman & Kou, 1999, pg.70-74

        // type,             strike, minmax, s,     q,    r,    t,    v,    l, t1, result,  tol
        {  Option::Call,     0,      100,    100.0, 0.00, 0.05, 1.00, 0.30, 0, 0,  23.7884, 1.0e-4},
        {  Option::Call,     0,      100,    100.0, 0.00, 0.05, 0.20, 0.30, 0, 0,  10.7190, 1.0e-4},
        {  Option::Call,     0,      100,    110.0, 0.00, 0.05, 0.20, 0.30, 0, 0,  14.4597, 1.0e-4},
        {  Option::Put,      0,      100,    100.0, 0.00, 0.10, 0.50, 0.30, 0, 0,  15.3526, 1.0e-4},
        {  Option::Put,      0,      110,    100.0, 0.00, 0.10, 0.50, 0.30, 0, 0,  16.8468, 1.0e-4},
        {  Option::Put,      0,      120,    100.0, 0.00, 0.10, 0.50, 0.30, 0, 0,  21.0645, 1.0e-4},
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (auto& value : values) {
        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        ext::shared_ptr<FloatingTypePayoff> payoff(new FloatingTypePayoff(value.type));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(
                                       Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engine(
                  new AnalyticContinuousFloatingLookbackEngine(stochProcess));

        ContinuousFloatingLookbackOption option(value.minmax, payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = value.result;
        Real error = std::fabs(calculated-expected);
        if (error > value.tol) {
            REPORT_FAILURE_FLOATING("value", values[i].minmax, payoff, exercise, value.s, value.q,
                                    value.r, today, value.v, expected, calculated, error,
                                    value.tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testAnalyticContinuousFixedLookback) {

    BOOST_TEST_MESSAGE(
              "Testing analytic continuous fixed-strike lookback options...");

    LookbackOptionData values[] = {
        // data from "Option Pricing Formulas", Haug, 1998, pg.63-64
        //type,            strike, minmax,  s,     q,    r,    t,    v,    l, t1, result,  tol
        { Option::Call,    95,     100,     100.0, 0.00, 0.10, 0.50, 0.10, 0, 0,  13.2687, 1.0e-4},
        { Option::Call,    95,     100,     100.0, 0.00, 0.10, 0.50, 0.20, 0, 0,  18.9263, 1.0e-4},
        { Option::Call,    95,     100,     100.0, 0.00, 0.10, 0.50, 0.30, 0, 0,  24.9857, 1.0e-4},
        { Option::Call,    100,    100,     100.0, 0.00, 0.10, 0.50, 0.10, 0, 0,   8.5126, 1.0e-4},
        { Option::Call,    100,    100,     100.0, 0.00, 0.10, 0.50, 0.20, 0, 0,  14.1702, 1.0e-4},
        { Option::Call,    100,    100,     100.0, 0.00, 0.10, 0.50, 0.30, 0, 0,  20.2296, 1.0e-4},
        { Option::Call,    105,    100,     100.0, 0.00, 0.10, 0.50, 0.10, 0, 0,   4.3908, 1.0e-4},
        { Option::Call,    105,    100,     100.0, 0.00, 0.10, 0.50, 0.20, 0, 0,   9.8905, 1.0e-4},
        { Option::Call,    105,    100,     100.0, 0.00, 0.10, 0.50, 0.30, 0, 0,  15.8512, 1.0e-4},
        { Option::Call,    95,     100,     100.0, 0.00, 0.10, 1.00, 0.10, 0, 0,  18.3241, 1.0e-4},
        { Option::Call,    95,     100,     100.0, 0.00, 0.10, 1.00, 0.20, 0, 0,  26.0731, 1.0e-4},
        { Option::Call,    95,     100,     100.0, 0.00, 0.10, 1.00, 0.30, 0, 0,  34.7116, 1.0e-4},
        { Option::Call,    100,    100,     100.0, 0.00, 0.10, 1.00, 0.10, 0, 0,  13.8000, 1.0e-4},
        { Option::Call,    100,    100,     100.0, 0.00, 0.10, 1.00, 0.20, 0, 0,  21.5489, 1.0e-4},
        { Option::Call,    100,    100,     100.0, 0.00, 0.10, 1.00, 0.30, 0, 0,  30.1874, 1.0e-4},
        { Option::Call,    105,    100,     100.0, 0.00, 0.10, 1.00, 0.10, 0, 0,   9.5445, 1.0e-4},
        { Option::Call,    105,    100,     100.0, 0.00, 0.10, 1.00, 0.20, 0, 0,  17.2965, 1.0e-4},
        { Option::Call,    105,    100,     100.0, 0.00, 0.10, 1.00, 0.30, 0, 0,  25.9002, 1.0e-4},

        {  Option::Put,    95,     100,     100.0, 0.00, 0.10, 0.50, 0.10, 0, 0,   0.6899, 1.0e-4},
        {  Option::Put,    95,     100,     100.0, 0.00, 0.10, 0.50, 0.20, 0, 0,   4.4448, 1.0e-4},
        {  Option::Put,    95,     100,     100.0, 0.00, 0.10, 0.50, 0.30, 0, 0,   8.9213, 1.0e-4},
        {  Option::Put,    100,    100,     100.0, 0.00, 0.10, 0.50, 0.10, 0, 0,   3.3917, 1.0e-4},
        {  Option::Put,    100,    100,     100.0, 0.00, 0.10, 0.50, 0.20, 0, 0,   8.3177, 1.0e-4},
        {  Option::Put,    100,    100,     100.0, 0.00, 0.10, 0.50, 0.30, 0, 0,  13.1579, 1.0e-4},
        {  Option::Put,    105,    100,     100.0, 0.00, 0.10, 0.50, 0.10, 0, 0,   8.1478, 1.0e-4},
        {  Option::Put,    105,    100,     100.0, 0.00, 0.10, 0.50, 0.20, 0, 0,  13.0739, 1.0e-4},
        {  Option::Put,    105,    100,     100.0, 0.00, 0.10, 0.50, 0.30, 0, 0,  17.9140, 1.0e-4},
        {  Option::Put,    95,     100,     100.0, 0.00, 0.10, 1.00, 0.10, 0, 0,   1.0534, 1.0e-4},
        {  Option::Put,    95,     100,     100.0, 0.00, 0.10, 1.00, 0.20, 0, 0,   6.2813, 1.0e-4},
        {  Option::Put,    95,     100,     100.0, 0.00, 0.10, 1.00, 0.30, 0, 0,  12.2376, 1.0e-4},
        {  Option::Put,    100,    100,     100.0, 0.00, 0.10, 1.00, 0.10, 0, 0,   3.8079, 1.0e-4},
        {  Option::Put,    100,    100,     100.0, 0.00, 0.10, 1.00, 0.20, 0, 0,  10.1294, 1.0e-4},
        {  Option::Put,    100,    100,     100.0, 0.00, 0.10, 1.00, 0.30, 0, 0,  16.3889, 1.0e-4},
        {  Option::Put,    105,    100,     100.0, 0.00, 0.10, 1.00, 0.10, 0, 0,   8.3321, 1.0e-4},
        {  Option::Put,    105,    100,     100.0, 0.00, 0.10, 1.00, 0.20, 0, 0,  14.6536, 1.0e-4},
        {  Option::Put,    105,    100,     100.0, 0.00, 0.10, 1.00, 0.30, 0, 0,  20.9130, 1.0e-4}

    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (auto& value : values) {
        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(value.type, value.strike));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(
                                       Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engine(
                     new AnalyticContinuousFixedLookbackEngine(stochProcess));

        ContinuousFixedLookbackOption option(value.minmax, payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = value.result;
        Real error = std::fabs(calculated-expected);
        if (error > value.tol) {
            REPORT_FAILURE_FIXED("value", values[i].minmax, payoff, exercise, value.s, value.q,
                                 value.r, today, value.v, expected, calculated, error, value.tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testAnalyticContinuousPartialFloatingLookback) {

    BOOST_TEST_MESSAGE(
           "Testing analytic continuous partial floating-strike lookback options...");


    LookbackOptionData values[] = {

        // data from "Option Pricing Formulas, Second Edition", Haug, 2006, pg.146

        //type,         strike, minmax, s,    q,    r,    t,    v,    l,  t1,     result,   tol
        { Option::Call, 0,       90,     90,   0,   0.06, 1,    0.1,  1,  0.25,   8.6524,   1.0e-4},
        { Option::Call, 0,       90,     90,   0,   0.06, 1,    0.1,  1,  0.5,    9.2128,   1.0e-4},
        { Option::Call, 0,       90,     90,   0,   0.06, 1,    0.1,  1,  0.75,   9.5567,   1.0e-4},

        { Option::Call, 0,      110,    110,   0,   0.06, 1,    0.1,  1,  0.25,  10.5751,   1.0e-4},
        { Option::Call, 0,      110,    110,   0,   0.06, 1,    0.1,  1,  0.5,   11.2601,   1.0e-4},
        { Option::Call, 0,      110,    110,   0,   0.06, 1,    0.1,  1,  0.75,  11.6804,   1.0e-4},

        { Option::Call, 0,       90,     90,   0,   0.06, 1,    0.2,  1,  0.25,  13.3402,   1.0e-4},
        { Option::Call, 0,       90,     90,   0,   0.06, 1,    0.2,  1,  0.5,   14.5121,   1.0e-4},
        { Option::Call, 0,       90,     90,   0,   0.06, 1,    0.2,  1,  0.75,  15.314,    1.0e-4},

        { Option::Call, 0,      110,    110,   0,   0.06, 1,    0.2,  1,  0.25,  16.3047,   1.0e-4},
        { Option::Call, 0,      110,    110,   0,   0.06, 1,    0.2,  1,  0.5,   17.737,    1.0e-4},
        { Option::Call, 0,      110,    110,   0,   0.06, 1,    0.2,  1,  0.75,  18.7171,   1.0e-4},

        { Option::Call, 0,      90,      90,    0,  0.06, 1,    0.3,  1,  0.25,  17.9831,   1.0e-4},
        { Option::Call, 0,      90,      90,    0,  0.06, 1,    0.3,  1,  0.5,   19.6618,   1.0e-4},
        { Option::Call, 0,      90,      90,    0,  0.06, 1,    0.3,  1,  0.75,  20.8493,   1.0e-4},

        { Option::Call, 0,      110,    110,   0,   0.06, 1,    0.3,  1,  0.25,  21.9793,   1.0e-4},
        { Option::Call, 0,      110,    110,   0,   0.06, 1,    0.3,  1,  0.5,   24.0311,   1.0e-4},
        { Option::Call, 0,      110,    110,   0,   0.06, 1,    0.3,  1,  0.75,  25.4825,   1.0e-4},

        { Option::Put, 0,       90,      90,   0,   0.06, 1,    0.1,  1,  0.25,   2.7189,   1.0e-4},
        { Option::Put, 0,       90,      90,   0,   0.06, 1,    0.1,  1,  0.5,    3.4639,   1.0e-4},
        { Option::Put, 0,       90,      90,   0,   0.06, 1,    0.1,  1,  0.75,   4.1912,   1.0e-4},

        { Option::Put, 0,      110,     110,   0,   0.06, 1,    0.1,  1,  0.25,   3.3231,   1.0e-4},
        { Option::Put, 0,      110,     110,   0,   0.06, 1,    0.1,  1,  0.5,    4.2336,   1.0e-4},
        { Option::Put, 0,      110,     110,   0,   0.06, 1,    0.1,  1,  0.75,   5.1226,   1.0e-4},

        { Option::Put, 0,       90,      90,   0,   0.06, 1,    0.2,  1,  0.25,   7.9153,   1.0e-4},
        { Option::Put, 0,       90,      90,   0,   0.06, 1,    0.2,  1,  0.5,    9.5825,   1.0e-4},
        { Option::Put, 0,       90,      90,   0,   0.06, 1,    0.2,  1,  0.75,  11.0362,   1.0e-4},

        { Option::Put, 0,      110,     110,   0,   0.06, 1,    0.2,  1,  0.25,   9.6743,   1.0e-4},
        { Option::Put, 0,      110,     110,   0,   0.06, 1,    0.2,  1,  0.5,   11.7119,   1.0e-4},
        { Option::Put, 0,      110,     110,   0,   0.06, 1,    0.2,  1,  0.75,  13.4887,   1.0e-4},

        { Option::Put, 0,       90,      90,   0,   0.06, 1,    0.3,  1,  0.25,  13.4719,   1.0e-4},
        { Option::Put, 0,       90,      90,   0,   0.06, 1,    0.3,  1,  0.5,   16.1495,   1.0e-4},
        { Option::Put, 0,       90,      90,   0,   0.06, 1,    0.3,  1,  0.75,  18.4071,   1.0e-4},

        { Option::Put, 0,      110,     110,   0,   0.06, 1,    0.3,  1,  0.25,  16.4657,   1.0e-4},
        { Option::Put, 0,      110,     110,   0,   0.06, 1,    0.3,  1,  0.5,   19.7383,   1.0e-4},
        { Option::Put, 0,      110,     110,   0,   0.06, 1,    0.3,  1,  0.75,  22.4976,   1.0e-4}
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (auto& value : values) {
        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        ext::shared_ptr<FloatingTypePayoff> payoff(new FloatingTypePayoff(value.type));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(
                                       Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engine(
                  new AnalyticContinuousPartialFloatingLookbackEngine(stochProcess));

        Date lookbackEnd = today + timeToDays(value.t1);
        ContinuousPartialFloatingLookbackOption option(value.minmax, value.l, lookbackEnd, payoff,
                                                       exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = value.result;
        Real error = std::fabs(calculated-expected);
        if (error > value.tol) {
            REPORT_FAILURE_FLOATING("value", values[i].minmax, payoff, exercise, value.s, value.q,
                                    value.r, today, value.v, expected, calculated, error,
                                    value.tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testAnalyticContinuousPartialFixedLookback) {

    BOOST_TEST_MESSAGE(
              "Testing analytic continuous fixed-strike lookback options...");

    LookbackOptionData values[] = {
        // data from "Option Pricing Formulas, Second Edition", Haug, 2006, pg.148
        //type,         strike, minmax, s,    q,    r,    t,    v, l,   t1,  result,   tol
        { Option::Call,     90, 0,    100,    0, 0.06,    1,  0.1, 0, 0.25,  20.2845, 1.0e-4},
        { Option::Call,     90, 0,    100,    0, 0.06,    1,  0.1, 0, 0.5,   19.6239, 1.0e-4},
        { Option::Call,     90, 0,    100,    0, 0.06,    1,  0.1, 0, 0.75,  18.6244, 1.0e-4},
        
        { Option::Call,    110, 0,    100,    0, 0.06,    1,  0.1, 0, 0.25,   4.0432, 1.0e-4},
        { Option::Call,    110, 0,    100,    0, 0.06,    1,  0.1, 0, 0.5,    3.958,  1.0e-4},
        { Option::Call,    110, 0,    100,    0, 0.06,    1,  0.1, 0, 0.75,   3.7015, 1.0e-4},
        
        { Option::Call,     90, 0,    100,    0, 0.06,    1,  0.2, 0, 0.25,  27.5385, 1.0e-4},
        { Option::Call,     90, 0,    100,    0, 0.06,    1,  0.2, 0, 0.5,   25.8126, 1.0e-4},
        { Option::Call,     90, 0,    100,    0, 0.06,    1,  0.2, 0, 0.75,  23.4957, 1.0e-4},
        
        { Option::Call,    110, 0,    100,    0, 0.06,    1,  0.2, 0, 0.25,  11.4895, 1.0e-4},
        { Option::Call,    110, 0,    100,    0, 0.06,    1,  0.2, 0, 0.5,   10.8995, 1.0e-4},
        { Option::Call,    110, 0,    100,    0, 0.06,    1,  0.2, 0, 0.75,   9.8244, 1.0e-4},
        
        { Option::Call,     90, 0,    100,    0, 0.06,    1,  0.3, 0, 0.25,  35.4578, 1.0e-4},
        { Option::Call,     90, 0,    100,    0, 0.06,    1,  0.3, 0, 0.5,   32.7172, 1.0e-4},
        { Option::Call,     90, 0,    100,    0, 0.06,    1,  0.3, 0, 0.75,  29.1473, 1.0e-4},
        
        { Option::Call,    110, 0,    100,    0, 0.06,    1,  0.3, 0, 0.25,  19.725,  1.0e-4},
        { Option::Call,    110, 0,    100,    0, 0.06,    1,  0.3, 0, 0.5,   18.4025, 1.0e-4},
        { Option::Call,    110, 0,    100,    0, 0.06,    1,  0.3, 0, 0.75,  16.2976, 1.0e-4},
        
        { Option::Put,      90, 0,    100,    0, 0.06,    1,  0.1, 0, 0.25,   0.4973, 1.0e-4},
        { Option::Put,      90, 0,    100,    0, 0.06,    1,  0.1, 0, 0.5,    0.4632, 1.0e-4},
        { Option::Put,      90, 0,    100,    0, 0.06,    1,  0.1, 0, 0.75,   0.3863, 1.0e-4},
        
        { Option::Put,     110, 0,    100,    0, 0.06,    1,  0.1, 0, 0.25,  12.6978, 1.0e-4},
        { Option::Put,     110, 0,    100,    0, 0.06,    1,  0.1, 0, 0.5,   10.9492, 1.0e-4},
        { Option::Put,     110, 0,    100,    0, 0.06,    1,  0.1, 0, 0.75,   9.1555, 1.0e-4},
        
        { Option::Put,      90, 0,    100,    0, 0.06,    1,  0.2, 0, 0.25,   4.5863, 1.0e-4},
        { Option::Put,      90, 0,    100,    0, 0.06,    1,  0.2, 0, 0.5,    4.1925, 1.0e-4},
        { Option::Put,      90, 0,    100,    0, 0.06,    1,  0.2, 0, 0.75,   3.5831, 1.0e-4},
        
        { Option::Put,     110, 0,    100,    0, 0.06,    1,  0.2, 0, 0.25,  19.0255, 1.0e-4},
        { Option::Put,     110, 0,    100,    0, 0.06,    1,  0.2, 0, 0.5,   16.9433, 1.0e-4},
        { Option::Put,     110, 0,    100,    0, 0.06,    1,  0.2, 0, 0.75,  14.6505, 1.0e-4},
        
        { Option::Put,      90, 0,    100,    0, 0.06,    1,  0.3, 0, 0.25,   9.9348, 1.0e-4},
        { Option::Put,      90, 0,    100,    0, 0.06,    1,  0.3, 0, 0.5,    9.1111, 1.0e-4},
        { Option::Put,      90, 0,    100,    0, 0.06,    1,  0.3, 0, 0.75,   7.9267, 1.0e-4},
        
        { Option::Put,     110, 0,    100,    0, 0.06,    1,  0.3, 0, 0.25,  25.2112, 1.0e-4},
        { Option::Put,     110, 0,    100,    0, 0.06,    1,  0.3, 0, 0.5,   22.8217, 1.0e-4},
        { Option::Put,     110, 0,    100,    0, 0.06,    1,  0.3, 0, 0.75,  20.0566, 1.0e-4}
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (auto& value : values) {
        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(value.type, value.strike));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(
                                       Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engine(
                     new AnalyticContinuousPartialFixedLookbackEngine(stochProcess));

        Date lookbackStart = today + timeToDays(value.t1);
        ContinuousPartialFixedLookbackOption option(lookbackStart,
                                             payoff,
                                             exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = value.result;
        Real error = std::fabs(calculated-expected);
        if (error > value.tol) {
            REPORT_FAILURE_FIXED("value", values[i].minmax, payoff, exercise, value.s, value.q,
                                 value.r, today, value.v, expected, calculated, error, value.tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testMonteCarloLookback, *precondition(if_speed(Slow))) {
    BOOST_TEST_MESSAGE("Testing Monte Carlo engines for lookback options...");

    Real tolerance = 0.1;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    Real strike = 90;
    Real t = 1;
    Real t1= 0.25;

    Date exDate = today + timeToDays(t);
    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    spot ->setValue(100);
    qRate->setValue(0);
    rRate->setValue(0.06);
    vol  ->setValue(0.1);

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
        new BlackScholesMertonProcess(
            Handle<Quote>(spot),
            Handle<YieldTermStructure>(qTS),
            Handle<YieldTermStructure>(rTS),
            Handle<BlackVolTermStructure>(volTS)));

    Option::Type types[] = { Option::Call, Option::Put };

    for (auto type : types) {
        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));

        /**
         * Partial Fixed
         * **/

        Date lookbackStart = today + timeToDays(t1);
        ContinuousPartialFixedLookbackOption partialFixedLookback(lookbackStart,
                                                                  payoff,
                                                                  exercise);
        ext::shared_ptr<PricingEngine> engine(
            new AnalyticContinuousPartialFixedLookbackEngine(stochProcess));
        partialFixedLookback.setPricingEngine(engine);

        Real analytical = partialFixedLookback.NPV();

        ext::shared_ptr<PricingEngine> mcpartialfixedengine =
            MakeMCLookbackEngine<ContinuousPartialFixedLookbackOption, PseudoRandom>
            (stochProcess)
            .withSteps(2000)
            .withAntitheticVariate()
            .withSeed(1)
            .withAbsoluteTolerance(tolerance);

        partialFixedLookback.setPricingEngine(mcpartialfixedengine);
        Real monteCarlo = partialFixedLookback.NPV();

        Real diff = std::abs(analytical - monteCarlo);

        if (diff > tolerance){
            REPORT_FAILURE_MC("Partial Fixed", type, analytical, monteCarlo, tolerance);
        }

        /**
         * Fixed
         * **/

        Real minMax = 100;

        ContinuousFixedLookbackOption fixedLookback(minMax,
                                                    payoff,
                                                    exercise);
        ext::shared_ptr<PricingEngine> analyticalfixedengine(
            new AnalyticContinuousFixedLookbackEngine(stochProcess));
        fixedLookback.setPricingEngine(analyticalfixedengine);

        analytical = fixedLookback.NPV();

        ext::shared_ptr<PricingEngine> mcfixedengine =
            MakeMCLookbackEngine<ContinuousFixedLookbackOption, PseudoRandom>
            (stochProcess)
            .withSteps(2000)
            .withAntitheticVariate()
            .withSeed(1)
            .withAbsoluteTolerance(tolerance);

        fixedLookback.setPricingEngine(mcfixedengine);
        monteCarlo = fixedLookback.NPV();

        diff = std::abs(analytical - monteCarlo);

        if (diff > tolerance){
            REPORT_FAILURE_MC("Fixed", type, analytical, monteCarlo, tolerance);
        }

        /**
         * Partial Floating
         * **/

        Real lambda = 1;
        Date lookbackEnd = today + timeToDays(t1);

        ext::shared_ptr<FloatingTypePayoff> floatingPayoff(new FloatingTypePayoff(type));

        ContinuousPartialFloatingLookbackOption partialFloating(minMax,
                                                                lambda,
                                                                lookbackEnd,
                                                                floatingPayoff,
                                                                exercise);
        ext::shared_ptr<PricingEngine> analyticalpartialFloatingengine(
            new AnalyticContinuousPartialFloatingLookbackEngine(stochProcess));
        partialFloating.setPricingEngine(analyticalpartialFloatingengine);

        analytical = partialFloating.NPV();

        ext::shared_ptr<PricingEngine> mcpartialfloatingengine =
            MakeMCLookbackEngine<ContinuousPartialFloatingLookbackOption, PseudoRandom>
            (stochProcess)
            .withSteps(2000)
            .withAntitheticVariate()
            .withSeed(1)
            .withAbsoluteTolerance(tolerance);

        partialFloating.setPricingEngine(mcpartialfloatingengine);
        monteCarlo = partialFloating.NPV();

        diff = std::abs(analytical - monteCarlo);

        if (diff > tolerance){
            REPORT_FAILURE_MC("Partial Floating", type, analytical, monteCarlo, tolerance);
        }

        /**
         * Floating
         * **/

        ContinuousFloatingLookbackOption floating(minMax,
                                                  floatingPayoff,
                                                  exercise);
        ext::shared_ptr<PricingEngine> analyticalFloatingengine(
            new AnalyticContinuousFloatingLookbackEngine(stochProcess));
        floating.setPricingEngine(analyticalFloatingengine);

        analytical = floating.NPV();

        ext::shared_ptr<PricingEngine> mcfloatingengine =
            MakeMCLookbackEngine<ContinuousFloatingLookbackOption, PseudoRandom>
            (stochProcess)
            .withSteps(2000)
            .withAntitheticVariate()
            .withSeed(1)
            .withAbsoluteTolerance(tolerance);

        floating.setPricingEngine(mcfloatingengine);
        monteCarlo = floating.NPV();

        diff = std::abs(analytical - monteCarlo);

        if (diff > tolerance){
            REPORT_FAILURE_MC("Floating", type, analytical, monteCarlo, tolerance);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()