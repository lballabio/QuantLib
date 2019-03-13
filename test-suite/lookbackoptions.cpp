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

#include "lookbackoptions.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/instruments/lookbackoption.hpp>
#include <ql/pricingengines/lookback/analyticcontinuousfloatinglookback.hpp>
#include <ql/pricingengines/lookback/analyticcontinuousfixedlookback.hpp>
#include <ql/pricingengines/lookback/analyticcontinuouspartialfloatinglookback.hpp>
#include <ql/pricingengines/lookback/analyticcontinuouspartialfixedlookback.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/processes/blackscholesprocess.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

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


void LookbackOptionTest::testAnalyticContinuousFloatingLookback() {

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

    for (Size i=0; i<LENGTH(values); i++) {
        Date exDate = today + Integer(values[i].t*360+0.5);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        ext::shared_ptr<FloatingTypePayoff> payoff(
                                      new FloatingTypePayoff(values[i].type));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(
                                       Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engine(
                  new AnalyticContinuousFloatingLookbackEngine(stochProcess));

        ContinuousFloatingLookbackOption option(values[i].minmax,
                                                payoff,
                                                exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = values[i].result;
        Real error = std::fabs(calculated-expected);
        if (error>values[i].tol) {
            REPORT_FAILURE_FLOATING("value", values[i].minmax, payoff,
                                    exercise, values[i].s, values[i].q,
                                    values[i].r, today, values[i].v,
                                    expected, calculated, error,
                                    values[i].tol);
        }
    }
}


void LookbackOptionTest::testAnalyticContinuousFixedLookback() {

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

    for (Size i=0; i<LENGTH(values); i++) {
        Date exDate = today + Integer(values[i].t*360+0.5);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        ext::shared_ptr<StrikedTypePayoff> payoff(
                     new PlainVanillaPayoff(values[i].type, values[i].strike));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(
                                       Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engine(
                     new AnalyticContinuousFixedLookbackEngine(stochProcess));

        ContinuousFixedLookbackOption option(values[i].minmax,
                                             payoff,
                                             exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = values[i].result;
        Real error = std::fabs(calculated-expected);
        if (error>values[i].tol) {
            REPORT_FAILURE_FIXED("value", values[i].minmax, payoff, exercise,
                                 values[i].s, values[i].q, values[i].r, today,
                                 values[i].v, expected, calculated, error,
                                 values[i].tol);
        }
    }
}

void LookbackOptionTest::testAnalyticContinuousPartialFloatingLookback() {

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

    for (Size i=0; i<LENGTH(values); i++) {
        Date exDate = today + Integer(values[i].t*360+0.5);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        ext::shared_ptr<FloatingTypePayoff> payoff(
                                      new FloatingTypePayoff(values[i].type));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(
                                       Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engine(
                  new AnalyticContinuousPartialFloatingLookbackEngine(stochProcess));

        Date lookbackEnd = today + Integer(values[i].t1*360+0.5);
        ContinuousPartialFloatingLookbackOption option(values[i].minmax,
                                                values[i].l,
                                                lookbackEnd,
                                                payoff,
                                                exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = values[i].result;
        Real error = std::fabs(calculated-expected);
        if (error>values[i].tol) {
            REPORT_FAILURE_FLOATING("value", values[i].minmax, payoff,
                                    exercise, values[i].s, values[i].q,
                                    values[i].r, today, values[i].v,
                                    expected, calculated, error,
                                    values[i].tol);
        }
    }
}

void LookbackOptionTest::testAnalyticContinuousPartialFixedLookback() {

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

    for (Size i=0; i<LENGTH(values); i++) {
        Date exDate = today + Integer(values[i].t*360+0.5);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        ext::shared_ptr<StrikedTypePayoff> payoff(
                     new PlainVanillaPayoff(values[i].type, values[i].strike));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(
                                       Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engine(
                     new AnalyticContinuousPartialFixedLookbackEngine(stochProcess));

        Date lookbackStart = today + Integer(values[i].t1*360+0.5);
        ContinuousPartialFixedLookbackOption option(lookbackStart,
                                             payoff,
                                             exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = values[i].result;
        Real error = std::fabs(calculated-expected);
        if (error>values[i].tol) {
            REPORT_FAILURE_FIXED("value", values[i].minmax, payoff, exercise,
                                 values[i].s, values[i].q, values[i].r, today,
                                 values[i].v, expected, calculated, error,
                                 values[i].tol);
        }
    }
}

test_suite* LookbackOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Lookback option tests");

    suite->add(QUANTLIB_TEST_CASE(
                &LookbackOptionTest::testAnalyticContinuousFloatingLookback));
    suite->add(QUANTLIB_TEST_CASE(
                &LookbackOptionTest::testAnalyticContinuousFixedLookback));
    suite->add(QUANTLIB_TEST_CASE(
                &LookbackOptionTest::testAnalyticContinuousPartialFloatingLookback));
    suite->add(QUANTLIB_TEST_CASE(
                &LookbackOptionTest::testAnalyticContinuousPartialFixedLookback));
    return suite;
}

