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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "americanoption.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/integrals/integral.hpp>
#include <ql/math/statistics/incrementalstatistics.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/baroneadesiwhaleyengine.hpp>
#include <ql/pricingengines/vanilla/bjerksundstenslandengine.hpp>
#include <ql/pricingengines/vanilla/juquadraticengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesshoutengine.hpp>
#include <ql/pricingengines/vanilla/qdfpamericanengine.hpp>
#include <ql/pricingengines/vanilla/qdplusamericanengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>

#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#undef REPORT_FAILURE
#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, \
                       v, expected, calculated, error, tolerance) \
    BOOST_ERROR(exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               <<"    spot value:        " << s << "\n" \
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

namespace {

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

}


void AmericanOptionTest::testBaroneAdesiWhaleyValues() {

    BOOST_TEST_MESSAGE("Testing Barone-Adesi and Whaley approximation "
                       "for American options...");

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


void AmericanOptionTest::testBjerksundStenslandValues() {

    BOOST_TEST_MESSAGE("Testing Bjerksund and Stensland approximation "
                       "for American options...");

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
      { Option::Put, 100, 110, 0.05, 0.05, 1.0, 10, 94.89543 }
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

namespace {

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

}


void AmericanOptionTest::testJuValues() {

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


void AmericanOptionTest::testFdValues() {

    BOOST_TEST_MESSAGE("Testing finite-difference and QR+ engine "
                       "for American options...");

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


namespace {

    template <class Engine>
    void testFdGreeks() {

        SavedSettings backup;

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
                                    // FLOATING_POINT_EXCEPTION
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

}


void AmericanOptionTest::testFdAmericanGreeks() {
    BOOST_TEST_MESSAGE("Testing finite-differences American option greeks...");
    testFdGreeks<FdBlackScholesVanillaEngine>();
}

void AmericanOptionTest::testFdShoutGreeks() {
    BOOST_TEST_MESSAGE("Testing finite-differences shout option greeks...");
    testFdGreeks<FdBlackScholesShoutEngine>();
}

void AmericanOptionTest::testFDShoutNPV() {
    BOOST_TEST_MESSAGE("Testing finite-differences shout option pricing...");

    SavedSettings backup;

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

void AmericanOptionTest::testZeroVolFDShoutNPV() {
    BOOST_TEST_MESSAGE("Testing zero volatility shout option pricing"
                       " with discrete dividends...");

    SavedSettings backup;

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

   DividendVanillaOption option(
       ext::make_shared<PlainVanillaPayoff>(Option::Put, 100.0),
       ext::make_shared<AmericanExercise>(today, maturityDate),
       std::vector<Date>{dividendDate},
       std::vector<Real>{10.0}
   );

   option.setPricingEngine(
       ext::make_shared<FdBlackScholesVanillaEngine>(process, 50, 50));

   const Real americanNPV = option.NPV();

   option.setPricingEngine(
       ext::make_shared<FdBlackScholesShoutEngine>(process, 50, 50));

   const Real shoutNPV = option.NPV();
   const DiscountFactor df = r->discount(maturityDate)/r->discount(dividendDate);

   const Real tol = 1e-3;
   const Real diff = std::fabs(americanNPV - shoutNPV/df);

   if (diff > tol) {
       BOOST_FAIL("failed to reproduce American option NPV with "
               "Shout option pricing engine for "
               << "\n    calculated: " << shoutNPV/df
               << "\n    expected  : " << americanNPV
               << "\n    difference: " << diff
               << "\n    tolerance:  " << tol);
   }
}

void AmericanOptionTest::testLargeDividendShoutNPV() {
    BOOST_TEST_MESSAGE("Testing zero strike shout option pricing"
                       " with discrete dividends...");

    SavedSettings backup;

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

   const Real strike = 80.0;
   DividendVanillaOption divOption(
       ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
       ext::make_shared<AmericanExercise>(today, maturityDate),
       std::vector<Date>{dividendDate},
       std::vector<Real>{divAmount}
   );

   divOption.setPricingEngine(
       ext::make_shared<FdBlackScholesShoutEngine>(process, 100, 400));

   const Real calculated = divOption.NPV();

   VanillaOption option(
       ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
       ext::make_shared<AmericanExercise>(today, dividendDate)
   );

   option.setPricingEngine(
       ext::make_shared<FdBlackScholesShoutEngine>(process, 100, 400));

   const Real expected = option.NPV()
       * r->discount(maturityDate) / r->discount(dividendDate);

   const Real tol = 5e-2;
   const Real diff = std::fabs(expected - calculated);

   if (diff > tol) {
       BOOST_FAIL("failed to reproduce American option NPV with "
               "Shout option pricing engine for "
               << "\n    calculated: " << calculated
               << "\n    expected  : " << expected
               << "\n    difference: " << diff
               << "\n    tolerance:  " << tol);
   }
}

void AmericanOptionTest::testEscrowedVsSpotAmericanOption() {
    BOOST_TEST_MESSAGE("Testing escrowed vs spot dividend model "
            "for American options...");

    SavedSettings backup;

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
   const Date dividendDate = today + Period(10, Months);
   const Real divAmount = 10.0;

   const Real strike = 100.0;
   DividendVanillaOption option(
       ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
       ext::make_shared<AmericanExercise>(today, maturityDate),
       std::vector<Date>{dividendDate},
       std::vector<Real>{divAmount}
   );

   option.setPricingEngine(
       ext::make_shared<FdBlackScholesVanillaEngine>(process, 100, 400));

   const Real spotNpv = option.NPV();
   const Real spotDelta = option.delta();

   vol->setValue(100/90.*0.3);

   option.setPricingEngine(
       MakeFdBlackScholesVanillaEngine(process)
           .withTGrid(100)
           .withXGrid(400)
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


void AmericanOptionTest::testTodayIsDividendDate() {
    BOOST_TEST_MESSAGE("Testing escrowed vs spot dividend model"
            " on dividend dates for American options...");

    SavedSettings backup;

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
    const Date divDate1 = today;
    const Date divDate2 = today + Period(11, Months);
    const Real divAmount = 5.0;

    const auto spotEngine =
        ext::make_shared<FdBlackScholesVanillaEngine>(process, 100, 400);

    const auto escrowedEngine =
        MakeFdBlackScholesVanillaEngine(process)
            .withTGrid(100)
            .withXGrid(400)
            .withCashDividendModel(FdBlackScholesVanillaEngine::Escrowed);

    const Real strike = 90.0;
    DividendVanillaOption option(
        ext::make_shared<PlainVanillaPayoff>(Option::Put, strike),
        ext::make_shared<AmericanExercise>(today, maturityDate),
        std::vector<Date>{divDate1, divDate2},
        std::vector<Real>{divAmount, divAmount}
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
    Real tol = 5e-2;

    if (diffNpv > tol) {
        BOOST_FAIL("failed to compare American option NPV with "
                "escrowed and spot dividend model "
                << "\n    escrowed div: " << escrowedNpv
                << "\n    spot div    : " << spotNpv
                << "\n    difference: " << diffNpv
                << "\n    tolerance:  " << tol);
    }

    const Real diffDelta = std::abs(escrowedDelta - spotDelta);

    tol = 1e-3;
    if (diffDelta > tol) {
        BOOST_FAIL("failed to compare American option Delta with "
                "escrowed and spot dividend model "
                << "\n    escrowed div: " << escrowedDelta
                << "\n    spot div    : " << spotDelta
                << "\n    difference: " << diffDelta
                << "\n    tolerance:  " << tol);
    }

    DividendVanillaOption optionTomorrow(
        ext::make_shared<PlainVanillaPayoff>(Option::Put, strike),
        ext::make_shared<AmericanExercise>(today, maturityDate),
        std::vector<Date>{today + Period(1, Days), divDate2},
        std::vector<Real>{divAmount, divAmount}
    );

    vol->setValue(0.3);

    optionTomorrow.setPricingEngine(spotEngine);
    spotNpv = optionTomorrow.NPV();

    vol->setValue(100/95.0*0.3);
    optionTomorrow.setPricingEngine(escrowedEngine);

    escrowedNpv = optionTomorrow.NPV();
    BOOST_CHECK_NO_THROW(optionTomorrow.theta());

    diffNpv = std::abs(escrowedNpv - spotNpv);
    tol = 5e-2;

    if (diffNpv > tol) {
        BOOST_FAIL("failed to compare American option NPV with "
                "escrowed and spot dividend model "
                << "\n    escrowed div: " << escrowedNpv
                << "\n    spot div    : " << spotNpv
                << "\n    difference: " << diffNpv
                << "\n    tolerance:  " << tol);
    }
}


void AmericanOptionTest::testCallPutParity() {
    BOOST_TEST_MESSAGE("Testing Call/Put parity for American options...");

    // R.L. McDonald, M.D. Schroder: A parity result for American option

    SavedSettings backup;

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
            BOOST_FAIL("failed to reproduce American Call/Put parity"
                    << "\n    Put NPV   : " << putNpv
                    << "\n    Call NPV  : " << callNpv
                    << "\n    difference: " << diff
                    << "\n    tolerance : " << tol);
        }
    }
}

void AmericanOptionTest::testQdPlusBoundaryValues() {
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

void AmericanOptionTest::testQdPlusBoundaryConvergence() {
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
        for (auto solverType: solverTypes) {
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

void AmericanOptionTest::testQdAmericanEngines() {
    BOOST_TEST_MESSAGE("Testing QD+ American Option pricing...");

    SavedSettings backup;

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
    const Real pde_values[] = {
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
    testCaseSpecs.reserve(LENGTH(pde_values) + LENGTH(edgeTestCases));

    PseudoRandom::rng_type rng(PseudoRandom::urng_type(12345UL));

    for (Size i=0; i < LENGTH(pde_values); ++i) {
        const Option::Type optionType
            = (rng.next().value > 0)? Option::Call : Option::Put;
        const Real spot = 100*std::exp(1.5*rng.next().value);
        const Real strike = 100*std::exp(1.5*rng.next().value);
        const Size maturityInDays = Size(1 + 365*std::exp(2*rng.next().value));
        const Volatility  vol = 0.5*std::exp(rng.next().value);
        const Rate r = 0.10*std::exp(rng.next().value);
        const Rate q = 0.10*std::exp(rng.next().value);

        const OptionSpec spec = {
            optionType, spot, strike, maturityInDays,
            vol, r, q, pde_values[i], -1
        };

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

void AmericanOptionTest::testQdFpIterationScheme() {
    BOOST_TEST_MESSAGE("Testing Legendre and tanh-sinh iteration "
                       "scheme for QD+ Fixed Point American engine...");

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

        BOOST_CHECK_SMALL(scheme->getFixedPointIntegrator()
            ->operator()(nd, -10.0, 10.0) - 1.0, tol);
        BOOST_CHECK_SMALL(scheme->getExerciseBoundaryToPriceIntegrator()
            ->operator()(nd, -10.0, 10.0) - 1.0, tol);
    }
}


void AmericanOptionTest::testAndersenLakeHighPrecisionExample() {
    BOOST_TEST_MESSAGE("Testing Andersen, Lake and Offengenden "
                        "high precision example...");

    SavedSettings backup;

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

        for (Size i=0; i < LENGTH(schemes); ++i) {

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


void AmericanOptionTest::testQdEngineStandardExample() {
    BOOST_TEST_MESSAGE("Testing Andersen, Lake and Offengenden "
                        "standard example...");

    SavedSettings backup;

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

    for (Size i=0; i < LENGTH(schemes); ++i) {
        americanOption.setPricingEngine(
            ext::make_shared<QdFpAmericanEngine>(
                bsProcess,
                ext::make_shared<QdFpLegendreScheme>(32, 2, 15, 48),
                schemes[i])
        );
        const Real calculated = americanOption.NPV() - europeanOption.NPV();

        const Real tol = 1e-15;
        const Real diff = std::abs(calculated - expected[i]);

        if (diff > tol) {
            BOOST_ERROR("failed to reproduce high precision test values"
                    << "\n    diff     : " << diff
                    << "\n    tol      : " << tol);
        }
    }
}

void AmericanOptionTest::testBulkQdFpAmericanEngine() {
    BOOST_TEST_MESSAGE("Testing Andersen, Lake and Offengenden "
                        "bulk examples...");

    // Examples are taken from
    //    Leif Andersen, Mark Lake and Dimitri Offengenden (2015)
    //    "High Performance American Option Pricing",
    //    https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2547027

    SavedSettings backup;

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
     const Real S[] = {25, 50, 90, 100, 110, 150, 200};
     const Volatility sig[] = {0.1, 0.2, 0.4, 0.6};

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

    IncrementalStatistics stats;
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

                        const Real diff = std::abs(fast-accurate);
                        stats.add(diff);
                    }
                }
            }
        }
    }


    const Real tolStdDev = 1e-4;
    if (stats.standardDeviation() > tolStdDev)
        BOOST_ERROR("failed to reproduce low RMSE with fast American engine"
                << "\n    RMSE diff: " << stats.standardDeviation()
                << "\n    tol      : " << tolStdDev);

    const Real tolMax = 2.5e-3;
    if (stats.max() > tolMax)
        BOOST_ERROR("failed to reproduce low max deviation "
                "with fast American engine"
                << "\n    max diff: " << stats.max()
                << "\n    tol     : " << tolMax);
}


test_suite* AmericanOptionTest::suite(SpeedLevel speed) {
    auto* suite = BOOST_TEST_SUITE("American option tests");

    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testBaroneAdesiWhaleyValues));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testBjerksundStenslandValues));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testJuValues));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testFdValues));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testFdAmericanGreeks));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testFDShoutNPV));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testZeroVolFDShoutNPV));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testLargeDividendShoutNPV));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testEscrowedVsSpotAmericanOption));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testTodayIsDividendDate));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testCallPutParity));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testQdPlusBoundaryValues));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testQdPlusBoundaryConvergence));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testQdAmericanEngines));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testQdFpIterationScheme));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testAndersenLakeHighPrecisionExample));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testQdEngineStandardExample));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testBulkQdFpAmericanEngine));

    if (speed <= Fast) {
        suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testFdShoutGreeks));
    }

    return suite;
}

