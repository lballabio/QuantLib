/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include "doublebinaryoption.hpp"
#include "utilities.hpp"
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/experimental/barrieroption/doublebarrieroption.hpp>
#include <ql/experimental/barrieroption/analyticdoublebarrierbinaryengine.hpp>
#include <ql/experimental/barrieroption/binomialdoublebarrierengine.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, payoff, exercise, barrierType, barrier_lo, \
                        barrier_hi, s, q, r, today, v, expected, calculated, \
                        error, tolerance) \
    BOOST_ERROR(payoff->optionType() << " option with " \
               << barrierType << " barrier type:\n" \
               << "    barrier_lo:       " << barrier_lo << "\n" \
               << "    barrier_hi:       " << barrier_hi << "\n" \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    spot value:       " << s << "\n" \
               << "    strike:           " << payoff->strike() << "\n" \
               << "    dividend yield:   " << io::rate(q) << "\n" \
               << "    risk-free rate:   " << io::rate(r) << "\n" \
               << "    reference date:   " << today << "\n" \
               << "    maturity:         " << exercise->lastDate() << "\n" \
               << "    volatility:       " << io::volatility(v) << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance << "\n");

namespace {

    struct DoubleBinaryOptionData {
        DoubleBarrier::Type barrierType;
        Real barrier_lo;
        Real barrier_hi;
        Real cash;     // cash payoff for cash-or-nothing
        Real s;        // spot
        Rate q;        // dividend
        Rate r;        // risk-free rate
        Time t;        // time to maturity
        Volatility v;  // volatility
        Real result;   // expected result
        Real tol;      // tolerance
    };
}


void DoubleBinaryOptionTest::testHaugValues() {

    BOOST_TEST_MESSAGE("Testing cash-or-nothing double barrier options against Haug's values...");

    DoubleBinaryOptionData values[] = {
        /* The data below are from
          "Option pricing formulas 2nd Ed.", E.G. Haug, McGraw-Hill 2007 pag. 181
          Note: book uses cost of carry b, instead of dividend rate q
        */
        //    barrierType,          bar_lo, bar_hi,  cash,   spot,    q,    r,    t,  vol,   value, tol
        { DoubleBarrier::KnockOut,   80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  9.8716, 1e-4 },
        { DoubleBarrier::KnockOut,   80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  8.9307, 1e-4 },
        { DoubleBarrier::KnockOut,   80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  6.3272, 1e-4 },
        { DoubleBarrier::KnockOut,   80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  1.9094, 1e-4 },

        { DoubleBarrier::KnockOut,   85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  9.7961, 1e-4 },
        { DoubleBarrier::KnockOut,   85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  7.2300, 1e-4 },
        { DoubleBarrier::KnockOut,   85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  3.7100, 1e-4 },
        { DoubleBarrier::KnockOut,   85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  0.4271, 1e-4 },

        { DoubleBarrier::KnockOut,   90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  8.9054, 1e-4 },
        { DoubleBarrier::KnockOut,   90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  3.6752, 1e-4 },
        { DoubleBarrier::KnockOut,   90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  0.7960, 1e-4 },
        { DoubleBarrier::KnockOut,   90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  0.0059, 1e-4 },

        { DoubleBarrier::KnockOut,   95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  3.6323, 1e-4 },
        { DoubleBarrier::KnockOut,   95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  0.0911, 1e-4 },
        { DoubleBarrier::KnockOut,   95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  0.0002, 1e-4 },
        { DoubleBarrier::KnockOut,   95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  0.0000, 1e-4 },

        { DoubleBarrier::KIKO,       80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  0.0000, 1e-4 },
        { DoubleBarrier::KIKO,       80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  0.2402, 1e-4 },
        { DoubleBarrier::KIKO,       80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  1.4076, 1e-4 },
        { DoubleBarrier::KIKO,       80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  3.8160, 1e-4 },

        { DoubleBarrier::KIKO,       85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  0.0075, 1e-4 },
        { DoubleBarrier::KIKO,       85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  0.9910, 1e-4 },
        { DoubleBarrier::KIKO,       85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  2.8098, 1e-4 },
        { DoubleBarrier::KIKO,       85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  4.6612, 1e-4 },

        { DoubleBarrier::KIKO,       90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  0.2656, 1e-4 },
        { DoubleBarrier::KIKO,       90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  2.7954, 1e-4 },
        { DoubleBarrier::KIKO,       90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  4.4024, 1e-4 },
        { DoubleBarrier::KIKO,       90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  4.9266, 1e-4 },

        { DoubleBarrier::KIKO,       95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  2.6285, 1e-4 },
        { DoubleBarrier::KIKO,       95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  4.7523, 1e-4 },
        { DoubleBarrier::KIKO,       95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  4.9096, 1e-4 },
        { DoubleBarrier::KIKO,       95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  4.9675, 1e-4 },

        // following values calculated with haug's VBA code
        { DoubleBarrier::KnockIn,    80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  0.0042, 1e-4 },
        { DoubleBarrier::KnockIn,    80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  0.9450, 1e-4 },
        { DoubleBarrier::KnockIn,    80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  3.5486, 1e-4 },
        { DoubleBarrier::KnockIn,    80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  7.9663, 1e-4 },

        { DoubleBarrier::KnockIn,    85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  0.0797, 1e-4 },
        { DoubleBarrier::KnockIn,    85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  2.6458, 1e-4 },
        { DoubleBarrier::KnockIn,    85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  6.1658, 1e-4 },
        { DoubleBarrier::KnockIn,    85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  9.4486, 1e-4 },

        { DoubleBarrier::KnockIn,    90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  0.9704, 1e-4 },
        { DoubleBarrier::KnockIn,    90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  6.2006, 1e-4 },
        { DoubleBarrier::KnockIn,    90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  9.0798, 1e-4 },
        { DoubleBarrier::KnockIn,    90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  9.8699, 1e-4 },

        { DoubleBarrier::KnockIn,    95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  6.2434, 1e-4 },
        { DoubleBarrier::KnockIn,    95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  9.7847, 1e-4 },
        { DoubleBarrier::KnockIn,    95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  9.8756, 1e-4 },
        { DoubleBarrier::KnockIn,    95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  9.8758, 1e-4 },

        { DoubleBarrier::KOKI,       80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  0.0041, 1e-4 },
        { DoubleBarrier::KOKI,       80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  0.7080, 1e-4 },
        { DoubleBarrier::KOKI,       80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  2.1581, 1e-4 },
        { DoubleBarrier::KOKI,       80.00, 120.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  4.2061, 1e-4 },

        { DoubleBarrier::KOKI,       85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  0.0723, 1e-4 },
        { DoubleBarrier::KOKI,       85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  1.6663, 1e-4 },
        { DoubleBarrier::KOKI,       85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  3.3930, 1e-4 },
        { DoubleBarrier::KOKI,       85.00, 115.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  4.8679, 1e-4 },

        { DoubleBarrier::KOKI,       90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  0.7080, 1e-4 },
        { DoubleBarrier::KOKI,       90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  3.4424, 1e-4 },
        { DoubleBarrier::KOKI,       90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  4.7496, 1e-4 },
        { DoubleBarrier::KOKI,       90.00, 110.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  5.0475, 1e-4 },

        { DoubleBarrier::KOKI,       95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.10,  3.6524, 1e-4 },
        { DoubleBarrier::KOKI,       95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.20,  5.1256, 1e-4 },
        { DoubleBarrier::KOKI,       95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.30,  5.0763, 1e-4 },
        { DoubleBarrier::KOKI,       95.00, 105.00, 10.00, 100.00, 0.02, 0.05, 0.25, 0.50,  5.0275, 1e-4 },

        // degenerate cases
        { DoubleBarrier::KnockOut,   95.00, 105.00, 10.00,  80.00, 0.02, 0.05, 0.25, 0.10,  0.0000, 1e-4 },
        { DoubleBarrier::KnockOut,   95.00, 105.00, 10.00, 110.00, 0.02, 0.05, 0.25, 0.10,  0.0000, 1e-4 },
        { DoubleBarrier::KnockIn,    95.00, 105.00, 10.00,  80.00, 0.02, 0.05, 0.25, 0.10, 10.0000, 1e-4 },
        { DoubleBarrier::KnockIn,    95.00, 105.00, 10.00, 110.00, 0.02, 0.05, 0.25, 0.10, 10.0000, 1e-4 },
        { DoubleBarrier::KIKO,       95.00, 105.00, 10.00,  80.00, 0.02, 0.05, 0.25, 0.10, 10.0000, 1e-4 },
        { DoubleBarrier::KIKO,       95.00, 105.00, 10.00, 110.00, 0.02, 0.05, 0.25, 0.10,  0.0000, 1e-4 },
        { DoubleBarrier::KOKI,       95.00, 105.00, 10.00,  80.00, 0.02, 0.05, 0.25, 0.10,  0.0000, 1e-4 },
        { DoubleBarrier::KOKI,       95.00, 105.00, 10.00, 110.00, 0.02, 0.05, 0.25, 0.10, 10.0000, 1e-4 },
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.01));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (Size i=0; i<LENGTH(values); i++) {

        ext::shared_ptr<StrikedTypePayoff> payoff(new CashOrNothingPayoff(
            Option::Call, 0, values[i].cash));

        Date exDate = today + Integer(values[i].t*360+0.5);
        ext::shared_ptr<Exercise> exercise;
        if (values[i].barrierType == DoubleBarrier::KIKO ||
            values[i].barrierType == DoubleBarrier::KOKI)
            exercise.reset(new AmericanExercise(today, exDate));
        else
            exercise.reset(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

        // checking with analytic engine
        ext::shared_ptr<PricingEngine> engine(
                             new AnalyticDoubleBarrierBinaryEngine(stochProcess));
        DoubleBarrierOption opt(values[i].barrierType, 
                          values[i].barrier_lo, 
                          values[i].barrier_hi, 
                          0,
                          payoff,
                          exercise);
        opt.setPricingEngine(engine);

        Real calculated = opt.NPV();
        Real expected = values[i].result;
        Real error = std::fabs(calculated-expected);
        if (error > values[i].tol) {
            REPORT_FAILURE("value", payoff, exercise, values[i].barrierType, 
                           values[i].barrier_lo, values[i].barrier_hi, values[i].s,
                           values[i].q, values[i].r, today, values[i].v,
                           values[i].result, calculated, error, values[i].tol);
        }

        Size steps = 500;
        // checking with binomial engine
        engine = ext::shared_ptr<PricingEngine>(
              new BinomialDoubleBarrierEngine<CoxRossRubinstein,
                              DiscretizedDoubleBarrierOption>(stochProcess, 
                                                                 steps));
        opt.setPricingEngine(engine);
        calculated = opt.NPV();
        expected = values[i].result;
        error = std::fabs(calculated-expected);
        double tol = 0.22;
        if (error>tol) {
            REPORT_FAILURE("Binomial value", payoff, exercise, values[i].barrierType, 
                           values[i].barrier_lo, values[i].barrier_hi, values[i].s,
                           values[i].q, values[i].r, today, values[i].v,
                           values[i].result, calculated, error, tol);
        }

    }
} 


test_suite* DoubleBinaryOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("DoubleBinary");
    suite->add(QUANTLIB_TEST_CASE(&DoubleBinaryOptionTest::testHaugValues));
    return suite;
}
