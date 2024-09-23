/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Dimitri Reiswich

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/experimental/fx/blackdeltacalculator.hpp>
#include <ql/experimental/fx/deltavolquote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

using std::sqrt;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(BlackDeltaCalculatorTests)

struct DeltaData {
    Option::Type ot;
    DeltaVolQuote::DeltaType dt;
    Real spot;
    DiscountFactor dDf;   // domestic discount
    DiscountFactor fDf;   // foreign  discount
    Real stdDev;
    Real strike;
    Real value;
};

struct EuropeanOptionData {
    Option::Type type;
    Real strike;
    Real s;        // spot
    Rate q;        // dividend
    Rate r;        // risk-free rate
    Time t;        // time to maturity
    Volatility v;  // volatility
    Real result;   // expected result
    Real tol;      // tolerance
};


BOOST_AUTO_TEST_CASE(testDeltaValues){

    BOOST_TEST_MESSAGE("Testing delta calculator values...");

    DeltaData values[] = {
        // Values taken from parallel implementation in R
        {Option::Call, DeltaVolQuote::Spot,     1.421, 0.997306, 0.992266,  0.1180654,  1.608080, 0.15},
        {Option::Call, DeltaVolQuote::PaSpot,   1.421, 0.997306, 0.992266,  0.1180654,  1.600545, 0.15},
        {Option::Call, DeltaVolQuote::Fwd,      1.421, 0.997306, 0.992266,  0.1180654,  1.609029, 0.15},
        {Option::Call, DeltaVolQuote::PaFwd,    1.421, 0.997306, 0.992266,  0.1180654,  1.601550, 0.15},
        {Option::Call, DeltaVolQuote::Spot,     122.121,  0.9695434,0.9872347,  0.0887676,  119.8031, 0.67},
        {Option::Call, DeltaVolQuote::PaSpot,   122.121,  0.9695434,0.9872347,  0.0887676,  117.7096, 0.67},
        {Option::Call, DeltaVolQuote::Fwd,      122.121,  0.9695434,0.9872347,  0.0887676,  120.0592, 0.67},
        {Option::Call, DeltaVolQuote::PaFwd,    122.121,  0.9695434,0.9872347,  0.0887676,  118.0532, 0.67},
        {Option::Put,  DeltaVolQuote::Spot,     3.4582,   0.99979, 0.9250616,   0.3199034,  4.964924, -0.821},
        {Option::Put,  DeltaVolQuote::PaSpot,   3.4582,   0.99979, 0.9250616,   0.3199034,  3.778327, -0.821},
        {Option::Put,  DeltaVolQuote::Fwd,      3.4582,   0.99979, 0.9250616,   0.3199034,  4.51896, -0.821},
        {Option::Put,  DeltaVolQuote::PaFwd,    3.4582,   0.99979, 0.9250616,   0.3199034,  3.65728, -0.821},
        // JPYUSD Data taken from Castagnas "FX Options and Smile Risk" (Wiley 2009)
        {Option::Put,  DeltaVolQuote::Spot,     103.00,   0.99482, 0.98508,     0.07247845, 97.47,  -0.25},
        {Option::Put,  DeltaVolQuote::PaSpot,   103.00,   0.99482, 0.98508,     0.07247845, 97.22,  -0.25}
    };

    Option::Type                currOt;
    DeltaVolQuote::DeltaType    currDt;
    Real currSpot;
    Real currdDf;
    Real currfDf;
    Real currStdDev;
    Real currStrike;
    Real expected;
    Real currDelta;
    Real calculated;
    Real error;
    Real tolerance;

    for (Size i=0; i<std::size(values); i++) {

        currOt      =values[i].ot;
        currDt      =values[i].dt;
        currSpot    =values[i].spot;
        currdDf     =values[i].dDf;
        currfDf     =values[i].fDf;
        currStdDev  =values[i].stdDev;
        currStrike  =values[i].strike;
        currDelta   =values[i].value;

        BlackDeltaCalculator myCalc(currOt, currDt, currSpot,
                                    currdDf, currfDf, currStdDev);

        tolerance=1.0e-3;

        expected    =currDelta;
        calculated  =myCalc.deltaFromStrike(currStrike);
        error       =std::fabs(calculated-expected);

        if (error>tolerance) {
            BOOST_ERROR("\n Delta-from-strike calculation failed for delta. \n"
                        << "Iteration: "<< i << "\n"
                        << "Calculated Strike:" << calculated << "\n"
                        << "Expected   Strike:" << expected << "\n"
                        << "Error: " << error);
        }

        tolerance=1.0e-2;
        // tolerance not that small, but sufficient for strikes in
        // particular since they might be results of a numerical
        // procedure

        expected    =currStrike;
        calculated  =myCalc.strikeFromDelta(currDelta);
        error       =std::fabs(calculated-expected);

        if (error>tolerance) {
            BOOST_ERROR("\n Strike-from-delta calculation failed for delta. \n"
                        << "Iteration: "<< i << "\n"
                        << "Calculated Strike:" << calculated << "\n"
                        << "Expected   Strike:" << expected << "\n"
                        << "Error: " << error);
        }
    }
}

BOOST_AUTO_TEST_CASE(testDeltaPriceConsistency) {

    BOOST_TEST_MESSAGE("Testing premium-adjusted delta price consistency...");

    // This function tests for price consistencies with the standard
    // Black Scholes calculator, since premium adjusted deltas can be calculated
    // from spot deltas by adding/subtracting the premium.

    // actually, value and tol won't be needed for testing
    EuropeanOptionData values[] = {
      //        type, strike,   spot,    rd,    rf,    t,  vol,   value,    tol
      { Option::Call,  0.9123,  1.2212, 0.0231, 0.0000, 0.25, 0.301,  0.0, 0.0},
      { Option::Call,  0.9234,  1.2212, 0.0231, 0.0000, 0.35, 0.111,  0.0, 0.0},
      { Option::Call,  0.9783,  1.2212, 0.0231, 0.0000, 0.45, 0.071,  0.0, 0.0},
      { Option::Call,  1.0000,  1.2212, 0.0231, 0.0000, 0.55, 0.082,  0.0, 0.0},
      { Option::Call,  1.1230,  1.2212, 0.0231, 0.0000, 0.65, 0.012,  0.0, 0.0},
      { Option::Call,  1.2212,  1.2212, 0.0231, 0.0000, 0.75, 0.129,  0.0, 0.0},
      { Option::Call,  1.3212,  1.2212, 0.0231, 0.0000, 0.85, 0.034,  0.0, 0.0},
      { Option::Call,  1.3923,  1.2212, 0.0131, 0.2344, 0.95, 0.001,  0.0, 0.0},
      { Option::Call,  1.3455,  1.2212, 0.0000, 0.0000, 1.00, 0.127,  0.0, 0.0},
      { Option::Put,   0.9123,  1.2212, 0.0231, 0.0000, 0.25, 0.301,  0.0, 0.0},
      { Option::Put,   0.9234,  1.2212, 0.0231, 0.0000, 0.35, 0.111,  0.0, 0.0},
      { Option::Put,   0.9783,  1.2212, 0.0231, 0.0000, 0.45, 0.071,  0.0, 0.0},
      { Option::Put,   1.0000,  1.2212, 0.0231, 0.0000, 0.55, 0.082,  0.0, 0.0},
      { Option::Put,   1.1230,  1.2212, 0.0231, 0.0000, 0.65, 0.012,  0.0, 0.0},
      { Option::Put,   1.2212,  1.2212, 0.0231, 0.0000, 0.75, 0.129,  0.0, 0.0},
      { Option::Put,   1.3212,  1.2212, 0.0231, 0.0000, 0.85, 0.034,  0.0, 0.0},
      { Option::Put,   1.3923,  1.2212, 0.0131, 0.2344, 0.95, 0.001,  0.0, 0.0},
      { Option::Put,   1.3455,  1.2212, 0.0000, 0.0000, 1.00, 0.127,  0.0, 0.0},
      // extreme case: zero vol
      { Option::Put,   1.3455,  1.2212, 0.0000, 0.0000, 0.50, 0.000,  0.0, 0.0},
      // extreme case: zero strike
      { Option::Put,   0.0000,  1.2212, 0.0000, 0.0000, 1.50, 0.133,  0.0, 0.0},
      // extreme case: zero strike+zero vol
      { Option::Put,   0.0000,  1.2212, 0.0000, 0.0000, 1.00, 0.133,  0.0, 0.0},
    };

    DayCounter dc       = Actual360();
    Calendar calendar   = TARGET();
    Date today          = Date::todaysDate();

    // Start setup of market data

    Real discFor        =0.0;
    Real discDom        =0.0;
    Real implVol        =0.0;
    Real expectedVal    =0.0;
    Real calculatedVal  =0.0;
    Real error          =0.0;

    ext::shared_ptr<SimpleQuote> spotQuote(new SimpleQuote(0.0));
    Handle<Quote> spotHandle(spotQuote);

    ext::shared_ptr<SimpleQuote> qQuote(new SimpleQuote(0.0));
    Handle<Quote> qHandle(qQuote);
    ext::shared_ptr<YieldTermStructure> qTS(
                                         new FlatForward(today, qHandle, dc));

    ext::shared_ptr<SimpleQuote> rQuote(new SimpleQuote(0.0));
    Handle<Quote> rHandle(qQuote);
    ext::shared_ptr<YieldTermStructure> rTS(
                                         new FlatForward(today, rHandle, dc));

    ext::shared_ptr<SimpleQuote> volQuote(new SimpleQuote(0.0));
    Handle<Quote> volHandle(volQuote);
    ext::shared_ptr<BlackVolTermStructure> volTS(
                        new BlackConstantVol(today, calendar, volHandle, dc));

    ext::shared_ptr<BlackScholesMertonProcess>    stochProcess;
    ext::shared_ptr<PricingEngine>                engine;
    ext::shared_ptr<StrikedTypePayoff>            payoff;
    Date exDate;
    ext::shared_ptr<Exercise>                     exercise;
    // Setup of market data finished

    Real tolerance=1.0e-10;

    for (auto& value : values) {

        payoff =
            ext::shared_ptr<StrikedTypePayoff>(new PlainVanillaPayoff(value.type, value.strike));
        exDate = today + timeToDays(value.t);
        exercise = ext::shared_ptr<Exercise>(new EuropeanExercise(exDate));

        spotQuote->setValue(value.s);
        volQuote->setValue(value.v);
        rQuote->setValue(value.r);
        qQuote->setValue(value.q);

        discDom =rTS->discount(exDate);
        discFor =qTS->discount(exDate);
        implVol =sqrt(volTS->blackVariance(exDate,0.0));

        BlackDeltaCalculator myCalc(value.type, DeltaVolQuote::PaSpot, spotQuote->value(), discDom,
                                    discFor, implVol);

        stochProcess=ext::make_shared<BlackScholesMertonProcess> (spotHandle,
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS));

        engine = ext::shared_ptr<PricingEngine>(
                                    new AnalyticEuropeanEngine(stochProcess));

        EuropeanOption option(payoff, exercise);
        option.setPricingEngine(engine);

        calculatedVal = myCalc.deltaFromStrike(value.strike);

        Real delta = 0.0;
        if (implVol > 0.0) {
            delta = option.delta();
        }
        else {
            const Real fwd = spotQuote->value()*discFor/discDom;
            if (payoff->optionType() == Option::Call && fwd > payoff->strike())
                delta = 1.0;
            else if (payoff->optionType() == Option::Put && fwd < payoff->strike())
                delta = -1.0;
        }

        expectedVal=delta-option.NPV()/spotQuote->value();

        error=std::fabs(expectedVal-calculatedVal);

        if(error>tolerance){
            BOOST_ERROR("\n Premium-adjusted spot delta test failed. \n" \
                        << "Calculated Delta: " << calculatedVal << "\n"
                        << "Expected Value:   " << expectedVal << "\n"
                        << "Error: "<< error);
        }

        myCalc.setDeltaType(DeltaVolQuote::PaFwd);

        calculatedVal = myCalc.deltaFromStrike(value.strike);
        expectedVal=expectedVal/discFor; // Premium adjusted Fwd Delta is PA spot without discount
        error=std::fabs(expectedVal-calculatedVal);

        if(error>tolerance){
            BOOST_ERROR("\n Premium-adjusted forward delta test failed. \n"
                        << "Calculated Delta: " << calculatedVal << "\n"
                        << "Expected Value:   " << expectedVal << "\n"
                        << "Error: "<< error);
        }


        // Test consistency with BlackScholes Calculator for Spot Delta
        myCalc.setDeltaType(DeltaVolQuote::Spot);

        calculatedVal = myCalc.deltaFromStrike(value.strike);
        expectedVal=delta;
        error=std::fabs(calculatedVal-expectedVal);

        if(error>tolerance){
            BOOST_ERROR("\n spot delta in BlackDeltaCalculator differs "
                        "from delta in BlackScholesCalculator. \n"
                        << "Calculated Value: " << calculatedVal << "\n"
                        << "Expected Value:   " << expectedVal << "\n"
                        << "Error: " << error);
        }
    }
}

BOOST_AUTO_TEST_CASE(testPutCallParity){

    BOOST_TEST_MESSAGE("Testing put-call parity for deltas...");

    // Test for put call parity between put and call deltas.

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
       pag 11-16
    */

    EuropeanOptionData values[] = {
      // pag 2-8
      //        type, strike,   spot,    q,    r,    t,  vol,   value,    tol
      { Option::Call,  65.00,  60.00, 0.00, 0.08, 0.25, 0.30,  2.1334, 1.0e-4},
      { Option::Put,   95.00, 100.00, 0.05, 0.10, 0.50, 0.20,  2.4648, 1.0e-4},
      { Option::Put,   19.00,  19.00, 0.10, 0.10, 0.75, 0.28,  1.7011, 1.0e-4},
      { Option::Call,  19.00,  19.00, 0.10, 0.10, 0.75, 0.28,  1.7011, 1.0e-4},
      { Option::Call,   1.60,   1.56, 0.08, 0.06, 0.50, 0.12,  0.0291, 1.0e-4},
      { Option::Put,   70.00,  75.00, 0.05, 0.10, 0.50, 0.35,  4.0870, 1.0e-4},
      // pag 24
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.15,  0.0205, 1.0e-4},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.15,  1.8734, 1.0e-4},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.15,  9.9413, 1.0e-4},
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.25,  0.3150, 1.0e-4},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.25,  3.1217, 1.0e-4},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.25, 10.3556, 1.0e-4},
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.35,  0.9474, 1.0e-4},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.35,  4.3693, 1.0e-4},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.35, 11.1381, 1.0e-4},
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.15,  0.8069, 1.0e-4},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.15,  4.0232, 1.0e-4},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.15, 10.5769, 1.0e-4},
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.25,  2.7026, 1.0e-4},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.25,  6.6997, 1.0e-4},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.25, 12.7857, 1.0e-4},
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.35,  4.9329, 1.0e-4},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.35,  9.3679, 1.0e-4},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.35, 15.3086, 1.0e-4},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.15,  9.9210, 1.0e-4},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.15,  1.8734, 1.0e-4},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.15,  0.0408, 1.0e-4},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.25, 10.2155, 1.0e-4},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.25,  3.1217, 1.0e-4},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.25,  0.4551, 1.0e-4},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.35, 10.8479, 1.0e-4},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.35,  4.3693, 1.0e-4},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.35,  1.2376, 1.0e-4},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.15, 10.3192, 1.0e-4},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.15,  4.0232, 1.0e-4},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.15,  1.0646, 1.0e-4},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.25, 12.2149, 1.0e-4},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.25,  6.6997, 1.0e-4},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.25,  3.2734, 1.0e-4},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.35, 14.4452, 1.0e-4},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.35,  9.3679, 1.0e-4},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.35,  5.7963, 1.0e-4},
      // pag 27
      { Option::Call,  40.00,  42.00, 0.08, 0.04, 0.75, 0.35,  5.0975, 1.0e-4}
    };

    DayCounter dc = Actual360();
    Calendar calendar = TARGET();
    Date today = Date::todaysDate();

    Real discFor        =0.0;
    Real discDom        =0.0;
    Real implVol        =0.0;
    Real deltaCall      =0.0;
    Real deltaPut       =0.0;
    Real expectedDiff   =0.0;
    Real calculatedDiff =0.0;
    Real error          =0.0;
    Real forward        =0.0;

    ext::shared_ptr<SimpleQuote> spotQuote(new SimpleQuote(0.0));

    ext::shared_ptr<SimpleQuote> qQuote(new SimpleQuote(0.0));
    Handle<Quote> qHandle(qQuote);
    ext::shared_ptr<YieldTermStructure> qTS(
                                         new FlatForward(today, qHandle, dc));

    ext::shared_ptr<SimpleQuote> rQuote(new SimpleQuote(0.0));
    Handle<Quote> rHandle(qQuote);
    ext::shared_ptr<YieldTermStructure> rTS(
                                         new FlatForward(today, rHandle, dc));

    ext::shared_ptr<SimpleQuote> volQuote(new SimpleQuote(0.0));
    Handle<Quote> volHandle(volQuote);
    ext::shared_ptr<BlackVolTermStructure> volTS(
                        new BlackConstantVol(today, calendar, volHandle, dc));

    ext::shared_ptr<StrikedTypePayoff> payoff;
    Date exDate;
    ext::shared_ptr<Exercise> exercise;

    Real tolerance=1.0e-10;

    for (auto& value : values) {

        payoff =
            ext::shared_ptr<StrikedTypePayoff>(new PlainVanillaPayoff(Option::Call, value.strike));
        exDate = today + timeToDays(value.t);
        exercise = ext::shared_ptr<Exercise>(new EuropeanExercise(exDate));

        spotQuote->setValue(value.s);
        volQuote->setValue(value.v);
        rQuote->setValue(value.r);
        qQuote->setValue(value.q);
        discDom=rTS->discount(exDate);
        discFor=qTS->discount(exDate);
        implVol=sqrt(volTS->blackVariance(exDate,0.0));
        forward=spotQuote->value()*discFor/discDom;

        BlackDeltaCalculator myCalc(Option::Call, DeltaVolQuote::Spot,
                                    spotQuote->value(),
                                    discDom, discFor, implVol);

        deltaCall = myCalc.deltaFromStrike(value.strike);
        ;
        myCalc.setOptionType(Option::Put);
        deltaPut = myCalc.deltaFromStrike(value.strike);
        ;
        myCalc.setOptionType(Option::Call);

        expectedDiff=discFor;
        calculatedDiff=deltaCall-deltaPut;
        error=std::fabs(expectedDiff-calculatedDiff);

        if(error>tolerance){
            BOOST_ERROR("\n Put-call parity failed for spot delta. \n"
                        << "Calculated Call Delta: " << deltaCall << "\n"
                        << "Calculated Put Delta:  " << deltaPut << "\n"
                        << "Expected Difference:   " << expectedDiff << "\n"
                        << "Calculated Difference: " << calculatedDiff);
        }
        myCalc.setDeltaType(DeltaVolQuote::Fwd);

        deltaCall = myCalc.deltaFromStrike(value.strike);
        ;
        myCalc.setOptionType(Option::Put);
        deltaPut = myCalc.deltaFromStrike(value.strike);
        ;
        myCalc.setOptionType(Option::Call);

        expectedDiff=1.0;
        calculatedDiff=deltaCall-deltaPut;
        error=std::fabs(expectedDiff-calculatedDiff);

        if(error>tolerance){
            BOOST_ERROR("\n Put-call parity failed for forward delta. \n"
                        << "Calculated Call Delta: " << deltaCall << "\n"
                        << "Calculated Put Delta:  " << deltaPut << "\n"
                        << "Expected Difference:   " << expectedDiff << "\n"
                        << "Calculated Difference: " << calculatedDiff );
        }

        myCalc.setDeltaType(DeltaVolQuote::PaSpot);

        deltaCall = myCalc.deltaFromStrike(value.strike);
        ;
        myCalc.setOptionType(Option::Put);
        deltaPut = myCalc.deltaFromStrike(value.strike);
        ;
        myCalc.setOptionType(Option::Call);

        expectedDiff = discFor * value.strike / forward;
        calculatedDiff=deltaCall-deltaPut;
        error=std::fabs(expectedDiff-calculatedDiff);

        if(error>tolerance){
            BOOST_ERROR("\n Put-call parity failed for "
                        "premium-adjusted spot delta. \n"
                        << "Calculated Call Delta: " << deltaCall << "\n"
                        << "Calculated Put Delta:  " << deltaPut << "\n"
                        << "Expected Difference:   " << expectedDiff << "\n"
                        << "Calculated Difference: " << calculatedDiff);
        }

        myCalc.setDeltaType(DeltaVolQuote::PaFwd);

        deltaCall = myCalc.deltaFromStrike(value.strike);
        ;
        myCalc.setOptionType(Option::Put);
        deltaPut = myCalc.deltaFromStrike(value.strike);
        ;
        myCalc.setOptionType(Option::Call);

        expectedDiff = value.strike / forward;
        calculatedDiff=deltaCall-deltaPut;
        error=std::fabs(expectedDiff-calculatedDiff);

        if(error>tolerance){
            BOOST_ERROR("\n Put-call parity failed for premium-adjusted "
                        "forward delta. \n"
                        << "Calculated Call Delta: " << deltaCall << "\n"
                        << "Calculated Put Delta:  " << deltaPut << "\n"
                        << "Expected Difference:   " << expectedDiff << "\n"
                        << "Calculated Difference: " << calculatedDiff);
        }
    }
}

BOOST_AUTO_TEST_CASE(testAtmCalcs){

    BOOST_TEST_MESSAGE("Testing delta-neutral ATM quotations...");

    DeltaData values[] = {
        {Option::Call, DeltaVolQuote::Spot,     1.421, 0.997306, 0.992266,          0.1180654,  1.608080, 0.15},
        {Option::Call, DeltaVolQuote::PaSpot,   1.421, 0.997306, 0.992266,      0.1180654,  1.600545, 0.15},
        {Option::Call, DeltaVolQuote::Fwd,      1.421, 0.997306, 0.992266,      0.1180654,  1.609029, 0.15},
        {Option::Call, DeltaVolQuote::PaFwd,    1.421, 0.997306, 0.992266,      0.1180654,  1.601550, 0.15},
        {Option::Call, DeltaVolQuote::Spot,     122.121,  0.9695434,0.9872347,  0.0887676,  119.8031, 0.67},
        {Option::Call, DeltaVolQuote::PaSpot,   122.121,  0.9695434,0.9872347,  0.0887676,  117.7096, 0.67},
        {Option::Call, DeltaVolQuote::Fwd,      122.121,  0.9695434,0.9872347,  0.0887676,  120.0592, 0.67},
        {Option::Call, DeltaVolQuote::PaFwd,    122.121,  0.9695434,0.9872347,  0.0887676,  118.0532, 0.67},
        {Option::Put,  DeltaVolQuote::Spot,     3.4582,   0.99979, 0.9250616,   0.3199034,  4.964924, -0.821},
        {Option::Put,  DeltaVolQuote::PaSpot,   3.4582,   0.99979, 0.9250616,   0.3199034,  3.778327, -0.821},
        {Option::Put,  DeltaVolQuote::Fwd,      3.4582,   0.99979, 0.9250616,   0.3199034,  4.51896, -0.821},
        {Option::Put,  DeltaVolQuote::PaFwd,    3.4582,   0.99979, 0.9250616,   0.3199034,  3.65728, -0.821},
        // Data taken from Castagnas "FX Options and Smile Risk" (Wiley 2009)
        {Option::Put,  DeltaVolQuote::Spot,     103.00,   0.99482, 0.98508,     0.07247845, 97.47,  -0.25},
        {Option::Put,  DeltaVolQuote::PaSpot,   103.00,   0.99482, 0.98508,     0.07247845, 97.22,  -0.25},
        // Extreme case: zero vol, ATM Fwd strike
        {Option::Call,  DeltaVolQuote::Fwd, 103.00,     0.99482, 0.98508,       0.0,    101.0013,0.5},
        {Option::Call,  DeltaVolQuote::Spot,    103.00,   0.99482, 0.98508,     0.0,    101.0013,0.99482*0.5}
    };

    DeltaVolQuote::DeltaType    currDt;
    Real currSpot;
    Real currdDf;
    Real currfDf;
    Real currStdDev;
    Real expected;
    Real calculated;
    Real error;
    Real tolerance=1.0e-2; // not that small, but sufficient for strikes
    Real currAtmStrike;
    Real currCallDelta;
    Real currPutDelta;
    Real currFwd;

    for (Size i=0; i<std::size(values); i++) {

        currDt      =values[i].dt;
        currSpot    =values[i].spot;
        currdDf     =values[i].dDf;
        currfDf     =values[i].fDf;
        currStdDev  =values[i].stdDev;
        currFwd     =currSpot*currfDf/currdDf;

        BlackDeltaCalculator myCalc(Option::Call, currDt, currSpot, currdDf,
                                    currfDf, currStdDev);

        currAtmStrike=myCalc.atmStrike(DeltaVolQuote::AtmDeltaNeutral);
        currCallDelta=myCalc.deltaFromStrike(currAtmStrike);
        myCalc.setOptionType(Option::Put);
        currPutDelta=myCalc.deltaFromStrike(currAtmStrike);
        myCalc.setOptionType(Option::Call);

        expected    =0.0;
        calculated  =currCallDelta+currPutDelta;
        error       =std::fabs(calculated-expected);

        if(error>tolerance){
            BOOST_ERROR("\n Delta neutrality failed for spot delta "
                        "in Delta Calculator. \n"
                        << "Iteration: "<< i << "\n"
                        << "Calculated Delta Sum: " << calculated << "\n"
                        << "Expected Delta Sum:   " << expected << "\n"
                        << "Error: "                << error);
        }

        myCalc.setDeltaType(DeltaVolQuote::Fwd);
        currAtmStrike=myCalc.atmStrike(DeltaVolQuote::AtmDeltaNeutral);
        currCallDelta=myCalc.deltaFromStrike(currAtmStrike);
        myCalc.setOptionType(Option::Put);
        currPutDelta=myCalc.deltaFromStrike(currAtmStrike);
        myCalc.setOptionType(Option::Call);

        expected    =0.0;
        calculated  =currCallDelta+currPutDelta;
        error       =std::fabs(calculated-expected);

        if(error>tolerance){
            BOOST_ERROR("\n Delta neutrality failed for forward delta "
                        "in Delta Calculator. \n"
                        << "Iteration: " << i << "\n"
                        << "Calculated Delta Sum: " << calculated << "\n"
                        << "Expected Delta Sum:   " << expected << "\n"
                        << "Error: "                << error);
        }

        myCalc.setDeltaType(DeltaVolQuote::PaSpot);
        currAtmStrike=myCalc.atmStrike(DeltaVolQuote::AtmDeltaNeutral);
        currCallDelta=myCalc.deltaFromStrike(currAtmStrike);
        myCalc.setOptionType(Option::Put);
        currPutDelta=myCalc.deltaFromStrike(currAtmStrike);
        myCalc.setOptionType(Option::Call);

        expected    =0.0;
        calculated  =currCallDelta+currPutDelta;
        error       =std::fabs(calculated-expected);

        if(error>tolerance){
            BOOST_ERROR("\n Delta neutrality failed for premium-adjusted "
                        "spot delta in Delta Calculator. \n"
                        << "Iteration: " << i << "\n"
                        << "Calculated Delta Sum: " << calculated << "\n"
                        << "Expected Delta Sum:   " << expected << "\n"
                        << "Error: "                << error);
        }


        myCalc.setDeltaType(DeltaVolQuote::PaFwd);
        currAtmStrike=myCalc.atmStrike(DeltaVolQuote::AtmDeltaNeutral);
        currCallDelta=myCalc.deltaFromStrike(currAtmStrike);
        myCalc.setOptionType(Option::Put);
        currPutDelta=myCalc.deltaFromStrike(currAtmStrike);
        myCalc.setOptionType(Option::Call);

        expected    =0.0;
        calculated  =currCallDelta+currPutDelta;
        error       =std::fabs(calculated-expected);

        if(error>tolerance){
            BOOST_ERROR("\n Delta neutrality failed for premium-adjusted "
                        "forward delta in Delta Calculator. \n"
                        << "Iteration: " << i << "\n"
                        << "Calculated Delta Sum: " << calculated << "\n"
                        << "Expected Delta Sum:   " << expected << "\n"
                        << "Error: " << error);
        }

        // Test ATM forward Calculations
        calculated=myCalc.atmStrike(DeltaVolQuote::AtmFwd);
        expected=currFwd;
        error=std::fabs(expected-calculated);

        if(error>tolerance){
            BOOST_ERROR("\n Atm forward test failed. \n"
                        << "Calculated Value: " << calculated << "\n"
                        << "Expected   Value: " << expected << "\n"
                        << "Error: " << error);
        }

        // Test ATM 0.50 delta calculations
        myCalc.setDeltaType(DeltaVolQuote::Fwd);
        Real atmFiftyStrike=myCalc.atmStrike(DeltaVolQuote::AtmPutCall50);
        calculated=std::fabs(myCalc.deltaFromStrike(atmFiftyStrike));
        expected=0.50;
        error=std::fabs(expected-calculated);

        if(error>tolerance){
            BOOST_ERROR("\n Atm 0.50 delta strike test failed. \n"
                        << "Iteration:" << i << "\n"
                        << "Calculated Value: " << calculated << "\n"
                        << "Expected   Value: " << expected << "\n"
                        << "Error: "    << error);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

