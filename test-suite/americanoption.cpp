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

    const auto dc = Actual365Fixed();
    const auto today = Date(8, April, 2022);
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

        if (diff > 0.001) {
            BOOST_FAIL("failed to reproduce American Call/Put parity"
                    << "\n    Put NPV   : " << putNpv
                    << "\n    Call NPV  : " << callNpv
                    << "\n    difference: " << diff
                    << "\n    tolerance : " << tol);
        }
    }
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

    if (speed <= Fast) {
        suite->add(QUANTLIB_TEST_CASE(&AmericanOptionTest::testFdShoutGreeks));
    }

    return suite;
}

