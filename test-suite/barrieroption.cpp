
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 RiskMap srl

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

#include "barrieroption.hpp"
#include "utilities.hpp"
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/DayCounters/simpledaycounter.hpp>
#include <ql/Instruments/barrieroption.hpp>
#include <ql/PricingEngines/Barrier/analyticbarrierengine.hpp>
#include <ql/PricingEngines/Barrier/mcbarrierengine.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, barrierType, barrier, rebate, payoff, \
                       exercise, s, q, r, today, v, expected, calculated, \
                       error, tolerance) \
    BOOST_FAIL(barrierTypeToString(barrierType) + " " + \
               exerciseTypeToString(exercise) + " " \
               + OptionTypeFormatter::toString(payoff->optionType()) + \
               " option with " \
               + payoffTypeToString(payoff) + " payoff:\n" \
               "    underlying value: " \
               + DoubleFormatter::toString(s) + "\n" \
               "    strike:           " \
               + DoubleFormatter::toString(payoff->strike()) +"\n" \
               "    barrier:          " \
               + DoubleFormatter::toString(barrier) +"\n" \
               "    rebate:           " \
               + DoubleFormatter::toString(rebate) +"\n" \
               "    dividend yield:   " \
               + DoubleFormatter::toString(q) + "\n" \
               "    risk-free rate:   " \
               + DoubleFormatter::toString(r) + "\n" \
               "    reference date:   " \
               + DateFormatter::toString(today) + "\n" \
               "    maturity:         " \
               + DateFormatter::toString(exercise->lastDate()) + "\n" \
               "    volatility:       " \
               + DoubleFormatter::toString(v) + "\n\n" \
               "    expected   " + greekName + ": " \
               + DoubleFormatter::toString(expected) + "\n" \
               "    calculated " + greekName + ": " \
               + DoubleFormatter::toString(calculated) + "\n" \
               "    error:            " \
               + DoubleFormatter::toString(error) + "\n" \
               + (tolerance==Null<double>() ? std::string("") : \
                  "    tolerance:        " \
                  + DoubleFormatter::toString(tolerance)));

namespace {

    std::string barrierTypeToString(Barrier::Type type) {

        switch(type){
        case Barrier::DownIn:
            return std::string("Down-and-in");
        case Barrier::UpIn:
            return std::string("Up-and-in");
        case Barrier::DownOut:
            return std::string("Down-and-out");
        case Barrier::UpOut:
            return std::string("Up-and-out");
        default:
            QL_FAIL("exerciseTypeToString : unknown exercise type");
        }
    }

    struct BarrierOptionData {
        Barrier::Type type;
        double volatility;
        double strike;
        double barrier;
        double callValue;
        double putValue;
    };

    struct NewBarrierOptionData {
        Barrier::Type barrierType;
        double barrier;
        double rebate;
        Option::Type type;
        double strike;
        double s;      // spot
        double q;      // dividend
        double r;      // risk-free rate
        Time t;        // time to maturity
        double v;      // volatility
        double result; // result
        double tol;    // tolerance
    };

}

void BarrierOptionTest::testHaugValues() {

    BOOST_MESSAGE("Testing barrier options against Haug's values...");

    NewBarrierOptionData values[] = {
        /* The data below are from
          "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 pag. 72
        */
        //     barrierType, barrier, rebate,         type, strike,     s,    q,    r,    t,    v,  result, tol
        { Barrier::DownOut,    95.0,    3.0, Option::Call,     90, 100.0, 0.04, 0.08, 0.50, 0.25,  9.0246, 1.0e-4},
        { Barrier::DownOut,    95.0,    3.0, Option::Call,    100, 100.0, 0.04, 0.08, 0.50, 0.25,  6.7924, 1.0e-4},
        { Barrier::DownOut,    95.0,    3.0, Option::Call,    110, 100.0, 0.04, 0.08, 0.50, 0.25,  4.8759, 1.0e-4},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,     90, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,    100, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,    110, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,     90, 100.0, 0.04, 0.08, 0.50, 0.25,  2.6789, 1.0e-4},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,    100, 100.0, 0.04, 0.08, 0.50, 0.25,  2.3580, 1.0e-4},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,    110, 100.0, 0.04, 0.08, 0.50, 0.25,  2.3453, 1.0e-4},

        { Barrier::DownIn,     95.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.25,  7.7627, 1.0e-4},
        { Barrier::DownIn,     95.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.25,  4.0109, 1.0e-4},
        { Barrier::DownIn,     95.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.25,  2.0576, 1.0e-4},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.25, 13.8333, 1.0e-4},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.25,  7.8494, 1.0e-4},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.25,  3.9795, 1.0e-4},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.25, 14.1112, 1.0e-4},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.25,  8.4482, 1.0e-4},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.25,  4.5910, 1.0e-4},

        { Barrier::DownOut,    95.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30,  8.8334, 1.0e-4},
        { Barrier::DownOut,    95.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30,  7.0285, 1.0e-4},
        { Barrier::DownOut,    95.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.30,  5.4137, 1.0e-4},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30,  3.0000, 1.0e-4},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30,  3.0000, 1.0e-4},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.30,  3.0000, 1.0e-4},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30,  2.6341, 1.0e-4},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30,  2.4389, 1.0e-4},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.30,  2.4315, 1.0e-4},

        { Barrier::DownIn,     95.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30,  9.0093, 1.0e-4},
        { Barrier::DownIn,     95.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30,  5.1370, 1.0e-4},
        { Barrier::DownIn,     95.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.30,  2.8517, 1.0e-4},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30, 14.8816, 1.0e-4},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30,  9.2045, 1.0e-4},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.30,  5.3043, 1.0e-4},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30, 15.2098, 1.0e-4},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30,  9.7278, 1.0e-4},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.30,  5.8350, 1.0e-4},



        { Barrier::DownOut,    95.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25,  2.2798, 1.0e-4 },
        { Barrier::DownOut,    95.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25,  2.2947, 1.0e-4 },
        { Barrier::DownOut,    95.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25,  2.6252, 1.0e-4 },
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4 },
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4 },
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4 },
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25,  3.7760, 1.0e-4 },
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25,  5.4932, 1.0e-4 },
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25,  7.5187, 1.0e-4 },

        { Barrier::DownIn,     95.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25,  2.9586, 1.0e-4 },
        { Barrier::DownIn,     95.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25,  6.5677, 1.0e-4 },
        { Barrier::DownIn,     95.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25, 11.9752, 1.0e-4 },
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25,  2.2845, 1.0e-4 },
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25,  5.9085, 1.0e-4 },
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25, 11.6465, 1.0e-4 },
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25,  1.4653, 1.0e-4 },
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25,  3.3721, 1.0e-4 },
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25,  7.0846, 1.0e-4 },

        { Barrier::DownOut,    95.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30,  2.4170, 1.0e-4 },
        { Barrier::DownOut,    95.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30,  2.4258, 1.0e-4 },
        { Barrier::DownOut,    95.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30,  2.6246, 1.0e-4 },
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30,  3.0000, 1.0e-4 },
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30,  3.0000, 1.0e-4 },
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30,  3.0000, 1.0e-4 },
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30,  4.2293, 1.0e-4 },
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30,  5.8032, 1.0e-4 },
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30,  7.5649, 1.0e-4 },

        { Barrier::DownIn,     95.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30,  3.8769, 1.0e-4 },
        { Barrier::DownIn,     95.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30,  7.7989, 1.0e-4 },
        { Barrier::DownIn,     95.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30, 13.3078, 1.0e-4 },
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30,  3.3328, 1.0e-4 },
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30,  7.2636, 1.0e-4 },
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30, 12.9713, 1.0e-4 },
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30,  2.0658, 1.0e-4 },
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30,  4.4226, 1.0e-4 },
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30,  8.3686, 1.0e-4 }

        /*
            Data from "Going to Extreme: Correcting Simulation Bias in Exotic
            Option Valuation"
            D.R. Beaglehole, P.H. Dybvig and G. Zhou
            Financial Analysts Journal; Jan / Feb 1997; 53, 1
        */
        //    barrierType, barrier, rebate,         type, strike,     s,    q,    r,    t,    v,  result, tol
        // { Barrier::DownOut,    45.0,    0.0,  Option::Put,     50,  50.0,-0.05, 0.10, 0.25, 0.50,   4.032, 1.0e-3 },
        // { Barrier::DownOut,    45.0,    0.0,  Option::Put,     50,  50.0,-0.05, 0.10, 1.00, 0.50,   5.477, 1.0e-3 }

    };


    DayCounter dc = Actual360();
    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<TermStructure> qTS = makeFlatCurve(qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<TermStructure> rTS = makeFlatCurve(rRate, dc);

    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS = 
        makeFlatVolatility(vol, dc);

    Date today = Date::todaysDate();

    for (Size i=0; i<LENGTH(values); i++) {
        Date exDate = today.plusDays(int(values[i].t*360+0.5));
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        boost::shared_ptr<BlackScholesStochasticProcess> stochProcess(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(spot),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS)));

        BarrierOption barrierOption(
                values[i].barrierType,
                values[i].barrier,
                values[i].rebate,
                stochProcess,
                payoff,
                exercise);
        double calculated = barrierOption.NPV();
        double expected = values[i].result;
        double error = QL_FABS(calculated-expected);
        if (error>values[i].tol) {
            REPORT_FAILURE("value", values[i].barrierType, values[i].barrier,
                           values[i].rebate, payoff, exercise, values[i].s,
                           values[i].q, values[i].r, today, values[i].v,
                           expected, calculated, error, values[i].tol);
        }

    }
}

void BarrierOptionTest::testBabsiriValues() {

    BOOST_MESSAGE("Testing barrier options against Babsiri's values...");

    double maxErrorAllowed = 1.0e-5;
    double maxMCErrorAllowed = 1.0e-1;

    double underlyingPrice = 100.0;
    double rebate = 0.0;
    Rate r = 0.05;
    Rate q = 0.02;

    Size timeSteps = 1;
    bool antitheticVariate = false;
    bool controlVariate = false;
    Size requiredSamples = 10000;
    double requiredTolerance = 0.02;
    Size maxSamples = 1000000;
    bool isBiased = false;

    /*
        Data from
        "Simulating Path-Dependent Options: A New Approach"
          - M. El Babsiri and G. Noel
            Journal of Derivatives; Winter 1998; 6, 2
    */
    BarrierOptionData values[] = {
        { Barrier::DownIn, 0.10,   100,       90,   0.07187,  0.0 },
        { Barrier::DownIn, 0.15,   100,       90,   0.60638,  0.0 },
        { Barrier::DownIn, 0.20,   100,       90,   1.64005,  0.0 },
        { Barrier::DownIn, 0.25,   100,       90,   2.98495,  0.0 },
        { Barrier::DownIn, 0.30,   100,       90,   4.50952,  0.0 },
        { Barrier::UpIn,   0.10,   100,      110,   4.79148,  0.0 },
        { Barrier::UpIn,   0.15,   100,      110,   7.08268,  0.0 },
        { Barrier::UpIn,   0.20,   100,      110,   9.11008,  0.0 },
        { Barrier::UpIn,   0.25,   100,      110,  11.06148,  0.0 },
        { Barrier::UpIn,   0.30,   100,      110,  12.98351,  0.0 }
    };

    DayCounter dc = SimpleDayCounter();
    boost::shared_ptr<SimpleQuote> underlying(
                                            new SimpleQuote(underlyingPrice));

    boost::shared_ptr<SimpleQuote> qH_SME(new SimpleQuote(q));
    boost::shared_ptr<TermStructure> qTS = makeFlatCurve(qH_SME, dc);

    boost::shared_ptr<SimpleQuote> rH_SME(new SimpleQuote(r));
    boost::shared_ptr<TermStructure> rTS = makeFlatCurve(rH_SME, dc);

    boost::shared_ptr<SimpleQuote> volatility(new SimpleQuote(0.10));
    boost::shared_ptr<BlackVolTermStructure> volTS = 
        makeFlatVolatility(volatility, dc);

    boost::shared_ptr<PricingEngine> engine(new AnalyticBarrierEngine);
    boost::shared_ptr<PricingEngine> mcEngine(
        new MCBarrierEngine<PseudoRandom>(timeSteps, antitheticVariate,
                                          controlVariate, requiredSamples,
                                          requiredTolerance, maxSamples,
                                          isBiased, 5));

    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();
    Date exDate = calendar.advance(today,1,Years);
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

    for (Size i=0; i<LENGTH(values); i++) {
        volatility->setValue(values[i].volatility);

        boost::shared_ptr<StrikedTypePayoff> callPayoff(new
            PlainVanillaPayoff(Option::Call, values[i].strike));

        boost::shared_ptr<BlackScholesStochasticProcess> stochProcess(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(underlying),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS)));

        // analytic
        BarrierOption barrierCallOption(
                values[i].type,
                values[i].barrier,
                rebate,
                stochProcess,
                callPayoff,
                exercise,
                engine);
        double calculated = barrierCallOption.NPV();
        double expected = values[i].callValue;
        if (QL_FABS(calculated-expected) > maxErrorAllowed) {
            BOOST_FAIL(
                "Data at index " + SizeFormatter::toString(i) + ", "
                "Barrier call option:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
        }

        barrierCallOption.setPricingEngine(mcEngine);
        calculated = barrierCallOption.NPV();
        if (QL_FABS(calculated-expected) > maxMCErrorAllowed) {
            BOOST_FAIL(
                "Data at index " + SizeFormatter::toString(i) + ", "
                "Barrier call option MC:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
        }

    }
}

void BarrierOptionTest::testBeagleholeValues() {

    BOOST_MESSAGE("Testing barrier options against Beaglehole's values...");

    double maxErrorAllowed = 1.0e-3;
    double maxMCErrorAllowed = 1.5e-1;

    double underlyingPrice = 50.0;
    double rebate = 0.0;
    Rate r = QL_LOG (1.1);
    Rate q = 0.00;

    Size timeSteps = 1;
    bool antitheticVariate = false;
    bool controlVariate = false;
    Size requiredSamples = 10000;
    double requiredTolerance = 0.02;
    Size maxSamples = 1000000;
    bool isBiased = false;

    /*
        Data from
        "Going to Extreme: Correcting Simulation Bias in Exotic
         Option Valuation"
          - D.R. Beaglehole, P.H. Dybvig and G. Zhou
            Financial Analysts Journal; Jan / Feb 1997; 53, 1
    */
    BarrierOptionData values[] = {
        { Barrier::DownOut, 0.50,   50,      45,  5.477,  0.0 }
    };

    DayCounter dc = SimpleDayCounter();
    boost::shared_ptr<SimpleQuote> underlying(
                                            new SimpleQuote(underlyingPrice));

    boost::shared_ptr<SimpleQuote> qH_SME(new SimpleQuote(q));
    boost::shared_ptr<TermStructure> qTS = makeFlatCurve(qH_SME, dc);

    boost::shared_ptr<SimpleQuote> rH_SME(new SimpleQuote(r));
    boost::shared_ptr<TermStructure> rTS = makeFlatCurve(rH_SME, dc);

    boost::shared_ptr<SimpleQuote> volatility(new SimpleQuote(0.10));
    boost::shared_ptr<BlackVolTermStructure> volTS = 
        makeFlatVolatility(volatility, dc);

    boost::shared_ptr<PricingEngine> engine(new AnalyticBarrierEngine);
    boost::shared_ptr<PricingEngine> mcEngine(
        new MCBarrierEngine<PseudoRandom>(timeSteps, antitheticVariate,
                                          controlVariate, requiredSamples,
                                          requiredTolerance, maxSamples,
                                          isBiased, 10));

    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();
    Date exDate = calendar.advance(today,1,Years);
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

    for (Size i=0; i<LENGTH(values); i++) {
        volatility->setValue(values[i].volatility);

        boost::shared_ptr<StrikedTypePayoff> callPayoff(new
            PlainVanillaPayoff(Option::Call, values[i].strike));

        boost::shared_ptr<BlackScholesStochasticProcess> stochProcess(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(underlying),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS)));

        // analytic
        BarrierOption barrierCallOption(
                values[i].type,
                values[i].barrier,
                rebate,
                stochProcess,
                callPayoff,
                exercise,
                engine);
        double calculated = barrierCallOption.NPV();
        double expected = values[i].callValue;
        if (QL_FABS(calculated-expected) > maxErrorAllowed) {
            BOOST_FAIL(
                "Data at index " + SizeFormatter::toString(i) + ", "
                "Barrier call option:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
        }

        barrierCallOption.setPricingEngine(mcEngine);
        calculated = barrierCallOption.NPV();
        if (QL_FABS(calculated-expected) > maxMCErrorAllowed) {
            BOOST_FAIL(
                "Data at index " + SizeFormatter::toString(i) + ", "
                "Barrier call option MC:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
        }
    }
}


test_suite* BarrierOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Barrier option tests");
    suite->add(BOOST_TEST_CASE(&BarrierOptionTest::testHaugValues));
    suite->add(BOOST_TEST_CASE(&BarrierOptionTest::testBabsiriValues));
    suite->add(BOOST_TEST_CASE(&BarrierOptionTest::testBeagleholeValues));
    return suite;
}

