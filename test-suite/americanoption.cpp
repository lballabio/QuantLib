
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "americanoption.hpp"
#include "utilities.hpp"
#include <ql/DayCounters/actual360.hpp>
#include <ql/Instruments/vanillaoption.hpp>
#include <ql/PricingEngines/Vanilla/baroneadesiwhaleyengine.hpp>
#include <ql/PricingEngines/Vanilla/bjerksundstenslandengine.hpp>
#include <ql/PricingEngines/Vanilla/juquadraticengine.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, \
                       v, expected, calculated, error, tolerance) \
    BOOST_FAIL(exerciseTypeToString(exercise) << " " \
               << OptionTypeFormatter::toString(payoff->optionType()) \
               << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               <<"    spot value:        " << s << "\n" \
               << "    strike:           " << payoff->strike() << "\n" \
               << "    dividend yield:   " \
               << RateFormatter::toString(q) << "\n" \
               << "    risk-free rate:   " \
               << RateFormatter::toString(r) << "\n" \
               << "    reference date:   " \
               << DateFormatter::toString(today) << "\n" \
               << "    maturity:         " \
               << DateFormatter::toString(exercise->lastDate()) << "\n" \
               << "    volatility:       " \
               << VolatilityFormatter::toString(v) << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
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
        Real tol;      // tolerance
    };

}

void AmericanOptionTest::testBaroneAdesiWhaleyValues() {

    BOOST_MESSAGE("Testing Barone-Adesi and Whaley approximation "
                  "for American options...");

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
       pag 24

       The following values were replicated only up to the second digit
       by the VB code provided by Haug, which was used as base for the
       C++ implementation

    */
    AmericanOptionData values[] = {
      //        type, strike,   spot,    q,    r,    t,  vol,   value, tol
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.15,  0.0206, 1e-2 },
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.15,  1.8771, 1e-2 },
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.15, 10.0089, 1e-2 },
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.25,  0.3159, 1e-2 },
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.25,  3.1280, 1e-2 },
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.25, 10.3919, 1e-2 },
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.35,  0.9495, 1e-2 },
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.35,  4.3777, 1e-2 },
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.35, 11.1679, 1e-2 },
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.15,  0.8208, 1e-2 },
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.15,  4.0842, 1e-2 },
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.15, 10.8087, 1e-2 },
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.25,  2.7437, 1e-2 },
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.25,  6.8015, 1e-2 },
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.25, 13.0170, 1e-2 },
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.35,  5.0063, 1e-2 },
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.35,  9.5106, 1e-2 },
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.35, 15.5689, 1e-2 },
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.15, 10.0000, 1e-2 },
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.15,  1.8770, 1e-2 },
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.15,  0.0410, 1e-2 },
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.25, 10.2533, 1e-2 },
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.25,  3.1277, 1e-2 },
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.25,  0.4562, 1e-2 },
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.35, 10.8787, 1e-2 },
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.35,  4.3777, 1e-2 },
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.35,  1.2402, 1e-2 },
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.15, 10.5595, 1e-2 },
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.15,  4.0842, 1e-2 },
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.15,  1.0822, 1e-2 },
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.25, 12.4419, 1e-2 },
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.25,  6.8014, 1e-2 },
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.25,  3.3226, 1e-2 },
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.35, 14.6945, 1e-2 },
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.35,  9.5104, 1e-2 },
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.35,  5.8823, 1e-2 }
    };

    Date today = Date::todaysDate();
    DayCounter dc = Actual360();
    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);
    boost::shared_ptr<PricingEngine> engine(
                                    new BaroneAdesiWhaleyApproximationEngine);

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        Date exDate = today + Integer(values[i].t*360+0.5);
        boost::shared_ptr<Exercise> exercise(
                                         new AmericanExercise(today, exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<BlackScholesProcess> stochProcess(new
            BlackScholesProcess(Handle<Quote>(spot),
                                Handle<YieldTermStructure>(qTS),
                                Handle<YieldTermStructure>(rTS),
                                Handle<BlackVolTermStructure>(volTS)));

        VanillaOption option(stochProcess, payoff, exercise,
                             engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated-values[i].result);
        if (error > values[i].tol) {
            REPORT_FAILURE("value", payoff, exercise, values[i].s, values[i].q,
                           values[i].r, today, values[i].v, values[i].result,
                           calculated, error, values[i].tol);
        }
    }

}


void AmericanOptionTest::testBjerksundStenslandValues() {

    BOOST_MESSAGE("Testing Bjerksund and Stensland approximation "
                  "for American options...");

    AmericanOptionData values[] = {
        //      type, strike,   spot,    q,    r,    t,  vol,   value, tol
        // from "Option pricing formulas", Haug, McGraw-Hill 1998, pag 27
      { Option::Call,  40.00,  42.00, 0.08, 0.04, 0.75, 0.35,  5.2704, 1e-4 },
        // from "Option pricing formulas", Haug, McGraw-Hill 1998, VBA code
      { Option::Put,   40.00,  36.00, 0.00, 0.06, 1.00, 0.20,  4.4531, 1e-4 }
    };

    Date today = Date::todaysDate();
    DayCounter dc = Actual360();
    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);
    boost::shared_ptr<PricingEngine> engine(
                                   new BjerksundStenslandApproximationEngine);

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        Date exDate = today + Integer(values[i].t*360+0.5);
        boost::shared_ptr<Exercise> exercise(
                                         new AmericanExercise(today, exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<BlackScholesProcess> stochProcess(new
            BlackScholesProcess(Handle<Quote>(spot),
                                Handle<YieldTermStructure>(qTS),
                                Handle<YieldTermStructure>(rTS),
                                Handle<BlackVolTermStructure>(volTS)));

        VanillaOption option(stochProcess, payoff, exercise,
                             engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated-values[i].result);
        if (error > values[i].tol) {
            REPORT_FAILURE("value", payoff, exercise, values[i].s, values[i].q,
                           values[i].r, today, values[i].v, values[i].result,
                           calculated, error, values[i].tol);
        }
    }

}

void AmericanOptionTest::testJuValues() {

    BOOST_MESSAGE("Testing Ju approximation for American options...");

    /* The data below are from
        An Approximate Formula for Pricing American Options
        Journal of Derivatives Winter 1999
        Ju, N.
    */
    AmericanOptionData values[] = {
      //        type, strike,   spot,    q,    r,    t,     vol,   value, tol
      // These values are from Exhibit 3 - Short dated Put Options
      { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.0833,  0.2,  0.006, 1e-3 },
      { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.3333,  0.2,  0.201, 1e-3 },
      { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.5833,  0.2,  0.433, 1e-3 },

      { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.0833,  0.2,  0.851, 1e-3 },
      { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.3333,  0.2,  1.576, 1e-3 },
      { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.5833,  0.2,  1.984, 1e-3 },

      { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.0833,  0.2,  5.000, 1e-3 },
      { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.3333,  0.2,  5.084, 1e-3 },
      { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.5833,  0.2,  5.260, 1e-3 },

      { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.0833,  0.3,  0.078, 1e-3 },
      { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.3333,  0.3,  0.697, 1e-3 },
      { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.5833,  0.3,  1.218, 1e-3 },

      { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.0833,  0.3,  1.309, 1e-3 },
      { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.3333,  0.3,  2.477, 1e-3 },
      { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.5833,  0.3,  3.161, 1e-3 },

      { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.0833,  0.3,  5.059, 1e-3 },
      { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.3333,  0.3,  5.699, 1e-3 },
      { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.5833,  0.3,  6.231, 1e-3 },

      { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.0833,  0.4,  0.247, 1e-3 },
      { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.3333,  0.4,  1.344, 1e-3 },
      { Option::Put, 35.00,   40.00,  0.0,  0.0488, 0.5833,  0.4,  2.150, 1e-3 },

      { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.0833,  0.4,  1.767, 1e-3 },
      { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.3333,  0.4,  3.381, 1e-3 },
      { Option::Put, 40.00,   40.00,  0.0,  0.0488, 0.5833,  0.4,  4.342, 1e-3 },

      { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.0833,  0.4,  5.288, 1e-3 },
      { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.3333,  0.4,  6.501, 1e-3 },
      { Option::Put, 45.00,   40.00,  0.0,  0.0488, 0.5833,  0.4,  7.367, 1e-3 },

      // Type in Exhibits 4 and 5 if you have some spare time ;-)

      //        type, strike,   spot,    q,    r,    t,     vol,   value, tol
      // These values are from Exhibit 6 - Long dated Call Options with dividends
      { Option::Call, 100.00,   80.00,  0.07,  0.03, 3.0,  0.2,  2.605, 1e-3 },
      { Option::Call, 100.00,   90.00,  0.07,  0.03, 3.0,  0.2,  5.182, 1e-3 },
      { Option::Call, 100.00,   100.00,  0.07,  0.03, 3.0,  0.2,  9.065, 1e-3 },
      { Option::Call, 100.00,   110.00,  0.07,  0.03, 3.0,  0.2,  14.430, 1e-3 },
      { Option::Call, 100.00,   120.00,  0.07,  0.03, 3.0,  0.2,  21.398, 1e-3 },

      { Option::Call, 100.00,   80.00,  0.07,  0.03, 3.0,  0.4,  11.336, 1e-3 },
      { Option::Call, 100.00,   90.00,  0.07,  0.03, 3.0,  0.4,  15.711, 1e-3 },
      { Option::Call, 100.00,   100.00,  0.07,  0.03, 3.0,  0.4,  20.760, 1e-3 },
      { Option::Call, 100.00,   110.00,  0.07,  0.03, 3.0,  0.4,  26.440, 1e-3 },
      { Option::Call, 100.00,   120.00,  0.07,  0.03, 3.0,  0.4,  32.709, 1e-3 },

      { Option::Call, 100.00,   80.00,  0.07,  0.0, 3.0,  0.3,  5.552, 1e-3 },
      { Option::Call, 100.00,   90.00,  0.07,  0.0, 3.0,  0.3,  8.868, 1e-3 },
      { Option::Call, 100.00,   100.00,  0.07,  0.0, 3.0,  0.3,  13.158, 1e-3 },
      { Option::Call, 100.00,   110.00,  0.07,  0.0, 3.0,  0.3,  18.458, 1e-3 },
      { Option::Call, 100.00,   120.00,  0.07,  0.0, 3.0,  0.3,  24.786, 1e-3 },

      { Option::Call, 100.00,   80.00,  0.03,  0.07, 3.0,  0.3,  12.177, 1e-3 },
      { Option::Call, 100.00,   90.00,  0.03,  0.07, 3.0,  0.3,  17.411, 1e-3 },
      { Option::Call, 100.00,   100.00,  0.03,  0.07, 3.0,  0.3,  23.402, 1e-3 },
      { Option::Call, 100.00,   110.00,  0.03,  0.07, 3.0,  0.3,  30.028, 1e-3 },
      { Option::Call, 100.00,   120.00,  0.03,  0.07, 3.0,  0.3,  37.177, 1e-3 }
    };

    Date today = Date::todaysDate();
    DayCounter dc = Actual360();
    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);
    boost::shared_ptr<PricingEngine> engine(
                                    new JuQuadraticApproximationEngine);

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        Date exDate = today + Integer(values[i].t*360+0.5);
        boost::shared_ptr<Exercise> exercise(
                                         new AmericanExercise(today, exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<BlackScholesProcess> stochProcess(new
            BlackScholesProcess(Handle<Quote>(spot),
                                Handle<YieldTermStructure>(qTS),
                                Handle<YieldTermStructure>(rTS),
                                Handle<BlackVolTermStructure>(volTS)));

        VanillaOption option(stochProcess, payoff, exercise,
                             engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated-values[i].result);
        if (error > values[i].tol) {
            REPORT_FAILURE("value", payoff, exercise, values[i].s, values[i].q,
                           values[i].r, today, values[i].v, values[i].result,
                           calculated, error, values[i].tol);
        }
    }

}


test_suite* AmericanOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("American option tests");
    suite->add(
          BOOST_TEST_CASE(&AmericanOptionTest::testBaroneAdesiWhaleyValues));
    suite->add(
          BOOST_TEST_CASE(&AmericanOptionTest::testBjerksundStenslandValues));
    suite->add(
          BOOST_TEST_CASE(&AmericanOptionTest::testJuValues));
    return suite;
}

