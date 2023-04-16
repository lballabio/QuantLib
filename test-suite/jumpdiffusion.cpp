/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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

#include "jumpdiffusion.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/jumpdiffusionengine.hpp>
#include <ql/processes/merton76process.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#undef REPORT_FAILURE_1
#define REPORT_FAILURE_1(greekName, payoff, exercise, s, q, r, today, v, \
                         intensity, meanLogJump, jumpVol, expected, \
                         calculated, error, tolerance) \
    BOOST_FAIL(exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    underlying value: " << s << "\n" \
               << "    strike:           " << payoff->strike() <<"\n" \
               << "    dividend yield:   " << io::rate(q) << "\n" \
               << "    risk-free rate:   " << io::rate(r) << "\n" \
               << "    reference date:   " << today << "\n" \
               << "    maturity:         " << exercise->lastDate() << "\n" \
               << "    volatility:       " << io::volatility(v) << "\n\n" \
               << "    intensity:        " << intensity << "\n" \
               << "    mean log-jump:    " << meanLogJump << "\n" \
               << "    jump volatility:  " << jumpVol << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        "  << tolerance);

#undef REPORT_FAILURE_2
#define REPORT_FAILURE_2(greekName, payoff, exercise, s, q, r, today, v, \
                         intensity, gamma, expected, calculated, \
                         error, tolerance) \
    BOOST_FAIL(exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    underlying value: " << s << "\n" \
               << "    strike:           " << payoff->strike() <<"\n" \
               << "    dividend yield:   " << io::rate(q) << "\n" \
               << "    risk-free rate:   " << io::rate(r) << "\n" \
               << "    reference date:   " << today << "\n" \
               << "    maturity:         " << exercise->lastDate() << "\n" \
               << "    volatility:       " << io::volatility(v) << "\n" \
               << "    intensity:        " << intensity << "\n" \
               << "    gamma:            " << gamma << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);

namespace {

    struct HaugMertonData {
        Option::Type type;
        Real strike;
        Real s;        // spot
        Rate q;        // dividend
        Rate r;        // risk-free rate
        Time t;        // time to maturity
        Volatility v;  // volatility
        Real jumpIntensity;
        Real gamma;
        Real result;   // result
        Real tol;      // tolerance
    };

}


void JumpDiffusionTest::testMerton76() {

    BOOST_TEST_MESSAGE("Testing Merton 76 jump-diffusion model "
                       "for European options...");

    SavedSettings backup;

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998, pag 9

       Haug use the arbitrary truncation criterium of 11 terms in the sum,
       which doesn't guarantee convergence up to 1e-2.
       Using Haug's criterium Haug's values have been correctly reproduced.
       the following values have the right 1e-2 accuracy: any value different
       from Haug has been noted.
    */
    HaugMertonData values[] = {
        //        type, strike,   spot,    q,    r,    t,  vol, int, gamma, value, tol
        // gamma = 0.25, strike = 80
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.10, 0.25, 1.0,  0.25, 20.67, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.25, 0.25, 1.0,  0.25, 21.74, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.50, 0.25, 1.0,  0.25, 23.63, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.10, 0.25, 5.0,  0.25, 20.65, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.25, 0.25, 5.0,  0.25, 21.70, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.50, 0.25, 5.0,  0.25, 23.61, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.10, 0.25,10.0,  0.25, 20.64, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.25, 0.25,10.0,  0.25, 21.70, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.50, 0.25,10.0,  0.25, 23.61, 1e-2 }, // Haug 23.28
        // gamma = 0.25, strike = 90
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.10, 0.25, 1.0,  0.25, 11.00, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.25, 0.25, 1.0,  0.25, 12.74, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.50, 0.25, 1.0,  0.25, 15.40, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.10, 0.25, 5.0,  0.25, 10.98, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.25, 0.25, 5.0,  0.25, 12.75, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.50, 0.25, 5.0,  0.25, 15.42, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.10, 0.25,10.0,  0.25, 10.98, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.25, 0.25,10.0,  0.25, 12.75, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.50, 0.25,10.0,  0.25, 15.42, 1e-2 }, // Haug 15.20
        // gamma = 0.25, strike = 100
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.10, 0.25, 1.0,  0.25,  3.42, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.25, 0.25, 1.0,  0.25,  5.88, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.50, 0.25, 1.0,  0.25,  8.95, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.10, 0.25, 5.0,  0.25,  3.51, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.25, 0.25, 5.0,  0.25,  5.96, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.50, 0.25, 5.0,  0.25,  9.02, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.10, 0.25,10.0,  0.25,  3.53, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.25, 0.25,10.0,  0.25,  5.97, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.50, 0.25,10.0,  0.25,  9.03, 1e-2 }, // Haug 8.89
        // gamma = 0.25, strike = 110
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.10, 0.25, 1.0,  0.25,  0.55, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.25, 0.25, 1.0,  0.25,  2.11, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.50, 0.25, 1.0,  0.25,  4.67, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.10, 0.25, 5.0,  0.25,  0.56, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.25, 0.25, 5.0,  0.25,  2.16, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.50, 0.25, 5.0,  0.25,  4.73, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.10, 0.25,10.0,  0.25,  0.56, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.25, 0.25,10.0,  0.25,  2.17, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.50, 0.25,10.0,  0.25,  4.74, 1e-2 }, // Haug 4.66
        // gamma = 0.25, strike = 120
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.10, 0.25, 1.0,  0.25,  0.10, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.25, 0.25, 1.0,  0.25,  0.64, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.50, 0.25, 1.0,  0.25,  2.23, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.10, 0.25, 5.0,  0.25,  0.06, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.25, 0.25, 5.0,  0.25,  0.63, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.50, 0.25, 5.0,  0.25,  2.25, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.10, 0.25,10.0,  0.25,  0.05, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.25, 0.25,10.0,  0.25,  0.62, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.50, 0.25,10.0,  0.25,  2.25, 1e-2 }, // Haug 2.21

        // gamma = 0.50, strike = 80
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.10, 0.25, 1.0,  0.50, 20.72, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.25, 0.25, 1.0,  0.50, 21.83, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.50, 0.25, 1.0,  0.50, 23.71, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.10, 0.25, 5.0,  0.50, 20.66, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.25, 0.25, 5.0,  0.50, 21.73, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.50, 0.25, 5.0,  0.50, 23.63, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.10, 0.25,10.0,  0.50, 20.65, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.25, 0.25,10.0,  0.50, 21.71, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.50, 0.25,10.0,  0.50, 23.61, 1e-2 }, // Haug 23.28
        // gamma = 0.50, strike = 90
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.10, 0.25, 1.0,  0.50, 11.04, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.25, 0.25, 1.0,  0.50, 12.72, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.50, 0.25, 1.0,  0.50, 15.34, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.10, 0.25, 5.0,  0.50, 11.02, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.25, 0.25, 5.0,  0.50, 12.76, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.50, 0.25, 5.0,  0.50, 15.41, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.10, 0.25,10.0,  0.50, 11.00, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.25, 0.25,10.0,  0.50, 12.75, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.50, 0.25,10.0,  0.50, 15.41, 1e-2 }, // Haug 15.18
        // gamma = 0.50, strike = 100
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.10, 0.25, 1.0,  0.50,  3.14, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.25, 0.25, 1.0,  0.50,  5.58, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.50, 0.25, 1.0,  0.50,  8.71, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.10, 0.25, 5.0,  0.50,  3.39, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.25, 0.25, 5.0,  0.50,  5.87, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.50, 0.25, 5.0,  0.50,  8.96, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.10, 0.25,10.0,  0.50,  3.46, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.25, 0.25,10.0,  0.50,  5.93, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.50, 0.25,10.0,  0.50,  9.00, 1e-2 }, // Haug 8.85
        // gamma = 0.50, strike = 110
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.10, 0.25, 1.0,  0.50,  0.53, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.25, 0.25, 1.0,  0.50,  1.93, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.50, 0.25, 1.0,  0.50,  4.42, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.10, 0.25, 5.0,  0.50,  0.58, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.25, 0.25, 5.0,  0.50,  2.11, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.50, 0.25, 5.0,  0.50,  4.67, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.10, 0.25,10.0,  0.50,  0.57, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.25, 0.25,10.0,  0.50,  2.14, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.50, 0.25,10.0,  0.50,  4.71, 1e-2 }, // Haug 4.62
        // gamma = 0.50, strike = 120
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.10, 0.25, 1.0,  0.50,  0.19, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.25, 0.25, 1.0,  0.50,  0.71, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.50, 0.25, 1.0,  0.50,  2.15, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.10, 0.25, 5.0,  0.50,  0.10, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.25, 0.25, 5.0,  0.50,  0.66, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.50, 0.25, 5.0,  0.50,  2.23, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.10, 0.25,10.0,  0.50,  0.07, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.25, 0.25,10.0,  0.50,  0.64, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.50, 0.25,10.0,  0.50,  2.24, 1e-2 }, // Haug 2.19

        // gamma = 0.75, strike = 80
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.10, 0.25, 1.0,  0.75, 20.79, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.25, 0.25, 1.0,  0.75, 21.96, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.50, 0.25, 1.0,  0.75, 23.86, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.10, 0.25, 5.0,  0.75, 20.68, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.25, 0.25, 5.0,  0.75, 21.78, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.50, 0.25, 5.0,  0.75, 23.67, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.10, 0.25,10.0,  0.75, 20.66, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.25, 0.25,10.0,  0.75, 21.74, 1e-2 },
        { Option::Call,  80.00, 100.00, 0.00, 0.08, 0.50, 0.25,10.0,  0.75, 23.64, 1e-2 }, // Haug 23.30
        // gamma = 0.75, strike = 90
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.10, 0.25, 1.0,  0.75, 11.11, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.25, 0.25, 1.0,  0.75, 12.75, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.50, 0.25, 1.0,  0.75, 15.30, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.10, 0.25, 5.0,  0.75, 11.09, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.25, 0.25, 5.0,  0.75, 12.78, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.50, 0.25, 5.0,  0.75, 15.39, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.10, 0.25,10.0,  0.75, 11.04, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.25, 0.25,10.0,  0.75, 12.76, 1e-2 },
        { Option::Call,  90.00, 100.00, 0.00, 0.08, 0.50, 0.25,10.0,  0.75, 15.40, 1e-2 }, // Haug 15.17
        // gamma = 0.75, strike = 100
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.10, 0.25, 1.0,  0.75,  2.70, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.25, 0.25, 1.0,  0.75,  5.08, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.50, 0.25, 1.0,  0.75,  8.24, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.10, 0.25, 5.0,  0.75,  3.16, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.25, 0.25, 5.0,  0.75,  5.71, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.50, 0.25, 5.0,  0.75,  8.85, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.10, 0.25,10.0,  0.75,  3.33, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.25, 0.25,10.0,  0.75,  5.85, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.00, 0.08, 0.50, 0.25,10.0,  0.75,  8.95, 1e-2 }, // Haug 8.79
        // gamma = 0.75, strike = 110
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.10, 0.25, 1.0,  0.75,  0.54, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.25, 0.25, 1.0,  0.75,  1.69, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.50, 0.25, 1.0,  0.75,  3.99, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.10, 0.25, 5.0,  0.75,  0.62, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.25, 0.25, 5.0,  0.75,  2.05, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.50, 0.25, 5.0,  0.75,  4.57, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.10, 0.25,10.0,  0.75,  0.60, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.25, 0.25,10.0,  0.75,  2.11, 1e-2 },
        { Option::Call, 110.00, 100.00, 0.00, 0.08, 0.50, 0.25,10.0,  0.75,  4.66, 1e-2 }, // Haug 4.56
        // gamma = 0.75, strike = 120
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.10, 0.25, 1.0,  0.75,  0.29, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.25, 0.25, 1.0,  0.75,  0.84, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.50, 0.25, 1.0,  0.75,  2.09, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.10, 0.25, 5.0,  0.75,  0.15, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.25, 0.25, 5.0,  0.75,  0.71, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.50, 0.25, 5.0,  0.75,  2.21, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.10, 0.25,10.0,  0.75,  0.11, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.25, 0.25,10.0,  0.75,  0.67, 1e-2 },
        { Option::Call, 120.00, 100.00, 0.00, 0.08, 0.50, 0.25,10.0,  0.75,  2.23, 1e-2 }  // Haug 2.17
};



    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    std::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    std::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    std::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    std::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    std::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    std::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    std::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    std::shared_ptr<SimpleQuote> jumpIntensity(new SimpleQuote(0.0));
    std::shared_ptr<SimpleQuote> meanLogJump(new SimpleQuote(0.0));
    std::shared_ptr<SimpleQuote> jumpVol(new SimpleQuote(0.0));

    std::shared_ptr<Merton76Process> stochProcess(
           new Merton76Process(Handle<Quote>(spot),
                               Handle<YieldTermStructure>(qTS),
                               Handle<YieldTermStructure>(rTS),
                               Handle<BlackVolTermStructure>(volTS),
                               Handle<Quote>(jumpIntensity),
                               Handle<Quote>(meanLogJump),
                               Handle<Quote>(jumpVol)));
    std::shared_ptr<PricingEngine> engine(
                                       new JumpDiffusionEngine(stochProcess));

    for (auto& value : values) {

        std::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(value.type, value.strike));

        Date exDate = today + timeToDays(value.t);
        std::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);


        jumpIntensity->setValue(value.jumpIntensity);

        // delta in Haug's notation
        Real jVol = value.v * std::sqrt(value.gamma / value.jumpIntensity);
        jumpVol->setValue(jVol);

        // z in Haug's notation
        Real diffusionVol = value.v * std::sqrt(1.0 - value.gamma);
        vol  ->setValue(diffusionVol);

        // Haug is assuming zero meanJump
        Real meanJump = 0.0;
        meanLogJump->setValue(std::log(1.0+meanJump)-0.5*jVol*jVol);

        Volatility totalVol =
            std::sqrt(value.jumpIntensity * jVol * jVol + diffusionVol * diffusionVol);
        Volatility volError = std::fabs(totalVol - value.v);
        QL_REQUIRE(volError<1e-13,
                   volError << " mismatch");

        EuropeanOption option(payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated - value.result);
        if (error > value.tol) {
            REPORT_FAILURE_2("value", payoff, exercise, value.s, value.q, value.r, today, value.v,
                             value.jumpIntensity, value.gamma, value.result, calculated, error,
                             value.tol);
        }
    }
}

void JumpDiffusionTest::testGreeks() {

    BOOST_TEST_MESSAGE("Testing jump-diffusion option greeks...");

    SavedSettings backup;

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"]  = 1.0e-4;
    tolerance["gamma"]  = 1.0e-4;
    tolerance["theta"]  = 1.1e-4;
    tolerance["rho"]    = 1.0e-4;
    tolerance["divRho"] = 1.0e-4;
    tolerance["vega"]   = 1.0e-4;

    Option::Type types[] = { Option::Put, Option::Call };
    Real strikes[] = { 50.0, 100.0, 150.0 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { -0.05, 0.0, 0.05 };
    Rate rRates[] = { 0.0, 0.01, 0.2 };
    // The testsuite check fails if a too short maturity is chosen(i.e. 1 year).
    // The problem is in the theta calculation. With the finite difference(fd) method
    // we might get values too close to the jump steps, invalidating the fd methodology
    // for calculating greeks.
    Time residualTimes[] = { 5.0 };
    Volatility vols[] = { 0.11 };
    Real jInt[] = { 1.0, 5.0 };
    Real mLJ[] = { -0.20, 0.0, 0.20 };
    Volatility jV[] = { 0.01, 0.25 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    std::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    std::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    std::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    std::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    std::shared_ptr<SimpleQuote> jumpIntensity(new SimpleQuote(0.0));
    std::shared_ptr<SimpleQuote> meanLogJump(new SimpleQuote(0.0));
    std::shared_ptr<SimpleQuote> jumpVol(new SimpleQuote(0.0));

    std::shared_ptr<Merton76Process> stochProcess(
          new Merton76Process(Handle<Quote>(spot), qTS, rTS, volTS,
                              Handle<Quote>(jumpIntensity),
                              Handle<Quote>(meanLogJump),
                              Handle<Quote>(jumpVol)));

    std::shared_ptr<StrikedTypePayoff> payoff;

    // The jumpdiffusionengine greeks are very sensitive to the
    // convergence level.  A tolerance of 1.0e-08 is usually
    // sufficient to get reasonable results
    std::shared_ptr<PricingEngine> engine(
                                 new JumpDiffusionEngine(stochProcess,1e-08));

    for (auto& type : types) {
        for (Real strike : strikes) {
            for (Real& jj1 : jInt) {
                jumpIntensity->setValue(jj1);
                for (Real& jj2 : mLJ) {
                    meanLogJump->setValue(jj2);
                    for (Real& jj3 : jV) {
                        jumpVol->setValue(jj3);
                        for (Real residualTime : residualTimes) {
                            Date exDate = today + timeToDays(residualTime);
                            std::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
                            for (Size kk = 0; kk < 1; kk++) {
                                // option to check
                                if (kk == 0) {
                                    payoff = std::shared_ptr<StrikedTypePayoff>(
                                        new PlainVanillaPayoff(type, strike));
                                } else if (kk == 1) {
                                    payoff = std::shared_ptr<StrikedTypePayoff>(
                                        new CashOrNothingPayoff(type, strike, 100.0));
                                }
                                EuropeanOption option(payoff, exercise);
                                option.setPricingEngine(engine);

                                for (Real u : underlyings) {
                                    for (Real q : qRates) {
                                        for (Real r : rRates) {
                                            for (Real v : vols) {
                                                spot->setValue(u);
                                                qRate->setValue(q);
                                                rRate->setValue(r);
                                                vol->setValue(v);

                                                Real value = option.NPV();
                                                calculated["delta"] = option.delta();
                                                calculated["gamma"] = option.gamma();
                                                calculated["theta"] = option.theta();
                                                calculated["rho"] = option.rho();
                                                calculated["divRho"] = option.dividendRho();
                                                calculated["vega"] = option.vega();

                                                if (value > spot->value() * 1.0e-5) {
                                                    // perturb spot and get delta and gamma
                                                    Real du = u * 1.0e-5;
                                                    spot->setValue(u + du);
                                                    Real value_p = option.NPV(),
                                                         delta_p = option.delta();
                                                    spot->setValue(u - du);
                                                    Real value_m = option.NPV(),
                                                         delta_m = option.delta();
                                                    spot->setValue(u);
                                                    expected["delta"] =
                                                        (value_p - value_m) / (2 * du);
                                                    expected["gamma"] =
                                                        (delta_p - delta_m) / (2 * du);

                                                    // perturb rates and get rho and dividend rho
                                                    Spread dr = 1.0e-5;
                                                    rRate->setValue(r + dr);
                                                    value_p = option.NPV();
                                                    rRate->setValue(r - dr);
                                                    value_m = option.NPV();
                                                    rRate->setValue(r);
                                                    expected["rho"] =
                                                        (value_p - value_m) / (2 * dr);

                                                    Spread dq = 1.0e-5;
                                                    qRate->setValue(q + dq);
                                                    value_p = option.NPV();
                                                    qRate->setValue(q - dq);
                                                    value_m = option.NPV();
                                                    qRate->setValue(q);
                                                    expected["divRho"] =
                                                        (value_p - value_m) / (2 * dq);

                                                    // perturb volatility and get vega
                                                    Volatility dv = v * 1.0e-4;
                                                    vol->setValue(v + dv);
                                                    value_p = option.NPV();
                                                    vol->setValue(v - dv);
                                                    value_m = option.NPV();
                                                    vol->setValue(v);
                                                    expected["vega"] =
                                                        (value_p - value_m) / (2 * dv);

                                                    // get theta from time-shifted options
                                                    Time dT = dc.yearFraction(today - 1, today + 1);
                                                    Settings::instance().evaluationDate() =
                                                        today - 1;
                                                    value_m = option.NPV();
                                                    Settings::instance().evaluationDate() =
                                                        today + 1;
                                                    value_p = option.NPV();
                                                    Settings::instance().evaluationDate() = today;
                                                    expected["theta"] = (value_p - value_m) / dT;
                                                    // compare
                                                    std::map<std::string, Real>::iterator it;
                                                    for (it = expected.begin();
                                                         it != expected.end(); ++it) {
                                                        std::string greek = it->first;
                                                        Real expct = expected[greek],
                                                             calcl = calculated[greek],
                                                             tol = tolerance[greek];
                                                        Real error = std::fabs(expct - calcl);
                                                        if (error > tol) {
                                                            REPORT_FAILURE_1(
                                                                greek, payoff, exercise, u, q, r,
                                                                today, v, jj1, jj2, jj3, expct,
                                                                calcl, error, tol);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    } // strike loop
                }
            }
        }
    } // type loop
}


test_suite* JumpDiffusionTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Jump-diffusion tests");
    suite->add(QUANTLIB_TEST_CASE(&JumpDiffusionTest::testMerton76));
    suite->add(QUANTLIB_TEST_CASE(&JumpDiffusionTest::testGreeks));
    return suite;
}
