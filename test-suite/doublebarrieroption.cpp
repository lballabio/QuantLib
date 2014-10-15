/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Thema Consulting SA (developer: Riccardo Ghetta)

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
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/instruments/doublebarrieroption.hpp>
#include <ql/pricingengines/barrier/analyticdoublebarrierengine.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancecurve.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancesurface.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>

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

namespace {

    struct NewBarrierOptionData {
        DoubleBarrier::Type barrierType;
        Real barrierlo;
        Real barrierhi;
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


void DoubleBarrierOptionTest::testEuropeanHaugValues() {

    BOOST_MESSAGE("Testing double barrier european options against Haug's values...");

    NewBarrierOptionData values[] = {
        /* The data below are from
          "The complete guide to option pricing formulas 2nd Ed",E.G. Haug, McGraw-Hill, p.156 and following. 

          Note:
          The book uses b instead of q (q=r-b)
        */
        //           BarrierType, barr.lo,  barr.hi,         type,  strk,     s,   q,   r,    t,    v,  result, tol
        { DoubleBarrier::KnockOut,   50.0,    150.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.15,  4.3515, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.25,  6.1644, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.35,  7.0373, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.15,  6.9853, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.25,  7.9336, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.35,  6.5088, 1.0e-4},

        { DoubleBarrier::KnockOut,   60.0,    140.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.15,  4.3505, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.25,  5.8500, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.35,  5.7726, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.15,  6.8082, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.25,  6.3383, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.35,  4.3841, 1.0e-4},

        { DoubleBarrier::KnockOut,   70.0,    130.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.15,  4.3139, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.25,  4.8293, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.35,  3.7765, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.15,  5.9697, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.25,  4.0004, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.35,  2.2563, 1.0e-4},

        { DoubleBarrier::KnockOut,   80.0,    120.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.15,  3.7516, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.25,  2.6387, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.35,  1.4903, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.15,  3.5805, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.25,  1.5098, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.35,  0.5635, 1.0e-4},

        { DoubleBarrier::KnockOut,   90.0,    110.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.15,  1.2055, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.25,  0.3098, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.35,  0.0477, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.15,  0.5537, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.25,  0.0441, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.35,  0.0011, 1.0e-4},

        //           BarrierType, barr.lo,  barr.hi,         type,  strk,     s,   q,   r,    t,    v,  result, tol
        { DoubleBarrier::KnockOut,   50.0,    150.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.25, 0.15,  1.8825, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.25, 0.25,  3.7855, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.25, 0.35,  5.7191, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.50, 0.15,  2.1374, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.50, 0.25,  4.7033, 1.0e-4},
        { DoubleBarrier::KnockOut,   50.0,    150.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.50, 0.35,  7.1683, 1.0e-4},

        { DoubleBarrier::KnockOut,   60.0,    140.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.25, 0.15,  1.8825, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.25, 0.25,  3.7845, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.25, 0.35,  5.6060, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.50, 0.15,  2.1374, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.50, 0.25,  4.6236, 1.0e-4},
        { DoubleBarrier::KnockOut,   60.0,    140.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.50, 0.35,  6.1062, 1.0e-4},

        { DoubleBarrier::KnockOut,   70.0,    130.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.25, 0.15,  1.8825, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.25, 0.25,  3.7014, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.25, 0.35,  4.6472, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.50, 0.15,  2.1325, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.50, 0.25,  3.8944, 1.0e-4},
        { DoubleBarrier::KnockOut,   70.0,    130.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.50, 0.35,  3.5868, 1.0e-4},

        { DoubleBarrier::KnockOut,   80.0,    120.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.25, 0.15,  1.8600, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.25, 0.25,  2.6866, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.25, 0.35,  2.0719, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.50, 0.15,  1.8883, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.50, 0.25,  1.7851, 1.0e-4},
        { DoubleBarrier::KnockOut,   80.0,    120.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.50, 0.35,  0.8244, 1.0e-4},

        { DoubleBarrier::KnockOut,   90.0,    110.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.25, 0.15,  0.9473, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.25, 0.25,  0.3449, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.25, 0.35,  0.0578, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.50, 0.15,  0.4555, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.50, 0.25,  0.0491, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,    110.0,  Option::Put,    100, 100.0, 0.0, 0.1, 0.50, 0.35,  0.0013, 1.0e-4},

        //           BarrierType, barr.lo,  barr.hi,         type,  strk,     s,   q,   r,    t,    v,  result, tol
        { DoubleBarrier::KnockIn,    50.0,    150.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.15,  0.0000, 1.0e-4},
        { DoubleBarrier::KnockIn,    50.0,    150.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.25,  0.0900, 1.0e-4},
        { DoubleBarrier::KnockIn,    50.0,    150.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.35,  1.1537, 1.0e-4},
        { DoubleBarrier::KnockIn,    50.0,    150.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.15,  0.0292, 1.0e-4},
        { DoubleBarrier::KnockIn,    50.0,    150.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.25,  1.6487, 1.0e-4},
        { DoubleBarrier::KnockIn,    50.0,    150.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.35,  5.7321, 1.0e-4},
                             
        { DoubleBarrier::KnockIn,    60.0,    140.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.15,  0.0010, 1.0e-4},
        { DoubleBarrier::KnockIn,    60.0,    140.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.25,  0.4045, 1.0e-4},
        { DoubleBarrier::KnockIn,    60.0,    140.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.35,  2.4184, 1.0e-4},
        { DoubleBarrier::KnockIn,    60.0,    140.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.15,  0.2062, 1.0e-4},
        { DoubleBarrier::KnockIn,    60.0,    140.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.25,  3.2439, 1.0e-4},
        { DoubleBarrier::KnockIn,    60.0,    140.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.35,  7.8569, 1.0e-4},
                             
        { DoubleBarrier::KnockIn,    70.0,    130.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.15,  0.0376, 1.0e-4},
        { DoubleBarrier::KnockIn,    70.0,    130.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.25,  1.4252, 1.0e-4},
        { DoubleBarrier::KnockIn,    70.0,    130.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.35,  4.4145, 1.0e-4},
        { DoubleBarrier::KnockIn,    70.0,    130.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.15,  1.0447, 1.0e-4},
        { DoubleBarrier::KnockIn,    70.0,    130.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.25,  5.5818, 1.0e-4},
        { DoubleBarrier::KnockIn,    70.0,    130.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.35,  9.9846, 1.0e-4},
                             
        { DoubleBarrier::KnockIn,    80.0,    120.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.15,  0.5999, 1.0e-4},
        { DoubleBarrier::KnockIn,    80.0,    120.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.25,  3.6158, 1.0e-4},
        { DoubleBarrier::KnockIn,    80.0,    120.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.35,  6.7007, 1.0e-4},
        { DoubleBarrier::KnockIn,    80.0,    120.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.15,  3.4340, 1.0e-4},
        { DoubleBarrier::KnockIn,    80.0,    120.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.25,  8.0724, 1.0e-4},
        { DoubleBarrier::KnockIn,    80.0,    120.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.35, 11.6774, 1.0e-4},
                             
        { DoubleBarrier::KnockIn,    90.0,    110.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.15,  3.1460, 1.0e-4},
        { DoubleBarrier::KnockIn,    90.0,    110.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.25,  5.9447, 1.0e-4},
        { DoubleBarrier::KnockIn,    90.0,    110.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.25, 0.35,  8.1432, 1.0e-4},
        { DoubleBarrier::KnockIn,    90.0,    110.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.15,  6.4608, 1.0e-4},
        { DoubleBarrier::KnockIn,    90.0,    110.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.25,  9.5382, 1.0e-4},
        { DoubleBarrier::KnockIn,    90.0,    110.0, Option::Call,   100, 100.0, 0.0, 0.1, 0.50, 0.35, 12.2398, 1.0e-4},
                             
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
                                     new AnalyticDoubleBarrierEngine(stochProcess));

        DoubleBarrierOption opt(
                values[i].barrierType,
                values[i].barrierlo,
                values[i].barrierhi,
                0,   // no rebate
                payoff,
                exercise);
        opt.setPricingEngine(engine);

        Real calculated = opt.NPV();
        Real expected = values[i].result;
        Real error = std::fabs(calculated-expected);
        if (error>values[i].tol) {
            REPORT_FAILURE("value", values[i].barrierType, values[i].barrierlo,
                           values[i].barrierhi, payoff, exercise, values[i].s,
                           values[i].q, values[i].r, today, values[i].v,
                           expected, calculated, error, values[i].tol);
        }
    }
}

test_suite* DoubleBarrierOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("DoubleBarrier");
    suite->add(QUANTLIB_TEST_CASE(&DoubleBarrierOptionTest::testEuropeanHaugValues));
    return suite;
}
