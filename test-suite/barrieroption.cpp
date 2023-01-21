/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003, 2004, 2005, 2007, 2008 StatPro Italia srl
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2013 Yue Tian
 Copyright (C) 2014 Thema Consulting SA

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
#include <ql/time/daycounters/business252.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/instruments/barrieroption.hpp>
#include <ql/instruments/dividendbarrieroption.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <ql/pricingengines/barrier/binomialbarrierengine.hpp>
#include <ql/pricingengines/barrier/fdhestonbarrierengine.hpp>
#include <ql/pricingengines/barrier/fdblackscholesbarrierengine.hpp>
#include <ql/pricingengines/barrier/mcbarrierengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/experimental/barrieroption/perturbativebarrieroptionengine.hpp>
#include <ql/experimental/barrieroption/vannavolgabarrierengine.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancecurve.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancesurface.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#undef REPORT_FAILURE
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

#undef REPORT_FX_FAILURE
#define REPORT_FX_FAILURE(greekName, barrierType, barrier, \
                          rebate, payoff, exercise, s, q, r, today, \
                          vol25Put, atmVol, vol25Call, v, \
                          expected, calculated, error, tolerance) \
    BOOST_ERROR("\n" << barrierTypeToString(barrierType) << " " \
               << exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " FX option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    underlying value: " << s << "\n" \
               << "    strike:           " << payoff->strike() << "\n" \
               << "    barrier:          " << barrier << "\n" \
               << "    rebate:           " << rebate << "\n" \
               << "    dividend yield:   " << io::rate(q) << "\n" \
               << "    risk-free rate:   " << io::rate(r) << "\n" \
               << "    reference date:   " << today << "\n" \
               << "    maturity:         " << exercise->lastDate() << "\n" \
               << "    25PutVol:         " << io::volatility(vol25Put) << "\n" \
               << "    atmVol:           " << io::volatility(atmVol) << "\n" \
               << "    25CallVol:        " << io::volatility(vol25Call) << "\n" \
               << "    volatility:       " << io::volatility(v) << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);


namespace barrier_option_test {

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
        Exercise::Type exType;
        Real strike;
        Real s;        // spot
        Rate q;        // dividend
        Rate r;        // risk-free rate
        Time t;        // time to maturity
        Volatility v;  // volatility
        Real result;   // result
        Real tol;      // tolerance
    };

    struct BarrierFxOptionData {
        Barrier::Type barrierType;
        Real barrier;
        Real rebate;
        Option::Type type;
        Real strike;
        Real s;                 // spot
        Rate q;                 // dividend
        Rate r;                 // risk-free rate
        Time t;                 // time to maturity
        Volatility vol25Put;    // 25 delta put vol
        Volatility volAtm;      // atm vol
        Volatility vol25Call;   // 25 delta call vol
        Volatility v;           // volatility at strike
        Real result;            // result
        Real tol;               // tolerance
    };

}

#define QL_ASSERT_EXCEPTION_THROWN(statement)  \
    auto exception_thrown = false;             \
    try {                                      \
        statement                              \
    } catch (const std::exception&) {          \
        exception_thrown = true;               \
    }                                          \
    if (!exception_thrown) {                   \
        BOOST_FAIL("exception expected");      \
    }                                          \

void BarrierOptionTest::testParity() {

    BOOST_TEST_MESSAGE("Testing that knock-in plus knock-out barrier options "
                       "replicate a European option...");

    Date today = Settings::instance().evaluationDate();

    DayCounter dc = Actual360();

    ext::shared_ptr<SimpleQuote> spot = ext::make_shared<SimpleQuote>(100.0);
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, 0.01, dc);
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, 0.20, dc);
    RelinkableHandle<BlackVolTermStructure> volHandle(volTS);

    ext::shared_ptr<BlackScholesProcess> stochProcess =
        ext::make_shared<BlackScholesProcess>(
                                      Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(rTS),
                                      volHandle);

    Date exerciseDate = today + 6*Months;

    ext::shared_ptr<StrikedTypePayoff> payoff =
        ext::make_shared<PlainVanillaPayoff>(Option::Call, 100.0);

    ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(exerciseDate);

    BarrierOption knockIn(Barrier::DownIn, 90.0, 0.0,
                          payoff, exercise);
    BarrierOption knockOut(Barrier::DownOut, 90.0, 0.0,
                           payoff, exercise);
    EuropeanOption european(payoff, exercise);

    ext::shared_ptr<PricingEngine> barrierEngine
        = ext::make_shared<AnalyticBarrierEngine>(stochProcess);

    ext::shared_ptr<PricingEngine> europeanEngine
        = ext::make_shared<AnalyticEuropeanEngine>(stochProcess);

    knockIn.setPricingEngine(barrierEngine);
    knockOut.setPricingEngine(barrierEngine);
    european.setPricingEngine(europeanEngine);

    Real replicated = knockIn.NPV() + knockOut.NPV();
    Real expected = european.NPV();
    Real error = std::fabs(replicated-expected);
    if (error > 1e-7) {
        BOOST_ERROR("Failed to replicate European option"
                    << "\n    knock-in:   " << knockIn.NPV()
                    << "\n    knock-out:  " << knockOut.NPV()
                    << "\n    replicated: " << replicated
                    << "\n    expected:   " << expected
                    << std::scientific << std::setprecision(3)
                    << "\n    error:      " << error);
    }

    // try again with different day counters

    volHandle.linkTo(flatVol(today, 0.20, Business252(TARGET())));

    replicated = knockIn.NPV() + knockOut.NPV();
    expected = european.NPV();
    error = std::fabs(replicated-expected);
    if (error > 1e-7) {
        BOOST_ERROR("Failed to replicate European option"
                    << "\n    knock-in:   " << knockIn.NPV()
                    << "\n    knock-out:  " << knockOut.NPV()
                    << "\n    replicated: " << replicated
                    << "\n    expected:   " << expected
                    << std::scientific << std::setprecision(3)
                    << "\n    error:      " << error);
    }
}

void BarrierOptionTest::testHaugValues() {

    BOOST_TEST_MESSAGE("Testing barrier options against Haug's values...");

    using namespace barrier_option_test;

    Exercise::Type european = Exercise::European;
    Exercise::Type american = Exercise::American;
    NewBarrierOptionData values[] = {
        /* The data below are from
          "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 pag. 72
        */
        //     barrierType, barrier, rebate,         type, exercise, strk,     s,    q,    r,    t,    v,  result, tol
        { Barrier::DownOut,    95.0,    3.0, Option::Call, european,   90, 100.0, 0.04, 0.08, 0.50, 0.25,  9.0246, 1.0e-4},
        { Barrier::DownOut,    95.0,    3.0, Option::Call, european,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  6.7924, 1.0e-4},
        { Barrier::DownOut,    95.0,    3.0, Option::Call, european,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  4.8759, 1.0e-4},
        { Barrier::DownOut,   100.0,    3.0, Option::Call, european,   90, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4},
        { Barrier::DownOut,   100.0,    3.0, Option::Call, european,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4},
        { Barrier::DownOut,   100.0,    3.0, Option::Call, european,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4},
        { Barrier::UpOut,     105.0,    3.0, Option::Call, european,   90, 100.0, 0.04, 0.08, 0.50, 0.25,  2.6789, 1.0e-4},
        { Barrier::UpOut,     105.0,    3.0, Option::Call, european,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  2.3580, 1.0e-4},
        { Barrier::UpOut,     105.0,    3.0, Option::Call, european,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  2.3453, 1.0e-4},

        { Barrier::DownIn,     95.0,    3.0, Option::Call, european,   90, 100.0, 0.04, 0.08, 0.50, 0.25,  7.7627, 1.0e-4},
        { Barrier::DownIn,     95.0,    3.0, Option::Call, european,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  4.0109, 1.0e-4},
        { Barrier::DownIn,     95.0,    3.0, Option::Call, european,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  2.0576, 1.0e-4},
        { Barrier::DownIn,    100.0,    3.0, Option::Call, european,   90, 100.0, 0.04, 0.08, 0.50, 0.25, 13.8333, 1.0e-4},
        { Barrier::DownIn,    100.0,    3.0, Option::Call, european,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  7.8494, 1.0e-4},
        { Barrier::DownIn,    100.0,    3.0, Option::Call, european,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  3.9795, 1.0e-4},
        { Barrier::UpIn,      105.0,    3.0, Option::Call, european,   90, 100.0, 0.04, 0.08, 0.50, 0.25, 14.1112, 1.0e-4},
        { Barrier::UpIn,      105.0,    3.0, Option::Call, european,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  8.4482, 1.0e-4},
        { Barrier::UpIn,      105.0,    3.0, Option::Call, european,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  4.5910, 1.0e-4},

        { Barrier::DownOut,    95.0,    3.0, Option::Call, european,   90, 100.0, 0.04, 0.08, 0.50, 0.30,  8.8334, 1.0e-4},
        { Barrier::DownOut,    95.0,    3.0, Option::Call, european,  100, 100.0, 0.04, 0.08, 0.50, 0.30,  7.0285, 1.0e-4},
        { Barrier::DownOut,    95.0,    3.0, Option::Call, european,  110, 100.0, 0.04, 0.08, 0.50, 0.30,  5.4137, 1.0e-4},
        { Barrier::DownOut,   100.0,    3.0, Option::Call, european,   90, 100.0, 0.04, 0.08, 0.50, 0.30,  3.0000, 1.0e-4},
        { Barrier::DownOut,   100.0,    3.0, Option::Call, european,  100, 100.0, 0.04, 0.08, 0.50, 0.30,  3.0000, 1.0e-4},
        { Barrier::DownOut,   100.0,    3.0, Option::Call, european,  110, 100.0, 0.04, 0.08, 0.50, 0.30,  3.0000, 1.0e-4},
        { Barrier::UpOut,     105.0,    3.0, Option::Call, european,   90, 100.0, 0.04, 0.08, 0.50, 0.30,  2.6341, 1.0e-4},
        { Barrier::UpOut,     105.0,    3.0, Option::Call, european,  100, 100.0, 0.04, 0.08, 0.50, 0.30,  2.4389, 1.0e-4},
        { Barrier::UpOut,     105.0,    3.0, Option::Call, european,  110, 100.0, 0.04, 0.08, 0.50, 0.30,  2.4315, 1.0e-4},

        { Barrier::DownIn,     95.0,    3.0, Option::Call, european,   90, 100.0, 0.04, 0.08, 0.50, 0.30,  9.0093, 1.0e-4},
        { Barrier::DownIn,     95.0,    3.0, Option::Call, european,  100, 100.0, 0.04, 0.08, 0.50, 0.30,  5.1370, 1.0e-4},
        { Barrier::DownIn,     95.0,    3.0, Option::Call, european,  110, 100.0, 0.04, 0.08, 0.50, 0.30,  2.8517, 1.0e-4},
        { Barrier::DownIn,    100.0,    3.0, Option::Call, european,   90, 100.0, 0.04, 0.08, 0.50, 0.30, 14.8816, 1.0e-4},
        { Barrier::DownIn,    100.0,    3.0, Option::Call, european,  100, 100.0, 0.04, 0.08, 0.50, 0.30,  9.2045, 1.0e-4},
        { Barrier::DownIn,    100.0,    3.0, Option::Call, european,  110, 100.0, 0.04, 0.08, 0.50, 0.30,  5.3043, 1.0e-4},
        { Barrier::UpIn,      105.0,    3.0, Option::Call, european,   90, 100.0, 0.04, 0.08, 0.50, 0.30, 15.2098, 1.0e-4},
        { Barrier::UpIn,      105.0,    3.0, Option::Call, european,  100, 100.0, 0.04, 0.08, 0.50, 0.30,  9.7278, 1.0e-4},
        { Barrier::UpIn,      105.0,    3.0, Option::Call, european,  110, 100.0, 0.04, 0.08, 0.50, 0.30,  5.8350, 1.0e-4},


        //     barrierType, barrier, rebate,         type, exercise, strk,     s,    q,    r,    t,    v,  result, tol
        { Barrier::DownOut,    95.0,    3.0,  Option::Put, european,   90, 100.0, 0.04, 0.08, 0.50, 0.25,  2.2798, 1.0e-4 },
        { Barrier::DownOut,    95.0,    3.0,  Option::Put, european,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  2.2947, 1.0e-4 },
        { Barrier::DownOut,    95.0,    3.0,  Option::Put, european,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  2.6252, 1.0e-4 },
        { Barrier::DownOut,   100.0,    3.0,  Option::Put, european,   90, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4 },
        { Barrier::DownOut,   100.0,    3.0,  Option::Put, european,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4 },
        { Barrier::DownOut,   100.0,    3.0,  Option::Put, european,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4 },
        { Barrier::UpOut,     105.0,    3.0,  Option::Put, european,   90, 100.0, 0.04, 0.08, 0.50, 0.25,  3.7760, 1.0e-4 },
        { Barrier::UpOut,     105.0,    3.0,  Option::Put, european,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  5.4932, 1.0e-4 },
        { Barrier::UpOut,     105.0,    3.0,  Option::Put, european,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  7.5187, 1.0e-4 },

        { Barrier::DownIn,     95.0,    3.0,  Option::Put, european,   90, 100.0, 0.04, 0.08, 0.50, 0.25,  2.9586, 1.0e-4 },
        { Barrier::DownIn,     95.0,    3.0,  Option::Put, european,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  6.5677, 1.0e-4 },
        { Barrier::DownIn,     95.0,    3.0,  Option::Put, european,  110, 100.0, 0.04, 0.08, 0.50, 0.25, 11.9752, 1.0e-4 },
        { Barrier::DownIn,    100.0,    3.0,  Option::Put, european,   90, 100.0, 0.04, 0.08, 0.50, 0.25,  2.2845, 1.0e-4 },
        { Barrier::DownIn,    100.0,    3.0,  Option::Put, european,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  5.9085, 1.0e-4 },
        { Barrier::DownIn,    100.0,    3.0,  Option::Put, european,  110, 100.0, 0.04, 0.08, 0.50, 0.25, 11.6465, 1.0e-4 },
        { Barrier::UpIn,      105.0,    3.0,  Option::Put, european,   90, 100.0, 0.04, 0.08, 0.50, 0.25,  1.4653, 1.0e-4 },
        { Barrier::UpIn,      105.0,    3.0,  Option::Put, european,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  3.3721, 1.0e-4 },
        { Barrier::UpIn,      105.0,    3.0,  Option::Put, european,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  7.0846, 1.0e-4 },

        { Barrier::DownOut,    95.0,    3.0,  Option::Put, european,   90, 100.0, 0.04, 0.08, 0.50, 0.30,  2.4170, 1.0e-4 },
        { Barrier::DownOut,    95.0,    3.0,  Option::Put, european,  100, 100.0, 0.04, 0.08, 0.50, 0.30,  2.4258, 1.0e-4 },
        { Barrier::DownOut,    95.0,    3.0,  Option::Put, european,  110, 100.0, 0.04, 0.08, 0.50, 0.30,  2.6246, 1.0e-4 },
        { Barrier::DownOut,   100.0,    3.0,  Option::Put, european,   90, 100.0, 0.04, 0.08, 0.50, 0.30,  3.0000, 1.0e-4 },
        { Barrier::DownOut,   100.0,    3.0,  Option::Put, european,  100, 100.0, 0.04, 0.08, 0.50, 0.30,  3.0000, 1.0e-4 },
        { Barrier::DownOut,   100.0,    3.0,  Option::Put, european,  110, 100.0, 0.04, 0.08, 0.50, 0.30,  3.0000, 1.0e-4 },
        { Barrier::UpOut,     105.0,    3.0,  Option::Put, european,   90, 100.0, 0.04, 0.08, 0.50, 0.30,  4.2293, 1.0e-4 },
        { Barrier::UpOut,     105.0,    3.0,  Option::Put, european,  100, 100.0, 0.04, 0.08, 0.50, 0.30,  5.8032, 1.0e-4 },
        { Barrier::UpOut,     105.0,    3.0,  Option::Put, european,  110, 100.0, 0.04, 0.08, 0.50, 0.30,  7.5649, 1.0e-4 },

        { Barrier::DownIn,     95.0,    3.0,  Option::Put, european,   90, 100.0, 0.04, 0.08, 0.50, 0.30,  3.8769, 1.0e-4 },
        { Barrier::DownIn,     95.0,    3.0,  Option::Put, european,  100, 100.0, 0.04, 0.08, 0.50, 0.30,  7.7989, 1.0e-4 },
        { Barrier::DownIn,     95.0,    3.0,  Option::Put, european,  110, 100.0, 0.04, 0.08, 0.50, 0.30, 13.3078, 1.0e-4 },
        { Barrier::DownIn,    100.0,    3.0,  Option::Put, european,   90, 100.0, 0.04, 0.08, 0.50, 0.30,  3.3328, 1.0e-4 },
        { Barrier::DownIn,    100.0,    3.0,  Option::Put, european,  100, 100.0, 0.04, 0.08, 0.50, 0.30,  7.2636, 1.0e-4 },
        { Barrier::DownIn,    100.0,    3.0,  Option::Put, european,  110, 100.0, 0.04, 0.08, 0.50, 0.30, 12.9713, 1.0e-4 },
        { Barrier::UpIn,      105.0,    3.0,  Option::Put, european,   90, 100.0, 0.04, 0.08, 0.50, 0.30,  2.0658, 1.0e-4 },
        { Barrier::UpIn,      105.0,    3.0,  Option::Put, european,  100, 100.0, 0.04, 0.08, 0.50, 0.30,  4.4226, 1.0e-4 },
        { Barrier::UpIn,      105.0,    3.0,  Option::Put, european,  110, 100.0, 0.04, 0.08, 0.50, 0.30,  8.3686, 1.0e-4 },

        // Options with american exercise: values computed with 400 steps of Haug's VBA code (handles only out options)
        //     barrierType, barrier, rebate,         type, exercise, strk,     s,    q,    r,    t,    v,  result, tol
        { Barrier::DownOut,    95.0,    0.0, Option::Call, american,   90, 100.0, 0.04, 0.08, 0.50, 0.25, 10.4655, 1.0e-4},
        { Barrier::DownOut,    95.0,    0.0, Option::Call, american,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  4.5159, 1.0e-4},
        { Barrier::DownOut,    95.0,    0.0, Option::Call, american,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  2.5971, 1.0e-4},
        { Barrier::DownOut,   100.0,    3.0, Option::Call, american,   90, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4},
        { Barrier::DownOut,   100.0,    3.0, Option::Call, american,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4},
        { Barrier::DownOut,   100.0,    3.0, Option::Call, american,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4},
        { Barrier::UpOut,     105.0,    0.0, Option::Call, american,   90, 100.0, 0.04, 0.08, 0.50, 0.25, 11.8076, 1.0e-4},
        { Barrier::UpOut,     105.0,    0.0, Option::Call, american,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  3.3993, 1.0e-4},
        { Barrier::UpOut,     105.0,    3.0, Option::Call, american,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  2.3457, 1.0e-4},

        { Barrier::DownOut,    95.0,    3.0,  Option::Put, american,   90, 100.0, 0.04, 0.08, 0.50, 0.25,  2.2795, 1.0e-4 },
        { Barrier::DownOut,    95.0,    0.0,  Option::Put, american,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  3.3512, 1.0e-4 },
        { Barrier::DownOut,    95.0,    0.0,  Option::Put, american,  110, 100.0, 0.04, 0.08, 0.50, 0.25, 11.5773, 1.0e-4 },
        { Barrier::DownOut,   100.0,    3.0,  Option::Put, american,   90, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4 },
        { Barrier::DownOut,   100.0,    3.0,  Option::Put, american,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4 },
        { Barrier::DownOut,   100.0,    3.0,  Option::Put, american,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  3.0000, 1.0e-4 },
        { Barrier::UpOut,     105.0,    0.0,  Option::Put, american,   90, 100.0, 0.04, 0.08, 0.50, 0.25,  1.4763, 1.0e-4 },
        { Barrier::UpOut,     105.0,    0.0,  Option::Put, american,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  3.3001, 1.0e-4 },
        { Barrier::UpOut,     105.0,    0.0,  Option::Put, american,  110, 100.0, 0.04, 0.08, 0.50, 0.25, 10.0000, 1.0e-4 },

        // some american in-options - results (roughly) verified with other numerical methods 
        //     barrierType, barrier, rebate,         type, exercise, strk,     s,    q,    r,    t,    v,  result, tol
        { Barrier::DownIn,     95.0,    3.0, Option::Call, american,   90, 100.0, 0.04, 0.08, 0.50, 0.25,  7.7615, 1.0e-4},
        { Barrier::DownIn,     95.0,    3.0, Option::Call, american,  100, 100.0, 0.04, 0.08, 0.50, 0.25,  4.0118, 1.0e-4},
        { Barrier::DownIn,     95.0,    3.0, Option::Call, american,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  2.0544, 1.0e-4},
        { Barrier::DownIn,    100.0,    3.0, Option::Call, american,   90, 100.0, 0.04, 0.08, 0.50, 0.25, 13.8308, 1.0e-4},
        { Barrier::UpIn,      105.0,    3.0, Option::Call, american,   90, 100.0, 0.04, 0.08, 0.50, 0.25, 14.1150, 1.0e-4},
        { Barrier::UpIn,      105.0,    3.0, Option::Call, american,  110, 100.0, 0.04, 0.08, 0.50, 0.25,  4.5900, 1.0e-4},

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

    ext::shared_ptr<SimpleQuote> spot = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<SimpleQuote> qRate = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    auto runTest = [&](const NewBarrierOptionData& value, const bool useZeroSpot, const bool useTriggeredBarrier) {
        Date exDate = today + timeToDays(value.t);

        spot->setValue(useZeroSpot ? 0.0 : value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::make_shared<PlainVanillaPayoff>(value.type, value.strike);

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess =
            ext::make_shared<BlackScholesMertonProcess>(
                                      Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS));

        ext::shared_ptr<Exercise> exercise;
        if (value.exType == Exercise::European)
            exercise = ext::make_shared<EuropeanExercise>(exDate);
        else
            exercise = ext::make_shared<AmericanExercise>(exDate);

        const auto barrier = useTriggeredBarrier
            ? (value.barrierType == Barrier::Type::DownIn
            || value.barrierType == Barrier::Type::DownOut ? value.s * 1.01 : value.s * 0.99)
            : value.barrier;

        BarrierOption barrierOption(value.barrierType, barrier, value.rebate, payoff, exercise);

        ext::shared_ptr<PricingEngine> engine;
        Real calculated;
        Real expected;
        Real error;
        Real tol;

        // AnalyticBarrierEngine and FdBlackScholesBarrierEngine support only european exercise type

        engine = ext::make_shared<AnalyticBarrierEngine>(stochProcess);
        barrierOption.setPricingEngine(engine);

        if (!useZeroSpot && !useTriggeredBarrier && value.exType == Exercise::European) {
            Real calculated = barrierOption.NPV();
            Real expected = value.result;
            Real error = std::fabs(calculated - expected);
            if (error > value.tol) {
                REPORT_FAILURE("value", value.barrierType, value.barrier, value.rebate, payoff,
                               exercise, value.s, value.q, value.r, today, value.v, expected,
                               calculated, error, value.tol);
            }
        } else {
            QL_ASSERT_EXCEPTION_THROWN(barrierOption.NPV();)
        }

        engine = ext::make_shared<FdBlackScholesBarrierEngine>(stochProcess, 200, 400);
        barrierOption.setPricingEngine(engine);

        if (!useZeroSpot && !useTriggeredBarrier && value.exType == Exercise::European) {
            calculated = barrierOption.NPV();
            expected = value.result;
            error = std::fabs(calculated - expected);
            if (error > 5.0e-3) {
                REPORT_FAILURE("fd value", value.barrierType, value.barrier, value.rebate, payoff,
                               exercise, value.s, value.q, value.r, today, value.v, expected,
                               calculated, error, value.tol);
            }
        } else {
            QL_ASSERT_EXCEPTION_THROWN(barrierOption.NPV();)
        }

        engine = ext::make_shared<BinomialBarrierEngine<CoxRossRubinstein,DiscretizedBarrierOption> >(stochProcess, 400);
        barrierOption.setPricingEngine(engine);

        if (!useZeroSpot && !useTriggeredBarrier) {
            calculated = barrierOption.NPV();
            expected = value.result;
            error = std::fabs(calculated-expected);
            tol = 1.1e-2;
            if (error>tol) {
                REPORT_FAILURE("Binomial (Boyle-lau) value", value.barrierType, value.barrier,
                               value.rebate, payoff, exercise, value.s, value.q, value.r, today,
                               value.v, expected, calculated, error, tol);
            }
        } else {
            QL_ASSERT_EXCEPTION_THROWN(barrierOption.NPV();)
        }

        // Note: here, to test Derman convergence, we force maxTimeSteps to 
        // timeSteps, effectively disabling Boyle-Lau barrier adjustment.
        // Production code should always enable Boyle-Lau. In most cases it
        // gives very good convergence with only a modest timeStep increment.
        engine = ext::make_shared<BinomialBarrierEngine<CoxRossRubinstein,DiscretizedDermanKaniBarrierOption> >(stochProcess, 400);
        barrierOption.setPricingEngine(engine);

        if (!useZeroSpot && !useTriggeredBarrier) {
            calculated = barrierOption.NPV();
            expected = value.result;
            error = std::fabs(calculated-expected);
            tol = 4e-2;
            if (error>tol) {
                REPORT_FAILURE("Binomial (Derman) value", value.barrierType, value.barrier,
                               value.rebate, payoff, exercise, value.s, value.q, value.r, today,
                               value.v, expected, calculated, error, tol);
            }
        } else {
            QL_ASSERT_EXCEPTION_THROWN(barrierOption.NPV();)
        }
    };

    for (auto& value : values) {
        runTest(value, false, false);
        runTest(value, true, false);
        runTest(value, false, true);
    }
}

void BarrierOptionTest::testBabsiriValues() {

    BOOST_TEST_MESSAGE("Testing barrier options against Babsiri's values...");

    using namespace barrier_option_test;

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
    ext::shared_ptr<SimpleQuote> underlying =
        ext::make_shared<SimpleQuote>(underlyingPrice);

    ext::shared_ptr<SimpleQuote> qH_SME = ext::make_shared<SimpleQuote>(q);
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qH_SME, dc);

    ext::shared_ptr<SimpleQuote> rH_SME = ext::make_shared<SimpleQuote>(r);
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rH_SME, dc);

    ext::shared_ptr<SimpleQuote> volatility =
        ext::make_shared<SimpleQuote>(0.10);
    ext::shared_ptr<BlackVolTermStructure> volTS =
        flatVol(today, volatility, dc);

    Date exDate = today+360;
    ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(exDate);

    for (auto& value : values) {
        volatility->setValue(value.volatility);

        ext::shared_ptr<StrikedTypePayoff> callPayoff =
            ext::make_shared<PlainVanillaPayoff>(Option::Call, value.strike);

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess =
            ext::make_shared<BlackScholesMertonProcess>(
                                      Handle<Quote>(underlying),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS));


        ext::shared_ptr<PricingEngine> engine =
            ext::make_shared<AnalyticBarrierEngine>(stochProcess);

        // analytic
        BarrierOption barrierCallOption(value.type, value.barrier, rebate, callPayoff, exercise);
        barrierCallOption.setPricingEngine(engine);
        Real calculated = barrierCallOption.NPV();
        Real expected = value.callValue;
        Real error = std::fabs(calculated-expected);
        Real maxErrorAllowed = 1.0e-5;
        if (error>maxErrorAllowed) {
            REPORT_FAILURE("value", value.type, value.barrier, rebate, callPayoff, exercise,
                           underlyingPrice, q, r, today, value.volatility, expected, calculated,
                           error, maxErrorAllowed);
        }

        Real maxMcRelativeErrorAllowed = 2.0e-2;

        ext::shared_ptr<PricingEngine> mcEngine =
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
            REPORT_FAILURE("value", value.type, value.barrier, rebate, callPayoff, exercise,
                           underlyingPrice, q, r, today, value.volatility, expected, calculated,
                           error, maxMcRelativeErrorAllowed);
        }
    }
}

void BarrierOptionTest::testBeagleholeValues() {

    BOOST_TEST_MESSAGE("Testing barrier options against Beaglehole's values...");

    using namespace barrier_option_test;

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

    ext::shared_ptr<SimpleQuote> underlying =
        ext::make_shared<SimpleQuote>(underlyingPrice);

    ext::shared_ptr<SimpleQuote> qH_SME = ext::make_shared<SimpleQuote>(q);
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qH_SME, dc);

    ext::shared_ptr<SimpleQuote> rH_SME = ext::make_shared<SimpleQuote>(r);
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rH_SME, dc);

    ext::shared_ptr<SimpleQuote> volatility =
        ext::make_shared<SimpleQuote>(0.10);
    ext::shared_ptr<BlackVolTermStructure> volTS =
        flatVol(today, volatility, dc);


    Date exDate = today+360;
    ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(exDate);

    for (auto& value : values) {
        volatility->setValue(value.volatility);

        ext::shared_ptr<StrikedTypePayoff> callPayoff =
            ext::make_shared<PlainVanillaPayoff>(Option::Call, value.strike);

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess =
            ext::make_shared<BlackScholesMertonProcess>(
                                      Handle<Quote>(underlying),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS));

        ext::shared_ptr<PricingEngine> engine =
            ext::make_shared<AnalyticBarrierEngine>(stochProcess);

        BarrierOption barrierCallOption(value.type, value.barrier, rebate, callPayoff, exercise);
        barrierCallOption.setPricingEngine(engine);
        Real calculated = barrierCallOption.NPV();
        Real expected = value.callValue;
        Real maxErrorAllowed = 1.0e-3;
        Real error = std::fabs(calculated-expected);
        if (error > maxErrorAllowed) {
            REPORT_FAILURE("value", value.type, value.barrier, rebate, callPayoff, exercise,
                           underlyingPrice, q, r, today, value.volatility, expected, calculated,
                           error, maxErrorAllowed);
        }

        Real maxMcRelativeErrorAllowed = 0.01;
        ext::shared_ptr<PricingEngine> mcEngine =
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
            REPORT_FAILURE("value", value.type, value.barrier, rebate, callPayoff, exercise,
                           underlyingPrice, q, r, today, value.volatility, expected, calculated,
                           error, maxMcRelativeErrorAllowed);
        }
    }
}

void BarrierOptionTest::testPerturbative() {
    BOOST_TEST_MESSAGE("Testing perturbative engine for barrier options...");

    Real S = 100.0;
    Real rebate = 0.0;
    Rate r = 0.03;
    Rate q = 0.02;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> underlying =
        ext::make_shared<SimpleQuote>(S);
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, q, dc);
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, r, dc);

    std::vector<Date> dates(2);
    std::vector<Volatility> vols(2);

    dates[0] = today + 90;  vols[0] = 0.105;
    dates[1] = today + 180; vols[1] = 0.11;

    ext::shared_ptr<BlackVolTermStructure> volTS =
        ext::make_shared<BlackVarianceCurve>(today, dates, vols, dc);

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess =
        ext::make_shared<BlackScholesMertonProcess>(
                                      Handle<Quote>(underlying),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS));

    Real strike = 101.0;
    Real barrier = 101.0;
    Date exDate = today+180;

    ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(exDate);
    ext::shared_ptr<StrikedTypePayoff> payoff =
        ext::make_shared<PlainVanillaPayoff>(Option::Put, strike);

    BarrierOption option(Barrier::UpOut, barrier, rebate, payoff, exercise);

    Natural order = 0;
    bool zeroGamma = false;
    ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<PerturbativeBarrierOptionEngine>(stochProcess,
                                                            order, zeroGamma);

    option.setPricingEngine(engine);

    Real calculated = option.NPV();
    Real expected = 0.897365;
    Real tolerance = 1.0e-6;
    if (std::fabs(calculated-expected) > tolerance) {
        BOOST_ERROR("Failed to reproduce expected value"
                    << "\n  calculated: " << std::setprecision(8) << calculated
                    << "\n  expected:   " << std::setprecision(8) << expected);
    }

    order = 1;
    engine = ext::make_shared<PerturbativeBarrierOptionEngine>(stochProcess,
                                                                 order,
                                                                 zeroGamma);

    option.setPricingEngine(engine);

    calculated = option.NPV();
    expected = 0.894374;
    if (std::fabs(calculated-expected) > tolerance) {
        BOOST_ERROR("Failed to reproduce expected value"
                    << "\n  calculated: " << std::setprecision(8) << calculated
                    << "\n  expected:   " << std::setprecision(8) << expected);
    }

    /* Too slow, skip
    order = 2;
    engine = ext::make_shared<PerturbativeBarrierOptionEngine>(stochProcess,
                                                                 order,
                                                                 zeroGamma);

    option.setPricingEngine(engine);

    calculated = option.NPV();
    expected = 0.8943769;
    if (std::fabs(calculated-expected) > tolerance) {
        BOOST_ERROR("Failed to reproduce expected value"
                    << "\n  calculated: " << std::setprecision(8) << calculated
                    << "\n  expected:   " << std::setprecision(8) << expected);
    }
    */
}

void BarrierOptionTest::testLocalVolAndHestonComparison() {
    BOOST_TEST_MESSAGE("Testing local volatility and Heston FD engines "
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
            ext::make_shared<ZeroCurve>(dates, rates, dayCounter));
    const Handle<YieldTermStructure> qTS(
                                   flatRate(settlementDate, 0.0, dayCounter));

    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(4500.00));
    
    const std::vector<Real> strikes = { 100 ,500 ,2000,3400,3600,3800,4000,4200,4400,4500,
                                        4600,4800,5000,5200,5400,5600,7500,10000,20000,30000 };
    
    const std::vector<Volatility> v =
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
    
    const ext::shared_ptr<BlackVarianceSurface> volTS =
        ext::make_shared<BlackVarianceSurface>(
                                 settlementDate, calendar,
                                 std::vector<Date>(dates.begin()+1,dates.end()),
                                 strikes, blackVolMatrix,
                                 dayCounter);
    volTS->setInterpolation<Bicubic>();
    const ext::shared_ptr<GeneralizedBlackScholesProcess> localVolProcess =
        ext::make_shared<BlackScholesMertonProcess>(
                                      s0, qTS, rTS, 
                                      Handle<BlackVolTermStructure>(volTS));
    
    const Real v0   =0.195662;
    const Real kappa=5.6628;
    const Real theta=0.0745911;
    const Real sigma=1.1619;
    const Real rho  =-0.511493;

    ext::shared_ptr<HestonProcess> hestonProcess =
        ext::make_shared<HestonProcess>(rTS, qTS, s0, v0,
                                          kappa, theta, sigma, rho);

    ext::shared_ptr<HestonModel> hestonModel =
        ext::make_shared<HestonModel>(hestonProcess);

    ext::shared_ptr<PricingEngine> fdHestonEngine =
        ext::make_shared<FdHestonBarrierEngine>(hestonModel, 100, 400, 50);
    
    ext::shared_ptr<PricingEngine> fdLocalVolEngine =
        ext::make_shared<FdBlackScholesBarrierEngine>(localVolProcess,
                                                        100, 400, 0,
                                                        FdmSchemeDesc::Douglas(), 
                                                        true, 0.35);
    
    const Real strike  = s0->value();
    const Real barrier = 3000;
    const Real rebate  = 100;
    const Date exDate  = settlementDate + Period(20, Months);
    
    const ext::shared_ptr<StrikedTypePayoff> payoff =
        ext::make_shared<PlainVanillaPayoff>(Option::Put, strike);

    const ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(exDate);

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
                   << "\n    calculated: " << calculatedLocalVolNPV
                   << "\n    expected:   " << expectedLocalVolNPV);
    }
}


void BarrierOptionTest::testVannaVolgaSimpleBarrierValues() {
    BOOST_TEST_MESSAGE("Testing barrier FX options against Vanna/Volga values...");

    using namespace barrier_option_test;

    SavedSettings backup;

    BarrierFxOptionData values[] = {

        //barrierType,barrier,rebate,type,strike,s,q,r,t,vol25Put,volAtm,vol25Call,vol, result, tol
        { Barrier::UpOut,1.5,0,     Option::Call,1.13321,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.11638,0.148127, 1.0e-4},
        { Barrier::UpOut,1.5,0,     Option::Call,1.22687,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.10088,0.075943, 1.0e-4},
        { Barrier::UpOut,1.5,0,     Option::Call,1.31179,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08925,0.0274771, 1.0e-4},
        { Barrier::UpOut,1.5,0,     Option::Call,1.38843,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08463,0.00573, 1.0e-4},
        { Barrier::UpOut,1.5,0,     Option::Call,1.46047,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08412,0.00012, 1.0e-4},

        { Barrier::UpOut,1.5,0,     Option::Put,1.13321,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.11638,0.00697606, 1.0e-4},
        { Barrier::UpOut,1.5,0,     Option::Put,1.22687,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.10088,0.020078, 1.0e-4},
        { Barrier::UpOut,1.5,0,     Option::Put,1.31179,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08925,0.0489395, 1.0e-4},
        { Barrier::UpOut,1.5,0,     Option::Put,1.38843,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08463,0.0969877, 1.0e-4},
        { Barrier::UpOut,1.5,0,     Option::Put,1.46047,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08412,0.157, 1.0e-4},

        { Barrier::UpIn,1.5,0,      Option::Call,1.13321,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.11638,0.0322202, 1.0e-4},
        { Barrier::UpIn,1.5,0,      Option::Call,1.22687,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.10088,0.0241491, 1.0e-4},
        { Barrier::UpIn,1.5,0,      Option::Call,1.31179,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08925,0.0164275, 1.0e-4},
        { Barrier::UpIn,1.5,0,      Option::Call,1.38843,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08463,0.01, 1.0e-4},
        { Barrier::UpIn,1.5,0,      Option::Call,1.46047,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08412,0.00489, 1.0e-4},

        { Barrier::UpIn,1.5,0,      Option::Put,1.13321,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.11638,0.000560713, 1.0e-4},
        { Barrier::UpIn,1.5,0,      Option::Put,1.22687,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.10088,0.000546804, 1.0e-4},
        { Barrier::UpIn,1.5,0,      Option::Put,1.31179,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08925,0.000130649, 1.0e-4},
        { Barrier::UpIn,1.5,0,      Option::Put,1.38843,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08463,0.000300828, 1.0e-4},
        { Barrier::UpIn,1.5,0,      Option::Put,1.46047,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08412,0.00135, 1.0e-4},

        { Barrier::DownOut,1.1,0,       Option::Call,1.13321,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.11638,0.17746, 1.0e-4},
        { Barrier::DownOut,1.1,0,       Option::Call,1.22687,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.10088,0.0994142, 1.0e-4},
        { Barrier::DownOut,1.1,0,       Option::Call,1.31179,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08925,0.0439, 1.0e-4},
        { Barrier::DownOut,1.1,0,       Option::Call,1.38843,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08463,0.01574, 1.0e-4},
        { Barrier::DownOut,1.1,0,       Option::Call,1.46047,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08412,0.00501, 1.0e-4},

        { Barrier::DownOut,1.3,0,       Option::Call,1.13321,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.11638,0.00612, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Call,1.22687,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.10088,0.00426, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Call,1.31179,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08925,0.00257, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Call,1.38843,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08463,0.00122, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Call,1.46047,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08412,0.00045, 1.0e-4},

        { Barrier::DownOut,1.1,0,       Option::Put,1.13321,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.11638,0.00022, 1.0e-4},
        { Barrier::DownOut,1.1,0,       Option::Put,1.22687,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.10088,0.00284, 1.0e-4},
        { Barrier::DownOut,1.1,0,       Option::Put,1.31179,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08925,0.02032, 1.0e-4},
        { Barrier::DownOut,1.1,0,       Option::Put,1.38843,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08463,0.058235, 1.0e-4},
        { Barrier::DownOut,1.1,0,       Option::Put,1.46047,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08412,0.109432, 1.0e-4},

        { Barrier::DownOut,1.3,0,       Option::Put,1.13321,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.11638,0, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Put,1.22687,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.10088,0, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Put,1.31179,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08925,0, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Put,1.38843,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08463,0.00017, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Put,1.46047,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08412,0.00083, 1.0e-4},

        { Barrier::DownIn,1.1,0,        Option::Call,1.13321,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.11638,0.00289, 1.0e-4},
        { Barrier::DownIn,1.1,0,        Option::Call,1.22687,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.10088,0.00067784, 1.0e-4},
        { Barrier::DownIn,1.1,0,        Option::Call,1.31179,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08925,0, 1.0e-4},
        { Barrier::DownIn,1.1,0,        Option::Call,1.38843,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08463,0, 1.0e-4},
        { Barrier::DownIn,1.1,0,        Option::Call,1.46047,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08412,0, 1.0e-4},

        { Barrier::DownIn,1.3,0,        Option::Call,1.13321,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.11638,0.17423, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Call,1.22687,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.10088,0.09584, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Call,1.31179,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08925,0.04133, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Call,1.38843,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08463,0.01452, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Call,1.46047,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08412,0.00456, 1.0e-4},

        { Barrier::DownIn,1.1,0,        Option::Put,1.13321,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.11638,0.00732, 1.0e-4},
        { Barrier::DownIn,1.1,0,        Option::Put,1.22687,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.10088,0.01778, 1.0e-4},
        { Barrier::DownIn,1.1,0,        Option::Put,1.31179,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08925,0.02875, 1.0e-4},
        { Barrier::DownIn,1.1,0,        Option::Put,1.38843,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08463,0.0390535, 1.0e-4},
        { Barrier::DownIn,1.1,0,        Option::Put,1.46047,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08412,0.0489236, 1.0e-4},

        { Barrier::DownIn,1.3,0,        Option::Put,1.13321,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.11638,0.00753, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Put,1.22687,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.10088,0.02062, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Put,1.31179,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08925,0.04907, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Put,1.38843,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08463,0.09711, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Put,1.46047,1.30265,0.0003541,0.0033871,1,0.10087,0.08925,0.08463,0.08412,0.15752, 1.0e-4},

        { Barrier::UpOut,1.6,0,     Option::Call,1.06145,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.12511,0.20493, 1.0e-4},
        { Barrier::UpOut,1.6,0,     Option::Call,1.19545,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.1089,0.105577, 1.0e-4},
        { Barrier::UpOut,1.6,0,     Option::Call,1.32238,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09444,0.0358872, 1.0e-4},
        { Barrier::UpOut,1.6,0,     Option::Call,1.44298,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09197,0.00634958, 1.0e-4},
        { Barrier::UpOut,1.6,0,     Option::Call,1.56345,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09261,0, 1.0e-4},

        { Barrier::UpOut,1.6,0,     Option::Put,1.06145,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.12511,0.0108218, 1.0e-4},
        { Barrier::UpOut,1.6,0,     Option::Put,1.19545,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.1089,0.0313339, 1.0e-4},
        { Barrier::UpOut,1.6,0,     Option::Put,1.32238,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09444,0.0751237, 1.0e-4},
        { Barrier::UpOut,1.6,0,     Option::Put,1.44298,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09197,0.153407, 1.0e-4},
        { Barrier::UpOut,1.6,0,     Option::Put,1.56345,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09261,0.253767, 1.0e-4},

        { Barrier::UpIn,1.6,0,      Option::Call,1.06145,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.12511,0.05402, 1.0e-4},
        { Barrier::UpIn,1.6,0,      Option::Call,1.19545,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.1089,0.0410069, 1.0e-4},
        { Barrier::UpIn,1.6,0,      Option::Call,1.32238,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09444,0.0279562, 1.0e-4},
        { Barrier::UpIn,1.6,0,      Option::Call,1.44298,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09197,0.0173055, 1.0e-4},
        { Barrier::UpIn,1.6,0,      Option::Call,1.56345,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09261,0.00764, 1.0e-4},

        { Barrier::UpIn,1.6,0,      Option::Put,1.06145,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.12511,0.000962737, 1.0e-4},
        { Barrier::UpIn,1.6,0,      Option::Put,1.19545,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.1089,0.00102637, 1.0e-4},
        { Barrier::UpIn,1.6,0,      Option::Put,1.32238,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09444,0.000419834, 1.0e-4},
        { Barrier::UpIn,1.6,0,      Option::Put,1.44298,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09197,0.00159277, 1.0e-4},
        { Barrier::UpIn,1.6,0,      Option::Put,1.56345,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09261,0.00473629, 1.0e-4},

        { Barrier::DownOut,1,0,     Option::Call,1.06145,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.12511,0.255098, 1.0e-4},
        { Barrier::DownOut,1,0,     Option::Call,1.19545,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.1089,0.145701, 1.0e-4},
        { Barrier::DownOut,1,0,     Option::Call,1.32238,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09444,0.06384, 1.0e-4},
        { Barrier::DownOut,1,0,     Option::Call,1.44298,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09197,0.02366, 1.0e-4},
        { Barrier::DownOut,1,0,     Option::Call,1.56345,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09261,0.00764, 1.0e-4},

        { Barrier::DownOut,1.3,0,       Option::Call,1.06145,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.12511,0.00592, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Call,1.19545,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.1089,0.00421, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Call,1.32238,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09444,0.00256, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Call,1.44298,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09197,0.0012, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Call,1.56345,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09261,0.0004, 1.0e-4},

        { Barrier::DownOut,1,0,     Option::Put,1.06145,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.12511,0, 1.0e-4},
        { Barrier::DownOut,1,0,     Option::Put,1.19545,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.1089,0.00280549, 1.0e-4},
        { Barrier::DownOut,1,0,     Option::Put,1.32238,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09444,0.0279945, 1.0e-4},
        { Barrier::DownOut,1,0,     Option::Put,1.44298,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09197,0.0896352, 1.0e-4},
        { Barrier::DownOut,1,0,     Option::Put,1.56345,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09261,0.175182, 1.0e-4},

        { Barrier::DownOut,1.3,0,       Option::Put,1.06145,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.12511,    0.00000, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Put,1.19545,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.1089,     0.00000, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Put,1.32238,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09444,    0.00000, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Put,1.44298,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09197,0.0002, 1.0e-4},
        { Barrier::DownOut,1.3,0,       Option::Put,1.56345,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09261,0.00096, 1.0e-4},

        { Barrier::DownIn,1,0,      Option::Call,1.06145,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.12511,0.00384783, 1.0e-4},
        { Barrier::DownIn,1,0,      Option::Call,1.19545,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.1089,0.000883232, 1.0e-4},
        { Barrier::DownIn,1,0,      Option::Call,1.32238,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09444,0, 1.0e-4},
        { Barrier::DownIn,1,0,      Option::Call,1.44298,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09197,   0.00000, 1.0e-4},
        { Barrier::DownIn,1,0,      Option::Call,1.56345,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09261,   0.00000, 1.0e-4},

        { Barrier::DownIn,1.3,0,        Option::Call,1.06145,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.12511,0.25302, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Call,1.19545,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.1089,0.14238, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Call,1.32238,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09444,0.06128, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Call,1.44298,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09197,0.02245, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Call,1.56345,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09261,0.00725, 1.0e-4},

        { Barrier::DownIn,1,0,      Option::Put,1.06145,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.12511,0.01178, 1.0e-4},
        { Barrier::DownIn,1,0,      Option::Put,1.19545,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.1089,0.0295548, 1.0e-4},
        { Barrier::DownIn,1,0,      Option::Put,1.32238,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09444,0.047549, 1.0e-4},
        { Barrier::DownIn,1,0,      Option::Put,1.44298,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09197,0.0653642, 1.0e-4},
        { Barrier::DownIn,1,0,      Option::Put,1.56345,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09261,0.0833221, 1.0e-4},

        { Barrier::DownIn,1.3,0,        Option::Put,1.06145,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.12511,0.01178, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Put,1.19545,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.1089,0.03236, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Put,1.32238,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09444,0.07554, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Put,1.44298,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09197,0.15479, 1.0e-4},
        { Barrier::DownIn,1.3,0,        Option::Put,1.56345,1.30265,0.0009418,0.0039788,2,0.10891,0.09525,0.09197,0.09261,0.25754, 1.0e-4},

    };

    DayCounter dc = Actual365Fixed();
    Date today(5, March, 2013);
    Settings::instance().evaluationDate() = today;

    ext::shared_ptr<SimpleQuote> spot = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<SimpleQuote> qRate = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol25Put = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<SimpleQuote> volAtm = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<SimpleQuote> vol25Call = ext::make_shared<SimpleQuote>(0.0);

    for (auto& value : values) {

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol25Put->setValue(value.vol25Put);
        volAtm->setValue(value.volAtm);
        vol25Call->setValue(value.vol25Call);

        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::make_shared<PlainVanillaPayoff>(value.type, value.strike);

        Date exDate = today + timeToDays(value.t, 365);
        ext::shared_ptr<Exercise> exercise =
            ext::make_shared<EuropeanExercise>(exDate);

        Handle<DeltaVolQuote> volAtmQuote = Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(
            Handle<Quote>(volAtm), DeltaVolQuote::Fwd, value.t, DeltaVolQuote::AtmDeltaNeutral));

        Handle<DeltaVolQuote> vol25PutQuote(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(
            -0.25, Handle<Quote>(vol25Put), value.t, DeltaVolQuote::Fwd)));

        Handle<DeltaVolQuote> vol25CallQuote(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(
            0.25, Handle<Quote>(vol25Call), value.t, DeltaVolQuote::Fwd)));

        BarrierOption barrierOption(value.barrierType, value.barrier, value.rebate, payoff,
                                    exercise);

        Real bsVanillaPrice =
            blackFormula(value.type, value.strike,
                         spot->value() * qTS->discount(value.t) / rTS->discount(value.t),
                         value.v * std::sqrt(value.t), rTS->discount(value.t));
        ext::shared_ptr<PricingEngine> vannaVolgaEngine =
            ext::make_shared<VannaVolgaBarrierEngine>(
                            volAtmQuote,
							vol25PutQuote,
							vol25CallQuote,
							Handle<Quote> (spot),
							Handle<YieldTermStructure> (rTS),
							Handle<YieldTermStructure> (qTS),
							true,
							bsVanillaPrice);
        barrierOption.setPricingEngine(vannaVolgaEngine);

        Real calculated = barrierOption.NPV();
        Real expected = value.result;
        Real error = std::fabs(calculated-expected);
        if (error > value.tol) {
            REPORT_FX_FAILURE("value", value.barrierType, value.barrier, value.rebate, payoff,
                              exercise, value.s, value.q, value.r, today, value.vol25Put,
                              value.volAtm, value.vol25Call, value.v, expected, calculated, error,
                              value.tol);
        }
    }
}

void BarrierOptionTest::testOldDividendBarrierOption() {
    BOOST_TEST_MESSAGE("Testing old-style barrier option pricing with discrete dividends...");

    SavedSettings backup;

    const DayCounter dc = Actual365Fixed();

    const Date today(11, February, 2018);
    const Date maturity = today + Period(1, Years);
    Settings::instance().evaluationDate() = today;

    const Real spot = 100.0;
    const Real strike = 105.0;
    const Real rebate = 5.0;

    const Real barriers[] = { 80.0, 120.0 };
    const Barrier::Type barrierTypes[] = { Barrier::DownOut, Barrier::UpOut };

    const Rate r = 0.05;
    const Rate q = 0.0;
    const Volatility v = 0.02;

    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(spot));
    const Handle<YieldTermStructure> qTS(flatRate(today, q, dc));
    const Handle<YieldTermStructure> rTS(flatRate(today, r, dc));
    const Handle<BlackVolTermStructure> volTS(flatVol(today, v, dc));

    const ext::shared_ptr<BlackScholesMertonProcess> bsProcess =
        ext::make_shared<BlackScholesMertonProcess>(s0, qTS, rTS, volTS);

    const ext::shared_ptr<PricingEngine> douglas =
        ext::make_shared<FdBlackScholesBarrierEngine>(
            bsProcess, 100, 100, 0, FdmSchemeDesc::Douglas());

    const ext::shared_ptr<PricingEngine> crankNicolson =
        ext::make_shared<FdBlackScholesBarrierEngine>(
            bsProcess, 100, 100, 0, FdmSchemeDesc::CrankNicolson());

    const ext::shared_ptr<PricingEngine> craigSnyed =
        ext::make_shared<FdBlackScholesBarrierEngine>(
            bsProcess, 100, 100, 0, FdmSchemeDesc::CraigSneyd());

    const ext::shared_ptr<PricingEngine> hundsdorfer =
        ext::make_shared<FdBlackScholesBarrierEngine>(
            bsProcess, 100, 100, 0, FdmSchemeDesc::Hundsdorfer());

    const ext::shared_ptr<PricingEngine> mol =
        ext::make_shared<FdBlackScholesBarrierEngine>(
            bsProcess, 100, 100, 0, FdmSchemeDesc::MethodOfLines());

    const ext::shared_ptr<PricingEngine> trPDF2 =
        ext::make_shared<FdBlackScholesBarrierEngine>(
            bsProcess, 100, 100, 0, FdmSchemeDesc::TrBDF2());

    const ext::shared_ptr<PricingEngine> hestonEngine =
        ext::make_shared<FdHestonBarrierEngine>(
            ext::make_shared<HestonModel>(
                ext::make_shared<HestonProcess>(
                    rTS, qTS, s0, v*v, 1.0, v*v, 0.005, 0.0)), 50, 101, 3);

    const ext::shared_ptr<PricingEngine> engines[] = {
        douglas, crankNicolson,
        trPDF2, craigSnyed, hundsdorfer, mol, hestonEngine
    };

    const ext::shared_ptr<StrikedTypePayoff> payoff =
        ext::make_shared<PlainVanillaPayoff>(Option::Put, strike);

    const ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(maturity);

    const Real divAmount = 30;
    const Date divDate = today + Period(6, Months);

    const Real expected[] = {
        rTS->discount(divDate)*rebate,
        (*payoff)(
            (spot - divAmount*rTS->discount(divDate))/rTS->discount(maturity))
            *rTS->discount(maturity)
    };

    const Real relTol = 1e-4;
    for (Size i=0; i < LENGTH(barriers); ++i) {
        for (const auto& engine : engines) {
            const Real barrier = barriers[i];
            const Barrier::Type barrierType = barrierTypes[i];

            QL_DEPRECATED_DISABLE_WARNING

            DividendBarrierOption barrierOption(
                barrierType, barrier, rebate, payoff, exercise,
                std::vector<Date>(1, divDate),
                std::vector<Real>(1, divAmount));

            QL_DEPRECATED_ENABLE_WARNING

            barrierOption.setPricingEngine(engine);

            const Real calculated = barrierOption.NPV();

            const Real diff = std::fabs(calculated - expected[i]);
            if (diff > relTol*expected[i]) {
                BOOST_FAIL("Failed to reproduce barrier price with "
                        "discrete dividends for "
                           << "\n    strike:     " << strike
                           << "\n    barrier:    " << barrier
                           << "\n    maturity:   " << maturity
                           << "\n    calculated: " << calculated
                           << "\n    expected:   " << expected[i]
                           << std::scientific
                           << "\n    difference  " << diff
                           << "\n    tolerance   " << relTol * expected[i]);
            }
        }
    }
}

void BarrierOptionTest::testDividendBarrierOption() {
    BOOST_TEST_MESSAGE("Testing barrier option pricing with discrete dividends...");

    SavedSettings backup;

    DayCounter dc = Actual365Fixed();

    Date today(11, February, 2018);
    Date maturity = today + Period(1, Years);
    Settings::instance().evaluationDate() = today;

    Real spot = 100.0;
    Real strike = 105.0;
    Real rebate = 5.0;

    Real barriers[] = { 80.0, 120.0, 80.0, 120.0 };
    Barrier::Type barrierTypes[] = { Barrier::DownOut, Barrier::UpOut, Barrier::DownIn, Barrier::UpIn };

    Rate r = 0.05;
    Rate q = 0.0;
    Volatility v = 0.02;

    Handle<Quote> s0(ext::make_shared<SimpleQuote>(spot));
    Handle<YieldTermStructure> qTS(flatRate(today, q, dc));
    Handle<YieldTermStructure> rTS(flatRate(today, r, dc));
    Handle<BlackVolTermStructure> volTS(flatVol(today, v, dc));

    auto bsProcess = ext::make_shared<BlackScholesMertonProcess>(s0, qTS, rTS, volTS);

    Real divAmount = 30;
    Date divDate = today + Period(6, Months);
    auto dividends = DividendVector({ divDate }, { divAmount });

    auto douglas =
        ext::make_shared<FdBlackScholesBarrierEngine>(
            bsProcess, dividends, 100, 100, 0, FdmSchemeDesc::Douglas());

    auto crankNicolson =
        ext::make_shared<FdBlackScholesBarrierEngine>(
            bsProcess, dividends, 100, 100, 0, FdmSchemeDesc::CrankNicolson());

    auto craigSnyed =
        ext::make_shared<FdBlackScholesBarrierEngine>(
            bsProcess, dividends, 100, 100, 0, FdmSchemeDesc::CraigSneyd());

    auto hundsdorfer =
        ext::make_shared<FdBlackScholesBarrierEngine>(
            bsProcess, dividends, 100, 100, 0, FdmSchemeDesc::Hundsdorfer());

    auto mol =
        ext::make_shared<FdBlackScholesBarrierEngine>(
            bsProcess, dividends, 100, 100, 0, FdmSchemeDesc::MethodOfLines());

    auto trPDF2 =
        ext::make_shared<FdBlackScholesBarrierEngine>(
            bsProcess, dividends, 100, 100, 0, FdmSchemeDesc::TrBDF2());

    auto hestonEngine =
        ext::make_shared<FdHestonBarrierEngine>(
            ext::make_shared<HestonModel>(
                ext::make_shared<HestonProcess>(
                    rTS, qTS, s0, v*v, 1.0, v*v, 0.005, 0.0)),
            dividends, 50, 101, 3);

    ext::shared_ptr<PricingEngine> engines[] = {
        douglas, crankNicolson,
        trPDF2, craigSnyed, hundsdorfer, mol, hestonEngine
    };

    auto payoff = ext::make_shared<PlainVanillaPayoff>(Option::Put, strike);
    auto exercise = ext::make_shared<EuropeanExercise>(maturity);

    Real expected[] = {
        rTS->discount(divDate)*rebate,
        (*payoff)(
            (spot - divAmount*rTS->discount(divDate))/rTS->discount(maturity))
            *rTS->discount(maturity),
        29.154,
        4.765
    };

    Real relTol = 2e-4;
    for (Size i=0; i < LENGTH(barriers); ++i) {
        for (Size j=0; j < LENGTH(engines); ++j) {
            Real barrier = barriers[i];
            Barrier::Type barrierType = barrierTypes[i];

            BarrierOption barrierOption(barrierType, barrier, rebate, payoff, exercise);
            barrierOption.setPricingEngine(engines[j]);

            Real calculated = barrierOption.NPV();
            Real diff = std::fabs(calculated - expected[i]);
            if (diff > relTol*expected[i]) {
                BOOST_ERROR("Failed to reproduce barrier price with discrete dividends:"
                            << "\n    engine:     " << j
                            << "\n    strike:     " << strike
                            << "\n    barrier:    " << barrier
                            << "\n    maturity:   " << maturity
                            << "\n    calculated: " << calculated
                            << "\n    expected:   " << expected[i]
                            << std::scientific
                            << "\n    difference  " << diff
                            << "\n    tolerance   " << relTol * expected[i]);
            }
        }
    }
}


void BarrierOptionTest::testDividendBarrierOptionWithDividendsPastMaturity() {
    BOOST_TEST_MESSAGE("Testing barrier option pricing with discrete dividends past maturity...");

    SavedSettings backup;

    DayCounter dc = Actual365Fixed();

    Date today(11, February, 2018);
    Date maturity = today + Period(1, Years);
    Settings::instance().evaluationDate() = today;

    Real spot = 100.0;
    Real strike = 105.0;
    Real rebate = 5.0;

    Real barriers[] = { 90.0, 110.0 };
    Barrier::Type barrierTypes[] = { Barrier::DownOut, Barrier::UpOut };

    Rate r = 0.05;
    Rate q = 0.0;
    Volatility v = 0.02;

    Handle<Quote> s0(ext::make_shared<SimpleQuote>(spot));
    Handle<YieldTermStructure> qTS(flatRate(today, q, dc));
    Handle<YieldTermStructure> rTS(flatRate(today, r, dc));
    Handle<BlackVolTermStructure> volTS(flatVol(today, v, dc));

    auto bsProcess = ext::make_shared<BlackScholesMertonProcess>(s0, qTS, rTS, volTS);

    Real divAmount = 30;
    Date divDate = today + Period(18, Months);
    auto dividends = DividendVector({ divDate }, { divAmount });

    ext::shared_ptr<PricingEngine> engines[] = {
        ext::make_shared<FdBlackScholesBarrierEngine>(
            bsProcess, 100, 100, 0, FdmSchemeDesc::Douglas()),
        ext::make_shared<FdHestonBarrierEngine>(
            ext::make_shared<HestonModel>(
                ext::make_shared<HestonProcess>(
                    rTS, qTS, s0, v*v, 1.0, v*v, 0.005, 0.0)),
            50, 101, 3)        
    };

    ext::shared_ptr<PricingEngine> enginesWithDividends[] = {
        ext::make_shared<FdBlackScholesBarrierEngine>(
            bsProcess, dividends, 100, 100, 0, FdmSchemeDesc::Douglas()),
        ext::make_shared<FdHestonBarrierEngine>(
            ext::make_shared<HestonModel>(
                ext::make_shared<HestonProcess>(
                    rTS, qTS, s0, v*v, 1.0, v*v, 0.005, 0.0)),
            dividends, 50, 101, 3)        
    };

    auto payoff = ext::make_shared<PlainVanillaPayoff>(Option::Put, strike);
    auto exercise = ext::make_shared<EuropeanExercise>(maturity);

    for (Size i=0; i < LENGTH(barriers); ++i) {
        for (Size j=0; j < LENGTH(engines); ++j) {
            Real barrier = barriers[i];
            Barrier::Type barrierType = barrierTypes[i];

            BarrierOption barrierOption(barrierType, barrier, rebate, payoff, exercise);

            barrierOption.setPricingEngine(engines[j]);
            Real withoutDividends = barrierOption.NPV();

            barrierOption.setPricingEngine(enginesWithDividends[j]);
            Real withDividends = barrierOption.NPV();

            Real diff = std::fabs(withDividends - withoutDividends);
            Real tolerance = 1e-12;
            if (diff > tolerance) {
                BOOST_ERROR("Dividends past maturity affected option price:"
                            << "\n    engine:           " << j
                            << "\n    strike:           " << strike
                            << "\n    barrier:          " << barrier
                            << "\n    maturity:         " << maturity
                            << "\n    without dividend: " << withoutDividends
                            << "\n    with dividend:    " << withDividends
                            << std::scientific
                            << "\n    difference        " << diff);
            }
        }
    }
}

void BarrierOptionTest::testBarrierAndDividendEngine() {
    BOOST_TEST_MESSAGE("Testing the use of a single engine for barrier and dividend options...");

    SavedSettings backup;

    auto today = Date(1, January, 2023);
    Settings::instance().evaluationDate() = today;

    auto u = Handle<Quote>(ext::make_shared<SimpleQuote>(100.0));
    auto r = Handle<YieldTermStructure>(ext::make_shared<FlatForward>(today, 0.01, Actual360()));
    auto sigma = Handle<BlackVolTermStructure>(
        ext::make_shared<BlackConstantVol>(today, TARGET(), 0.20, Actual360()));
    auto process = ext::make_shared<BlackScholesProcess>(u, r, sigma);

    auto engine = ext::make_shared<FdBlackScholesBarrierEngine>(process);

    auto payoff = ext::make_shared<PlainVanillaPayoff>(Option::Call, 90.0);

    auto option1 = BarrierOption(Barrier::DownIn, 80.0, 0.0, payoff,
                                 ext::make_shared<EuropeanExercise>(Date(1, June, 2023)));
    QL_DEPRECATED_DISABLE_WARNING
    auto option2 = DividendBarrierOption(Barrier::DownIn, 80.0, 0.0, payoff,
                                         ext::make_shared<EuropeanExercise>(Date(1, June, 2023)),
                                         {Date(1, February, 2023)}, {1.0});
    QL_DEPRECATED_ENABLE_WARNING

    option1.setPricingEngine(engine);
    option2.setPricingEngine(engine);

    auto npv_before = option1.NPV();
    option2.NPV();

    option1.recalculate();
    auto npv_after = option1.NPV();

    if (npv_after != npv_before) {
        BOOST_FAIL("Failed to price barrier option correctly "
                   "after using the engine on a dividend option: "
                   << "\n    before usage: " << npv_before << "\n    after usage:  " << npv_after);
    }
}

test_suite* BarrierOptionTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Barrier option tests");
    suite->add(QUANTLIB_TEST_CASE(&BarrierOptionTest::testParity));
    suite->add(QUANTLIB_TEST_CASE(&BarrierOptionTest::testHaugValues));
    suite->add(QUANTLIB_TEST_CASE(&BarrierOptionTest::testBabsiriValues));
    suite->add(QUANTLIB_TEST_CASE(&BarrierOptionTest::testBeagleholeValues));
    suite->add(QUANTLIB_TEST_CASE(&BarrierOptionTest::testLocalVolAndHestonComparison));
    suite->add(QUANTLIB_TEST_CASE(&BarrierOptionTest::testOldDividendBarrierOption));
    suite->add(QUANTLIB_TEST_CASE(&BarrierOptionTest::testDividendBarrierOption));
    suite->add(QUANTLIB_TEST_CASE(&BarrierOptionTest::testDividendBarrierOptionWithDividendsPastMaturity));
    suite->add(QUANTLIB_TEST_CASE(&BarrierOptionTest::testBarrierAndDividendEngine));
    return suite;
}

test_suite* BarrierOptionTest::experimental() {
    auto* suite = BOOST_TEST_SUITE("Barrier option experimental tests");
    suite->add(QUANTLIB_TEST_CASE(&BarrierOptionTest::testPerturbative));
    suite->add(QUANTLIB_TEST_CASE(&BarrierOptionTest::testVannaVolgaSimpleBarrierValues));
    return suite;
}
