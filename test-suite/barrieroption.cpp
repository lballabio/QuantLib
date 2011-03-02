/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003, 2004, 2005, 2007, 2008 StatPro Italia srl
 Copyright (C) 2004 Ferdinando Ametrano

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

#include "barrieroption.hpp"
#include "utilities.hpp"
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/instruments/barrieroption.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <ql/experimental/finitedifferences/fdhestonbarrierengine.hpp>
#include <ql/experimental/finitedifferences/fdblackscholesbarrierengine.hpp>
#include <ql/experimental/barrieroption/perturbativebarrieroptionengine.hpp>
#include <ql/pricingengines/barrier/mcbarrierengine.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancecurve.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancesurface.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, barrierType, barrier, rebate, payoff, \
                       exercise, s, q, r, today, v, expected, calculated, \
                       error, tolerance) \
    BOOST_ERROR("\n" << barrierTypeToString(barrierType) << " " \
               << exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    underlying value: " << s << "\n" \
               << "    strike:           " << payoff->strike() << "\n" \
               << "    barrier:          " << barrier << "\n" \
               << "    rebate:           " << rebate << "\n" \
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
            QL_FAIL("unknown exercise type");
        }
    }

    struct BarrierOptionData {
        Barrier::Type type;
        Volatility volatility;
        Real strike;
        Real barrier;
        Real callValue;
        Real putValue;
    };

    struct NewBarrierOptionData {
        Barrier::Type barrierType;
        Real barrier;
        Real rebate;
        Option::Type type;
        Real strike;
        Real s;        // spot
        Rate q;        // dividend
        Rate r;        // risk-free rate
        Time t;        // time to maturity
        Volatility v;  // volatility
        Real result;   // result
        Real tol;      // tolerance
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
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (Size i=0; i<LENGTH(values); i++) {
        Date exDate = today + Integer(values[i].t*360+0.5);
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        boost::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

        boost::shared_ptr<PricingEngine> engine(
                                     new AnalyticBarrierEngine(stochProcess));

        BarrierOption barrierOption(
                values[i].barrierType,
                values[i].barrier,
                values[i].rebate,
                payoff,
                exercise);
        barrierOption.setPricingEngine(engine);

        Real calculated = barrierOption.NPV();
        Real expected = values[i].result;
        Real error = std::fabs(calculated-expected);
        if (error>values[i].tol) {
            REPORT_FAILURE("value", values[i].barrierType, values[i].barrier,
                           values[i].rebate, payoff, exercise, values[i].s,
                           values[i].q, values[i].r, today, values[i].v,
                           expected, calculated, error, values[i].tol);
        }

        engine = boost::shared_ptr<PricingEngine>(
                new FdBlackScholesBarrierEngine(stochProcess, 200, 400));
        barrierOption.setPricingEngine(engine);

        calculated = barrierOption.NPV();
        expected = values[i].result;
        error = std::fabs(calculated-expected);
        if (error>5.0e-3) {
            REPORT_FAILURE("fd value", values[i].barrierType, values[i].barrier,
                           values[i].rebate, payoff, exercise, values[i].s,
                           values[i].q, values[i].r, today, values[i].v,
                           expected, calculated, error, values[i].tol);
        }

    }
}

void BarrierOptionTest::testBabsiriValues() {

    BOOST_MESSAGE("Testing barrier options against Babsiri's values...");

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

    Real underlyingPrice = 100.0;
    Real rebate = 0.0;
    Rate r = 0.05;
    Rate q = 0.02;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    boost::shared_ptr<SimpleQuote> underlying(
                                            new SimpleQuote(underlyingPrice));

    boost::shared_ptr<SimpleQuote> qH_SME(new SimpleQuote(q));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qH_SME, dc);

    boost::shared_ptr<SimpleQuote> rH_SME(new SimpleQuote(r));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rH_SME, dc);

    boost::shared_ptr<SimpleQuote> volatility(new SimpleQuote(0.10));
    boost::shared_ptr<BlackVolTermStructure> volTS =
        flatVol(today, volatility, dc);

    Date exDate = today+360;
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

    for (Size i=0; i<LENGTH(values); i++) {
        volatility->setValue(values[i].volatility);

        boost::shared_ptr<StrikedTypePayoff> callPayoff(new
            PlainVanillaPayoff(Option::Call, values[i].strike));

        boost::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(underlying),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));


        boost::shared_ptr<PricingEngine> engine(
                                     new AnalyticBarrierEngine(stochProcess));

        // analytic
        BarrierOption barrierCallOption(
                values[i].type,
                values[i].barrier,
                rebate,
                callPayoff,
                exercise);
        barrierCallOption.setPricingEngine(engine);
        Real calculated = barrierCallOption.NPV();
        Real expected = values[i].callValue;
        Real error = std::fabs(calculated-expected);
        Real maxErrorAllowed = 1.0e-5;
        if (error>maxErrorAllowed) {
            REPORT_FAILURE("value", values[i].type, values[i].barrier,
                           rebate, callPayoff, exercise, underlyingPrice,
                           q, r, today, values[i].volatility,
                           expected, calculated, error, maxErrorAllowed);
        }

        Real maxMcRelativeErrorAllowed = 2.0e-2;

        boost::shared_ptr<PricingEngine> mcEngine =
            MakeMCBarrierEngine<LowDiscrepancy>(stochProcess)
            .withStepsPerYear(1)
            .withBrownianBridge()
            .withSamples(131071) // 2^17-1
            .withMaxSamples(1048575) // 2^20-1
            .withSeed(5);

        barrierCallOption.setPricingEngine(mcEngine);
        calculated = barrierCallOption.NPV();
        error = std::fabs(calculated-expected)/expected;
        if (error>maxMcRelativeErrorAllowed) {
            REPORT_FAILURE("value", values[i].type, values[i].barrier,
                           rebate, callPayoff, exercise, underlyingPrice,
                           q, r, today, values[i].volatility,
                           expected, calculated, error,
                           maxMcRelativeErrorAllowed);
        }

    }
}

void BarrierOptionTest::testBeagleholeValues() {

    BOOST_MESSAGE("Testing barrier options against Beaglehole's values...");


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

    Real underlyingPrice = 50.0;
    Real rebate = 0.0;
    Rate r = std::log(1.1);
    Rate q = 0.00;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> underlying(
                                            new SimpleQuote(underlyingPrice));

    boost::shared_ptr<SimpleQuote> qH_SME(new SimpleQuote(q));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qH_SME, dc);

    boost::shared_ptr<SimpleQuote> rH_SME(new SimpleQuote(r));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rH_SME, dc);

    boost::shared_ptr<SimpleQuote> volatility(new SimpleQuote(0.10));
    boost::shared_ptr<BlackVolTermStructure> volTS =
        flatVol(today, volatility, dc);


    Date exDate = today+360;
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

    for (Size i=0; i<LENGTH(values); i++) {
        volatility->setValue(values[i].volatility);

        boost::shared_ptr<StrikedTypePayoff> callPayoff(new
            PlainVanillaPayoff(Option::Call, values[i].strike));

        boost::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(underlying),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));


        boost::shared_ptr<PricingEngine> engine(
                                     new AnalyticBarrierEngine(stochProcess));

        BarrierOption barrierCallOption(
                values[i].type,
                values[i].barrier,
                rebate,
                callPayoff,
                exercise);
        barrierCallOption.setPricingEngine(engine);
        Real calculated = barrierCallOption.NPV();
        Real expected = values[i].callValue;
        Real maxErrorAllowed = 1.0e-3;
        Real error = std::fabs(calculated-expected);
        if (error > maxErrorAllowed) {
            REPORT_FAILURE("value", values[i].type, values[i].barrier,
                           rebate, callPayoff, exercise, underlyingPrice,
                           q, r, today, values[i].volatility,
                           expected, calculated, error, maxErrorAllowed);
        }

        Real maxMcRelativeErrorAllowed = 0.01;
        boost::shared_ptr<PricingEngine> mcEngine =
            MakeMCBarrierEngine<LowDiscrepancy>(stochProcess)
            .withStepsPerYear(1)
            .withBrownianBridge()
            .withSamples(131071) // 2^17-1
            .withMaxSamples(1048575) // 2^20-1
            .withSeed(10);

        barrierCallOption.setPricingEngine(mcEngine);
        calculated = barrierCallOption.NPV();
        error = std::fabs(calculated-expected)/expected;
        if (error>maxMcRelativeErrorAllowed) {
            REPORT_FAILURE("value", values[i].type, values[i].barrier,
                           rebate, callPayoff, exercise, underlyingPrice,
                           q, r, today, values[i].volatility,
                           expected, calculated, error,
                           maxMcRelativeErrorAllowed);
        }
    }
}

void BarrierOptionTest::testPerturbative() {
    BOOST_MESSAGE("Testing perturbative engine for barrier options...");

    Real S = 100.0;
    Real rebate = 0.0;
    Rate r = 0.03;
    Rate q = 0.02;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> underlying(new SimpleQuote(S));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, q, dc);
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, r, dc);

    std::vector<Date> dates(2);
    std::vector<Volatility> vols(2);

    dates[0] = today + 90;  vols[0] = 0.105;
    dates[1] = today + 180; vols[1] = 0.11;

    boost::shared_ptr<BlackVolTermStructure> volTS(
                              new BlackVarianceCurve(today, dates, vols, dc));

    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
        new BlackScholesMertonProcess(Handle<Quote>(underlying),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

    Real strike = 101.0;
    Real barrier = 101.0;
    Date exDate = today+180;

    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
    boost::shared_ptr<StrikedTypePayoff> payoff(
                                 new PlainVanillaPayoff(Option::Put, strike));

    BarrierOption option(Barrier::UpOut, barrier, rebate, payoff, exercise);

    Natural order = 0;
    bool zeroGamma = false;
    boost::shared_ptr<PricingEngine> engine(
         new PerturbativeBarrierOptionEngine(stochProcess, order, zeroGamma));

    option.setPricingEngine(engine);

    Real calculated = option.NPV();
    Real expected = 0.897365;
    Real tolerance = 1.0e-6;
    if (std::fabs(calculated-expected) > tolerance) {
        BOOST_ERROR("Failed to reproduce expected value"
                    << "\n  calculated: " << std::setprecision(5) << calculated
                    << "\n  expected:   " << std::setprecision(5) << expected);
    }

    order = 1;
    engine = boost::shared_ptr<PricingEngine>(
         new PerturbativeBarrierOptionEngine(stochProcess, order, zeroGamma));

    option.setPricingEngine(engine);

    calculated = option.NPV();
    expected = 0.894374;
    if (std::fabs(calculated-expected) > tolerance) {
        BOOST_ERROR("Failed to reproduce expected value"
                    << "\n  calculated: " << std::setprecision(5) << calculated
                    << "\n  expected:   " << std::setprecision(5) << expected);
    }

    order = 2;
    engine = boost::shared_ptr<PricingEngine>(
         new PerturbativeBarrierOptionEngine(stochProcess, order, zeroGamma));

    option.setPricingEngine(engine);

    calculated = option.NPV();
    expected = 0.894375;
    if (std::fabs(calculated-expected) > tolerance) {
        BOOST_ERROR("Failed to reproduce expected value"
                    << "\n  calculated: " << std::setprecision(5) << calculated
                    << "\n  expected:   " << std::setprecision(5) << expected);
    }
}

void BarrierOptionTest::testLocalVolAndHestonComparison() {
    BOOST_MESSAGE("Testing local volatility and Heston FD engines "
                  "for barrier options...");

    SavedSettings backup;

    const Date settlementDate(5, July, 2002);
    Settings::instance().evaluationDate() = settlementDate;

    const DayCounter dayCounter = Actual365Fixed();
    const Calendar calendar = TARGET();

    Integer t[] = { 13, 41, 75, 165, 256, 345, 524, 703 };
    Rate r[] = { 0.0357,0.0349,0.0341,0.0355,0.0359,0.0368,0.0386,0.0401 };

    std::vector<Rate> rates(1, 0.0357);
    std::vector<Date> dates(1, settlementDate);
    for (Size i = 0; i < 8; ++i) {
        dates.push_back(settlementDate + t[i]);
        rates.push_back(r[i]);
    }
    const Handle<YieldTermStructure> rTS(
            boost::shared_ptr<YieldTermStructure>(
                                    new ZeroCurve(dates, rates, dayCounter)));
    const Handle<YieldTermStructure> qTS(
                                   flatRate(settlementDate, 0.0, dayCounter));

    const Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(4500.00)));
    
    Real tmp[] = { 100 ,500 ,2000,3400,3600,3800,4000,4200,4400,4500,
                   4600,4800,5000,5200,5400,5600,7500,10000,20000,30000 };
    const std::vector<Real> strikes(tmp, tmp+LENGTH(tmp));
    
    Volatility v[] =
      { 1.015873, 1.015873, 1.015873, 0.89729, 0.796493, 0.730914, 0.631335, 0.568895,
        0.711309, 0.711309, 0.711309, 0.641309, 0.635593, 0.583653, 0.508045, 0.463182,
        0.516034, 0.500534, 0.500534, 0.500534, 0.448706, 0.416661, 0.375470, 0.353442,
        0.516034, 0.482263, 0.447713, 0.387703, 0.355064, 0.337438, 0.316966, 0.306859,
        0.497587, 0.464373, 0.430764, 0.374052, 0.344336, 0.328607, 0.310619, 0.301865,
        0.479511, 0.446815, 0.414194, 0.361010, 0.334204, 0.320301, 0.304664, 0.297180,
        0.461866, 0.429645, 0.398092, 0.348638, 0.324680, 0.312512, 0.299082, 0.292785,
        0.444801, 0.413014, 0.382634, 0.337026, 0.315788, 0.305239, 0.293855, 0.288660,
        0.428604, 0.397219, 0.368109, 0.326282, 0.307555, 0.298483, 0.288972, 0.284791,
        0.420971, 0.389782, 0.361317, 0.321274, 0.303697, 0.295302, 0.286655, 0.282948,
        0.413749, 0.382754, 0.354917, 0.316532, 0.300016, 0.292251, 0.284420, 0.281164,
        0.400889, 0.370272, 0.343525, 0.307904, 0.293204, 0.286549, 0.280189, 0.277767,
        0.390685, 0.360399, 0.334344, 0.300507, 0.287149, 0.281380, 0.276271, 0.274588,
        0.383477, 0.353434, 0.327580, 0.294408, 0.281867, 0.276746, 0.272655, 0.271617,
        0.379106, 0.349214, 0.323160, 0.289618, 0.277362, 0.272641, 0.269332, 0.268846,
        0.377073, 0.347258, 0.320776, 0.286077, 0.273617, 0.269057, 0.266293, 0.266265,
        0.399925, 0.369232, 0.338895, 0.289042, 0.265509, 0.255589, 0.249308, 0.249665,
        0.423432, 0.406891, 0.373720, 0.314667, 0.281009, 0.263281, 0.246451, 0.242166,
        0.453704, 0.453704, 0.453704, 0.381255, 0.334578, 0.305527, 0.268909, 0.251367,
        0.517748, 0.517748, 0.517748, 0.416577, 0.364770, 0.331595, 0.287423, 0.264285 };
    
    Matrix blackVolMatrix(strikes.size(), dates.size()-1);
    for (Size i=0; i < strikes.size(); ++i)
        for (Size j=1; j < dates.size(); ++j) {
            blackVolMatrix[i][j-1] = v[i*(dates.size()-1)+j-1];
        }
    
    const boost::shared_ptr<BlackVarianceSurface> volTS(
        new BlackVarianceSurface(settlementDate, calendar,
                                 std::vector<Date>(dates.begin()+1,dates.end()),
                                 strikes, blackVolMatrix,
                                 dayCounter));
    volTS->setInterpolation<Bicubic>();
    const boost::shared_ptr<GeneralizedBlackScholesProcess> localVolProcess(
        new BlackScholesMertonProcess(s0, qTS, rTS, 
                                      Handle<BlackVolTermStructure>(volTS)));
    
    const Real v0   =0.195662;
    const Real kappa=5.6628;
    const Real theta=0.0745911;
    const Real sigma=1.1619;
    const Real rho  =-0.511493;

    boost::shared_ptr<HestonProcess> hestonProcess(new HestonProcess(
                                  rTS, qTS, s0, v0, kappa, theta, sigma, rho));

    boost::shared_ptr<HestonModel> hestonModel(new HestonModel(hestonProcess));

    boost::shared_ptr<PricingEngine> fdHestonEngine(
                         new FdHestonBarrierEngine(hestonModel, 100, 400, 50));
    
    boost::shared_ptr<PricingEngine> fdLocalVolEngine(
                   new FdBlackScholesBarrierEngine(localVolProcess, 100, 400, 0,
                                                   FdmSchemeDesc::Douglas(), 
                                                   true, 0.35));
    
    const Real strike  = s0->value();
    const Real barrier = 3000;
    const Real rebate  = 100;
    const Date exDate  = settlementDate + Period(20, Months);
    
    const boost::shared_ptr<StrikedTypePayoff> payoff(new
                                     PlainVanillaPayoff(Option::Put, strike));

    const boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

    BarrierOption barrierOption(Barrier::DownOut, 
                                barrier, rebate, payoff, exercise);
    
    barrierOption.setPricingEngine(fdHestonEngine);
    const Real expectedHestonNPV = 111.5;
    const Real calculatedHestonNPV = barrierOption.NPV();

    barrierOption.setPricingEngine(fdLocalVolEngine);
    const Real expectedLocalVolNPV = 132.8;
    const Real calculatedLocalVolNPV = barrierOption.NPV();
    
    const Real tol = 0.01;
    
    if (std::fabs(expectedHestonNPV - calculatedHestonNPV) 
                                                > tol*expectedHestonNPV) {
        BOOST_FAIL("Failed to reproduce Heston barrier price for "
                   << "\n    strike:     " << payoff->strike()
                   << "\n    barrier:    " << barrier
                   << "\n    maturity:   " << exDate
                   << "\n    calculated: " << calculatedHestonNPV
                   << "\n    expected:   " << expectedHestonNPV);
    }
    if (std::fabs(expectedLocalVolNPV - calculatedLocalVolNPV) 
                                                > tol*expectedLocalVolNPV) {
        BOOST_FAIL("Failed to reproduce Heston barrier price for "
                   << "\n    strike:     " << payoff->strike()
                   << "\n    barrier:    " << barrier
                   << "\n    maturity:   " << exDate
                   << "\n    calculated: " << calculatedHestonNPV
                   << "\n    expected:   " << expectedHestonNPV);
    }
}


test_suite* BarrierOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Barrier option tests");
    suite->add(QUANTLIB_TEST_CASE(&BarrierOptionTest::testHaugValues));
    suite->add(QUANTLIB_TEST_CASE(&BarrierOptionTest::testBabsiriValues));
    suite->add(QUANTLIB_TEST_CASE(&BarrierOptionTest::testBeagleholeValues));
    suite->add(QUANTLIB_TEST_CASE(&BarrierOptionTest::testPerturbative));
    suite->add(QUANTLIB_TEST_CASE(
                        &BarrierOptionTest::testLocalVolAndHestonComparison));
    return suite;
}
