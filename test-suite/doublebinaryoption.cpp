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
#include <ql/instruments/doublebarrieroption.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/barrier/analyticdoublebarrierbinaryengine.hpp>
#include <ql/pricingengines/barrier/fdhestondoublebarrierengine.hpp>
#include <ql/experimental/barrieroption/binomialdoublebarrierengine.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#undef REPORT_FAILURE
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

    std::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    std::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    std::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    std::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.01));
    std::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    std::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    std::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (auto& value : values) {

        std::shared_ptr<StrikedTypePayoff> payoff(
            new CashOrNothingPayoff(Option::Call, 0, value.cash));

        Date exDate = today + timeToDays(value.t);
        std::shared_ptr<Exercise> exercise;
        if (value.barrierType == DoubleBarrier::KIKO || value.barrierType == DoubleBarrier::KOKI)
            exercise.reset(new AmericanExercise(today, exDate));
        else
            exercise.reset(new EuropeanExercise(exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        std::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

        // checking with analytic engine
        std::shared_ptr<PricingEngine> engine(
                             new AnalyticDoubleBarrierBinaryEngine(stochProcess));
        DoubleBarrierOption opt(value.barrierType, value.barrier_lo, value.barrier_hi, 0, payoff,
                                exercise);
        opt.setPricingEngine(engine);

        Real calculated = opt.NPV();
        Real expected = value.result;
        Real error = std::fabs(calculated-expected);
        if (error > value.tol) {
            REPORT_FAILURE("value", payoff, exercise, value.barrierType, value.barrier_lo,
                           value.barrier_hi, value.s, value.q, value.r, today, value.v,
                           value.result, calculated, error, value.tol);
        }

        Size steps = 500;
        // checking with binomial engine
        engine = std::shared_ptr<PricingEngine>(
              new BinomialDoubleBarrierEngine<CoxRossRubinstein,
                              DiscretizedDoubleBarrierOption>(stochProcess, 
                                                                 steps));
        opt.setPricingEngine(engine);
        calculated = opt.NPV();
        expected = value.result;
        error = std::fabs(calculated-expected);
        double tol = 0.22;
        if (error>tol) {
            REPORT_FAILURE("Binomial value", payoff, exercise, value.barrierType, value.barrier_lo,
                           value.barrier_hi, value.s, value.q, value.r, today, value.v,
                           value.result, calculated, error, tol);
        }
    }
} 

void DoubleBinaryOptionTest::testPdeDoubleBarrierWithAnalytical() {
    BOOST_TEST_MESSAGE("Testing cash-or-nothing double barrier options "
            "against PDE Heston version...");

    SavedSettings backup;

    const DayCounter dc = Actual360();
    const Date todaysDate(30, Jan, 2023);
    const Date maturityDate = todaysDate + Period(1, Years);
    Settings::instance().evaluationDate() = todaysDate;

    const Handle<Quote> spot(std::make_shared<SimpleQuote>(100));
    const Rate r = 0.075;
    const Rate q = 0.03;
    const Volatility vol = 0.4;

    const Real kappa =   1.0;
    const Real theta =   vol*vol;
    const Real rho   =   0.0;
    const Real sigma =   1e-4;
    const Real v0    =   theta;

    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
    const Handle<YieldTermStructure> qTS(flatRate(q, dc));

    const std::shared_ptr<HestonModel> hestonModel =
        std::make_shared<HestonModel>(
            std::make_shared<HestonProcess>(
                rTS, qTS, spot, v0, kappa, theta, sigma, rho));

    const std::shared_ptr<BlackScholesMertonProcess> bsProcess =
        std::make_shared<BlackScholesMertonProcess>(
            Handle<Quote>(spot),
            Handle<YieldTermStructure>(qTS),
            Handle<YieldTermStructure>(rTS),
            Handle<BlackVolTermStructure>(flatVol(todaysDate, vol, dc)));

    const std::shared_ptr<PricingEngine> analyticEngine =
        std::make_shared<AnalyticDoubleBarrierBinaryEngine>(bsProcess);

    const std::shared_ptr<PricingEngine> fdEngine =
        std::make_shared<FdHestonDoubleBarrierEngine>(
            hestonModel, 201, 101, 3, 0,
            FdmSchemeDesc::Hundsdorfer());

    const std::shared_ptr<Exercise> europeanExercise(
        std::make_shared<EuropeanExercise>(maturityDate));

    const Real tol = 5e-3;
    for (Size i=5; i < 18; i+=2) {
        const Real dist = 10.0+5.0*i;

        const Real barrier_lo = std::max(spot->value() - dist, 1e-2);
        const Real barrier_hi = spot->value() + dist;
        DoubleBarrierOption doubleBarrier(
            DoubleBarrier::KnockOut, barrier_lo, barrier_hi, 0.0,
            std::make_shared<CashOrNothingPayoff>(
                Option::Call, 0.0, 1.0),
            europeanExercise);

        doubleBarrier.setPricingEngine(analyticEngine);
        const Real bsNPV = doubleBarrier.NPV();

        doubleBarrier.setPricingEngine(fdEngine);
        const Real slvNPV = doubleBarrier.NPV();

        const Real diff = slvNPV - bsNPV;
        if (std::fabs(diff) > tol) {
            BOOST_ERROR(
                "Failed to reproduce price difference for a Double-No-Touch "
                << "option between Black-Scholes and "
                << "Heston Stochastic Local Volatility model"
                << "\n Barrier Low        : " << barrier_lo
                << "\n Barrier High       : " << barrier_hi
                << "\n Black-Scholes Price: " << bsNPV
                << "\n Heston SLV Price   : " << slvNPV
                << "\n diff               : " << diff
                << "\n tolerance          : " << tol);
        }
    }
}


test_suite* DoubleBinaryOptionTest::suite() {
    auto* suite = BOOST_TEST_SUITE("DoubleBinary");
    suite->add(QUANTLIB_TEST_CASE(&DoubleBinaryOptionTest::testHaugValues));
    suite->add(QUANTLIB_TEST_CASE(&DoubleBinaryOptionTest::testPdeDoubleBarrierWithAnalytical));
    return suite;
}
