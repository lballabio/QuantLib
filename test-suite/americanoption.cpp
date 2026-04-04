/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2005, 2007 StatPro Italia srl
 Copyright (C) 2005 Joseph Wang

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

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/any.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/integrals/integral.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/statistics/incrementalstatistics.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/baroneadesiwhaleyengine.hpp>
#include <ql/pricingengines/vanilla/bjerksundstenslandengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesshoutengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/juquadraticengine.hpp>
#include <ql/pricingengines/vanilla/qdfpamericanengine.hpp>
#include <ql/pricingengines/vanilla/qdplusamericanengine.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(AmericanOptionTests)

#undef REPORT_FAILURE
#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, \
                       v, expected, calculated, error, tolerance) \
    BOOST_ERROR(exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    spot value:       " << s << "\n" \
               << "    strike:           " << payoff->strike() << "\n" \
               << "    dividend yield:   " << io::rate(q) << "\n" \
               << "    risk-free rate:   " << io::rate(r) << "\n" \
               << "    reference date:   " << today << "\n" \
               << "    maturity:         " << exercise->lastDate() << "\n" \
               << "    volatility:       " << io::volatility(v) << "\n\n" \
               << std::fixed << std::setprecision(4) \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << std::scientific \
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);

struct AmericanOptionData {
    Option::Type type;
    Real strike;
    Real s;        // spot
    Rate q;        // dividend
    Rate r;        // risk-free rate
    Time t;        // time to maturity
    Volatility v;  // volatility
    Real result;   // expected result
};

BOOST_AUTO_TEST_CASE(testBaroneAdesiWhaleyValues) {

    BOOST_TEST_MESSAGE("Testing Barone-Adesi and Whaley approximation for American options...");

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
       pag 24

       The following values were replicated only up to the second digit
       by the VB code provided by Haug, which was used as base for the
       C++ implementation

    */
    AmericanOptionData values[] = {
      //        type, strike,   spot,    q,    r,    t,  vol,   value
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.15,  0.0206 },
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.15,  1.8771 },
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.15, 10.0089 },
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.25,  0.3159 },
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.25,  3.1280 },
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.25, 10.3919 },
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.35,  0.9495 },
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.35,  4.3777 },
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.35, 11.1679 },
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.15,  0.8208 },
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.15,  4.0842 },
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.15, 10.8087 },
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.25,  2.7437 },
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.25,  6.8015 },
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.25, 13.0170 },
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.35,  5.0063 },
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.35,  9.5106 },
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.35, 15.5689 },
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.15, 10.0000 },
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.15,  1.8770 },
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.15,  0.0410 },
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.25, 10.2533 },
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.25,  3.1277 },
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.25,  0.4562 },
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.35, 10.8787 },
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.35,  4.3777 },
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.35,  1.2402 },
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.15, 10.5595 },
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.15,  4.0842 },
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.15,  1.0822 },
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.25, 12.4419 },
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.25,  6.8014 },
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.25,  3.3226 },
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.35, 14.6945 },
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.35,  9.5104 },
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.35,  5.8823 },
      { Option::Put,  100.00, 100.00, 0.00, 0.00, 0.50, 0.15,  4.2294 }
    };

    Date today = Date::todaysDate();
    DayCounter dc = Actual360();
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    Real tolerance = 3.0e-3;

    for (auto& value : values) {

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(value.type, value.strike));
        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(
                                         new AmericanExercise(today, exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engine(
                      new BaroneAdesiWhaleyApproximationEngine(stochProcess));

        VanillaOption option(payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated - value.result);
        if (error > tolerance) {
            REPORT_FAILURE("value", payoff, exercise, value.s, value.q, value.r, today, value.v,
                           value.result, calculated, error, tolerance);
        }
    }
}

BOOST_AUTO_TEST_CASE(testBjerksundStenslandValues) {

    BOOST_TEST_MESSAGE("Testing Bjerksund and Stensland approximation for American options...");

    AmericanOptionData values[] = {
        //      type, strike,   spot,    q,    r,    t,  vol,   value, tol
        // from "Option pricing formulas", Haug, McGraw-Hill 1998, pag 27
      { Option::Call,  40.00,  42.00, 0.08, 0.04, 0.75, 0.35,  5.2704 },
        // from "Option pricing formulas", Haug, McGraw-Hill 1998, VBA code
      { Option::Put,   40.00,  36.00, 0.00, 0.06, 1.00, 0.20,  4.4531 },
        // ATM option with very small volatility, reference value taken from R
      { Option::Call, 100, 100, 0.05, 0.05, 1.0, 0.0021, 0.08032314 },
        // ATM option with very small volatility,
        // reference value taken from Barone-Adesi and Whaley Approximation
      { Option::Call, 100, 100, 0.05, 0.05, 1.0, 0.0001, 0.003860656 },
      { Option::Call, 100, 99.99, 0.05, 0.05, 1.0, 0.0001, 0.00081 },
        // ITM option with a very small volatility
      { Option::Call, 100, 110, 0.05, 0.05, 1.0, 0.0001, 10.0 },
      { Option::Put, 110, 100, 0.05, 0.05, 1.0, 0.0001, 10.0 },
        // ATM option with a very large volatility
      { Option::Put, 100, 110, 0.05, 0.05, 1.0, 10, 95.12289 }
    };

    Date today = Date::todaysDate();
    DayCounter dc = Actual360();
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    Real tolerance = 5.0e-5;

    for (auto& value : values) {

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(value.type, value.strike));
        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new AmericanExercise(today, exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engine(
                     new BjerksundStenslandApproximationEngine(stochProcess));

        VanillaOption option(payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated - value.result);
        if (error > tolerance) {
            REPORT_FAILURE("value", payoff, exercise, value.s, value.q, value.r, today, value.v,
                           value.result, calculated, error, tolerance);
        }
    }
}

/* The data below are from
   An Approximate Formula for Pricing American Options
   Journal of Derivatives Winter 1999
   Ju, N.
*/
AmericanOptionData juValues[] = {
    //        type, strike,   spot,    q,    r,    t,     vol,   value, tol
    // These values are from Exhibit 3 - Short dated Put Options
    { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.0833,  0.2,  0.006 },
    { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.3333,  0.2,  0.201 },
    { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.5833,  0.2,  0.433 },

    { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.0833,  0.2,  0.851 },
    { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.3333,  0.2,  1.576 },
    { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.5833,  0.2,  1.984 },

    { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.0833,  0.2,  5.000 },
    { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.3333,  0.2,  5.084 },
    { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.5833,  0.2,  5.260 },

    { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.0833,  0.3,  0.078 },
    { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.3333,  0.3,  0.697 },
    { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.5833,  0.3,  1.218 },

    { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.0833,  0.3,  1.309 },
    { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.3333,  0.3,  2.477 },
    { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.5833,  0.3,  3.161 },

    { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.0833,  0.3,  5.059 },
    { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.3333,  0.3,  5.699 },
    { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.5833,  0.3,  6.231 },

    { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.0833,  0.4,  0.247 },
    { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.3333,  0.4,  1.344 },
    { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.5833,  0.4,  2.150 },

    { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.0833,  0.4,  1.767 },
    { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.3333,  0.4,  3.381 },
    { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.5833,  0.4,  4.342 },

    { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.0833,  0.4,  5.288 },
    { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.3333,  0.4,  6.501 },
    { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.5833,  0.4,  7.367 },

    // Type in Exhibits 4 and 5 if you have some spare time ;-)

    //        type, strike,   spot,    q,    r,    t,     vol,   value, tol
    // values from Exhibit 6 - Long dated Call Options with dividends
    { Option::Call, 100.00,   80.00,  0.07,  0.03, 3.0,  0.2,   2.605 },
    { Option::Call, 100.00,   90.00,  0.07,  0.03, 3.0,  0.2,   5.182 },
    { Option::Call, 100.00,  100.00,  0.07,  0.03, 3.0,  0.2,   9.065 },
    { Option::Call, 100.00,  110.00,  0.07,  0.03, 3.0,  0.2,  14.430 },
    { Option::Call, 100.00,  120.00,  0.07,  0.03, 3.0,  0.2,  21.398 },

    { Option::Call, 100.00,   80.00,  0.07,  0.03, 3.0,  0.4,  11.336 },
    { Option::Call, 100.00,   90.00,  0.07,  0.03, 3.0,  0.4,  15.711 },
    { Option::Call, 100.00,  100.00,  0.07,  0.03, 3.0,  0.4,  20.760 },
    { Option::Call, 100.00,  110.00,  0.07,  0.03, 3.0,  0.4,  26.440 },
    { Option::Call, 100.00,  120.00,  0.07,  0.03, 3.0,  0.4,  32.709 },

    { Option::Call, 100.00,   80.00,  0.07,  0.00001, 3.0,  0.3,   5.552 },
    { Option::Call, 100.00,   90.00,  0.07,  0.00001, 3.0,  0.3,   8.868 },
    { Option::Call, 100.00,  100.00,  0.07,  0.00001, 3.0,  0.3,  13.158 },
    { Option::Call, 100.00,  110.00,  0.07,  0.00001, 3.0,  0.3,  18.458 },
    { Option::Call, 100.00,  120.00,  0.07,  0.00001, 3.0,  0.3,  24.786 },

    { Option::Call, 100.00,   80.00,  0.03,  0.07, 3.0,  0.3,  12.177 },
    { Option::Call, 100.00,   90.00,  0.03,  0.07, 3.0,  0.3,  17.411 },
    { Option::Call, 100.00,  100.00,  0.03,  0.07, 3.0,  0.3,  23.402 },
    { Option::Call, 100.00,  110.00,  0.03,  0.07, 3.0,  0.3,  30.028 },
    { Option::Call, 100.00,  120.00,  0.03,  0.07, 3.0,  0.3,  37.177 }
};


BOOST_AUTO_TEST_CASE(testJuValues) {

    BOOST_TEST_MESSAGE("Testing Ju approximation for American options...");

    Date today = Date::todaysDate();
    DayCounter dc = Actual360();
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    Real tolerance = 1.0e-3;

    for (auto& juValue : juValues) {

        ext::shared_ptr<StrikedTypePayoff> payoff(
            new PlainVanillaPayoff(juValue.type, juValue.strike));
        Date exDate = today + timeToDays(juValue.t);
        ext::shared_ptr<Exercise> exercise(
                                         new AmericanExercise(today, exDate));

        spot->setValue(juValue.s);
        qRate->setValue(juValue.q);
        rRate->setValue(juValue.r);
        vol->setValue(juValue.v);

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engine(
                            new JuQuadraticApproximationEngine(stochProcess));

        VanillaOption option(payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated - juValue.result);
        if (error > tolerance) {
            REPORT_FAILURE("value", payoff, exercise, juValue.s, juValue.q, juValue.r, today,
                           juValue.v, juValue.result, calculated, error, tolerance);
        }
    }
}

BOOST_AUTO_TEST_CASE(testFdValues) {

    BOOST_TEST_MESSAGE("Testing finite-difference and QR+ engine for American options...");

    Date today = Date::todaysDate();
    DayCounter dc = Actual360();
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    Real tolerance = 8.0e-2;

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess =
        ext::make_shared<BlackScholesMertonProcess>(
            Handle<Quote>(spot),
            Handle<YieldTermStructure>(qTS),
            Handle<YieldTermStructure>(rTS),
            Handle<BlackVolTermStructure>(volTS));

    ext::shared_ptr<PricingEngine> pdeEngine =
        ext::make_shared<FdBlackScholesVanillaEngine>(stochProcess, 100, 400);

    ext::shared_ptr<PricingEngine> qrPlusEngine =
        ext::make_shared<FdBlackScholesVanillaEngine>(stochProcess);

    for (auto& juValue : juValues) {

        ext::shared_ptr<StrikedTypePayoff> payoff(
            new PlainVanillaPayoff(juValue.type, juValue.strike));

        Date exDate = today + timeToDays(juValue.t);
        ext::shared_ptr<Exercise> exercise(
                                         new AmericanExercise(today, exDate));

        spot->setValue(juValue.s);
        qRate->setValue(juValue.q);
        rRate->setValue(juValue.r);
        vol->setValue(juValue.v);

        VanillaOption option(payoff, exercise);
        option.setPricingEngine(pdeEngine);

        Real pdeCalculated = option.NPV();
        Real error = std::fabs(pdeCalculated - juValue.result);
        if (error > tolerance) {
            REPORT_FAILURE("value", payoff, exercise, juValue.s, juValue.q, juValue.r, today,
                           juValue.v, juValue.result, pdeCalculated, error, tolerance);
        }

        option.setPricingEngine(qrPlusEngine);

        Real qrPlusCalculated = option.NPV();
        if (std::abs(pdeCalculated - qrPlusCalculated) > 2e-2)
            BOOST_FAIL("QR+ boundary approximation failed to "
                    "reproduce PDE value for "
                    << "\n    OptionType: " <<
                    ((juValue.type == Option::Call)? "Call" : "Put")
                    << std::setprecision(16)
                    << "\n    spot:       " << spot->value()
                    << "\n    strike:     " << juValue.strike
                    << "\n    r:          " << rRate->value()
                    << "\n    q:          " << qRate->value()
                    << "\n    vol:        " << vol->value()
                    << "\n    PDE value:  " << pdeCalculated
                    << "\n    QR+ value:  " << qrPlusCalculated);
    }
}


template <class Engine>
void testFdGreeks() {

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"]  = 7.0e-4;
    tolerance["gamma"]  = 2.0e-4;
    //tolerance["theta"]  = 1.0e-4;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer years[] = { 1, 2 };
    Volatility vols[] = { 0.11, 0.50, 1.20 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    ext::shared_ptr<StrikedTypePayoff> payoff;

    for (auto& type : types) {
        for (Real strike : strikes) {
            for (int year : years) {
                Date exDate = today + year * Years;
                ext::shared_ptr<Exercise> exercise(new AmericanExercise(today, exDate));
                ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));
                ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                    new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

                ext::shared_ptr<PricingEngine> engine(new Engine(stochProcess, 50));

                VanillaOption option(payoff, exercise);
                option.setPricingEngine(engine);

                for (Real u : underlyings) {
                    for (Real m : qRates) {
                        for (Real n : rRates) {
                            for (Real v : vols) {
                                Rate q = m, r = n;
                                spot->setValue(u);
                                qRate->setValue(q);
                                rRate->setValue(r);
                                vol->setValue(v);
                                Real value = option.NPV();
                                calculated["delta"] = option.delta();
                                calculated["gamma"] = option.gamma();
                                // calculated["theta"]  = option.theta();

                                if (value > spot->value() * 1.0e-5) {
                                    // perturb spot and get delta and gamma
                                    Real du = u * 1.0e-4;
                                    spot->setValue(u + du);
                                    Real value_p = option.NPV(), delta_p = option.delta();
                                    spot->setValue(u - du);
                                    Real value_m = option.NPV(), delta_m = option.delta();
                                    spot->setValue(u);
                                    expected["delta"] = (value_p - value_m) / (2 * du);
                                    expected["gamma"] = (delta_p - delta_m) / (2 * du);

                                    /*
                                    // perturb date and get theta
                                    Time dT = dc.yearFraction(today-1, today+1);
                                    Settings::instance().setEvaluationDate(today-1);
                                    value_m = option.NPV();
                                    Settings::instance().setEvaluationDate(today+1);
                                    value_p = option.NPV();
                                    Settings::instance().setEvaluationDate(today);
                                    expected["theta"] = (value_p - value_m)/dT;
                                    */

                                    // compare
                                    std::map<std::string, Real>::iterator it;
                                    for (it = calculated.begin(); it != calculated.end();
                                         ++it) {
                                        std::string greek = it->first;
                                        Real expct = expected[greek], calcl = calculated[greek],
                                            tol = tolerance[greek];
                                        Real error = relativeError(expct, calcl, u);
                                        if (error > tol) {
                                            REPORT_FAILURE(greek, payoff, exercise, u, q, r,
                                                           today, v, expct, calcl, error, tol);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testFdAmericanGreeks) {
    BOOST_TEST_MESSAGE("Testing finite-differences American option greeks...");
    testFdGreeks<FdBlackScholesVanillaEngine>();
}

BOOST_AUTO_TEST_CASE(testFdShoutGreeks, *precondition(if_speed(Fast))) {
    BOOST_TEST_MESSAGE("Testing finite-differences shout option greeks...");
    testFdGreeks<FdBlackScholesShoutEngine>();
}

BOOST_AUTO_TEST_CASE(testFDShoutNPV) {
    BOOST_TEST_MESSAGE("Testing finite-differences shout option pricing...");

    const auto dc = Actual365Fixed();
    const auto today = Date(4, February, 2021);
    Settings::instance().evaluationDate() = today;

    const auto spot = Handle<Quote>(ext::make_shared<SimpleQuote>(100.0));
    const auto q = Handle<YieldTermStructure>(flatRate(0.03, dc));
    const auto r = Handle<YieldTermStructure>(flatRate(0.06, dc));

    const auto volTS = Handle<BlackVolTermStructure>(flatVol(0.25, dc));
    const auto process = ext::make_shared<BlackScholesMertonProcess>(
            spot, q, r, volTS);

    const auto maturityDate = today + Period(5, Years);

    struct TestDescription { Real strike; Option::Type type; Real expected; };

    const TestDescription testDescriptions[] = {
            {105, Option::Put, 19.136},
            {105, Option::Call, 28.211},
            {120, Option::Put, 28.02},
            {80, Option::Call, 40.785}
    };

    const auto engine = ext::make_shared<FdBlackScholesShoutEngine>(
        process, 400, 200);

    for (const TestDescription& desc: testDescriptions) {
        const Real strike = desc.strike;
        const Option::Type type = desc.type;

        auto option = VanillaOption(
            ext::make_shared<PlainVanillaPayoff>(type, strike),
            ext::make_shared<AmericanExercise>(maturityDate));

        option.setPricingEngine(engine);

        const Real expected = desc.expected;
        const Real tol = 2e-2;
        const Real calculated = option.NPV();
        const Real diff = std::fabs(calculated-expected);

        if (diff > tol) {
            BOOST_FAIL("failed to reproduce known shout option price for "
                    << "\n    strike:     " << strike
                    << "\n    option type:" <<
                        ((type == Option::Call)?"Call" : "Put")
                    << "\n    calculated: " << calculated
                    << "\n    expected:   " << expected
                    << "\n    difference: " << diff
                    << "\n    tolerance:  " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testZeroVolFDShoutNPV) {
    BOOST_TEST_MESSAGE("Testing zero volatility shout option pricing with discrete dividends...");

    const auto dc = Actual365Fixed();
    const auto today = Date(14, February, 2021);
    Settings::instance().evaluationDate() = today;

    const auto spot = Handle<Quote>(ext::make_shared<SimpleQuote>(100.0));
    const auto q = Handle<YieldTermStructure>(flatRate(0.03, dc));
    const auto r = Handle<YieldTermStructure>(flatRate(0.07, dc));

    const auto volTS = Handle<BlackVolTermStructure>(flatVol(1e-6, dc));
    const auto process = ext::make_shared<BlackScholesMertonProcess>(
            spot, q, r, volTS);

   const auto maturityDate = today + Period(1, Years);
   const Date dividendDate = today + Period(3, Months);
   const Real dividendAmount = 10.0;
   auto dividends = DividendVector({ dividendDate },{ dividendAmount });

   VanillaOption option(
       ext::make_shared<PlainVanillaPayoff>(Option::Put, 100.0),
       ext::make_shared<AmericanExercise>(today, maturityDate)
   );

   option.setPricingEngine(
       ext::make_shared<FdBlackScholesVanillaEngine>(
           process, dividends, 50, 50));

   const Real americanNPV = option.NPV();

   VanillaOption option2(
       ext::make_shared<PlainVanillaPayoff>(Option::Put, 100.0),
       ext::make_shared<AmericanExercise>(today, maturityDate)
   );

   option2.setPricingEngine(
       ext::make_shared<FdBlackScholesShoutEngine>(process, dividends, 50, 50));

   Real shoutNPV = option2.NPV();

   const DiscountFactor df = r->discount(maturityDate)/r->discount(dividendDate);

   const Real tol = 1e-3;
   Real diff = std::fabs(americanNPV - shoutNPV/df);

   if (diff > tol) {
       BOOST_FAIL("failed to reproduce American option NPV with "
                  "shout option pricing engine for "
                  << "\n    calculated: " << shoutNPV/df
                  << "\n    expected  : " << americanNPV
                  << "\n    difference: " << diff
                  << "\n    tolerance:  " << tol);
   }
}

BOOST_AUTO_TEST_CASE(testLargeDividendShoutNPV) {
    BOOST_TEST_MESSAGE("Testing zero strike shout option pricing with discrete dividends...");

    const auto dc = Actual365Fixed();
    const auto today = Date(21, February, 2021);
    Settings::instance().evaluationDate() = today;

    const Real s0 = 100.0;
    const Volatility vol = 0.25;

    const auto q = Handle<YieldTermStructure>(flatRate(0.00, dc));
    const auto r = Handle<YieldTermStructure>(flatRate(0.00, dc));
    const auto vTS = Handle<BlackVolTermStructure>(flatVol(vol, dc));

    const auto process = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(ext::make_shared<SimpleQuote>(s0)), q, r, vTS);

   const auto maturityDate = today + Period(6, Months);
   const Date dividendDate = today + Period(3, Months);
   const Real divAmount = 30.0;
   auto dividends = DividendVector({ dividendDate }, { divAmount });

   const Real strike = 80.0;
   VanillaOption option(
       ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
       ext::make_shared<AmericanExercise>(today, maturityDate)
   );

   option.setPricingEngine(
       ext::make_shared<FdBlackScholesShoutEngine>(process, dividends, 100, 400));

   Real calculated = option.NPV();

   VanillaOption ref_option(
       ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
       ext::make_shared<AmericanExercise>(today, dividendDate)
   );

   ref_option.setPricingEngine(
       ext::make_shared<FdBlackScholesShoutEngine>(process, 100, 400));

   const Real expected = ref_option.NPV()
       * r->discount(maturityDate) / r->discount(dividendDate);

   const Real tol = 5e-2;
   Real diff = std::fabs(expected - calculated);

   if (diff > tol) {
       BOOST_FAIL("failed to reproduce American option NPV with "
                  "shout option pricing engine for "
                  << "\n    calculated: " << calculated
                  << "\n    expected  : " << expected
                  << "\n    difference: " << diff
                  << "\n    tolerance:  " << tol);
   }
}

BOOST_AUTO_TEST_CASE(testEscrowedVsSpotAmericanOption) {
    BOOST_TEST_MESSAGE("Testing escrowed vs spot dividend model for American options...");

    const auto dc = Actual360();
    const auto today = Date(27, February, 2021);
    Settings::instance().evaluationDate() = today;

    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.3));

    const auto process = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(ext::make_shared<SimpleQuote>(100)),
        Handle<YieldTermStructure>(flatRate(0.08, dc)),
        Handle<YieldTermStructure>(flatRate(0.04, dc)),
        Handle<BlackVolTermStructure>(flatVol(vol, dc))
    );

   const auto maturityDate = today + Period(12, Months);
   std::vector<Date> dividendDates = { today + Period(10, Months) };
   std::vector<Real> dividendAmounts = { 10.0 };
   auto dividends = DividendVector(dividendDates, dividendAmounts);

   const Real strike = 100.0;
   VanillaOption option(
       ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
       ext::make_shared<AmericanExercise>(today, maturityDate)
   );

   option.setPricingEngine(
       ext::make_shared<FdBlackScholesVanillaEngine>(process, dividends, 100, 400));

   const Real spotNpv = option.NPV();
   const Real spotDelta = option.delta();

   vol->setValue(100/90.*0.3);

   option.setPricingEngine(
       MakeFdBlackScholesVanillaEngine(process)
       .withTGrid(100)
       .withXGrid(400)
       .withCashDividends(dividendDates, dividendAmounts)
       .withCashDividendModel(FdBlackScholesVanillaEngine::Escrowed)
   );

   const Real escrowedNpv = option.NPV();
   const Real escrowedDelta = option.delta();

   const Real diffNpv = std::abs(escrowedNpv - spotNpv);
   const Real tol = 1e-2;

   if (diffNpv > tol) {
       BOOST_FAIL("failed to compare American option NPV with "
                  "escrowed and spot dividend model "
                  << "\n    escrowed div: " << escrowedNpv
                  << "\n    spot div    : " << spotNpv
                  << "\n    difference: " << diffNpv
                  << "\n    tolerance:  " << tol);
   }


   const Real diffDelta = std::abs(escrowedDelta - spotDelta);

   if (diffDelta > tol) {
       BOOST_FAIL("failed to compare American option Delta with "
                  "escrowed and spot dividend model "
                  << "\n    escrowed div: " << escrowedDelta
                  << "\n    spot div    : " << spotDelta
                  << "\n    difference: " << diffDelta
                  << "\n    tolerance:  " << tol);
   }
}

BOOST_AUTO_TEST_CASE(testTodayIsDividendDate) {
    BOOST_TEST_MESSAGE("Testing escrowed vs spot dividend model on dividend dates for American options...");

    const auto dc = Actual360();
    const auto today = Date(27, February, 2021);
    Settings::instance().evaluationDate() = today;

    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.3));

    const auto process = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(ext::make_shared<SimpleQuote>(100)),
        Handle<YieldTermStructure>(flatRate(0.05, dc)),
        Handle<YieldTermStructure>(flatRate(0.07, dc)),
        Handle<BlackVolTermStructure>(flatVol(vol, dc))
    );

    const auto maturityDate = today + Period(12, Months);
    std::vector<Date> dividendDates = { today, today + Period(11, Months) };
    std::vector<Real> dividendAmounts = { 5.0, 5.0 };

    ext::shared_ptr<PricingEngine> spotEngine =
        MakeFdBlackScholesVanillaEngine(process)
        .withTGrid(100)
        .withXGrid(400)
        .withCashDividends(dividendDates, dividendAmounts)
        .withCashDividendModel(FdBlackScholesVanillaEngine::Spot);

    ext::shared_ptr<PricingEngine> escrowedEngine =
        MakeFdBlackScholesVanillaEngine(process)
        .withTGrid(100)
        .withXGrid(400)
        .withCashDividends(dividendDates, dividendAmounts)
        .withCashDividendModel(FdBlackScholesVanillaEngine::Escrowed);

    const Real strike = 90.0;
    VanillaOption option(
        ext::make_shared<PlainVanillaPayoff>(Option::Put, strike),
        ext::make_shared<AmericanExercise>(today, maturityDate)
    );

    option.setPricingEngine(spotEngine);

    Real spotNpv = option.NPV();
    const Real spotDelta = option.delta();
    BOOST_CHECK_THROW(option.theta(), QuantLib::Error);

    vol->setValue(100/95.*0.3);

    option.setPricingEngine(escrowedEngine);

    Real escrowedNpv = option.NPV();
    const Real escrowedDelta = option.delta();
    BOOST_CHECK_THROW(option.theta(), QuantLib::Error);

    Real diffNpv = std::abs(escrowedNpv - spotNpv);
    const Real tol = 5e-2;

    if (diffNpv > tol) {
        BOOST_FAIL("failed to compare American option NPV with "
                   "escrowed and spot dividend model "
                   << "\n    escrowed div: " << escrowedNpv
                   << "\n    spot div    : " << spotNpv
                   << "\n    difference:   " << diffNpv
                   << "\n    tolerance:    " << tol);
    }

    const Real diffDelta = std::abs(escrowedDelta - spotDelta);

    if (diffDelta > tol) {
        BOOST_FAIL("failed to compare American option Delta with "
                   "escrowed and spot dividend model "
                   << "\n    escrowed div: " << escrowedDelta
                   << "\n    spot div    : " << spotDelta
                   << "\n    difference:   " << diffDelta
                   << "\n    tolerance:    " << tol);
    }

    dividendDates[0] = today + 1;

    spotEngine =
        MakeFdBlackScholesVanillaEngine(process)
        .withTGrid(100)
        .withXGrid(400)
        .withCashDividends(dividendDates, dividendAmounts)
        .withCashDividendModel(FdBlackScholesVanillaEngine::Spot);

    escrowedEngine =
        MakeFdBlackScholesVanillaEngine(process)
        .withTGrid(100)
        .withXGrid(400)
        .withCashDividends(dividendDates, dividendAmounts)
        .withCashDividendModel(FdBlackScholesVanillaEngine::Escrowed);

    vol->setValue(0.3);

    option.setPricingEngine(spotEngine);
    spotNpv = option.NPV();

    vol->setValue(100/95.0*0.3);
    option.setPricingEngine(escrowedEngine);

    escrowedNpv = option.NPV();
    BOOST_CHECK_NO_THROW(option.theta());

    diffNpv = std::abs(escrowedNpv - spotNpv);

    if (diffNpv > tol) {
        BOOST_FAIL("failed to compare American option NPV with "
                   "escrowed and spot dividend model "
                   << "\n    escrowed div: " << escrowedNpv
                   << "\n    spot div    : " << spotNpv
                   << "\n    difference:   " << diffNpv
                   << "\n    tolerance:    " << tol);
    }
}

BOOST_AUTO_TEST_CASE(testCallPutParity) {
    BOOST_TEST_MESSAGE("Testing call/put parity for American options...");

    // R.L. McDonald, M.D. Schroder: A parity result for American option

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(8, April, 2022);
    Settings::instance().evaluationDate() = today;

    struct OptionSpec {
        Real spot;
        Real strike;
        Size maturityInDays;
        Real volatility;
        Real r;
        Real q;
    };

    auto buildStochProcess = [&dc](const OptionSpec& testCase) {
        return ext::make_shared<BlackScholesMertonProcess>(
            Handle<Quote>(ext::make_shared<SimpleQuote>(testCase.spot)),
            Handle<YieldTermStructure>(flatRate(testCase.q, dc)),
            Handle<YieldTermStructure>(flatRate(testCase.r, dc)),
            Handle<BlackVolTermStructure>(flatVol(testCase.volatility, dc))
        );
    };
    const OptionSpec testCaseSpecs[] = {
        {100.0, 100.0, 365, 0.5, 0.15, 0.02},
        {100.0, 90.0, 365, 0.5, 0.15, 0.02},
        {100.0, 125.0, 730, 0.4, 0.15, 0.05},
        {100.0, 125.0, 730, 0.4, 0.06, 0.05}
    };

    const Size xGrid = 400;
    const Size timeStepsPerYear=50;

    for (const auto& testCaseSpec: testCaseSpecs) {
        const auto maturityDate =
            today + Period(testCaseSpec.maturityInDays, Days);
        const Time maturityTime = dc.yearFraction(today,  maturityDate);
        const Size tGrid = Size(maturityTime * timeStepsPerYear);

        const auto exercise =
            ext::make_shared<AmericanExercise>(today, maturityDate);

        VanillaOption putOption(
            ext::make_shared<PlainVanillaPayoff>(
                Option::Put, testCaseSpec.strike),
            exercise
        );
        putOption.setPricingEngine(
            ext::make_shared<FdBlackScholesVanillaEngine>(
                buildStochProcess(testCaseSpec), tGrid, xGrid)
        );
        const Real putNpv = putOption.NPV();

        OptionSpec callOptionSpec = {
            testCaseSpec.strike,
            testCaseSpec.spot,
            testCaseSpec.maturityInDays,
            testCaseSpec.volatility,
            testCaseSpec.q,
            testCaseSpec.r
        };
        VanillaOption callOption(
            ext::make_shared<PlainVanillaPayoff>(
                Option::Call, callOptionSpec.strike),
            exercise
        );
        callOption.setPricingEngine(
            ext::make_shared<FdBlackScholesVanillaEngine>(
                buildStochProcess(callOptionSpec), tGrid, xGrid)
        );
        const Real callNpv = callOption.NPV();

        const Real diff = std::fabs(putNpv -callNpv);
        const Real tol = 0.001;

        if (diff > tol) {
            BOOST_FAIL("failed to reproduce American call/put parity"
                    << "\n    Put NPV   : " << putNpv
                    << "\n    Call NPV  : " << callNpv
                    << "\n    difference: " << diff
                    << "\n    tolerance : " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testQdPlusBoundaryValues) {
    BOOST_TEST_MESSAGE("Testing QD+ boundary approximation...");

    const DayCounter dc = Actual365Fixed();
    const Real S = 100;
    const Real K = 120;
    const Rate r = 0.1;
    const Rate q = 0.03;
    const Volatility sigma = 0.25;
    const Time maturity = 5.0;

    const QdPlusAmericanEngine qrPlusEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess>(), 10);

    std::vector<std::pair<Real, Real> > testCaseSpecs = {
        {4.9, 87.76960949965387},
        {4.0, 88.39053003614612},
        {2.5, 90.14327315762256},
        {1.0, 94.49793803095984},
        {0.1, 106.2588964442338}
    };

    for (const auto& testCaseSpec: testCaseSpecs) {
        const auto calculated
            = qrPlusEngine.putExerciseBoundaryAtTau(
                S, K, r, q, sigma, maturity, testCaseSpec.first);

        const Real boundary = calculated.second;
        const Size nrEvaluations = calculated.first;

        const Real expected = testCaseSpec.second;

        const Real diff = std::fabs(boundary - expected);
        const Real tol = 1e-12;

        if (diff > tol) {
            BOOST_FAIL("failed to reproduce QR+ boundary approximation"
                    << "\n    calculated: " << boundary
                    << "\n    expected:   " << expected
                    << "\n    difference: " << diff
                    << "\n    tolerance : " << tol);
        }

        if (nrEvaluations > 10) {
            BOOST_FAIL("failed to reproduce rate of convergence"
                    << "\n    evaluations: " << nrEvaluations
                    << "\n    max eval :   " << 10);
        }
    }
}

BOOST_AUTO_TEST_CASE(testQdPlusBoundaryConvergence) {
    BOOST_TEST_MESSAGE("Testing QD+ boundary convergence...");

    const DayCounter dc = Actual365Fixed();
    const Real S = 100;
    const Volatility sigma = 0.25;
    const Time maturity = 10.0;

    struct TestCaseSpec {
        Real r, q, strike;
        Size maxEvaluations;
    };

    TestCaseSpec testCases[] = {
        { 0.10, 0.03, 120, 2000 },
        { 0.0001, 0.03, 120, 2000 },
        { 0.0001, 0.000002, 120, 2000 },
        { 0.01, 0.75, 120, 2000 },
        { 0.03, 0.0, 30, 2000 },
        { 0.03, 0.0, 1e7, 2500 },
        { 0.075, 0.0, 1e-8, 2000 }
    };

    const std::vector<std::pair<QdPlusAmericanEngine::SolverType, std::string> >
        solverTypes{
        { QdPlusAmericanEngine::Brent, "Brent" },
        { QdPlusAmericanEngine::Newton, "Newton" },
        { QdPlusAmericanEngine::Ridder, "Ridder" },
        { QdPlusAmericanEngine::Halley, "Halley" },
        { QdPlusAmericanEngine::SuperHalley, "SuperHalley" }
    };

    for (const auto& testCase: testCases) {
        for (const auto& solverType : solverTypes) {
            const QdPlusAmericanEngine qrPlusEngine(
                ext::shared_ptr<GeneralizedBlackScholesProcess>(),
                Null<Size>(), solverType.first, 1e-8);

            Size nrEvaluations = 0;

            for (Real t=0.0; t < maturity; t+=0.1) {
                const auto calculated = qrPlusEngine.putExerciseBoundaryAtTau(
                    S, testCase.strike, testCase.r,
                    testCase.q, sigma, maturity, t);
                nrEvaluations += calculated.first;
            }

            const Size maxEvaluations =
                (   solverType.first == QdPlusAmericanEngine::Halley
                 || solverType.first == QdPlusAmericanEngine::SuperHalley)
                 ? 750 : testCase.maxEvaluations;

            if (nrEvaluations > maxEvaluations) {
                BOOST_FAIL("QR+ boundary approximation failed to converge "
                        << "\n    evaluations: " << nrEvaluations
                        << "\n    max eval:    " << maxEvaluations
                        << "\n    Solver:      " << solverType.second
                        << "\n    r :          " << testCase.r
                        << "\n    q :          " << testCase.q
                        << "\n    K :          " << testCase.strike);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testQdAmericanEngines) {
    BOOST_TEST_MESSAGE("Testing QD+ American option pricing...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(1, June, 2022);
    Settings::instance().evaluationDate() = today;

    struct OptionSpec {
        Option::Type optionType;
        Real spot;
        Real strike;
        Size maturityInDays;
        Real volatility;
        Real r;
        Real q;
        Real expectedValue;
        Real precision;
    };

    // high precision edge cases
    const OptionSpec edgeTestCases[] = {

        // standard put option
        {Option::Put, 100.0, 120.0, 3650, 0.25, 0.10, 0.03, 22.97383256003585, 1e-8},
        // call-put parity on standard option
        {Option::Call, 120.0, 100.0, 3650, 0.25, 0.03, 0.10, 22.97383256003585, 1e-8},

        // zero strike put
        {Option::Put, 100.0, 0.0, 365, 0.25, 0.02, 0.02, 0.0, 1e-14},
        {Option::Put, 100.0, 1e-8, 365, 0.25, 0.02, 0.02, 0.0, 1e-14},

        // zero strike call
        {Option::Call, 100.0, 0.0, 365, 0.25, 0.05, 0.01, 100.0, 1e-11},
        {Option::Call, 100.0, 1e-7, 365, 0.25, 0.05, 0.01, 100.0-1e-7, 1e-9},

        // zero vol call
        {Option::Call, 100.0, 50.0, 365, 0.0, 0.05, 0.01, 51.4435121498811085, 1e-10},
        {Option::Call, 100.0, 50.0, 365, 1e-8, 0.05, 0.01, 51.4435121498811156, 1e-8},

        // zero vol put 1
        {Option::Put, 100.0, 120.0, 4*3650, 1e-6, 0.01, 0.50, 108.980920365700442, 1e-4},
        {Option::Put, 100.0, 120.0, 4*3650, 0.0, 0.01, 0.50, 108.980904561184602, 1e-10},

        // zero vol put 2
        {Option::Put, 100.0, 120.0, 365, 1e-7, 0.05, 0.01, 20.0, 1e-9},
        {Option::Put, 100.0, 120.0, 365, 0.0, 0.05, 0.01, 20.0, 1e-12},

        // zero vol put 3
        {Option::Put, 100.0, 120.0, 365, 1e-7, 0.00, 0.05, 24.8770575499286082, 1e-8},
        {Option::Put, 100.0, 120.0, 365, 0.0, 0.00, 0.05, 24.8770575499286082, 1e-10},

        // zero spot put
        {Option::Put, 1e-6, 120.0, 365, 0.25, -0.075, 0.05, 129.346097154926355, 1e-9},
        {Option::Put, 0.0, 120.0, 365, 0.25, -0.075, 0.05, 129.346098106155779, 1e-10},

        // zero spot call
        {Option::Call, 1e-6, 120.0, 365, 0.25, 0.075, 0.05, 0.0, 1e-14},
        {Option::Call, 0.0, 120.0, 365, 0.25, 0.075, 0.05, 0.0, 1e-14},

        // put option with one day left
        {Option::Put, 100.0, 120.0, 1, 0.25, 0.05, 0.0, 20.0, 1e-10},

        // put option at maturity
        {Option::Put, 100.0, 120.0, 0, 0.25, 0.05, 0.0, 0.0, 1e-14},

        // zero everything
        {Option::Put, 0.0, 0.0, 365, 0.0, 0.0, 0.0, 0.0, 1e-14},

        // zero strike call with zero vol
        {Option::Call, 100.0, 1e-7, 365, 1e-8, 0.05, 0.025, 100.0-1e-7, 1e-8},
        {Option::Call, 100.0, 0.0, 365, 1e-8, 0.05, 0.025, 100.0, 1e-8},
        {Option::Call, 100.0, 1e-7, 365, 0.0, 0.05, 0.025, 100.0-1e-7, 1e-8},
        {Option::Call, 100.0, 0.0, 365, 0.0, 0.05, 0.025, 100.0, 1e-8},

        // zero spot call with zero vol
        {Option::Call, 1e-8, 100, 365, 1e-8, 0.05, 0.025, 0.0, 1e-10},
        {Option::Call, 0.0, 100, 365, 0.0, 0.05, 0.025, 0.0, 1e-14},

        // zero interest rate call
        {Option::Call, 100, 100, 365, 0.25, 0.0, 0.025, 8.871505915120776, 1e-8},

        // zero dividend rate call
        {Option::Call, 100, 100, 365, 0.25, 0.05, 0.0, 12.3359989303687243, 1e-8},

        // extreme spot call
        {Option::Call, 1e10, 100, 365, 0.25, 0.01, 0.05, 1e10-100.0, -1},

        // extreme strike call
        {Option::Call, 100, 1e10, 365, 0.25, 0.01, 0.05, 0.0, 1e-14},

        // extreme vol call
        {Option::Call, 100, 100, 365, 100.0, 0.01, 0.05, 99.9874942266127, 1e-8},

        // extreme dividend yield call
        {Option::Call, 100, 100, 365, 0.25, 0.10, 10.0, 0.1159627202107989, 1e-8},

        // extreme maturity call
        {Option::Call, 100, 100, 170*365, 0.25, 0.01, 0.002, 80.37468392429741, 1e-8}
    };

    // random test cases
    const double pde_values[] = {
        581.46895,113.78442,581.44547,1408.579,49.19448,1060.27367,
        834.83366,176.48305,120.38008,307.11264,602.7006,233.80171,
        204.74596,0.30987,0,0,5.36215,0.01711,0,84.51193,0.67131,
        0.06414,152.67188,54.75257,90.31861,168.50289,18.38926,0,
        282.4995,0,0.08428,12.30929,42.26359,139.87748,0.28724,0.00421,
        0,0.00206,0,658.60427,140.51139,23.17387,0.35612,0,909.14828,
        0,0.11549,5.46749,144.25428,2576.6754,562.16484,0,122.725,
        383.48463,278.7447,3.52566,82.34348,81.06139,0,10.42824,
        4.95917,25.28602,31.38869,3.53697,0,0.012,0,0.4263,162.16184,
        0.4618,97.714,283.03442,0.38176,70.25367,134.94142,2.19293,
        226.4746,76.74309,46.03123,15.76214,0.01666,1806.26208,0,
        103.93726,6.82956,337.81301,0.64236,677.63248,25.01763,
        443.79052,1793.78327,118.6293,185.79849,11.59313,679.01736,
        17.99005,403.57554,1.67418,0,0.03795,3326.09089,71.1996,
        0,485.10353,0,1681.25166,0,43.15432,0.75825,0.05895,34.71493,
        0.00015,5.58671,115.98793,37.7713,399.24494,0.00766,445.42207,
        152.65397,0,47.05874,0.96921,14.21875,257.84754,109.62533,
        2553.99295,138.46663,192.33614,81.41877,18.21403,113.926,
        27.28409,174.77093,42.70527,0.90326,0,967.9901,616.0143,
        253.56442,0.00397,2493.82098,9.29406,11.00023,0,0,234.12481,
        0,72.46356,0,9.00932,48.67934,29.42756,13.4271,0,0,0,0,20.71417,
        48.57474,2.26452,0,109.0243,0,21.26801,1.21164,0,86.25232,
        36.00437,4.53844,7.40503,313.53602,379.76105,165.84347,77.19665,
        9.02466,0.10634,214.84982,6.13387,133.44645,303.25953,0,
        134.26724,246.89804,0,123.32975,32.83429,9.56819,7.42582,0,
        73.82832,196.84831,0.00001,72.70391,2173.8649,123.00513,
        153.83539,21.63003,209.84752,30.12425,0,197.6502,0,164.02863,
        7.65143,56.57631,2392.70018,0,0,34.23457,171.08459,0.49387,
        31.13395,237.68801,0.01262,0,0,0,0,41.56635,0,8.41535,55.01775,
        310.50094,0,14.85456,174.34018,7.19772,0.00001,0,91.70874,
        0.00001,17.51724,0.00587,0,532.24902,2.05553,36.80843,0,
        33.39288,0.00006,0.04439,1.3434,0,0.41816,926.37642,0,247.61559,
        151.98965,0.35243,4.33198,23294.47744,0.00791,12.51996,53.47727,
        167.95572,0.0062,6.8482,0,347.83408,852.85742,558.21422,0,
        53.89293,78.61011,187.3978,9.18927,0.00553,113.48101,1467.30556,
        74.82251,94.84476,0,101.3649,59.27007,0,773.81251,0,542.7889,0,
        68.96209,96.0435,0.00004,0.10738,0.00187,324.97758,245.68455,
        30.52818,129.84472,0,46.86288,368.41675,139.29763,4.4393,16.29594,
        25.7554,64.02621,89.41363,0.62751,219.65237,0.26039,0,12.02172,
        101.97733,69.37456,45.81122,1263.33603,164.31607,15.88788,0,48.77797,
        0.13233,147.16808,10.31217,7.50634,7.48611,177.95409,225.77562,3.56947,
        0.02531,4.88869,8.76632,0,0,0.02214,305.08468,44.52185,182.17332,
        538.31458,0,46.97229,0,31.94202,410.43038,0,70.35432,15.58346,74.14177,
        953.67663,11.79128,59.83061,0,37.86557,1184.22731,2411.37823,0,0,0,0,
        49.3179,236.38654,21.36225,218.048,517.57006,0,0,12.52933,256.71967,
        0.00025,1.47981,158.19166,0,1923.70709,4.94441,1199.81196,45.92353,
        85.73255,14.91338,88.81459,21.42459,3456.9466,31.97838,233.26863,
        49.34801,2684.07758,0,0,32.24149,0,111.79552,0.00506,8.77602,0,
        406.54213,0.32974,365.53998,1.49714,19.65603,37.33877,205.06928,
        0.01805,589.23478,9.58273,0.02946,286.48706,463.34512,528.21392,0,
        47.71294,21.0864,114.81771,80.489,21.30905,41.95873,19.03598,156.09295,
        0,73.6509,0,0,168.17576,0,32.71243,36.75044,177.64583,0.05618,
        156.38616,1370.4754,24.5976,59.83173,0,354.93074,34.96889,0.00532,
        16.95287,1259.72993,241.05777,18.9778,0.57635,43.98093,25.2678,
        369.39896,0.31549,0,31.95512,101.60559,11.22079,970.16273,0,0,
        1.55445,0,18.6067,0,1124.20117,52.67762,10.38273,0,10.22588,251.27813,
        0,431.82244,0,1.31252,0,84.72154,100.98411,160.95557,129.51372,
        0.00026,103.81663,421.64767,0.00031,0,104.48529,162.59225,0,
        1504.0869,88.11253,4.14052,0.07195,203.78754,0.00002,42.5395,0,
        17.05087,26.89157,64.64923,0,390.87453,124.55406,0.01018,94.23963};

    std::vector<OptionSpec> testCaseSpecs;
    testCaseSpecs.reserve(std::size(pde_values) + std::size(edgeTestCases));

    PseudoRandom::rng_type rng(PseudoRandom::urng_type(12345UL));

    for (double pde_value : pde_values) {
        const Option::Type optionType
            = (rng.next().value > 0)? Option::Call : Option::Put;
        const Real spot = 100*std::exp(1.5*rng.next().value);
        const Real strike = 100*std::exp(1.5*rng.next().value);
        const Size maturityInDays = Size(1 + 365*std::exp(2*rng.next().value));
        const Volatility  vol = 0.5*std::exp(rng.next().value);
        const Rate r = 0.10*std::exp(rng.next().value);
        const Rate q = 0.10*std::exp(rng.next().value);

        const OptionSpec spec = {optionType, spot,      strike, maturityInDays, vol, r,
                                 q,          pde_value, -1};

        testCaseSpecs.push_back(spec);
    }

    testCaseSpecs.insert(
        testCaseSpecs.end(),std::begin(edgeTestCases), std::end(edgeTestCases));

    const auto spot = ext::make_shared<SimpleQuote>(1.0);
    const auto rRate = ext::make_shared<SimpleQuote>(0.0);
    const auto qRate = ext::make_shared<SimpleQuote>(0.0);
    const auto vol = ext::make_shared<SimpleQuote>(0.0);

    const auto bsProcess = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(spot),
        Handle<YieldTermStructure>(flatRate(today, qRate, dc)),
        Handle<YieldTermStructure>(flatRate(today, rRate, dc)),
        Handle<BlackVolTermStructure>(flatVol(today, vol, dc))
    );

    const auto qrPlusAmericanEngine =
        ext::make_shared<QdPlusAmericanEngine>(
            bsProcess, 8, QdPlusAmericanEngine::Halley, 1e-10
        );

    for (const auto& testCaseSpec: testCaseSpecs) {
        const Date maturityDate =
            today + Period(testCaseSpec.maturityInDays, Days);

        spot->setValue(testCaseSpec.spot);
        rRate->setValue(testCaseSpec.r);
        qRate->setValue(testCaseSpec.q);
        vol->setValue(testCaseSpec.volatility);

        VanillaOption option(
            ext::make_shared<PlainVanillaPayoff>(
                testCaseSpec.optionType, testCaseSpec.strike),
            ext::make_shared<AmericanExercise>(today, maturityDate)
        );
        option.setPricingEngine(qrPlusAmericanEngine);

        const Real calculated = option.NPV();
        const Real expected = testCaseSpec.expectedValue;

        if ((testCaseSpec.precision > 0
                && std::abs(expected-calculated) > testCaseSpec.precision)
            || (testCaseSpec.precision < 0
                    && expected > 0.1 && std::abs(calculated-expected)/expected > 0.005)
            || (testCaseSpec.precision < 0 && expected <= 0.1
                    && std::abs(expected-calculated) > 5e-4)) {
            BOOST_ERROR("QR+ boundary approximation failed to "
                    "reproduce cached edge and PDE values for "
                    << "\n    OptionType: " <<
                    ((testCaseSpec.optionType == Option::Call)? "Call" : "Put")
                    << std::setprecision(16)
                    << "\n    spot:       " << spot->value()
                    << "\n    strike:     " << testCaseSpec.strike
                    << "\n    r:          " << rRate->value()
                    << "\n    q:          " << qRate->value()
                    << "\n    vol:        " << vol->value()
                    << "\n    calculated: " << calculated
                    << "\n    expected:   " << expected);
        }
    };
}

BOOST_AUTO_TEST_CASE(testQdFpIterationScheme) {
    BOOST_TEST_MESSAGE("Testing Legendre and tanh-sinh iteration scheme for QD+ fixed-point American engine...");

    const Real tol = 1e-8;
    const Size l=32, m=6, n=18, p=36;

    const ext::shared_ptr<QdFpIterationScheme> schemes[] = {
        ext::make_shared<QdFpLegendreScheme>(l, m, n, p),
        ext::make_shared<QdFpLegendreTanhSinhScheme>(l, m, n, tol),
        ext::make_shared<QdFpTanhSinhIterationScheme>(m, n, tol)
    };

    const NormalDistribution nd;

    for (const auto& scheme: schemes) {
        BOOST_CHECK_EQUAL(n, scheme->getNumberOfChebyshevInterpolationNodes());
        BOOST_CHECK_EQUAL(1, scheme->getNumberOfJacobiNewtonFixedPointSteps());
        BOOST_CHECK_EQUAL(m-1, scheme->getNumberOfNaiveFixedPointSteps());

        QL_CHECK_SMALL(scheme->getFixedPointIntegrator()
            ->operator()(nd, -10.0, 10.0) - 1.0, tol);
        QL_CHECK_SMALL(scheme->getExerciseBoundaryToPriceIntegrator()
            ->operator()(nd, -10.0, 10.0) - 1.0, tol);
    }
}

BOOST_AUTO_TEST_CASE(testAndersenLakeHighPrecisionExample) {
    BOOST_TEST_MESSAGE("Testing Andersen, Lake and Offengenden high precision example...");

    // Example and results are taken from
    //    Leif Andersen, Mark Lake and Dimitri Offengenden (2015)
    //    "High Performance American Option Pricing",
    //    https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2547027

    struct SchemeSpec {
        Size l, m, n;
        Rate r;
        Real expected[2];
        Real tol;
    };
    
    const SchemeSpec testCases[] = {
        { 24, 3, 9,  0.05, {0.1069528125898476, 0.1069524359360852}, 1e-6},
        {  5, 1, 4,  0.05, {0.1070237787625299, 0.1070042740171235}, 1e-3},
        { 11, 2, 5,  0.05, {0.106938750864602, 0.1069479057531648}, 1e-4},
        { 35, 8, 16, 0.05, {0.1069527032381714, 0.106952558361499}, 1e-9},
        { 65, 8, 32, 0.05, {0.1069527028247546, 0.1069526779971959}, 1e-11},
        {  5, 1, 4, 0.075, {0.3674420299196104, 0.3674766444325588}, 1e-3},
        { 11, 2, 5, 0.075, {0.3671056766787473, 0.3671024005532715}, 1e-4},
        { 35, 8, 16,0.075, {0.3671116758420414, 0.3671111055677869}, 1e-9},
        { 65, 8, 32,0.075, {0.3671112309062572, 0.3671111267813689}, 1e-11}
    };

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(25, July, 2022);
    Settings::instance().evaluationDate() = today;

    const auto spot = ext::make_shared<SimpleQuote>(100.0);
    const Real strike = 100.0;
    const Rate q = 0.05;
    const Volatility vol = 0.25;
    const Date maturityDate = today + Period(1, Years);

    const auto payoff =
        ext::make_shared<PlainVanillaPayoff>(Option::Put, strike);

    for (const auto& testCase: testCases) {
        const Size l = testCase.l;
        const Size m = testCase.m;
        const Size n = testCase.n;
        const Rate r = testCase.r;
        const Real tol = testCase.tol;

        const auto bsProcess = ext::make_shared<BlackScholesMertonProcess>(
            Handle<Quote>(spot),
            Handle<YieldTermStructure>(flatRate(today, q, dc)),
            Handle<YieldTermStructure>(flatRate(today, r, dc)),
            Handle<BlackVolTermStructure>(flatVol(today, vol, dc))
        );

        VanillaOption americanOption(
            payoff, ext::make_shared<AmericanExercise>(today, maturityDate));

        VanillaOption europeanOption(
            payoff, ext::make_shared<EuropeanExercise>(maturityDate));

        europeanOption.setPricingEngine(
            ext::make_shared<AnalyticEuropeanEngine>(bsProcess));

        const Real europeanNPV = europeanOption.NPV();

        const QdFpAmericanEngine::FixedPointEquation schemes[] = {
            QdFpAmericanEngine::FP_A, QdFpAmericanEngine::FP_B
        };

        for (Size i=0; i < std::size(schemes); ++i) {

            americanOption.setPricingEngine(
                ext::make_shared<QdFpAmericanEngine>(
                    bsProcess,
                    ext::make_shared<QdFpLegendreTanhSinhScheme>(l, m, n, tol),
                    schemes[i])
            );

            const Real americanNPV = americanOption.NPV();
            const Real americanPremium = americanNPV - europeanNPV;

            const Real diff = std::abs(americanPremium - testCase.expected[i]);
            if (diff > tol) {
                BOOST_ERROR("failed to reproduce high precision literature values"
                        << "\n    FP-Scheme: " <<
                        ((schemes[i] == QdFpAmericanEngine::FP_A)? "FP-A" : "FP-B")
                        << "\n    r        : " << r
                        << "\n    (l,m,n)  : (" << l << "," << m << "," << n << ")"
                        << "\n    diff     : " << diff
                        << "\n    tol      : " << tol);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testQdEngineStandardExample) {
    BOOST_TEST_MESSAGE("Testing Andersen, Lake and Offengenden standard example...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(1, June, 2022);
    Settings::instance().evaluationDate() = today;

    const Real S = 100;
    const Real K = 95;
    const Rate r = 0.075;
    const Rate q = 0.05;
    const Volatility sigma = 0.25;
    const Date maturityDate = today + Period(1, Years);

    const auto bsProcess = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(ext::make_shared<SimpleQuote>(S)),
        Handle<YieldTermStructure>(flatRate(today, q, dc)),
        Handle<YieldTermStructure>(flatRate(today, r, dc)),
        Handle<BlackVolTermStructure>(flatVol(today, sigma, dc))
    );

    const auto payoff =
        ext::make_shared<PlainVanillaPayoff>(Option::Put, K);

    VanillaOption europeanOption(
        payoff, ext::make_shared<EuropeanExercise>(maturityDate));

    europeanOption.setPricingEngine(
        ext::make_shared<AnalyticEuropeanEngine>(bsProcess));

    VanillaOption americanOption(
        payoff, ext::make_shared<AmericanExercise>(today, maturityDate));


    const QdFpAmericanEngine::FixedPointEquation schemes[] = {
        QdFpAmericanEngine::FP_A, QdFpAmericanEngine::FP_B
    };
    const Real expected[] = { 0.2386475283369327, 0.2386596962737606 };

    for (Size i=0; i < std::size(schemes); ++i) {
        americanOption.setPricingEngine(
            ext::make_shared<QdFpAmericanEngine>(
                bsProcess,
                ext::make_shared<QdFpLegendreScheme>(32, 2, 15, 48),
                schemes[i])
        );
        const Real calculated = americanOption.NPV() - europeanOption.NPV();

        const Real tol = 7e-15;
        const Real diff = std::abs(calculated - expected[i]);

        if (diff > tol) {
            BOOST_ERROR("failed to reproduce high precision test values"
                    << "\n    diff     : " << diff
                    << "\n    tol      : " << tol);
        }
    }
}

class QdFpGaussLobattoScheme: public QdFpIterationScheme {
  public:
    QdFpGaussLobattoScheme(Size m, Size n, Real eps)
    : m_(m), n_(n),
      integrator_(ext::make_shared<GaussLobattoIntegral>(
                                                         100000, QL_MAX_REAL, 0.1*eps)) {
    }
    Size getNumberOfChebyshevInterpolationNodes() const override {
        return n_;
    }
    Size getNumberOfNaiveFixedPointSteps() const override {
        return m_-1;
    }
    Size getNumberOfJacobiNewtonFixedPointSteps() const override {
        return Size(1);
    }
    ext::shared_ptr<Integrator>
    getFixedPointIntegrator() const override {
        return integrator_;
    }
    ext::shared_ptr<Integrator>
    getExerciseBoundaryToPriceIntegrator() const override {
        return integrator_;
    }

  private:
    const Size m_, n_;
    const ext::shared_ptr<Integrator> integrator_;
};


BOOST_AUTO_TEST_CASE(testBulkQdFpAmericanEngine) {
    BOOST_TEST_MESSAGE("Testing Andersen, Lake and Offengenden bulk examples...");

    // Examples are taken from
    //    Leif Andersen, Mark Lake and Dimitri Offengenden (2015)
    //    "High Performance American Option Pricing",
    //    https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2547027

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(1, June, 2022);
    Settings::instance().evaluationDate() = today;

    const auto spot = ext::make_shared<SimpleQuote>(1.0);
    const auto rRate = ext::make_shared<SimpleQuote>(0.0);
    const auto qRate = ext::make_shared<SimpleQuote>(0.0);
    const auto vol = ext::make_shared<SimpleQuote>(0.0);

    // original test set from the article, takes too long
    // const Size T[] = {30, 91, 182, 273, 365};
    // const Rate rf[] = {0.02, 0.04, 0.06, 0.08, 0.1};
    // const Rate qy[] = {0, 0.04, 0.08, 0.12};
    // const Real S[] = {25, 50, 80, 90, 100, 110, 120, 150, 175, 200};
    // const Volatility sig[] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6};

     const Size T[] = {30, 182, 365};
     const Rate rf[] = {0.02, 0.04, 0.06, 0.1};
     const Rate qy[] = {0, 0.04, 0.08, 0.12};
     const Real S[] = {25, 75, 100, 125, 200};
     const Volatility sig[] = {0.1, 0.25, 0.6};

    const auto payoff = ext::make_shared<PlainVanillaPayoff>(Option::Put, 100);

    const auto bsProcess = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(spot),
        Handle<YieldTermStructure>(flatRate(today, qRate, dc)),
        Handle<YieldTermStructure>(flatRate(today, rRate, dc)),
        Handle<BlackVolTermStructure>(flatVol(today, vol, dc))
    );

    const auto qdFpFastAmericanEngine =
        ext::make_shared<QdFpAmericanEngine>(
            bsProcess, QdFpAmericanEngine::fastScheme());

    const auto qdFpAccurateAmericanEngine =
        ext::make_shared<QdFpAmericanEngine>(
            bsProcess, QdFpAmericanEngine::accurateScheme());

    const auto qdFpGaussLobattoAmericanEngine =
        ext::make_shared<QdFpAmericanEngine>(
            bsProcess,ext::make_shared<QdFpGaussLobattoScheme>(3, 7, 1e-5));

    IncrementalStatistics statsAccurate, statsLobatto;
    for (auto t: T) {
        const Date maturityDate = today + Period(t, Days);
        VanillaOption option(
            payoff, ext::make_shared<AmericanExercise>(today, maturityDate));

        for (auto r: rf) {
            rRate->setValue(r);
            for (auto q: qy) {
                qRate->setValue(q);
                for (auto v: sig) {
                    vol->setValue(v);
                    for (auto s: S) {
                        spot->setValue(s);

                        option.setPricingEngine(qdFpFastAmericanEngine);
                        const Real fast = option.NPV();

                        option.setPricingEngine(qdFpAccurateAmericanEngine);
                        const Real accurate = option.NPV();
                        statsAccurate.add(std::abs(fast-accurate));

                        option.setPricingEngine(qdFpGaussLobattoAmericanEngine);
                        const Real lobatto = option.NPV();
                        statsLobatto.add(std::abs(accurate-lobatto));
                    }
                }
            }
        }
    }


    const Real tolStdDev = 1e-4;
    if (statsAccurate.standardDeviation() > tolStdDev)
        BOOST_ERROR("failed to reproduce low RMSE with fast American engine"
                << "\n    RMSE diff: " << statsAccurate.standardDeviation()
                << "\n    tol      : " << tolStdDev);

    if (statsLobatto.standardDeviation() > tolStdDev)
        BOOST_ERROR("failed to reproduce low RMSE with fast Lobatto American engine"
                << "\n    RMSE diff: " << statsLobatto.standardDeviation()
                << "\n    tol      : " << tolStdDev);

    const Real tolMax = 2.5e-3;
    if (statsAccurate.max() > tolMax)
        BOOST_ERROR("failed to reproduce low max deviation "
                "with fast American engine"
                << "\n    max diff: " << statsAccurate.max()
                << "\n    tol     : " << tolMax);

    if (statsLobatto.max() > tolMax)
        BOOST_ERROR("failed to reproduce low max deviation "
                "with fast Lobatto American engine"
                << "\n    max diff: " << statsLobatto.max()
                << "\n    tol     : " << tolMax);
}

BOOST_AUTO_TEST_CASE(testQdEngineWithLobattoIntegral) {
    BOOST_TEST_MESSAGE("Testing Andersen, Lake and Offengenden "
                       "with high precision Gauss-Lobatto integration...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(5, November, 2022);
    Settings::instance().evaluationDate() = today;

    const auto spot = ext::make_shared<SimpleQuote>(36);
    const Real K = 40;
    const Rate r = 0.075;
    const Rate q = 0.01;
    const Volatility sigma = 0.40;
    const Date maturityDate = today + Period(2, Years);

    const auto bsProcess = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(spot),
        Handle<YieldTermStructure>(flatRate(today, q, dc)),
        Handle<YieldTermStructure>(flatRate(today, r, dc)),
        Handle<BlackVolTermStructure>(flatVol(today, sigma, dc))
    );

    VanillaOption option(
        ext::make_shared<PlainVanillaPayoff>(Option::Put, K),
        ext::make_shared<AmericanExercise>(today, maturityDate)
    );

    const QdFpAmericanEngine::FixedPointEquation schemes[] = {
        QdFpAmericanEngine::FP_A, QdFpAmericanEngine::FP_B
    };

    const auto gaussLobattoScheme =
        ext::make_shared<QdFpGaussLobattoScheme>(10, 30, 1e-10);

    for (auto scheme: schemes) {
        const auto highPrecisionEngine =
            ext::make_shared<QdFpAmericanEngine>(
                bsProcess, QdFpAmericanEngine::highPrecisionScheme(), scheme);
        const auto lobattoEngine =
            ext::make_shared<QdFpAmericanEngine>(
                bsProcess, gaussLobattoScheme, scheme);

        for (Real s: std::list<Real>{36, 40-1e-8, 40, 40+1e-8, 50}) {
            spot->setValue(s);

            option.setPricingEngine(highPrecisionEngine);
            const Real highPrecisionNPV = option.NPV();

            option.setPricingEngine(lobattoEngine);
            const Real lobattoNPV = option.NPV();

            const Real diff = std::abs(lobattoNPV - highPrecisionNPV);
            const Real tol = 1e-11;

            if (diff > tol || std::isnan(lobattoNPV)) {
                BOOST_ERROR("failed to reproduce high precision American "
                        "option values with QD+ fixed point and Lobatto integration"
                        << "\n    FP-Scheme: " <<
                        ((scheme == QdFpAmericanEngine::FP_A)? "FP-A" : "FP-B")
                        << "\n    spot     : " << s
                        << "\n    diff     : " << diff
                        << "\n    tol      : " << tol);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testQdNegativeDividendYield) {
    BOOST_TEST_MESSAGE("Testing Andersen, Lake and Offengenden "
                        "with positive or zero interest rate and "
                        "negative dividend yield...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(5, December, 2022);
    Settings::instance().evaluationDate() = today;
    const Date maturityDate = today + Period(18, Months);

    const Real K = 90;
    const auto spot = ext::make_shared<SimpleQuote>(100);
    const auto qRate = ext::make_shared<SimpleQuote>(0);
    const auto rRate = ext::make_shared<SimpleQuote>(0);

    const auto process = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(spot),
        Handle<YieldTermStructure>(flatRate(qRate, dc)),
        Handle<YieldTermStructure>(flatRate(rRate, dc)),
        Handle<BlackVolTermStructure>(flatVol(0.4, dc))
    );

    VanillaOption option(
        ext::make_shared<PlainVanillaPayoff>(Option::Put, K),
        ext::make_shared<AmericanExercise>(today, maturityDate)
    );
    const auto qdPlusEngine =
        ext::make_shared<QdPlusAmericanEngine>(process);
    const auto qdFpEngine =
        ext::make_shared<QdFpAmericanEngine>(process);
    const auto fdmEngine =
        ext::make_shared<FdBlackScholesVanillaEngine>(process, 800, 200);

    const Rate rRates[] = {0.025, 0.5, 0.0, 1e-6};
    const Rate qRates[] = {-0.05, -0.1, -0.5, 0.0};

    for (auto r: rRates) {
        rRate->setValue(r);
        for (auto q: qRates) {
            qRate->setValue(q);

            option.setPricingEngine(qdFpEngine);
            const Real qdFpNPV = option.NPV();
            option.setPricingEngine(qdPlusEngine);
            const Real qdPlusNPV = option.NPV();
            option.setPricingEngine(fdmEngine);
            const Real fdmNPV = option.NPV();

            const Real tol = 1.5e-2;
            const Real diffFdmQqFp = std::abs(fdmNPV - qdFpNPV);

            if (diffFdmQqFp > tol || std::isnan(diffFdmQqFp)) {
                BOOST_ERROR("failed to reproduce QD+ fixed point values"
                        << "\n    r        : " << r
                        << "\n    q        : " << q
                        << "\n    diff     : " << diffFdmQqFp
                        << "\n    tol      : " << tol);
            }

            const Real diffFdmQdPlus = std::abs(fdmNPV - qdPlusNPV);
            if (diffFdmQdPlus > 5*tol || std::isnan(diffFdmQdPlus)) {
                BOOST_ERROR("failed to reproduce QD+ values"
                        << "\n    r        : " << r
                        << "\n    q        : " << q
                        << "\n    diff     : " << diffFdmQdPlus
                        << "\n    tol      : " << 5*tol);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testBjerksundStenslandEuropeanGreeks) {
    BOOST_TEST_MESSAGE("Testing Bjerksund-Stensland greeks when early exercise is not optimal...");

    const Date today = Date(5, November, 2022);
    Settings::instance().evaluationDate() = today;

    const auto spot = ext::make_shared<SimpleQuote>(100);
    const Real K = 105;

    const Volatility sigma = 0.40;
    const Date maturityDate = today + Period(724, Days);

    const auto qTS = ext::make_shared<SimpleQuote>(0.0);
    const auto rTS = ext::make_shared<SimpleQuote>(0.0);

    const auto bsProcess = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(spot),
        Handle<YieldTermStructure>(flatRate(qTS, Actual365Fixed())),
        Handle<YieldTermStructure>(flatRate(rTS, Actual360())),
        Handle<BlackVolTermStructure>(
            flatVol(today, sigma, Thirty360(Thirty360::European)))
    );

    struct OptionSpec {
        Option::Type type;
        Real r;
        Real q;
    };

    const OptionSpec testCaseSpecs[] = {
        {Option::Put, -0.05, 0.02},
        {Option::Call, 0.05, -0.025}
    };

    const auto europeanExercise =
        ext::make_shared<EuropeanExercise>(maturityDate);
    const auto americanExercise =
        ext::make_shared<AmericanExercise>(today, maturityDate);

    const auto europeanEngine =
        ext::make_shared<AnalyticEuropeanEngine>(bsProcess);

    const auto bjerksundStenslandEngine =
        ext::make_shared<BjerksundStenslandApproximationEngine>(bsProcess);


    for (const auto& testCaseSpec: testCaseSpecs) {
        qTS->setValue(testCaseSpec.q);
        rTS->setValue(testCaseSpec.r);

        VanillaOption americanOption(
            ext::make_shared<PlainVanillaPayoff>(testCaseSpec.type, K),
            americanExercise
        );
        americanOption.setPricingEngine(bjerksundStenslandEngine);

        VanillaOption europeanOption(
            ext::make_shared<PlainVanillaPayoff>(testCaseSpec.type, K),
            europeanExercise
        );
        europeanOption.setPricingEngine(europeanEngine);

        constexpr double tol = 1000*QL_EPSILON;

        QL_CHECK_CLOSE(europeanOption.NPV(), americanOption.NPV(), tol);
        QL_CHECK_CLOSE(europeanOption.delta(), americanOption.delta(), tol);
        QL_CHECK_CLOSE(europeanOption.strikeSensitivity(), americanOption.strikeSensitivity(), tol);
        QL_CHECK_CLOSE(europeanOption.gamma(), americanOption.gamma(), tol);
        QL_CHECK_CLOSE(europeanOption.vega(), americanOption.vega(), tol);
        QL_CHECK_CLOSE(europeanOption.theta(), americanOption.theta(), tol);
        QL_CHECK_CLOSE(europeanOption.thetaPerDay(), americanOption.thetaPerDay(), tol);
        QL_CHECK_CLOSE(europeanOption.rho(), americanOption.rho(), tol);
        QL_CHECK_CLOSE(europeanOption.dividendRho(), americanOption.dividendRho(), tol);
    }
}

BOOST_AUTO_TEST_CASE(testBjerksundStenslandAmericanGreeks) {
    BOOST_TEST_MESSAGE("Testing Bjerksund-Stensland American greeks...");

    const Date today = Date(5, December, 2022);
    Settings::instance().evaluationDate() = today;

    const auto spot = ext::make_shared<SimpleQuote>(0);
    const auto vol = ext::make_shared<SimpleQuote>(0);

    const auto qRate = ext::make_shared<SimpleQuote>(0);
    const auto rRate = ext::make_shared<SimpleQuote>(0);

    const auto bsProcess = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(spot),
        Handle<YieldTermStructure>(flatRate(qRate, Actual360())),
        Handle<YieldTermStructure>(flatRate(rRate, Actual365Fixed())),
        Handle<BlackVolTermStructure>(
            flatVol(today, vol, Thirty360(Thirty360::ISDA)))
    );

    const auto bjerksundStenslandEngine =
        ext::make_shared<BjerksundStenslandApproximationEngine>(bsProcess);

    const Real strike = 100;
    const Option::Type types[] = { Option::Call, Option::Put };
    const Rate rf[] = {0.0, 0.02, 0.06, 0.1, 0.2};
    const Rate qy[] = {0.0, 0.08, 0.12};

    const Volatility sig[] = {0.1, 0.2, 0.4, 1.0};
    const Real S[] = {25, 50, 99.9, 110, 150, 200};
    const Size T[] = {30, 182, 365, 1825};

    const Real f_d = 1e-5;
    const Real f_g = 5e-5;
    const Real f_q = 1e-6;

    for (auto type: types) {
        const auto payoff = [type](Real strike) {
            return ext::make_shared<PlainVanillaPayoff>(type, strike);};

        const auto stdPayoff = payoff(strike);

        for (auto t: T) {
            const Date maturityDate = today + Period(t, Days);
            const auto exercise = [today, maturityDate](const Period& offset) {
                return ext::make_shared<AmericanExercise>(
                    today, maturityDate + offset);};

            const auto stdExercise = exercise(Period(0, Days));

            VanillaOption option(stdPayoff, stdExercise);
            option.setPricingEngine(bjerksundStenslandEngine);

            VanillaOption strike_up(payoff(strike*(1+f_d)), stdExercise);
            strike_up.setPricingEngine(bjerksundStenslandEngine);
            VanillaOption strike_down(payoff(strike*(1-f_d)), stdExercise);
            strike_down.setPricingEngine(bjerksundStenslandEngine);

            VanillaOption day_up(stdPayoff, exercise(Period(1, Days)));
            day_up.setPricingEngine(bjerksundStenslandEngine);
            VanillaOption day_down(stdPayoff, exercise(Period(-1, Days)));
            day_down.setPricingEngine(bjerksundStenslandEngine);


            for (auto r: rf) {
                rRate->setValue(r);
                for (auto q: qy) {
                    qRate->setValue(q);
                    for (auto v: sig) {
                        vol->setValue(v);
                        for (auto s: S) {
                            spot->setValue(s);

                            const Real npv = option.NPV();
                            const Real delta = option.delta();
                            const Real gamma = option.gamma();
                            const Real strikeSensitivity = option.strikeSensitivity();
                            const Real dividendRho = option.dividendRho();
                            const Real rho = option.rho();
                            const Real vega = option.vega();
                            const Real theta = option.theta();
                            const auto exerciseType = ext::any_cast<std::string>(
                                option.additionalResults().find("exerciseType")->second);

                            OneAssetOption::results numericalResults;

                            spot->setValue(s*(1+f_d));
                            const Real f2 = option.NPV();
                            spot->setValue(s*(1-f_d));
                            const Real f1 = option.NPV();
                            spot->setValue(s);
                            numericalResults.delta = (f2 - f1)/(2*f_d*s);

                            Real error = std::abs(delta - numericalResults.delta);
                            if (error > 5e-6)
                                REPORT_FAILURE("delta", \
                                    stdPayoff, stdExercise, s, q, r, today, v, \
                                    numericalResults.delta, delta, error, 5e-6);

                            spot->setValue(s*(1+2*f_g));
                            const Real gp2 = option.NPV();
                            spot->setValue(s*(1+f_g));
                            const Real gp1 = option.NPV();
                            spot->setValue(s*(1-f_g));
                            const Real gm1 = option.NPV();
                            spot->setValue(s*(1-2*f_g));
                            const Real gm2 = option.NPV();
                            spot->setValue(s);
                            numericalResults.gamma
                                = (-gp2 + 16*gp1 - 30*npv + 16*gm1 - gm2)/(12*squared(f_g*s));

                            error = std::abs(gamma - numericalResults.gamma);
                            if (error > 1e-4 && t < 1000)
                                REPORT_FAILURE("gamma", \
                                    stdPayoff, stdExercise, s, q, r, today, v, \
                                    numericalResults.gamma, gamma, error, 5e-5);

                            const Real k2 = strike_up.NPV();
                            const Real k1 = strike_down.NPV();
                            numericalResults.strikeSensitivity = (k2 - k1)/(2*f_d*strike);
                            error = std::abs(strikeSensitivity - numericalResults.strikeSensitivity);

                            if (error > 5e-6)
                                REPORT_FAILURE("strikeSensitivity", \
                                    stdPayoff, stdExercise, s, q, r, today, v, \
                                    numericalResults.strikeSensitivity, strikeSensitivity, error, 5e-6);

                            if (q != 0.0) {
                                qRate->setValue(q + f_q);
                                const Real q2 = option.NPV();
                                qRate->setValue(q - f_q);
                                const Real q1 = option.NPV();
                                qRate->setValue(q);
                                numericalResults.dividendRho = (q2-q1)/(2*f_q);

                                error = std::abs(dividendRho - numericalResults.dividendRho);

                                if (error > 3e-2)
                                    REPORT_FAILURE("dividendRho", \
                                        stdPayoff, stdExercise, s, q, r, today, v, \
                                        numericalResults.dividendRho, dividendRho, error, 1e-3);

                                rRate->setValue(r + f_q);
                                const Real r2 = option.NPV();
                                rRate->setValue(r - f_q);
                                const Real r1 = option.NPV();
                                rRate->setValue(r);
                                numericalResults.rho = (r2 - r1)/(2*f_q);

                                error = std::abs(rho - numericalResults.rho);
                                if (error > 3e-2)
                                    REPORT_FAILURE("rho", \
                                        stdPayoff, stdExercise, s, q, r, today, v, \
                                        numericalResults.rho, rho, error, 1e-3);
                            }

                            vol->setValue(v + f_d);
                            const Real v2 = option.NPV();
                            vol->setValue(v - f_d);
                            const Real v1 = option.NPV();
                            vol->setValue(v);
                            numericalResults.vega = (v2 - v1)/(2*f_d);

                            error = std::abs(vega - numericalResults.vega);
                            if (error > 5e-4)
                                REPORT_FAILURE("vega", \
                                    stdPayoff, stdExercise, s, q, r, today, v, \
                                    numericalResults.vega, vega, error, 5e-4);

                            if (exerciseType == "American") {
                                const Real t2 = day_up.NPV();
                                const Real t1 = day_down.NPV();
                                numericalResults.thetaPerDay = (t1-t2)/2;
                                numericalResults.theta = 365*numericalResults.thetaPerDay;
                                error = std::abs(theta - numericalResults.theta);
                                const Real thetaTol = (t < 60) ? 3.0: 5e-4;
                                if (error > thetaTol) {
                                  REPORT_FAILURE("theta", \
                                      stdPayoff, stdExercise, s, q, r, today, v, \
                                      numericalResults.theta, theta, error, thetaTol);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testSingleBjerksundStenslandGreeks) {
    BOOST_TEST_MESSAGE("Testing a single Bjerksund-Stensland greeks set...");

    const Date today = Date(20, January, 2023);
    Settings::instance().evaluationDate() = today;

    const Real s = 100;
    const Volatility v = 0.3;
    const Rate q = 0.04;
    const Rate r = 0.07;

    const auto spot = ext::make_shared<SimpleQuote>(s);
    const auto vol = ext::make_shared<SimpleQuote>(v);

    const auto qRate = ext::make_shared<SimpleQuote>(q);
    const auto rRate = ext::make_shared<SimpleQuote>(r);

    const auto bsProcess = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(spot),
        Handle<YieldTermStructure>(flatRate(qRate, Actual365Fixed())),
        Handle<YieldTermStructure>(flatRate(rRate, Actual365Fixed())),
        Handle<BlackVolTermStructure>(
            flatVol(today, vol, Actual365Fixed()))
    );

    const Date maturityDate = today + Period(2, Years);

    const auto exercise
        = ext::make_shared<AmericanExercise>(today, maturityDate);
    const auto payoff
        = ext::make_shared<PlainVanillaPayoff>(Option::Call, 100);

    VanillaOption option(payoff, exercise);

    option.setPricingEngine(
        ext::make_shared<BjerksundStenslandApproximationEngine>(bsProcess)
    );

    const Real npv = option.NPV();
    const Real delta = option.delta();
    const Real gamma = option.gamma();
    const Real strikeSensitivity = option.strikeSensitivity();
    const Real divRho = option.dividendRho();
    const Real rho = option.rho();
    const Real vega = option.vega();
    const Real theta = option.theta();
    const Real thetaPerDay = option.thetaPerDay();
    const auto exerciseType = ext::any_cast<std::string>(
        option.additionalResults().find("exerciseType")->second);

    const Real expectedNpv = 17.9251834488399169;
    const Real expectedDelta = 0.590801845261082592;
    const Real expectedGamma = 0.00825347110063545664;
    const Real expectedStrikeSensitivity = -0.411550010772683383;
    const Real expectedDivRho = -114.137818682236826;
    const Real expectedRho = 80.4900013901554416;
    const Real expectedVega = 49.2906331545933227;
    const Real expectedTheta = -4.22540293840206704;

    const auto report = [=](Real value, Real expectedValue, const std::string& name) {
        constexpr double tol = 1e6*QL_EPSILON;
        const Real error = std::abs(value-expectedValue);
        if (error > tol)
            REPORT_FAILURE(name, \
                payoff, exercise, s, q, r, today, v, \
                value, expectedValue, error, tol);
    };

    report(npv, expectedNpv, "npv");
    report(delta, expectedDelta, "delta");
    report(gamma, expectedGamma, "gamma");
    report(strikeSensitivity, expectedStrikeSensitivity,
            "strikeSensitivity");
    report(divRho, expectedDivRho, "dividendRho");
    report(rho, expectedRho, "rho");
    report(vega, expectedVega, "vega");
    report(theta, expectedTheta, "theta");
    report(thetaPerDay, expectedTheta/365, "thetaPerDay");

    if (exerciseType != "American")
        BOOST_FAIL("American exercise type expected");
}

BOOST_AUTO_TEST_CASE(testFdEarliestExerciseDate) {
    BOOST_TEST_MESSAGE(
        "Testing that the FD engine respects the earliest date for American exercise...");

    // A deep ITM American put where early exercise is valuable.
    // Restricting the exercise window should reduce the price toward
    // the European value.

    const Date today(15, January, 2025);
    Settings::instance().evaluationDate() = today;
    DayCounter dc = Actual365Fixed();

    const Real S0 = 80.0;
    const Real K = 100.0;
    const Volatility sigma = 0.25;
    const Rate r = 0.05;
    const Rate q = 0.0;

    Handle<Quote> spot(ext::make_shared<SimpleQuote>(S0));
    Handle<YieldTermStructure> qTS(flatRate(today, q, dc));
    Handle<YieldTermStructure> rTS(flatRate(today, r, dc));
    Handle<BlackVolTermStructure> volTS(flatVol(today, sigma, dc));

    auto bsmProcess = ext::make_shared<BlackScholesMertonProcess>(
        spot, qTS, rTS, volTS);

    const Date maturity = today + Period(1, Years);
    auto payoff = ext::make_shared<PlainVanillaPayoff>(Option::Put, K);

    // Full American exercise
    auto fullExercise = ext::make_shared<AmericanExercise>(today, maturity);
    VanillaOption fullOption(payoff, fullExercise);
    auto fdEngine = ext::make_shared<FdBlackScholesVanillaEngine>(
        bsmProcess, 200, 200, 0);
    fullOption.setPricingEngine(fdEngine);
    const Real fullPrice = fullOption.NPV();

    // European benchmark
    auto euroExercise = ext::make_shared<EuropeanExercise>(maturity);
    VanillaOption euroOption(payoff, euroExercise);
    auto euroEngine = ext::make_shared<AnalyticEuropeanEngine>(bsmProcess);
    euroOption.setPricingEngine(euroEngine);
    const Real euroPrice = euroOption.NPV();

    const Real earlyExPremium = fullPrice - euroPrice;

    // Sanity: the early exercise premium should be significant
    // for this deep ITM put with 5% rates
    BOOST_CHECK(earlyExPremium > 1.0);

    // Restricted exercise: only last 3 months
    const Date lateStart = maturity - Period(3, Months);
    auto lateExercise = ext::make_shared<AmericanExercise>(lateStart, maturity);
    VanillaOption lateOption(payoff, lateExercise);
    lateOption.setPricingEngine(fdEngine);
    const Real latePrice = lateOption.NPV();

    // The restricted option should be worth less than full American
    BOOST_CHECK_MESSAGE(fullPrice - latePrice > 0.01,
        "Restricting exercise window should reduce price: "
        "full=" << fullPrice << " late=" << latePrice);

    // The restricted option should be worth more than European
    // (it still has some early exercise value in the last 3 months)
    BOOST_CHECK_MESSAGE(latePrice > euroPrice + 0.01,
        "Restricted American should exceed European: "
        "late=" << latePrice << " euro=" << euroPrice);

    // Monotonicity: longer exercise window -> higher price
    const Date midStart = maturity - Period(6, Months);
    auto midExercise = ext::make_shared<AmericanExercise>(midStart, maturity);
    VanillaOption midOption(payoff, midExercise);
    midOption.setPricingEngine(fdEngine);
    const Real midPrice = midOption.NPV();

    BOOST_CHECK_MESSAGE(midPrice >= latePrice - 1e-8,
        "Wider window should give higher price: "
        "6M=" << midPrice << " 3M=" << latePrice);
    BOOST_CHECK_MESSAGE(fullPrice >= midPrice - 1e-8,
        "Full window should give highest price: "
        "full=" << fullPrice << " 6M=" << midPrice);
}

BOOST_AUTO_TEST_CASE(testBaroneAdesiWhaleyNegativeRates) {

    BOOST_TEST_MESSAGE("Testing Barone-Adesi-Whaley engine with negative rates...");

    // Issue #1291: BAW crashes with a cryptic error when rates are negative.
    // Verify that it now throws a clear error message instead.

    Date today = Date::todaysDate();
    DayCounter dc = Actual360();

    auto spot = ext::make_shared<SimpleQuote>(36.0);
    auto qRate = ext::make_shared<SimpleQuote>(0.0);
    auto rRate = ext::make_shared<SimpleQuote>(-0.012);
    auto vol = ext::make_shared<SimpleQuote>(0.20);

    auto stochProcess = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(spot),
        Handle<YieldTermStructure>(flatRate(today, qRate, dc)),
        Handle<YieldTermStructure>(flatRate(today, rRate, dc)),
        Handle<BlackVolTermStructure>(flatVol(today, vol, dc)));

    auto payoff = ext::make_shared<PlainVanillaPayoff>(Option::Put, 40.0);
    Date exDate = today + Period(1, Years);
    auto exercise = ext::make_shared<AmericanExercise>(today, exDate);

    VanillaOption option(payoff, exercise);
    option.setPricingEngine(
        ext::make_shared<BaroneAdesiWhaleyApproximationEngine>(stochProcess));

    BOOST_CHECK_EXCEPTION(option.NPV(), Error,
                          ExpectedErrorMessage("negative interest rates"));

    // also verify with a call and positive dividends
    auto callPayoff = ext::make_shared<PlainVanillaPayoff>(Option::Call, 40.0);
    qRate->setValue(0.06);
    VanillaOption callOption(callPayoff, exercise);
    callOption.setPricingEngine(
        ext::make_shared<BaroneAdesiWhaleyApproximationEngine>(stochProcess));

    BOOST_CHECK_EXCEPTION(callOption.NPV(), Error,
                          ExpectedErrorMessage("negative interest rates"));
}

BOOST_AUTO_TEST_CASE(testQdAmericanGreeks) {
    BOOST_TEST_MESSAGE("Testing QD+ and QdFp American option greeks "
                       "against bump and revalue...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(1, June, 2022);
    Settings::instance().evaluationDate() = today;

    const auto spot = ext::make_shared<SimpleQuote>(100.0);
    const auto rRate = ext::make_shared<SimpleQuote>(0.05);
    const auto qRate = ext::make_shared<SimpleQuote>(0.03);
    const auto vol = ext::make_shared<SimpleQuote>(0.25);

    const auto bsProcess = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(spot), Handle<YieldTermStructure>(flatRate(today, qRate, dc)),
        Handle<YieldTermStructure>(flatRate(today, rRate, dc)),
        Handle<BlackVolTermStructure>(flatVol(today, vol, dc)));

    const Option::Type types[] = {Option::Put, Option::Call};
    const Real strikes[] = {80.0, 100.0, 120.0};
    const Real spots[] = {80.0, 100.0, 120.0};
    const Rate rates[] = {0.02, 0.08};
    const Rate divs[] = {0.0, 0.04, 0.10};
    const Volatility vols[] = {0.15, 0.35};
    const Integer maturities[] = {182, 365};

    const Real tolDelta = 7.0e-4;
    const Real tolGamma = 2.0e-4;

    // Vega, rho, dividendRho now use analytical boundary sensitivity.
    const Real tolVega = 7.0e-4;
    const Real tolRho = 7.0e-4;
    const Real tolDivRho = 7.0e-4;

    using EngineSpec = std::pair<std::string, ext::shared_ptr<PricingEngine>>;
    std::vector<EngineSpec> engines = {
        {"QdPlus",
         ext::make_shared<QdPlusAmericanEngine>(bsProcess, 8, QdPlusAmericanEngine::Halley, 1e-10)},
        {"QdFpAccurate",
         ext::make_shared<QdFpAmericanEngine>(bsProcess, QdFpAmericanEngine::accurateScheme())}};

    for (const auto& [engineName, engine] : engines) {
        // Both engines now compute analytical vega/rho/dividendRho
        const bool testParamGreeks = true;

        for (auto type : types) {
            for (Real K : strikes) {
                for (Integer mat : maturities) {
                    const Date matDate = today + Period(mat, Days);
                    const auto payoff = ext::make_shared<PlainVanillaPayoff>(type, K);
                    const auto exercise = ext::make_shared<AmericanExercise>(today, matDate);
                    VanillaOption option(payoff, exercise);
                    option.setPricingEngine(engine);

                    for (Real S : spots) {
                        for (Rate r : rates) {
                            for (Rate q : divs) {
                                for (Volatility v : vols) {
                                    spot->setValue(S);
                                    rRate->setValue(r);
                                    qRate->setValue(q);
                                    vol->setValue(v);

                                    const Real price = option.NPV();
                                    if (price < S * 1e-5)
                                        continue;

                                    const Real anDelta = option.delta();
                                    const Real anGamma = option.gamma();

                                    // Delta and Gamma via spot bump
                                    const Real du = S * 1e-4;
                                    spot->setValue(S + du);
                                    const Real pUp = option.NPV();
                                    const Real dUp = option.delta();
                                    spot->setValue(S - du);
                                    const Real pDn = option.NPV();
                                    const Real dDn = option.delta();
                                    spot->setValue(S);

                                    const Real fdDelta = (pUp - pDn) / (2 * du);
                                    const Real fdGamma = (dUp - dDn) / (2 * du);

                                    Real err = relativeError(fdDelta, anDelta, S);
                                    if (err > tolDelta) {
                                        BOOST_ERROR(engineName << " delta mismatch for " << type
                                                               << " K=" << K << " S=" << S
                                                               << " r=" << r << " q=" << q
                                                               << " v=" << v << " T=" << mat << "d"
                                                               << "\n    analytical: " << anDelta
                                                               << "\n    FD:         " << fdDelta
                                                               << "\n    rel error:  " << err);
                                    }

                                    err = relativeError(fdGamma, anGamma, S);
                                    if (err > tolGamma) {
                                        BOOST_ERROR(engineName << " gamma mismatch for " << type
                                                               << " K=" << K << " S=" << S
                                                               << " r=" << r << " q=" << q
                                                               << " v=" << v << " T=" << mat << "d"
                                                               << "\n    analytical: " << anGamma
                                                               << "\n    FD:         " << fdGamma
                                                               << "\n    rel error:  " << err);
                                    }

                                    if (!testParamGreeks)
                                        continue;

                                    const Real anVega = option.vega();
                                    const Real anRho = option.rho();
                                    const Real anDivRho = option.dividendRho();

                                    // (logging moved after FD computation below)

                                    // Vega via vol bump
                                    const Real dv = v * 1e-4;
                                    vol->setValue(v + dv);
                                    const Real pVolUp = option.NPV();
                                    vol->setValue(v - dv);
                                    const Real pVolDn = option.NPV();
                                    vol->setValue(v);

                                    const Real fdVega = (pVolUp - pVolDn) / (2 * dv);
                                    err = relativeError(fdVega, anVega, S);
                                    if (err > tolVega) {
                                        BOOST_ERROR(engineName << " vega mismatch for " << type
                                                               << " K=" << K << " S=" << S
                                                               << " r=" << r << " q=" << q
                                                               << " v=" << v << " T=" << mat << "d"
                                                               << "\n    analytical: " << anVega
                                                               << "\n    FD:         " << fdVega
                                                               << "\n    rel error:  " << err);
                                    }

                                    // Rho via rate bump
                                    const Real dr = r * 1e-4;
                                    rRate->setValue(r + dr);
                                    const Real pRUp = option.NPV();
                                    rRate->setValue(r - dr);
                                    const Real pRDn = option.NPV();
                                    rRate->setValue(r);

                                    const Real fdRho = (pRUp - pRDn) / (2 * dr);
                                    err = relativeError(fdRho, anRho, S);
                                    if (err > tolRho) {
                                        BOOST_ERROR(engineName << " rho mismatch for " << type
                                                               << " K=" << K << " S=" << S
                                                               << " r=" << r << " q=" << q
                                                               << " v=" << v << " T=" << mat << "d"
                                                               << "\n    analytical: " << anRho
                                                               << "\n    FD:         " << fdRho
                                                               << "\n    rel error:  " << err);
                                    }

                                    // DividendRho via dividend bump (skip q=0)
                                    Real fdDivRho = 0.0;
                                    if (q > QL_EPSILON) {
                                        const Real dq = q * 1e-4;
                                        qRate->setValue(q + dq);
                                        const Real pQUp = option.NPV();
                                        qRate->setValue(q - dq);
                                        const Real pQDn = option.NPV();
                                        qRate->setValue(q);

                                        fdDivRho = (pQUp - pQDn) / (2 * dq);
                                        err = relativeError(fdDivRho, anDivRho, S);
                                        if (err > tolDivRho) {
                                            BOOST_ERROR(engineName
                                                        << " dividendRho mismatch for " << type
                                                        << " K=" << K << " S=" << S << " r=" << r
                                                        << " q=" << q << " v=" << v << " T=" << mat
                                                        << "d"
                                                        << "\n    analytical: " << anDivRho
                                                        << "\n    FD:         " << fdDivRho
                                                        << "\n    rel error:  " << err);
                                        }
                                    }

                                    BOOST_TEST_MESSAGE(
                                        engineName
                                        << " " << type << " K=" << K << " S=" << S << " r=" << r
                                        << " q=" << q << " v=" << v << " T=" << mat << "d"
                                        << "\n  price=" << price << "\n  delta:  an=" << anDelta
                                        << "  fd=" << fdDelta << "\n  gamma:  an=" << anGamma
                                        << "  fd=" << fdGamma << "\n  vega:   an=" << anVega
                                        << "  fd=" << fdVega << "\n  rho:    an=" << anRho
                                        << "  fd=" << fdRho << "\n  divRho: an=" << anDivRho
                                        << "  fd=" << fdDivRho);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testQdDoubleBoundary) {
    BOOST_TEST_MESSAGE("Testing QD American engine double-boundary case (q<r<0)...");

    const Date today = Date(15, March, 2026);
    Settings::instance().evaluationDate() = today;

    // Parameters: q < r < 0
    const Real S = 100.0;
    const Real K = 100.0;
    const Rate r = -0.02;
    const Rate q = -0.05;
    const Volatility vol = 0.20;
    const Integer matDays = 365;
    const Date matDate = today + matDays;

    const auto spot = ext::make_shared<SimpleQuote>(S);
    const auto rRate = ext::make_shared<SimpleQuote>(r);
    const auto qRate = ext::make_shared<SimpleQuote>(q);
    const auto sigma = ext::make_shared<SimpleQuote>(vol);

    const auto rTS = flatRate(today, rRate, Actual365Fixed());
    const auto qTS = flatRate(today, qRate, Actual365Fixed());
    const auto volTS = flatVol(today, sigma, Actual365Fixed());

    const auto process = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(spot), Handle<YieldTermStructure>(qTS), Handle<YieldTermStructure>(rTS),
        Handle<BlackVolTermStructure>(volTS));

    const auto exercise = ext::make_shared<AmericanExercise>(matDate);
    const auto payoff = ext::make_shared<PlainVanillaPayoff>(Option::Put, K);
    VanillaOption option(payoff, exercise);

    // Test QdPlus
    option.setPricingEngine(ext::make_shared<QdPlusAmericanEngine>(process));
    const Real qdPlusPrice = option.NPV();
    BOOST_TEST_MESSAGE("  QdPlus price: " << qdPlusPrice);

    // Test QdFp (accurate)
    option.setPricingEngine(
        ext::make_shared<QdFpAmericanEngine>(process, QdFpAmericanEngine::accurateScheme()));
    const Real qdFpPrice = option.NPV();
    BOOST_TEST_MESSAGE("  QdFpAccurate price: " << qdFpPrice);

    // Sanity: price should be positive and reasonable
    BOOST_CHECK_MESSAGE(qdFpPrice > 0.0, "QdFp price should be positive, got " << qdFpPrice);
    BOOST_CHECK_MESSAGE(qdPlusPrice > 0.0, "QdPlus price should be positive, got " << qdPlusPrice);

    // Compare against FD engine
    option.setPricingEngine(ext::make_shared<FdBlackScholesVanillaEngine>(process, 200, 800));
    const Real fdPrice = option.NPV();
    BOOST_TEST_MESSAGE("  FD price: " << fdPrice);

    // QdFp and QdPlus should agree reasonably with FD
    Real diff = std::abs(qdFpPrice - fdPrice);
    BOOST_CHECK_MESSAGE(diff < 0.05,
                        "QdFp " << qdFpPrice << " vs FD " << fdPrice << " differ by " << diff);
    diff = std::abs(qdPlusPrice - fdPrice);
    BOOST_CHECK_MESSAGE(diff < 0.1,
                        "QdPlus " << qdPlusPrice << " vs FD " << fdPrice << " differ by " << diff);

    // Test with a call (r < q < 0, which maps to put double-boundary via parity)
    const auto callPayoff = ext::make_shared<PlainVanillaPayoff>(Option::Call, K);
    VanillaOption callOption(callPayoff, exercise);
    callOption.setPricingEngine(
        ext::make_shared<QdFpAmericanEngine>(process, QdFpAmericanEngine::accurateScheme()));
    const Real callPrice = callOption.NPV();
    BOOST_TEST_MESSAGE("  Call price (r<q<0): " << callPrice);
    BOOST_CHECK_MESSAGE(callPrice > 0.0, "Call price should be positive");

    // Reference prices from Andersen-Lake "Fast American Option Pricing:
    // The Double-Boundary Case", Table 3.
    // Parameters: K=100, r=-1.2%, q=-1.6%, sigma=10%, S=101
    // Benchmark computed with (m,n,l) = (64,128,257)
    {
        const Real S2 = 100.0;
        const Real K2 = 100.0;
        const Rate r2 = -0.012;
        const Rate q2 = -0.016;
        const Volatility vol2 = 0.10;

        const auto spot2 = ext::make_shared<SimpleQuote>(S2);
        const auto rRate2 = ext::make_shared<SimpleQuote>(r2);
        const auto qRate2 = ext::make_shared<SimpleQuote>(q2);
        const auto sigma2 = ext::make_shared<SimpleQuote>(vol2);

        const auto rTS2 = flatRate(today, rRate2, Actual365Fixed());
        const auto qTS2 = flatRate(today, qRate2, Actual365Fixed());
        const auto volTS2 = flatVol(today, sigma2, Actual365Fixed());

        const auto process2 = ext::make_shared<BlackScholesMertonProcess>(
            Handle<Quote>(spot2), Handle<YieldTermStructure>(qTS2),
            Handle<YieldTermStructure>(rTS2), Handle<BlackVolTermStructure>(volTS2));

        struct TestCase {
            Integer days;
            Real reference; // n=16 benchmark from Table 3
            Real fpTol;     // QdFp tolerance
            Real qpTol;     // QdPlus tolerance
        };
        const TestCase cases[] = {
            {45, 1.380533089, 1e-3, 0.01},
            {90, 1.942381237, 1e-3, 0.01},
            {180, 2.729267252, 1e-3, 0.01},
            {360, 3.830520425, 1e-3, 0.01},
            {3600, 12.189323541, 0.05, 0.1} // long-dated: harder case
        };

        for (const auto& tc : cases) {
            const Date mat2 = today + tc.days;
            const auto ex2 = ext::make_shared<AmericanExercise>(mat2);
            const auto po2 = ext::make_shared<PlainVanillaPayoff>(Option::Put, K2);
            VanillaOption opt2(po2, ex2);

            opt2.setPricingEngine(ext::make_shared<QdFpAmericanEngine>(
                process2, QdFpAmericanEngine::accurateScheme()));
            const Real fpPrice = opt2.NPV();

            opt2.setPricingEngine(ext::make_shared<QdPlusAmericanEngine>(process2));
            const Real qpPrice = opt2.NPV();

            const Real fpErr = std::abs(fpPrice - tc.reference);
            const Real qpErr = std::abs(qpPrice - tc.reference);

            BOOST_TEST_MESSAGE("  T=" << tc.days << "d: QdFp=" << fpPrice << " QdPlus=" << qpPrice
                                      << " ref=" << tc.reference << " fpErr=" << fpErr
                                      << " qpErr=" << qpErr);

            BOOST_CHECK_MESSAGE(fpErr < tc.fpTol, "QdFp T=" << tc.days << "d: " << fpPrice
                                                            << " vs ref " << tc.reference
                                                            << " diff " << fpErr);

            BOOST_CHECK_MESSAGE(qpErr < tc.qpTol, "QdPlus T=" << tc.days << "d: " << qpPrice
                                                              << " vs ref " << tc.reference
                                                              << " diff " << qpErr);
        }
    }
}

BOOST_AUTO_TEST_CASE(testQdDoubleBoundaryGreeks) {
    BOOST_TEST_MESSAGE("Testing QD American engine double-boundary greeks "
                       "against FD reference values...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(15, March, 2026);
    Settings::instance().evaluationDate() = today;

    const auto spot = ext::make_shared<SimpleQuote>(100.0);
    const auto rRate = ext::make_shared<SimpleQuote>(-0.02);
    const auto qRate = ext::make_shared<SimpleQuote>(-0.05);
    const auto vol = ext::make_shared<SimpleQuote>(0.20);

    const auto bsProcess = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(spot), Handle<YieldTermStructure>(flatRate(today, qRate, dc)),
        Handle<YieldTermStructure>(flatRate(today, rRate, dc)),
        Handle<BlackVolTermStructure>(flatVol(today, vol, dc)));

    struct RefCase {
        Option::Type type;
        Real K, S;
        Rate r, q;
        Volatility v;
        Integer mat;
        // FD reference values (from FdBlackScholesVanillaEngine 6400x25600)
        Real price, delta, gamma, vega, rho, divRho;
    };

    // --- Generate FD reference values ---
    // Uncomment the block below to regenerate reference values with the FD
    // pricer.  The output is in C++ struct-initializer syntax that can be
    // pasted directly into the refCases array.
    //
    // const auto fdEngine = ext::make_shared<FdBlackScholesVanillaEngine>(
    //     bsProcess, 6400, 25600);
    if (false) {
        const auto fdEngine = ext::make_shared<FdBlackScholesVanillaEngine>(bsProcess, 6400, 25600);

        struct GenCase {
            Option::Type type;
            Real K, S;
            Rate r, q;
            Volatility v;
            Integer mat;
        };
        const GenCase gcases[] = {
            // Double-boundary puts (q < r < 0) — 1y
            {Option::Put, 100, 100, -0.02, -0.03, 0.25, 365},
            {Option::Put, 100, 100, -0.02, -0.03, 0.10, 365},
            {Option::Put, 100, 100, -0.02, -0.03, 0.40, 365},
            {Option::Put, 100, 110, -0.02, -0.03, 0.25, 365},
            {Option::Put, 100, 90, -0.02, -0.03, 0.25, 365},
            {Option::Put, 100, 90, -0.02, -0.03, 0.40, 365},
            {Option::Put, 100, 100, -0.02, -0.05, 0.25, 365},
            {Option::Put, 100, 100, -0.02, -0.05, 0.10, 365},
            {Option::Put, 100, 110, -0.02, -0.05, 0.25, 365},
            {Option::Put, 100, 90, -0.02, -0.05, 0.25, 365},
            {Option::Put, 100, 100, -0.01, -0.05, 0.25, 365},
            {Option::Put, 100, 100, -0.01, -0.05, 0.10, 365},
            // Double-boundary puts — 6m
            {Option::Put, 100, 100, -0.02, -0.03, 0.25, 182},
            {Option::Put, 100, 110, -0.02, -0.03, 0.25, 182},
            {Option::Put, 100, 90, -0.02, -0.03, 0.25, 182},
            {Option::Put, 100, 100, -0.02, -0.05, 0.25, 182},
            // Double-boundary puts — 3m
            {Option::Put, 100, 100, -0.02, -0.03, 0.25, 91},
            {Option::Put, 100, 110, -0.02, -0.03, 0.25, 91},
            {Option::Put, 100, 100, -0.02, -0.05, 0.25, 91},
            // Double-boundary puts — 1m
            {Option::Put, 100, 100, -0.02, -0.03, 0.25, 30},
            {Option::Put, 100, 100, -0.02, -0.05, 0.25, 30},
            // Double-boundary calls (r < q < 0) — 1y
            {Option::Call, 100, 100, -0.03, -0.02, 0.25, 365},
            {Option::Call, 100, 100, -0.03, -0.02, 0.10, 365},
            {Option::Call, 100, 100, -0.03, -0.02, 0.40, 365},
            {Option::Call, 100, 90, -0.03, -0.02, 0.25, 365},
            {Option::Call, 100, 110, -0.03, -0.02, 0.25, 365},
            {Option::Call, 100, 100, -0.05, -0.02, 0.25, 365},
            {Option::Call, 100, 100, -0.05, -0.01, 0.25, 365},
            // Double-boundary calls — 6m, 3m
            {Option::Call, 100, 100, -0.03, -0.02, 0.25, 182},
            {Option::Call, 100, 110, -0.03, -0.02, 0.25, 182},
            {Option::Call, 100, 100, -0.03, -0.02, 0.25, 91},
            {Option::Call, 100, 100, -0.05, -0.02, 0.25, 91},
            // Single-boundary puts (r > 0) — 1y
            {Option::Put, 100, 100, 0.05, 0.02, 0.25, 365},
            {Option::Put, 100, 110, 0.05, 0.02, 0.25, 365},
            {Option::Put, 100, 90, 0.05, 0.02, 0.25, 365},
            {Option::Put, 100, 100, 0.05, 0.02, 0.10, 365},
            {Option::Put, 100, 100, 0.05, 0.00, 0.25, 365},
            // Single-boundary puts — shorter dated
            {Option::Put, 100, 100, 0.05, 0.02, 0.25, 182},
            {Option::Put, 100, 100, 0.05, 0.02, 0.25, 91},
            // Single-boundary calls (q > 0) — 1y
            {Option::Call, 100, 100, 0.02, 0.05, 0.25, 365},
            {Option::Call, 100, 90, 0.02, 0.05, 0.25, 365},
            {Option::Call, 100, 110, 0.02, 0.05, 0.25, 365},
            {Option::Call, 100, 100, 0.02, 0.05, 0.10, 365},
            // Single-boundary calls — shorter dated
            {Option::Call, 100, 100, 0.02, 0.05, 0.25, 182},
            {Option::Call, 100, 100, 0.02, 0.05, 0.25, 91},
            // Immediate exercise: deep ITM puts with low vol in double-boundary
            // S between Y(T) and B(T), option at intrinsic
            {Option::Put, 110, 90, -0.02, -0.03, 0.10, 365},  // intrinsic=20
            {Option::Put, 120, 90, -0.02, -0.03, 0.10, 365},  // intrinsic=30
            {Option::Put, 110, 100, -0.02, -0.03, 0.10, 182}, // intrinsic=10
            // Immediate exercise: deep ITM calls with low vol in double-boundary
            {Option::Call, 90, 110, -0.03, -0.02, 0.10, 365}, // intrinsic=20
            {Option::Call, 80, 110, -0.03, -0.02, 0.10, 365}, // intrinsic=30
            {Option::Call, 90, 100, -0.03, -0.02, 0.10, 182}, // intrinsic=10
        };

        for (const auto& gc : gcases) {
            spot->setValue(gc.S);
            rRate->setValue(gc.r);
            qRate->setValue(gc.q);
            vol->setValue(gc.v);
            const Date matDate = today + Period(gc.mat, Days);
            VanillaOption opt(ext::make_shared<PlainVanillaPayoff>(gc.type, gc.K),
                              ext::make_shared<AmericanExercise>(today, matDate));
            opt.setPricingEngine(fdEngine);
            const Real fdP = opt.NPV();
            const Real fdDelta = opt.delta();

            // Gamma via forward differences to avoid straddling exercise boundary kink
            const Real h = gc.S * 5e-3;
            spot->setValue(gc.S + h);
            const Real fdP1 = opt.NPV();
            spot->setValue(gc.S + 2 * h);
            const Real fdP2 = opt.NPV();
            spot->setValue(gc.S);
            const Real fdGamma = (fdP2 - 2 * fdP1 + fdP) / (h * h);

            // vega, rho, divRho via bump-and-revalue (FD engine doesn't provide them)
            const Real dv = gc.v * 5e-5;
            vol->setValue(gc.v + dv);
            const Real fdVup = opt.NPV();
            vol->setValue(gc.v - dv);
            const Real fdVdn = opt.NPV();
            vol->setValue(gc.v);
            const Real fdVega = (fdVup - fdVdn) / (2 * dv);

            const Real dr = std::max(std::abs(gc.r), 0.01) * 5e-5;
            rRate->setValue(gc.r + dr);
            const Real fdRup = opt.NPV();
            rRate->setValue(gc.r - dr);
            const Real fdRdn = opt.NPV();
            rRate->setValue(gc.r);
            const Real fdRho = (fdRup - fdRdn) / (2 * dr);

            const Real dq = std::max(std::abs(gc.q), 0.01) * 5e-5;
            qRate->setValue(gc.q + dq);
            const Real fdQup = opt.NPV();
            qRate->setValue(gc.q - dq);
            const Real fdQdn = opt.NPV();
            qRate->setValue(gc.q);
            const Real fdDivRho = (fdQup - fdQdn) / (2 * dq);


            const char* typeStr = (gc.type == Option::Put) ? "Option::Put" : "Option::Call";
            std::cerr << std::setprecision(15) << "{" << typeStr << ", " << gc.K << ", " << gc.S
                      << ", " << gc.r << ", " << gc.q << ", " << gc.v << ", " << gc.mat << ", "
                      << fdP << ", " << fdDelta << ", " << fdGamma << ", " << fdVega << ", "
                      << fdRho << ", " << fdDivRho << "},\n";
        }
    }

    // --- QdFp profile sweep for analysis/plotting ---
    // Produces CSV on stderr: spot sweep and vol sweep for interesting cases.
    // Enable by changing if(false) to if(true).
    if (false) {
        const auto sweepEngine =
            ext::make_shared<QdFpAmericanEngine>(bsProcess, QdFpAmericanEngine::accurateScheme());

        struct SweepCase {
            const char* label;
            Option::Type type;
            Real K;
            Rate r, q;
            Volatility v;
            Integer mat;
        };
        const SweepCase sweepCases[] = {
            // Double-boundary put, narrow gap
            {"dbl_put_narrow_1y", Option::Put, 100, -0.02, -0.03, 0.25, 365},
            // Double-boundary put, wide gap
            {"dbl_put_wide_1y", Option::Put, 100, -0.01, -0.05, 0.25, 365},
            // Double-boundary put, low vol
            {"dbl_put_lowvol_1y", Option::Put, 100, -0.02, -0.03, 0.10, 365},
            // Double-boundary call, narrow gap
            {"dbl_call_narrow_1y", Option::Call, 100, -0.03, -0.02, 0.25, 365},
            // Single-boundary put (standard)
            {"sgl_put_1y", Option::Put, 100, 0.05, 0.02, 0.25, 365},
            // Single-boundary call (standard)
            {"sgl_call_1y", Option::Call, 100, 0.02, 0.05, 0.25, 365},
            // Double-boundary put, short dated
            {"dbl_put_narrow_3m", Option::Put, 100, -0.02, -0.03, 0.25, 91},
        };

        // Spot sweep: S from 70 to 130
        std::cerr << "SPOT_SWEEP\n";
        std::cerr << "label,type,K,r,q,v,T,S,price,delta,gamma,vega,rho,divRho\n";
        for (const auto& sc : sweepCases) {
            rRate->setValue(sc.r);
            qRate->setValue(sc.q);
            vol->setValue(sc.v);
            const Date matDate = today + Period(sc.mat, Days);
            VanillaOption opt(ext::make_shared<PlainVanillaPayoff>(sc.type, sc.K),
                              ext::make_shared<AmericanExercise>(today, matDate));
            opt.setPricingEngine(sweepEngine);

            for (Real S = 70.0; S <= 130.01; S += 0.5) {
                spot->setValue(S);
                std::cerr << std::setprecision(10) << sc.label << ","
                          << (sc.type == Option::Put ? "Put" : "Call") << "," << sc.K << "," << sc.r
                          << "," << sc.q << "," << sc.v << "," << sc.mat << "," << S << ","
                          << opt.NPV() << "," << opt.delta() << "," << opt.gamma() << ","
                          << opt.vega() << "," << opt.rho() << "," << opt.dividendRho() << "\n";
            }
        }

        // Vol sweep: vol from 0.05 to 0.60, at S=100 (ATM)
        std::cerr << "VOL_SWEEP\n";
        std::cerr << "label,type,K,r,q,S,T,v,price,delta,gamma,vega,rho,divRho\n";
        spot->setValue(100.0);
        for (const auto& sc : sweepCases) {
            rRate->setValue(sc.r);
            qRate->setValue(sc.q);
            const Date matDate = today + Period(sc.mat, Days);
            VanillaOption opt(ext::make_shared<PlainVanillaPayoff>(sc.type, sc.K),
                              ext::make_shared<AmericanExercise>(today, matDate));
            opt.setPricingEngine(sweepEngine);

            for (Real v = 0.05; v <= 0.601; v += 0.01) {
                vol->setValue(v);
                std::cerr << std::setprecision(10) << sc.label << ","
                          << (sc.type == Option::Put ? "Put" : "Call") << "," << sc.K << "," << sc.r
                          << "," << sc.q << "," << 100.0 << "," << sc.mat << "," << v << ","
                          << opt.NPV() << "," << opt.delta() << "," << opt.gamma() << ","
                          << opt.vega() << "," << opt.rho() << "," << opt.dividendRho() << "\n";
            }
        }
    }

    // Reference values from FdBlackScholesVanillaEngine(6400, 25600)
    // Greeks computed via bump-and-revalue with relative bump 5e-5;
    // gamma via forward differences with 1% bump
    const RefCase refCases[] = {
        // Double-boundary puts (q < r < 0) — 1y
        {Option::Put, 100, 100, -0.02, -0.03, 0.25, 365, 9.70634871891378, -0.449149920684487,
         0.0161117554440651, 40.4325420979035, -49.7305992688268, 41.1211151464623},
        {Option::Put, 100, 100, -0.02, -0.03, 0.10, 365, 3.64590769414805, -0.46308197469402,
         0.0410550676300847, 40.250315494017, -39.5038269058734, 36.9498067658292},
        {Option::Put, 100, 100, -0.02, -0.03, 0.40, 365, 15.7486419445511, -0.424784208456321,
         0.00996385375447773, 40.0588527441403, -56.7279489578354, 41.3104383121995},
        {Option::Put, 100, 110, -0.02, -0.03, 0.25, 365, 5.98553175979762, -0.301688245147016,
         0.0126795572790804, 38.8968635008524, -36.2320249660541, 30.8933657583073},
        {Option::Put, 100, 90, -0.02, -0.03, 0.25, 365, 15.0334199706047, -0.619782929211281,
         0.0176593155826081, 35.6077802050692, -63.9089959948436, 50.5343370290253},
        {Option::Put, 100, 90, -0.02, -0.03, 0.40, 365, 20.5097856412248, -0.531021556463035,
         0.0113563975412735, 36.9452440279971, -66.7936644536127, 46.6584266689551},
        {Option::Put, 100, 100, -0.02, -0.05, 0.25, 365, 8.96769519525504, -0.431009456915774,
         0.0165866833616732, 40.178739903709, -40.5972343031635, 34.4303783752054},
        {Option::Put, 100, 100, -0.02, -0.05, 0.10, 365, 3.00584702665286, -0.42862370829589,
         0.0455937834869307, 38.6256212425273, -29.4683204513646, 27.8122341020648},
        {Option::Put, 100, 110, -0.02, -0.05, 0.25, 365, 5.41728344849056, -0.284121602335388,
         0.0125959168280032, 37.9000228735649, -30.7832070149949, 26.6704784220195},
        {Option::Put, 100, 90, -0.02, -0.05, 0.25, 365, 14.1786059865793, -0.613485117553759,
         0.0189719251641075, 35.4535320184368, -46.7986304437673, 38.5648085995172},
        {Option::Put, 100, 100, -0.01, -0.05, 0.25, 365, 8.5775244406978, -0.422766145564709,
         0.0169284840289876, 39.6985105058434, -37.6001141546567, 32.0620328562171},
        {Option::Put, 100, 100, -0.01, -0.05, 0.10, 365, 2.73086268374839, -0.413720610385531,
         0.0486380443455197, 37.2728131330824, -25.6433523100341, 24.2883307733699},
        // Double-boundary puts — 6m
        {Option::Put, 100, 100, -0.02, -0.03, 0.25, 182, 6.88245013988051, -0.459571993884531,
         0.0226277275072029, 28.3276039322899, -23.1574223983344, 20.3778610785577},
        {Option::Put, 100, 110, -0.02, -0.03, 0.25, 182, 3.32714702710883, -0.260035361689148,
         0.0164402079842633, 25.3490088232766, -14.6094502806449, 13.1810888879258},
        {Option::Put, 100, 90, -0.02, -0.03, 0.25, 182, 12.6665171459094, -0.695549959414845,
         0.0228772144891679, 22.7242583986254, -30.460479361949, 25.7265516806863},
        {Option::Put, 100, 100, -0.02, -0.05, 0.25, 182, 6.49974274749274, -0.448196661623547,
         0.0231099790799085, 28.2493668472839, -20.4849502725146, 18.1828823528463},
        // Double-boundary puts — 3m
        {Option::Put, 100, 100, -0.02, -0.03, 0.25, 91, 4.89041800156482, -0.469903537558339,
         0.0318721565007216, 19.971779320862, -11.3850989476028, 10.4051044903149},
        {Option::Put, 100, 110, -0.02, -0.03, 0.25, 91, 1.63032027762322, -0.200570952600447,
         0.0197055661359183, 15.4418621646535, -5.5134074191665, 5.15616616122057},
        {Option::Put, 100, 100, -0.02, -0.05, 0.25, 91, 4.6918617119121, -0.461969567992806,
         0.0323093103669798, 19.9382117095226, -10.3902374521958, 9.53693902854269},
        // Double-boundary puts — 1m
        {Option::Put, 100, 100, -0.02, -0.03, 0.25, 30, 2.82642347938117, -0.482150015914476,
         0.0552272105490808, 11.4463475614812, -3.73157326682971, 3.54337625048847},
        {Option::Put, 100, 100, -0.02, -0.05, 0.25, 30, 2.75782216780659, -0.475353754281258,
         0.0555678417457734, 11.4374173679721, -3.50907097046438, 3.33810339538942},
        // Double-boundary calls (r < q < 0) — 1y
        {Option::Call, 100, 100, -0.03, -0.02, 0.25, 365, 9.70634873011048, 0.546213233339622,
         0.0161360481654143, 40.4325433551378, 41.1229147404176, -49.7288653420469},
        {Option::Call, 100, 100, -0.03, -0.02, 0.10, 365, 3.64590769656091, 0.499541011015061,
         0.0415272830826794, 40.2503160227496, 36.9505014381488, -39.5032025357622},
        {Option::Call, 100, 100, -0.03, -0.02, 0.40, 365, 15.7486420140371, 0.582270513785861,
         0.00996997076028805, 40.0588648903799, 41.3132181685446, -56.7249055425378},
        {Option::Call, 100, 90, -0.03, -0.02, 0.25, 365, 5.09237180265451, 0.376139931211938,
         0.0171300729859175, 34.5692357397098, 26.8134783384018, -31.3576761667988},
        {Option::Call, 100, 110, -0.03, -0.02, 0.25, 365, 15.9250060291346, 0.693617651239593,
         0.0130709368607326, 39.9527729126703, 54.7552232556351, -68.9079981306406},
        {Option::Call, 100, 100, -0.05, -0.02, 0.25, 365, 8.96769519428094, 0.520686297698953,
         0.0166762734441974, 40.1787414146071, 34.4307732671467, -40.5957890068365},
        {Option::Call, 100, 100, -0.05, -0.01, 0.25, 365, 8.57752442660377, 0.508541274188651,
         0.0170692991195764, 39.6985270182881, 32.0625275094244, -37.5892976371972},
        // Double-boundary calls — 6m, 3m
        {Option::Call, 100, 100, -0.03, -0.02, 0.25, 182, 6.8824501339981, 0.528396395405936,
         0.0226615696132306, 28.3275948875783, 20.3814311374728, -23.1530688106218},
        {Option::Call, 100, 110, -0.03, -0.02, 0.25, 182, 13.2479893568667, 0.733849103769163,
         0.0169356378709736, 25.9307127365815, 27.9541693301392, -32.9286216311786},
        {Option::Call, 100, 100, -0.03, -0.02, 0.25, 91, 4.89041800481184, 0.518807645661719,
         0.0319237737504672, 19.9717855024417, 10.4030494224953, -11.385761693905},
        {Option::Call, 100, 100, -0.05, -0.02, 0.25, 91, 4.69186171081853, 0.508888162273453,
         0.0324987619469468, 19.938211594237, 9.53599736206456, -10.3910036473032},
        // Single-boundary puts (r > 0) — 1y
        {Option::Put, 100, 100, 0.05, 0.02, 0.25, 365, 8.56516754564992, -0.417498564515722,
         0.016621053064668, 38.1258099609028, -36.3853448671847, 30.9993555491772},
        {Option::Put, 100, 110, 0.05, 0.02, 0.25, 365, 5.1448369208686, -0.272220115245049,
         0.012285114453521, 35.9828294574172, -28.1767442595893, 24.4831557409952},
        {Option::Put, 100, 90, 0.05, 0.02, 0.25, 365, 13.6591033384705, -0.605536555936136,
         0.0201218681751941, 33.043132469075, -39.6804399354522, 32.9460442154428},
        {Option::Put, 100, 100, 0.05, 0.02, 0.10, 365, 2.89533078318739, -0.419876864564183,
         0.0462348707087603, 36.8729507430832, -27.2285744687295, 25.7224514195187},
        {Option::Put, 100, 100, 0.05, 0.00, 0.25, 365, 7.97439342909439, -0.40864507977181,
         0.0174773907560066, 37.8333182794321, -32.8126366699877, 28.1869264124879},
        // Single-boundary puts — shorter dated
        {Option::Put, 100, 100, 0.05, 0.02, 0.25, 182, 6.3451006584151, -0.440268624461285,
         0.0230569802255474, 27.4990108782092, -19.2733765683073, 17.1454777162516},
        {Option::Put, 100, 100, 0.05, 0.02, 0.25, 91, 4.63340476090454, -0.457492685608646,
         0.0322213244793979, 19.6654315759659, -10.0445785371051, 9.2263260000891},
        // Single-boundary calls (q > 0) — 1y
        {Option::Call, 100, 100, 0.02, 0.05, 0.25, 365, 8.56516754429367, 0.503150128720948,
         0.0167753673117801, 38.1258160767572, 30.9981308550533, -36.3857261145739},
        {Option::Call, 100, 90, 0.02, 0.05, 0.25, 365, 4.36500082455977, 0.335802795618739,
         0.0165962925640462, 31.8913772857243, 21.3099466472855, -24.4792960376827},
        {Option::Call, 100, 110, 0.02, 0.05, 0.25, 365, 14.4286567619336, 0.665022320946486,
         0.0148917716733521, 37.2554478762765, 36.4196035107511, -43.7517875827353},
        {Option::Call, 100, 100, 0.02, 0.05, 0.10, 365, 2.89533078212857, 0.448830143007674,
         0.0489020992523734, 36.8729543108071, 25.7220759454224, -27.228765929177},
        // Single-boundary calls — shorter dated
        {Option::Call, 100, 100, 0.02, 0.05, 0.25, 182, 6.34510064461839, 0.503719590713749,
         0.0232471939003638, 27.4990190098379, 17.1415625969118, -19.2736572133256},
        {Option::Call, 100, 100, 0.02, 0.05, 0.25, 91, 4.6334047598945, 0.503826710409155,
         0.0324637591295414, 19.665433094822, 9.22713331830138, -10.0436372770574},
        // Immediate exercise — double-boundary puts
        {Option::Put, 110, 90, -0.02, -0.03, 0.10, 365, 20.0, -1.0, 0.0, 0.0, 0.0, 0.0},
        {Option::Put, 120, 90, -0.02, -0.03, 0.10, 365, 30.0, -1.0, 0.0, 0.0, 0.0, 0.0},
        {Option::Put, 110, 100, -0.02, -0.03, 0.10, 182, 10.1090129265466, -0.927921861900638,
         0.0305232774434501, 9.63850817026213, -18.0582480115987, 16.6383846718077},
        // Immediate exercise — double-boundary calls
        {Option::Call, 90, 110, -0.03, -0.02, 0.10, 365, 20.0, 1.0, 0.0, 0.0, 0.0, 0.0},
        {Option::Call, 80, 110, -0.03, -0.02, 0.10, 365, 30.0, 1.0, 0.0, 0.0, 0.0, 0.0},
        {Option::Call, 90, 100, -0.03, -0.02, 0.10, 182, 10.0457853012681, 0.959112950493978,
         0.0192937600670717, 5.99007261197215, 11.5121098674583, -12.5514394717641},
    };

    const auto fpEngine =
        ext::make_shared<QdFpAmericanEngine>(bsProcess, QdFpAmericanEngine::accurateScheme());

    // Tolerances: price and greeks vs FD reference
    // FD reference: 6400x25600 grid, forward-difference gamma (1% bump),
    // central-difference vega/rho/divRho (5e-5 relative bump)
    const Real tolPrice = 5.0e-3;
    const Real tolDelta = 2.0e-2;
    const Real tolGamma = 5.0e-2;
    const Real tolVega = 0.5;
    const Real tolRho = 1.0;
    const Real tolDivRho = 1.0;

    for (const auto& rc : refCases) {
        if (rc.price == 0.0)
            continue; // skip placeholder

        spot->setValue(rc.S);
        rRate->setValue(rc.r);
        qRate->setValue(rc.q);
        vol->setValue(rc.v);
        const Date matDate = today + Period(rc.mat, Days);
        VanillaOption opt(ext::make_shared<PlainVanillaPayoff>(rc.type, rc.K),
                          ext::make_shared<AmericanExercise>(today, matDate));
        opt.setPricingEngine(fpEngine);

        const Real price = opt.NPV();
        const Real delta = opt.delta();
        const Real gamma = opt.gamma();
        const Real vega = opt.vega();
        const Real rho = opt.rho();
        const Real divRho = opt.dividendRho();

        const char* typeStr = (rc.type == Option::Put) ? "Put" : "Call";

        BOOST_TEST_MESSAGE(
            std::fixed << std::setprecision(6) << typeStr << " K=" << rc.K << " S=" << rc.S
                       << " r=" << rc.r << " q=" << rc.q << " v=" << rc.v << " T=" << rc.mat << "d"
                       << "\n             " << std::setw(14) << "QdFp" << "  " << std::setw(14)
                       << "FD ref" << "  " << std::setw(12) << "diff"
                       << "\n  price:   " << std::setw(14) << price << "  " << std::setw(14)
                       << rc.price << "  " << std::setw(12) << (price - rc.price) << "\n  delta:   "
                       << std::setw(14) << delta << "  " << std::setw(14) << rc.delta << "  "
                       << std::setw(12) << (delta - rc.delta) << "\n  gamma:   " << std::setw(14)
                       << gamma << "  " << std::setw(14) << rc.gamma << "  " << std::setw(12)
                       << (gamma - rc.gamma) << "\n  vega:    " << std::setw(14) << vega << "  "
                       << std::setw(14) << rc.vega << "  " << std::setw(12) << (vega - rc.vega)
                       << "\n  rho:     " << std::setw(14) << rho << "  " << std::setw(14) << rc.rho
                       << "  " << std::setw(12) << (rho - rc.rho)
                       << "\n  divRho:  " << std::setw(14) << divRho << "  " << std::setw(14)
                       << rc.divRho << "  " << std::setw(12) << (divRho - rc.divRho));

        if (std::abs(price - rc.price) > tolPrice) {
            BOOST_ERROR(typeStr << " K=" << rc.K << " S=" << rc.S << " r=" << rc.r << " q=" << rc.q
                                << " v=" << rc.v << " T=" << rc.mat << "d: price " << price
                                << " vs ref " << rc.price << " diff=" << (price - rc.price));
        }
        if (std::abs(delta - rc.delta) > tolDelta) {
            BOOST_ERROR(typeStr << " K=" << rc.K << " S=" << rc.S << " r=" << rc.r << " q=" << rc.q
                                << " v=" << rc.v << " T=" << rc.mat << "d: delta " << delta
                                << " vs ref " << rc.delta << " diff=" << (delta - rc.delta));
        }
        if (std::abs(gamma - rc.gamma) > tolGamma) {
            BOOST_ERROR(typeStr << " K=" << rc.K << " S=" << rc.S << " r=" << rc.r << " q=" << rc.q
                                << " v=" << rc.v << " T=" << rc.mat << "d: gamma " << gamma
                                << " vs ref " << rc.gamma << " diff=" << (gamma - rc.gamma));
        }
        if (std::abs(vega - rc.vega) > tolVega) {
            BOOST_ERROR(typeStr << " K=" << rc.K << " S=" << rc.S << " r=" << rc.r << " q=" << rc.q
                                << " v=" << rc.v << " T=" << rc.mat << "d: vega " << vega
                                << " vs ref " << rc.vega << " diff=" << (vega - rc.vega));
        }
        if (std::abs(rho - rc.rho) > tolRho) {
            BOOST_ERROR(typeStr << " K=" << rc.K << " S=" << rc.S << " r=" << rc.r << " q=" << rc.q
                                << " v=" << rc.v << " T=" << rc.mat << "d: rho " << rho
                                << " vs ref " << rc.rho << " diff=" << (rho - rc.rho));
        }
        if (std::abs(divRho - rc.divRho) > tolDivRho) {
            BOOST_ERROR(typeStr << " K=" << rc.K << " S=" << rc.S << " r=" << rc.r << " q=" << rc.q
                                << " v=" << rc.v << " T=" << rc.mat << "d: divRho " << divRho
                                << " vs ref " << rc.divRho << " diff=" << (divRho - rc.divRho));
        }
    }

    // --- QdPlus engine comparison (informational, no assertions) ---
    const auto plusEngine = ext::make_shared<QdPlusAmericanEngine>(bsProcess);

    BOOST_TEST_MESSAGE("\n=== QdPlus vs FD reference ===");
    for (const auto& rc : refCases) {
        if (rc.price == 0.0)
            continue;

        spot->setValue(rc.S);
        rRate->setValue(rc.r);
        qRate->setValue(rc.q);
        vol->setValue(rc.v);
        const Date matDate = today + Period(rc.mat, Days);
        VanillaOption opt(ext::make_shared<PlainVanillaPayoff>(rc.type, rc.K),
                          ext::make_shared<AmericanExercise>(today, matDate));
        opt.setPricingEngine(plusEngine);

        const Real price = opt.NPV();
        const Real delta = opt.delta();
        const Real gamma = opt.gamma();
        const Real vega = opt.vega();
        const Real rho = opt.rho();
        const Real divRho = opt.dividendRho();

        const char* typeStr = (rc.type == Option::Put) ? "Put" : "Call";

        BOOST_TEST_MESSAGE(
            std::fixed << std::setprecision(6) << typeStr << " K=" << rc.K << " S=" << rc.S
                       << " r=" << rc.r << " q=" << rc.q << " v=" << rc.v << " T=" << rc.mat << "d"
                       << "\n             " << std::setw(14) << "QdPlus" << "  " << std::setw(14)
                       << "FD ref" << "  " << std::setw(12) << "diff"
                       << "\n  price:   " << std::setw(14) << price << "  " << std::setw(14)
                       << rc.price << "  " << std::setw(12) << (price - rc.price) << "\n  delta:   "
                       << std::setw(14) << delta << "  " << std::setw(14) << rc.delta << "  "
                       << std::setw(12) << (delta - rc.delta) << "\n  gamma:   " << std::setw(14)
                       << gamma << "  " << std::setw(14) << rc.gamma << "  " << std::setw(12)
                       << (gamma - rc.gamma) << "\n  vega:    " << std::setw(14) << vega << "  "
                       << std::setw(14) << rc.vega << "  " << std::setw(12) << (vega - rc.vega)
                       << "\n  rho:     " << std::setw(14) << rho << "  " << std::setw(14) << rc.rho
                       << "  " << std::setw(12) << (rho - rc.rho)
                       << "\n  divRho:  " << std::setw(14) << divRho << "  " << std::setw(14)
                       << rc.divRho << "  " << std::setw(12) << (divRho - rc.divRho));
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
