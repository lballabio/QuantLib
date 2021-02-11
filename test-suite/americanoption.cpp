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
#include <ql/pricingengines/vanilla/baroneadesiwhaleyengine.hpp>
#include <ql/pricingengines/vanilla/bjerksundstenslandengine.hpp>
#include <ql/pricingengines/vanilla/juquadraticengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesshoutengine.hpp>
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

    BOOST_TEST_MESSAGE("Testing finite-difference engine "
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

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess =
            ext::make_shared<BlackScholesMertonProcess>(Handle<Quote>(spot),
                                                        Handle<YieldTermStructure>(qTS),
                                                        Handle<YieldTermStructure>(rTS),
                                                        Handle<BlackVolTermStructure>(volTS));

        ext::shared_ptr<PricingEngine> engine =
            ext::make_shared<FdBlackScholesVanillaEngine>(stochProcess, 100, 100);

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
            for (double strike : strikes) {
                for (int year : years) {
                    Date exDate = today + year * Years;
                    ext::shared_ptr<Exercise> exercise(new AmericanExercise(today, exDate));
                    ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));
                    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                        new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

                    ext::shared_ptr<PricingEngine> engine(new Engine(stochProcess, 50));

                    VanillaOption option(payoff, exercise);
                    option.setPricingEngine(engine);

                    for (double u : underlyings) {
                        for (double m : qRates) {
                            for (double n : rRates) {
                                for (double v : vols) {
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

test_suite* AmericanOptionTest::suite() {
    auto* suite = BOOST_TEST_SUITE("American option tests");
    suite->add(
        QUANTLIB_TEST_CASE(&AmericanOptionTest::testBaroneAdesiWhaleyValues));
    suite->add(
        QUANTLIB_TEST_CASE(&AmericanOptionTest::testBjerksundStenslandValues));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testJuValues));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testFdValues));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testFdAmericanGreeks));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testFdShoutGreeks));
    suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testFDShoutNPV));
    return suite;
}

