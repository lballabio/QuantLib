/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Yue Tian
 Copyright (C) 2015 Thema Consulting SA

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

#include "doublebarrieroption.hpp"
#include "utilities.hpp"
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/experimental/barrieroption/doublebarrieroption.hpp>
#include <ql/experimental/barrieroption/analyticdoublebarrierengine.hpp>
#include <ql/experimental/barrieroption/binomialdoublebarrierengine.hpp>
#include <ql/experimental/barrieroption/wulinyongdoublebarrierengine.hpp>
#include <ql/experimental/barrieroption/vannavolgadoublebarrierengine.hpp>
#include <ql/experimental/finitedifferences/fdhestondoublebarrierengine.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancecurve.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancesurface.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/models/equity/hestonmodel.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, barrierType, barrierlo, barrierhi, \
                       payoff, exercise, s, q, r, today, v, expected, \
                       calculated, error, tolerance) \
    BOOST_ERROR("\n" << barrierType << " " \
               << exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    underlying value: " << s << "\n" \
               << "    strike:           " << payoff->strike() << "\n" \
               << "    barrier low:      " << barrierlo << "\n" \
               << "    barrier high:     " << barrierhi << "\n" \
               << "    dividend yield:   " << io::rate(q) << "\n" \
               << "    risk-free rate:   " << io::rate(r) << "\n" \
               << "    reference date:   " << today << "\n" \
               << "    maturity:         " << exercise->lastDate() << "\n" \
               << "    volatility:       " << io::volatility(v) << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);

#define REPORT_FAILURE_VANNAVOLGA(greekName, barrierType, \
                                  barrier1, barrier2, rebate, payoff, \
                                  exercise, s, q, r, today, \
                                  vol25Put, atmVol, vol25Call, v, \
                                  expected, calculated, error, tolerance) \
    BOOST_ERROR("\n" <<"Double Barrier Option " \
               << barrierType << " " \
               << exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    underlying value: " << s << "\n" \
               << "    strike:           " << payoff->strike() << "\n" \
               << "    barrier1:         " << barrier1 << "\n" \
               << "    barrier2:         " << barrier2 << "\n" \
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

namespace {

    struct NewBarrierOptionData {
        DoubleBarrier::Type barrierType;
        Real barrierlo;
        Real barrierhi;
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

    struct DoubleBarrierFxOptionData {
        DoubleBarrier::Type barrierType;
        Real barrier1;
        Real barrier2;
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


void DoubleBarrierOptionTest::testEuropeanHaugValues() {

    BOOST_TEST_MESSAGE("Testing double barrier european options against Haug's values...");

    Exercise::Type european = Exercise::European;
    NewBarrierOptionData values[] = {
        /* The data below are from
          "The complete guide to option pricing formulas 2nd Ed",E.G. Haug, McGraw-Hill, p.156 and following. 

          Note:
          The book uses b instead of q (q=r-b)
        */
        //           BarrierType, barr.lo,  barr.hi,         type, exercise,strk,     s,   q,   r,    t,    v,  result, tol
        { DoubleBarrier::KnockOut,   50.0,    150.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.15,  4.3515, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.25,  6.1644, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.35,  7.0373, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.15,  6.9853, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.25,  7.9336, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.35,  6.5088, 1.0e-4},

        { DoubleBarrier::KnockOut,   60.0,    140.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.15,  4.3505, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.25,  5.8500, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.35,  5.7726, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.15,  6.8082, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.25,  6.3383, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.35,  4.3841, 1.0e-4},

        { DoubleBarrier::KnockOut,   70.0,    130.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.15,  4.3139, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.25,  4.8293, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.35,  3.7765, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.15,  5.9697, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.25,  4.0004, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.35,  2.2563, 1.0e-4},

        { DoubleBarrier::KnockOut,   80.0,    120.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.15,  3.7516, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.25,  2.6387, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.35,  1.4903, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.15,  3.5805, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.25,  1.5098, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.35,  0.5635, 1.0e-4},

        { DoubleBarrier::KnockOut,   90.0,    110.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.15,  1.2055, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.25,  0.3098, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.35,  0.0477, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.15,  0.5537, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.25,  0.0441, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.35,  0.0011, 1.0e-4},

        //           BarrierType, barr.lo,  barr.hi,         type, exercise,strk,     s,   q,   r,    t,    v,  result, tol
        { DoubleBarrier::KnockOut,   50.0,    150.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.25, 0.15,  1.8825, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.25, 0.25,  3.7855, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.25, 0.35,  5.7191, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.50, 0.15,  2.1374, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.50, 0.25,  4.7033, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.50, 0.35,  7.1683, 1.0e-4},

        { DoubleBarrier::KnockOut,   60.0,    140.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.25, 0.15,  1.8825, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.25, 0.25,  3.7845, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.25, 0.35,  5.6060, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.50, 0.15,  2.1374, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.50, 0.25,  4.6236, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.50, 0.35,  6.1062, 1.0e-4},

        { DoubleBarrier::KnockOut,   70.0,    130.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.25, 0.15,  1.8825, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.25, 0.25,  3.7014, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.25, 0.35,  4.6472, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.50, 0.15,  2.1325, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.50, 0.25,  3.8944, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.50, 0.35,  3.5868, 1.0e-4},

        { DoubleBarrier::KnockOut,   80.0,    120.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.25, 0.15,  1.8600, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.25, 0.25,  2.6866, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.25, 0.35,  2.0719, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.50, 0.15,  1.8883, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.50, 0.25,  1.7851, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.50, 0.35,  0.8244, 1.0e-4},

        { DoubleBarrier::KnockOut,   90.0,    110.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.25, 0.15,  0.9473, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.25, 0.25,  0.3449, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.25, 0.35,  0.0578, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.50, 0.15,  0.4555, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.50, 0.25,  0.0491, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0,  Option::Put, european, 100, 100.0, 0.0, 0.1, 0.50, 0.35,  0.0013, 1.0e-4},

        //           BarrierType, barr.lo,  barr.hi,         type,  strk,     s,   q,   r,    t,    v,  result, tol
        { DoubleBarrier::KnockIn,    50.0,    150.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.15,  0.0000, 1.0e-4},
        { DoubleBarrier::KnockIn,    50.0,    150.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.25,  0.0900, 1.0e-4},
        { DoubleBarrier::KnockIn,    50.0,    150.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.35,  1.1537, 1.0e-4},
        { DoubleBarrier::KnockIn,    50.0,    150.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.15,  0.0292, 1.0e-4},
        { DoubleBarrier::KnockIn,    50.0,    150.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.25,  1.6487, 1.0e-4},
        { DoubleBarrier::KnockIn,    50.0,    150.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.35,  5.7321, 1.0e-4},
                             
        { DoubleBarrier::KnockIn,    60.0,    140.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.15,  0.0010, 1.0e-4},
        { DoubleBarrier::KnockIn,    60.0,    140.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.25,  0.4045, 1.0e-4},
        { DoubleBarrier::KnockIn,    60.0,    140.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.35,  2.4184, 1.0e-4},
        { DoubleBarrier::KnockIn,    60.0,    140.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.15,  0.2062, 1.0e-4},
        { DoubleBarrier::KnockIn,    60.0,    140.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.25,  3.2439, 1.0e-4},
        { DoubleBarrier::KnockIn,    60.0,    140.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.35,  7.8569, 1.0e-4},
                             
        { DoubleBarrier::KnockIn,    70.0,    130.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.15,  0.0376, 1.0e-4},
        { DoubleBarrier::KnockIn,    70.0,    130.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.25,  1.4252, 1.0e-4},
        { DoubleBarrier::KnockIn,    70.0,    130.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.35,  4.4145, 1.0e-4},
        { DoubleBarrier::KnockIn,    70.0,    130.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.15,  1.0447, 1.0e-4},
        { DoubleBarrier::KnockIn,    70.0,    130.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.25,  5.5818, 1.0e-4},
        { DoubleBarrier::KnockIn,    70.0,    130.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.35,  9.9846, 1.0e-4},
                             
        { DoubleBarrier::KnockIn,    80.0,    120.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.15,  0.5999, 1.0e-4},
        { DoubleBarrier::KnockIn,    80.0,    120.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.25,  3.6158, 1.0e-4},
        { DoubleBarrier::KnockIn,    80.0,    120.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.35,  6.7007, 1.0e-4},
        { DoubleBarrier::KnockIn,    80.0,    120.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.15,  3.4340, 1.0e-4},
        { DoubleBarrier::KnockIn,    80.0,    120.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.25,  8.0724, 1.0e-4},
        { DoubleBarrier::KnockIn,    80.0,    120.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.35, 11.6774, 1.0e-4},
                             
        { DoubleBarrier::KnockIn,    90.0,    110.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.15,  3.1460, 1.0e-4},
        { DoubleBarrier::KnockIn,    90.0,    110.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.25,  5.9447, 1.0e-4},
        { DoubleBarrier::KnockIn,    90.0,    110.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.25, 0.35,  8.1432, 1.0e-4},
        { DoubleBarrier::KnockIn,    90.0,    110.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.15,  6.4608, 1.0e-4},
        { DoubleBarrier::KnockIn,    90.0,    110.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.25,  9.5382, 1.0e-4},
        { DoubleBarrier::KnockIn,    90.0,    110.0, Option::Call, european, 100, 100.0, 0.0, 0.1, 0.50, 0.35, 12.2398, 1.0e-4},
                             
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (Size i=0; i<LENGTH(values); i++) {
        Date exDate = today + Integer(values[i].t*360+0.5);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q); 
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        ext::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

        DoubleBarrierOption opt(
                values[i].barrierType,
                values[i].barrierlo,
                values[i].barrierhi,
                0,   // no rebate
                payoff,
                exercise);

        // Ikeda/Kunitomo engine
        ext::shared_ptr<PricingEngine> engine(
                                     new AnalyticDoubleBarrierEngine(stochProcess));
        opt.setPricingEngine(engine);

        Real calculated = opt.NPV();
        Real expected = values[i].result;
        Real error = std::fabs(calculated-expected);
        if (error>values[i].tol) {
            REPORT_FAILURE("Ikeda/Kunitomo value", values[i].barrierType, values[i].barrierlo,
                           values[i].barrierhi, payoff, exercise, values[i].s,
                           values[i].q, values[i].r, today, values[i].v,
                           expected, calculated, error, values[i].tol);
        }

        // Wulin Suo/Yong Wang engine
        engine = ext::shared_ptr<PricingEngine>(
                                     new WulinYongDoubleBarrierEngine(stochProcess));
        opt.setPricingEngine(engine);

        calculated = opt.NPV();
        expected = values[i].result;
        error = std::fabs(calculated-expected);
        if (error>values[i].tol) {
            REPORT_FAILURE("Wulin/Yong value", values[i].barrierType, values[i].barrierlo,
                           values[i].barrierhi, payoff, exercise, values[i].s,
                           values[i].q, values[i].r, today, values[i].v,
                           expected, calculated, error, values[i].tol);
        }

        engine = ext::shared_ptr<PricingEngine>(
              new BinomialDoubleBarrierEngine<CoxRossRubinstein,
                              DiscretizedDoubleBarrierOption>(stochProcess, 
                                                                 300));
        opt.setPricingEngine(engine);
        calculated = opt.NPV();
        expected = values[i].result;
        error = std::fabs(calculated-expected);
        double tol = 0.28;
        if (error>tol) {
            REPORT_FAILURE("Binomial value", values[i].barrierType, 
                           values[i].barrierlo, values[i].barrierhi, payoff, 
                           exercise, values[i].s, values[i].q, values[i].r, 
                           today, values[i].v, expected, calculated, error, 
                           tol);
        }

        engine = ext::shared_ptr<PricingEngine>(
              new BinomialDoubleBarrierEngine<CoxRossRubinstein,
                           DiscretizedDermanKaniDoubleBarrierOption>(
                                                stochProcess, 300));
        opt.setPricingEngine(engine);
        calculated = opt.NPV();
        expected = values[i].result;
        error = std::fabs(calculated-expected);
        tol = 0.033; // error one order of magnitude lower than plain binomial
        if (error>tol) {
            REPORT_FAILURE("Binomial (Derman) value", values[i].barrierType, 
                           values[i].barrierlo, values[i].barrierhi, payoff, 
                           exercise, values[i].s, values[i].q, values[i].r, 
                           today, values[i].v, expected, calculated, error, 
                           tol);
        }

        if (values[i].barrierType == DoubleBarrier::KnockOut) {
            engine = ext::make_shared<FdHestonDoubleBarrierEngine>(
                ext::make_shared<HestonModel>(
                    ext::make_shared<HestonProcess>(
                        Handle<YieldTermStructure>(rTS),
                        Handle<YieldTermStructure>(qTS),
                        Handle<Quote>(spot),
                        square<Real>()(vol->value()), 1.0,
                        square<Real>()(vol->value()), 0.001, 0.0)), 251, 76, 3);

            opt.setPricingEngine(engine);
            calculated = opt.NPV();
            expected = values[i].result;
            error = std::fabs(calculated-expected);

            tol = 0.025; // error one order of magnitude lower than plain binomial
            if (error>tol) {
                REPORT_FAILURE("Heston value", values[i].barrierType,
                               values[i].barrierlo, values[i].barrierhi, payoff,
                               exercise, values[i].s, values[i].q, values[i].r,
                               today, values[i].v, expected, calculated, error,
                               tol);
            }
        }
    }
}

void DoubleBarrierOptionTest::testVannaVolgaDoubleBarrierValues() {
    BOOST_TEST_MESSAGE(
         "Testing double-barrier FX options against Vanna/Volga values...");

    SavedSettings backup;

    DoubleBarrierFxOptionData values[] = {

        //            BarrierType, barr.1, barr.2, rebate,         type,    strike,          s,         q,         r,  t, vol25Put,    volAtm,vol25Call,      vol,    result,   tol

        { DoubleBarrier::KnockOut,    1.1,    1.5,    0.0, Option::Call,   1.13321,    1.30265, 0.0003541, 0.0033871, 1.0, 0.10087,   0.08925, 0.08463,   0.11638,   0.14413, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.1,    1.5,    0.0, Option::Call,   1.22687,    1.30265, 0.0003541, 0.0033871, 1.0, 0.10087,   0.08925, 0.08463,   0.10088,   0.07456, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.1,    1.5,    0.0, Option::Call,   1.31179,    1.30265, 0.0003541, 0.0033871, 1.0, 0.10087,   0.08925, 0.08463,   0.08925,   0.02710, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.1,    1.5,    0.0, Option::Call,   1.38843,    1.30265, 0.0003541, 0.0033871, 1.0, 0.10087,   0.08925, 0.08463,   0.08463,   0.00569, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.1,    1.5,    0.0, Option::Call,   1.46047,    1.30265, 0.0003541, 0.0033871, 1.0, 0.10087,   0.08925, 0.08463,   0.08412,   0.00013, 1.0e-4},

        { DoubleBarrier::KnockOut,    1.1,    1.5,    0.0, Option::Put,   1.13321,    1.30265, 0.0003541, 0.0033871, 1.0, 0.10087,   0.08925, 0.08463,   0.11638,    0.00017, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.1,    1.5,    0.0, Option::Put,   1.22687,    1.30265, 0.0003541, 0.0033871, 1.0, 0.10087,   0.08925, 0.08463,   0.10088,    0.00353, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.1,    1.5,    0.0, Option::Put,   1.31179,    1.30265, 0.0003541, 0.0033871, 1.0, 0.10087,   0.08925, 0.08463,   0.08925,    0.02221, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.1,    1.5,    0.0, Option::Put,   1.38843,    1.30265, 0.0003541, 0.0033871, 1.0, 0.10087,   0.08925, 0.08463,   0.08463,    0.06049, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.1,    1.5,    0.0, Option::Put,   1.46047,    1.30265, 0.0003541, 0.0033871, 1.0, 0.10087,   0.08925, 0.08463,   0.08412,    0.11103, 1.0e-4},

        { DoubleBarrier::KnockOut,    1.0,    1.6,    0.0, Option::Call,   1.06145,    1.30265, 0.0009418, 0.0039788, 2.0, 0.10891,   0.09525, 0.09197,   0.12511,   0.19981, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.0,    1.6,    0.0, Option::Call,   1.19545,    1.30265, 0.0009418, 0.0039788, 2.0, 0.10891,   0.09525, 0.09197,   0.10890,   0.10389, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.0,    1.6,    0.0, Option::Call,   1.32238,    1.30265, 0.0009418, 0.0039788, 2.0, 0.10891,   0.09525, 0.09197,   0.09444,   0.03555, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.0,    1.6,    0.0, Option::Call,   1.44298,    1.30265, 0.0009418, 0.0039788, 2.0, 0.10891,   0.09525, 0.09197,   0.09197,   0.00634, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.0,    1.6,    0.0, Option::Call,   1.56345,    1.30265, 0.0009418, 0.0039788, 2.0, 0.10891,   0.09525, 0.09197,   0.09261,   0.00000, 1.0e-4},

        { DoubleBarrier::KnockOut,    1.0,    1.6,    0.0, Option::Put,   1.06145,    1.30265, 0.0009418, 0.0039788, 2.0, 0.10891,   0.09525, 0.09197,   0.12511,    0.00000, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.0,    1.6,    0.0, Option::Put,   1.19545,    1.30265, 0.0009418, 0.0039788, 2.0, 0.10891,   0.09525, 0.09197,   0.10890,    0.00436, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.0,    1.6,    0.0, Option::Put,   1.32238,    1.30265, 0.0009418, 0.0039788, 2.0, 0.10891,   0.09525, 0.09197,   0.09444,    0.03173, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.0,    1.6,    0.0, Option::Put,   1.44298,    1.30265, 0.0009418, 0.0039788, 2.0, 0.10891,   0.09525, 0.09197,   0.09197,    0.09346, 1.0e-4},
        { DoubleBarrier::KnockOut,    1.0,    1.6,    0.0, Option::Put,   1.56345,    1.30265, 0.0009418, 0.0039788, 2.0, 0.10891,   0.09525, 0.09197,   0.09261,    0.17704, 1.0e-4}

    };

    DayCounter dc = Actual360();
    Date today(05, Mar, 2013);
    Settings::instance().evaluationDate() = today;

    ext::shared_ptr<SimpleQuote> spot = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<SimpleQuote> qRate = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol25Put = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<SimpleQuote> volAtm = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<SimpleQuote> vol25Call = ext::make_shared<SimpleQuote>(0.0);

    for (Size i=0; i<LENGTH(values); i++) {

        for (Size j=0; j<=1; j++) {

           DoubleBarrier::Type barrierType = static_cast<DoubleBarrier::Type>(j);

            spot->setValue(values[i].s);
            qRate->setValue(values[i].q);
            rRate->setValue(values[i].r);
            vol25Put->setValue(values[i].vol25Put);
            volAtm->setValue(values[i].volAtm);
            vol25Call->setValue(values[i].vol25Call);

            ext::shared_ptr<StrikedTypePayoff> payoff =
                ext::make_shared<PlainVanillaPayoff>(values[i].type,
                                                       values[i].strike);

            Date exDate = today + Integer(values[i].t*365+0.5);
            ext::shared_ptr<Exercise> exercise =
                ext::make_shared<EuropeanExercise>(exDate);

            Handle<DeltaVolQuote> volAtmQuote = Handle<DeltaVolQuote>(
                        ext::make_shared<DeltaVolQuote>(
                            Handle<Quote>(volAtm),
                            DeltaVolQuote::Fwd,
                            values[i].t,
                            DeltaVolQuote::AtmDeltaNeutral));

                                //always delta neutral atm
            Handle<DeltaVolQuote> vol25PutQuote(Handle<DeltaVolQuote>(
                            ext::make_shared<DeltaVolQuote>(
                                -0.25,
                                Handle<Quote>(vol25Put),
                                values[i].t,
                                DeltaVolQuote::Fwd)));

            Handle<DeltaVolQuote> vol25CallQuote(Handle<DeltaVolQuote>(
                            ext::make_shared<DeltaVolQuote>(
                                0.25,
                                Handle<Quote>(vol25Call),
                                values[i].t,
                                DeltaVolQuote::Fwd)));

            DoubleBarrierOption doubleBarrierOption(barrierType,
                                                    values[i].barrier1,
                                                    values[i].barrier2,
                                                    values[i].rebate,
                                                    payoff,
                                                    exercise);

            Real bsVanillaPrice =
                blackFormula(values[i].type, values[i].strike,
                             spot->value()*qTS->discount(values[i].t)/rTS->discount(values[i].t),
                             values[i].v * sqrt(values[i].t), rTS->discount(values[i].t));
            ext::shared_ptr<PricingEngine> vannaVolgaEngine =
                ext::make_shared<VannaVolgaDoubleBarrierEngine<WulinYongDoubleBarrierEngine> >(
                                volAtmQuote,
                                vol25PutQuote,
                                vol25CallQuote,
                                Handle<Quote> (spot),
                                Handle<YieldTermStructure> (rTS),
                                Handle<YieldTermStructure> (qTS),
                                true,
                                bsVanillaPrice);
            doubleBarrierOption.setPricingEngine(vannaVolgaEngine);

            // Expected result for KO is given in array, for KI is evaluated as vanilla - KO
            Real expected = 0;
            if (barrierType == DoubleBarrier::KnockOut)
               expected = values[i].result;
            else if (barrierType == DoubleBarrier::KnockIn)
               expected = (bsVanillaPrice - values[i].result);
           
            Real calculated = doubleBarrierOption.NPV();
            Real error = std::fabs(calculated-expected);
            if (error>values[i].tol) {
                REPORT_FAILURE_VANNAVOLGA(
                    "value", barrierType,
                    values[i].barrier1, values[i].barrier2,
                    values[i].rebate, payoff, exercise, values[i].s,
                    values[i].q, values[i].r, today, values[i].vol25Put,
                    values[i].volAtm, values[i].vol25Call, values[i].v,
                    expected, calculated, error, values[i].tol);
            }

            vannaVolgaEngine =
                ext::make_shared<VannaVolgaDoubleBarrierEngine<AnalyticDoubleBarrierEngine> >(
                                volAtmQuote,
                                vol25PutQuote,
                                vol25CallQuote,
                                Handle<Quote> (spot),
                                Handle<YieldTermStructure> (rTS),
                                Handle<YieldTermStructure> (qTS),
                                true,
                                bsVanillaPrice);
            doubleBarrierOption.setPricingEngine(vannaVolgaEngine);

            calculated = doubleBarrierOption.NPV();
            error = std::fabs(calculated-expected);
            Real maxtol = 5.0e-3; // different engines have somewhat different results
            if (error>maxtol) {
                REPORT_FAILURE_VANNAVOLGA(
                    "value", barrierType,
                    values[i].barrier1, values[i].barrier2,
                    values[i].rebate, payoff, exercise, values[i].s,
                    values[i].q, values[i].r, today, values[i].vol25Put,
                    values[i].volAtm, values[i].vol25Call, values[i].v,
                    expected, calculated, error, values[i].tol);
            }
        }

    }
}

test_suite* DoubleBarrierOptionTest::suite(SpeedLevel speed) {
    test_suite* suite = BOOST_TEST_SUITE("DoubleBarrier");

    if (speed <= Fast) {
        suite->add(QUANTLIB_TEST_CASE(
            &DoubleBarrierOptionTest::testEuropeanHaugValues));
    }

    return suite;
}

test_suite* DoubleBarrierOptionTest::experimental() {
    test_suite* suite = BOOST_TEST_SUITE("DoubleBarrier_experimental");
    suite->add(QUANTLIB_TEST_CASE(
                      &DoubleBarrierOptionTest::testVannaVolgaDoubleBarrierValues));
    return suite;
}
