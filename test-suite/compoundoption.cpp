/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Dimitri Reiswich

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

#include "compoundoption.hpp"
#include "utilities.hpp"
#include <ql/experimental/compoundoption/compoundoption.hpp>
#include <ql/experimental/compoundoption/analyticcompoundoptionengine.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, payoffM, payoffD, exerciseM,    \
                       exerciseD, s, q, r, today,                 \
                       v, expected, calculated, error, tolerance) \
            BOOST_FAIL(\
               "\nmother option type:   " << payoffM->optionType() << \
               "\ndaughter option type: " << payoffD->optionType() << \
               "\nspot value:           " << s << \
               "\nstrike mother:        " << payoffM->strike() << \
               "\nstrike daughter:      " << payoffD->strike() << \
               "\ndividend yield:       " << io::rate(q) << \
               "\nrisk-free rate:       " << io::rate(r) << \
               "\nreference date:       " << today << \
               "\nmaturity mother:      " << exerciseM->lastDate() << \
               "\nmaturity daughter:    " << exerciseD->lastDate() << \
               "\nvolatility:           " << io::volatility(v) << \
               "\n  expected " << greekName << ": " << expected << \
               "\ncalculated " << greekName << ": " << calculated << \
               "\nerror:                " << error << \
               "\ntolerance:            " << tolerance);

namespace {

    Integer timeToDays(Time t) {
        return Integer(t*360+0.5);
    }

    struct CompoundOptionData {
        Option::Type typeMother;
        Option::Type typeDaughter;
        Real strikeMother;
        Real strikeDaughter;
        Real s;        // spot
        Rate q;        // dividend
        Rate r;        // risk-free rate
        Time tMother;  // time to maturity
        Time tDaughter;// time to maturity
        Volatility v;  // volatility
        Real npv;   // expected result
        Real tol;      // tolerance
        Real delta;
        Real gamma;
        Real vega;
        Real theta;
    };

}


void CompoundOptionTest::testPutCallParity(){

    BOOST_TEST_MESSAGE("Testing compound-option put-call parity...");

    // Test Put Call Parity for compound options.
    // Formula taken from: "Foreign Exchange Risk", Wystup, Risk 2002
    // Page 81, Equation 9.5


    CompoundOptionData values[] = {
        // type Mother, typeDaughter, strike Mother, strike Daughter,  spot,    q,    r,    t Mother, t Daughter,  vol
        { Option::Put, Option::Call,  50.0,            520.0   ,      500.0,   0.03, 0.08,  0.25,     0.5,        0.35},
        { Option::Call, Option::Call,  50.0,           520.0   ,      500.0,   0.03, 0.08,  0.25,     0.5,        0.35},
        { Option::Call, Option::Put,  50.0,            520.0   ,      500.0,   0.03, 0.08,  0.25,     0.5,        0.35},
        { Option::Call, Option::Call,  0.05,           1.14   ,      1.20,  0.0, 0.01,  0.5,     2.0,         0.11},
        { Option::Call, Option::Put ,  0.05,           1.14   ,      1.20,  0.0, 0.01,  0.5,     2.0,         0.11},
        { Option::Call, Option::Call,  10.0,           122.0   ,      120.0,    0.06, 0.02,  0.1,     0.7,        0.22},
        { Option::Call, Option::Put,  10.0,           122.0   ,      120.0,     0.06, 0.02,  0.1,     0.7,        0.22},
        { Option::Call, Option::Call,  0.4,           8.2   ,      8.0,     0.05, 0.00,  2.0,     3.0,        0.08},
        { Option::Call, Option::Put,  0.4,           8.2   ,      8.0,  0.05, 0.00,  2.0,     3.0,        0.08},
        { Option::Call, Option::Call,  0.02,           1.6   ,      1.6,    0.013, 0.022,  0.45,     0.5,        0.17},
        { Option::Call, Option::Put,  0.02,           1.6   ,      1.6,     0.013, 0.022,  0.45,     0.5,         0.17},
    };

    SavedSettings backup;

    Calendar calendar = TARGET();

    DayCounter dc = Actual360();
    Date todaysDate = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));

    boost::shared_ptr<YieldTermStructure> rTS(
              new FlatForward(0, NullCalendar(), Handle<Quote>(rRate), dc));

    boost::shared_ptr<YieldTermStructure> qTS(
              new FlatForward(0, NullCalendar(), Handle<Quote>(qRate), dc));

    boost::shared_ptr<BlackVolTermStructure> volTS(
                              new BlackConstantVol(todaysDate, NullCalendar(),
                                                   Handle<Quote>(vol), dc));

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoffMotherCall(
                new PlainVanillaPayoff(Option::Call, values[i].strikeMother));

        boost::shared_ptr<StrikedTypePayoff> payoffMotherPut(
                new PlainVanillaPayoff(Option::Put, values[i].strikeMother));

        boost::shared_ptr<StrikedTypePayoff> payoffDaughter(
                            new PlainVanillaPayoff(values[i].typeDaughter,
                                                   values[i].strikeDaughter));

        Date matDateMom = todaysDate + timeToDays(values[i].tMother);
        Date matDateDaughter = todaysDate + timeToDays(values[i].tDaughter);

        boost::shared_ptr<Exercise> exerciseCompound(
                                            new EuropeanExercise(matDateMom));
        boost::shared_ptr<Exercise> exerciseDaughter(
                                       new EuropeanExercise(matDateDaughter));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        CompoundOption compoundOptionCall(payoffMotherCall,exerciseCompound,
                                          payoffDaughter, exerciseDaughter);

        CompoundOption compoundOptionPut(payoffMotherPut,exerciseCompound,
                                         payoffDaughter, exerciseDaughter);

        VanillaOption vanillaOption(EuropeanOption(payoffDaughter,
                                                   exerciseDaughter));

        boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
            new BlackScholesMertonProcess(
                      Handle<Quote>(spot),
                      Handle<YieldTermStructure>(qTS),
                      Handle<YieldTermStructure>(rTS),
                      Handle<BlackVolTermStructure>(volTS)));


        boost::shared_ptr<PricingEngine> engineCompound(
                              new AnalyticCompoundOptionEngine(stochProcess));

        boost::shared_ptr<PricingEngine> engineEuropean(
                                     new AnalyticEuropeanEngine(stochProcess));

        compoundOptionCall.setPricingEngine(engineCompound);
        compoundOptionPut.setPricingEngine(engineCompound);
        vanillaOption.setPricingEngine(engineEuropean);

        Real discFact=rTS->discount(matDateMom);
        Real discStrike=values[i].strikeMother*discFact;

        Real calculated =
            compoundOptionCall.NPV() + discStrike - compoundOptionPut.NPV()
            - vanillaOption.NPV();

        Real expected=0.0;
        Real error=std::abs(calculated-expected);
        Real tolerance=1.0e-8;

        if(error>tolerance){
            REPORT_FAILURE("put call parity", payoffMotherCall, payoffDaughter,
                           exerciseCompound, exerciseDaughter, values[i].s,
                           values[i].q, values[i].r, todaysDate,
                           values[i].v, values[i].delta, calculated,
                           error, tolerance);
        }
    }
}

void CompoundOptionTest::testValues(){

    BOOST_TEST_MESSAGE("Testing compound-option values and greeks...");

    CompoundOptionData values[] = {
        // type Mother, typeDaughter, strike Mother, strike Daughter,  spot,    q,    r,    t Mother, t Daughter,  vol,   value,    tol, delta, gamma, vega, theta
        // Tolerance is taken to be pretty high with 1.0e-3, since the price/theta is very sensitive with respect to
        // the implementation of the bivariate normal - which differs in the various implementations.
        // Option Value Taken from Haug 2007, Greeks from www.sitmo.com
        { Option::Put, Option::Call,  50.0,            520.0   ,      500.0,   0.03, 0.08,  0.25,     0.5,        0.35,  21.1965,   1.0e-3, -0.1966,0.0007, -32.1241, -3.3837},
        //*********************************************************
        // Option Values and Greeks taken from www.sitmo.com
        { Option::Call, Option::Call,  50.0,           520.0   ,      500.0,   0.03, 0.08,  0.25,     0.5,        0.35,  17.5945,   1.0e-3,  0.3219,0.0038, 106.5185, -65.1614},
        { Option::Call, Option::Put,  50.0,            520.0   ,      500.0,   0.03, 0.08,  0.25,     0.5,        0.35,  18.7128,   1.0e-3,  -0.2906,0.0036, 103.3856, -46.6982},
        { Option::Put, Option::Put,  50.0,            520.0   ,      500.0,    0.03, 0.08,  0.25,     0.5,        0.35,  15.2601,   1.0e-3,  0.1760,0.0005, -35.2570, -10.1126},
        // type Mother, typeDaughter, strike Mother, strike Daughter,  spot,    q,    r,    t Mother, t Daughter,  vol,   value,    tol, delta, gamma, vega, theta
        { Option::Call, Option::Call,  0.05,           1.14   ,      1.20,  0.0, 0.01,  0.5,     2.0,         0.11,  0.0729,   1.0e-3,  0.6614,2.5762, 0.5812, -0.0297},
        { Option::Call, Option::Put ,  0.05,           1.14   ,      1.20,  0.0, 0.01,  0.5,     2.0,         0.11,  0.0074,   1.0e-3,  -0.1334,1.9681, 0.2933, -0.0155},
        { Option::Put  ,Option::Call,  0.05,           1.14   ,      1.20,  0.0, 0.01,  0.5,     2.0,         0.11,  0.0021,   1.0e-3,  -0.0426,0.7252, -0.0052, -0.0058},
        { Option::Put, Option::Put ,  0.05,           1.14   ,      1.20,   0.0, 0.01,  0.5,     2.0,         0.11,  0.0192,   1.0e-3,  0.1626,0.1171, -0.2931, -0.0028},
        // type Mother, typeDaughter, strike Mother, strike Daughter,  spot,    q,    r,    t Mother, t Daughter,  vol,   value,    tol, delta, gamma, vega, theta
        { Option::Call, Option::Call,  10.0,           122.0   ,      120.0,    0.06, 0.02,  0.1,     0.7,        0.22,  0.4419,   1.0e-3,  0.1049,0.0195, 11.3368, -6.2871},
        { Option::Call, Option::Put,  10.0,           122.0   ,      120.0,     0.06, 0.02,  0.1,     0.7,        0.22,  2.6112,   1.0e-3,  -0.3618,0.0337, 28.4843, -13.4124},
        { Option::Put, Option::Call,  10.0,           122.0   ,      120.0,     0.06, 0.02,  0.1,     0.7,        0.22,  4.1616,   1.0e-3,  -0.3174,0.0024, -26.6403, -2.2720},
        { Option::Put, Option::Put,  10.0,           122.0   ,      120.0,  0.06, 0.02,  0.1,     0.7,        0.22,  1.0914,   1.0e-3,  0.1748,0.0165, -9.4928, -4.8995},
        //*********************************************************
        //*********************************************************
        // Option Values and Greeks taken from mathfinance VBA implementation
        // type Mother, typeDaughter, strike Mother, strike Daughter,  spot,    q,    r,    t Mother, t Daughter,  vol,   value,    tol, delta, gamma, vega, theta
        { Option::Call, Option::Call,  0.4,           8.2   ,      8.0,  0.05, 0.00,  2.0,     3.0,        0.08,  0.0099,   1.0e-3,  0.0285,0.0688, 0.7764, -0.0027},
        { Option::Call, Option::Put,  0.4,           8.2   ,      8.0,   0.05, 0.00,  2.0,     3.0,        0.08,  0.9826,   1.0e-3,  -0.7224,0.2158, 2.7279, -0.3332},
        { Option::Put, Option::Call,  0.4,           8.2   ,      8.0,   0.05, 0.00,  2.0,     3.0,        0.08,  0.3585,   1.0e-3,  -0.0720,-0.0835, -1.5633, -0.0117},
        { Option::Put, Option::Put,  0.4,           8.2   ,      8.0,    0.05, 0.00,  2.0,     3.0,        0.08,  0.0168,   1.0e-3,  0.0378, 0.0635, 0.3882, 0.0021},
        // type Mother, typeDaughter, strike Mother, strike Daughter,  spot,    q,    r,    t Mother, t Daughter,  vol,   value,    tol, delta, gamma, vega, theta
        { Option::Call, Option::Call,  0.02,           1.6   ,      1.6,     0.013, 0.022,  0.45,     0.5,         0.17,  0.0680,   1.0e-3,  0.4937,2.1271, 0.4418, -0.0843},
        { Option::Call, Option::Put,  0.02,           1.6   ,      1.6,  0.013, 0.022,  0.45,     0.5,         0.17,  0.0605,   1.0e-3,  -0.4169,2.0836, 0.4330, -0.0697},
        { Option::Put, Option::Call,  0.02,           1.6   ,      1.6,  0.013, 0.022,  0.45,     0.5,         0.17,  0.0081,   1.0e-3,  -0.0417,0.0761, -0.0045, -0.0020},
        { Option::Put, Option::Put,  0.02,           1.6   ,      1.6,   0.013, 0.022,  0.45,     0.5,         0.17,  0.0078,   1.0e-3,   0.0413,0.0326, -0.0133, -0.0016}
    };

    SavedSettings backup;

    Calendar calendar = TARGET();

    DayCounter dc = Actual360();
    Date todaysDate = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));

    boost::shared_ptr<YieldTermStructure> rTS(
              new FlatForward(0, NullCalendar(), Handle<Quote>(rRate), dc));

    boost::shared_ptr<YieldTermStructure> qTS(
              new FlatForward(0, NullCalendar(), Handle<Quote>(qRate), dc));

    boost::shared_ptr<BlackVolTermStructure> volTS(
                              new BlackConstantVol(todaysDate, NullCalendar(),
                                                   Handle<Quote>(vol), dc));

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoffMother(
                    new PlainVanillaPayoff(values[i].typeMother,
                                           values[i].strikeMother));

        boost::shared_ptr<StrikedTypePayoff> payoffDaughter(
                    new PlainVanillaPayoff(values[i].typeDaughter,
                                           values[i].strikeDaughter));

        Date matDateMom = todaysDate + timeToDays(values[i].tMother);
        Date matDateDaughter = todaysDate + timeToDays(values[i].tDaughter);

        boost::shared_ptr<Exercise> exerciseMother(
                                            new EuropeanExercise(matDateMom));
        boost::shared_ptr<Exercise> exerciseDaughter(
                                       new EuropeanExercise(matDateDaughter));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        CompoundOption compoundOption(payoffMother,exerciseMother,
                                      payoffDaughter, exerciseDaughter);

        boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
            new BlackScholesMertonProcess(
                      Handle<Quote>(spot),
                      Handle<YieldTermStructure>(qTS),
                      Handle<YieldTermStructure>(rTS),
                      Handle<BlackVolTermStructure>(volTS)));

        boost::shared_ptr<PricingEngine> engineCompound(
                              new AnalyticCompoundOptionEngine(stochProcess));

        compoundOption.setPricingEngine(engineCompound);

        Real calculated = compoundOption.NPV();
        Real error= std::fabs(calculated-values[i].npv); //-values[i].npv
        Real tolerance = values[i].tol;

        if (error>tolerance) {
            REPORT_FAILURE("value", payoffMother, payoffDaughter,
                           exerciseMother, exerciseDaughter, values[i].s,
                           values[i].q, values[i].r, todaysDate,
                           values[i].v, values[i].npv, calculated,
                           error, tolerance);
        }

        calculated = compoundOption.delta();
        error= std::fabs(calculated-values[i].delta);
        tolerance = values[i].tol;

        if (error>tolerance) {
            REPORT_FAILURE("delta", payoffMother, payoffDaughter,
                           exerciseMother, exerciseDaughter, values[i].s,
                           values[i].q, values[i].r, todaysDate,
                           values[i].v, values[i].delta, calculated,
                           error, tolerance);
        }

        calculated = compoundOption.gamma();
        error= std::fabs(calculated-values[i].gamma);
        tolerance = values[i].tol;

        if (error>tolerance) {
            REPORT_FAILURE("gamma", payoffMother, payoffDaughter,
                           exerciseMother, exerciseDaughter, values[i].s,
                           values[i].q, values[i].r, todaysDate,
                           values[i].v, values[i].gamma, calculated,
                           error, tolerance);
        }

        calculated = compoundOption.vega();
        error= std::fabs(calculated-values[i].vega);
        tolerance = values[i].tol;

        if (error>tolerance) {
            REPORT_FAILURE("vega", payoffMother, payoffDaughter,
                           exerciseMother, exerciseDaughter, values[i].s,
                           values[i].q, values[i].r, todaysDate,
                           values[i].v, values[i].vega, calculated,
                           error, tolerance);
        }

        calculated = compoundOption.theta();
        error= std::fabs(calculated-values[i].theta);
        tolerance = values[i].tol;

        if (error>tolerance) {
            REPORT_FAILURE("theta", payoffMother, payoffDaughter,
                           exerciseMother, exerciseDaughter, values[i].s,
                           values[i].q, values[i].r, todaysDate,
                           values[i].v, values[i].theta, calculated,
                           error, tolerance);
        }
    }
}


test_suite* CompoundOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Compound option tests");

    suite->add(QUANTLIB_TEST_CASE(&CompoundOptionTest::testValues));
    suite->add(QUANTLIB_TEST_CASE(&CompoundOptionTest::testPutCallParity));

    return suite;
}


