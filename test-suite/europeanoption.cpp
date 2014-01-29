/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2007 Ferdinando Ametrano
 Copyright (C) 2003, 2007 StatPro Italia srl
 Copyright (C) 2009 Klaus Spanderen
 
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

#include "europeanoption.hpp"
#include "utilities.hpp"
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/binomialengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/experimental/variancegamma/fftvanillaengine.hpp>
#include <ql/pricingengines/vanilla/fdeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/mceuropeanengine.hpp>
#include <ql/pricingengines/vanilla/integralengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancesurface.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <boost/progress.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, \
                       v, expected, calculated, error, tolerance) \
    BOOST_ERROR(exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    spot value:       " << s << "\n" \
               << "    strike:           " << payoff->strike() << "\n" \
               << "    dividend yield:   " << io::rate(q) << "\n" \
               << "    risk-free rate:   " << io::rate(r) << "\n" \
               << "    reference date:   " << today << "\n" \
               << "    maturity:         " << exercise->lastDate() << "\n" \
               << "    volatility:       " << io::volatility(v) << "\n\n" \
               << "    expected " << greekName << ":   " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);

namespace {

    // utilities

    struct EuropeanOptionData {
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

    enum EngineType { Analytic,
                      JR, CRR, EQP, TGEO, TIAN, LR, JOSHI,
                      FiniteDifferences,
                      Integral,
                      PseudoMonteCarlo, QuasiMonteCarlo,
                      FFT };

    boost::shared_ptr<GeneralizedBlackScholesProcess>
    makeProcess(const boost::shared_ptr<Quote>& u,
                const boost::shared_ptr<YieldTermStructure>& q,
                const boost::shared_ptr<YieldTermStructure>& r,
                const boost::shared_ptr<BlackVolTermStructure>& vol) {
        return boost::shared_ptr<BlackScholesMertonProcess>(
           new BlackScholesMertonProcess(Handle<Quote>(u),
                                         Handle<YieldTermStructure>(q),
                                         Handle<YieldTermStructure>(r),
                                         Handle<BlackVolTermStructure>(vol)));
    }

    boost::shared_ptr<VanillaOption>
    makeOption(const boost::shared_ptr<StrikedTypePayoff>& payoff,
               const boost::shared_ptr<Exercise>& exercise,
               const boost::shared_ptr<Quote>& u,
               const boost::shared_ptr<YieldTermStructure>& q,
               const boost::shared_ptr<YieldTermStructure>& r,
               const boost::shared_ptr<BlackVolTermStructure>& vol,
               EngineType engineType,
               Size binomialSteps,
               Size samples) {

        boost::shared_ptr<GeneralizedBlackScholesProcess> stochProcess =
            makeProcess(u,q,r,vol);

        boost::shared_ptr<PricingEngine> engine;
        switch (engineType) {
          case Analytic:
            engine = boost::shared_ptr<PricingEngine>(
                                    new AnalyticEuropeanEngine(stochProcess));
            break;
          case JR:
            engine = boost::shared_ptr<PricingEngine>(
                        new BinomialVanillaEngine<JarrowRudd>(stochProcess,
                                                              binomialSteps));
            break;
          case CRR:
            engine = boost::shared_ptr<PricingEngine>(
                 new BinomialVanillaEngine<CoxRossRubinstein>(stochProcess,
                                                              binomialSteps));
            break;
          case EQP:
            engine = boost::shared_ptr<PricingEngine>(
                 new BinomialVanillaEngine<AdditiveEQPBinomialTree>(
                                                              stochProcess,
                                                              binomialSteps));
            break;
          case TGEO:
            engine = boost::shared_ptr<PricingEngine>(
                        new BinomialVanillaEngine<Trigeorgis>(stochProcess,
                                                              binomialSteps));
            break;
          case TIAN:
            engine = boost::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<Tian>(stochProcess, binomialSteps));
            break;
          case LR:
            engine = boost::shared_ptr<PricingEngine>(
                      new BinomialVanillaEngine<LeisenReimer>(stochProcess,
                                                              binomialSteps));
            break;
          case JOSHI:
            engine = boost::shared_ptr<PricingEngine>(
              new BinomialVanillaEngine<Joshi4>(stochProcess, binomialSteps));
            break;
          case FiniteDifferences:
            engine = boost::shared_ptr<PricingEngine>(
                            new FDEuropeanEngine<CrankNicolson>(stochProcess,
                                                                binomialSteps,
                                                                samples));
            break;
          case Integral:
            engine = boost::shared_ptr<PricingEngine>(
                                            new IntegralEngine(stochProcess));
            break;
          case PseudoMonteCarlo:
            engine = MakeMCEuropeanEngine<PseudoRandom>(stochProcess)
                .withSteps(1)
                .withSamples(samples)
                .withSeed(42);
            break;
          case QuasiMonteCarlo:
            engine = MakeMCEuropeanEngine<LowDiscrepancy>(stochProcess)
                .withSteps(1)
                .withSamples(samples);
            break;
          case FFT:
              engine = boost::shared_ptr<PricingEngine>(
                                          new FFTVanillaEngine(stochProcess));
            break;
          default:
            QL_FAIL("unknown engine type");
        }

        boost::shared_ptr<VanillaOption> option(
            new EuropeanOption(payoff, exercise));
        option->setPricingEngine(engine);
        return option;
    }

    Integer timeToDays(Time t) {
        // FLOATING_POINT_EXCEPTION
        return Integer(t*360+0.5);
    }

}


void EuropeanOptionTest::testValues() {

    BOOST_TEST_MESSAGE("Testing European option values...");

    SavedSettings backup;

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
    */
    EuropeanOptionData values[] = {
      // pag 2-8
      //        type, strike,   spot,    q,    r,    t,  vol,   value,    tol
      { Option::Call,  65.00,  60.00, 0.00, 0.08, 0.25, 0.30,  2.1334, 1.0e-4},
      { Option::Put,   95.00, 100.00, 0.05, 0.10, 0.50, 0.20,  2.4648, 1.0e-4},
      { Option::Put,   19.00,  19.00, 0.10, 0.10, 0.75, 0.28,  1.7011, 1.0e-4},
      { Option::Call,  19.00,  19.00, 0.10, 0.10, 0.75, 0.28,  1.7011, 1.0e-4},
      { Option::Call,   1.60,   1.56, 0.08, 0.06, 0.50, 0.12,  0.0291, 1.0e-4},
      { Option::Put,   70.00,  75.00, 0.05, 0.10, 0.50, 0.35,  4.0870, 1.0e-4},
      // pag 24
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.15,  0.0205, 1.0e-4},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.15,  1.8734, 1.0e-4},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.15,  9.9413, 1.0e-4},
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.25,  0.3150, 1.0e-4},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.25,  3.1217, 1.0e-4},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.25, 10.3556, 1.0e-4},
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.35,  0.9474, 1.0e-4},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.35,  4.3693, 1.0e-4},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.35, 11.1381, 1.0e-4},
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.15,  0.8069, 1.0e-4},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.15,  4.0232, 1.0e-4},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.15, 10.5769, 1.0e-4},
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.25,  2.7026, 1.0e-4},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.25,  6.6997, 1.0e-4},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.25, 12.7857, 1.0e-4},
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.35,  4.9329, 1.0e-4},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.35,  9.3679, 1.0e-4},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.35, 15.3086, 1.0e-4},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.15,  9.9210, 1.0e-4},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.15,  1.8734, 1.0e-4},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.15,  0.0408, 1.0e-4},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.25, 10.2155, 1.0e-4},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.25,  3.1217, 1.0e-4},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.25,  0.4551, 1.0e-4},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.35, 10.8479, 1.0e-4},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.35,  4.3693, 1.0e-4},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.35,  1.2376, 1.0e-4},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.15, 10.3192, 1.0e-4},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.15,  4.0232, 1.0e-4},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.15,  1.0646, 1.0e-4},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.25, 12.2149, 1.0e-4},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.25,  6.6997, 1.0e-4},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.25,  3.2734, 1.0e-4},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.35, 14.4452, 1.0e-4},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.35,  9.3679, 1.0e-4},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.35,  5.7963, 1.0e-4},
      // pag 27
      { Option::Call,  40.00,  42.00, 0.08, 0.04, 0.75, 0.35,  5.0975, 1.0e-4}
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

        boost::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));
        Date exDate = today + timeToDays(values[i].t);
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));
        boost::shared_ptr<PricingEngine> engine(
                                    new AnalyticEuropeanEngine(stochProcess));

        EuropeanOption option(payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated-values[i].result);
        Real tolerance = values[i].tol;
        if (error>tolerance) {
            REPORT_FAILURE("value", payoff, exercise, values[i].s,
                           values[i].q, values[i].r, today,
                           values[i].v, values[i].result, calculated,
                           error, tolerance);
        }

        engine = boost::shared_ptr<PricingEngine>(
                    new FdBlackScholesVanillaEngine(stochProcess,200,400));
        option.setPricingEngine(engine);

        calculated = option.NPV();
        error = std::fabs(calculated-values[i].result);
        tolerance = 1.0e-3;
        if (error>tolerance) {
            REPORT_FAILURE("value", payoff, exercise, values[i].s,
                           values[i].q, values[i].r, today,
                           values[i].v, values[i].result, calculated,
                           error, tolerance);
        }
    }

}



void EuropeanOptionTest::testGreekValues() {

    BOOST_TEST_MESSAGE("Testing European option greek values...");

    SavedSettings backup;

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
       pag 11-16
    */
    EuropeanOptionData values[] = {
      //        type, strike,   spot,    q,    r,        t,  vol,  value
      // delta
      { Option::Call, 100.00, 105.00, 0.10, 0.10, 0.500000, 0.36,  0.5946, 0 },
      { Option::Put,  100.00, 105.00, 0.10, 0.10, 0.500000, 0.36, -0.3566, 0 },
      // elasticity
      { Option::Put,  100.00, 105.00, 0.10, 0.10, 0.500000, 0.36, -4.8775, 0 },
      // gamma
      { Option::Call,  60.00,  55.00, 0.00, 0.10, 0.750000, 0.30,  0.0278, 0 },
      { Option::Put,   60.00,  55.00, 0.00, 0.10, 0.750000, 0.30,  0.0278, 0 },
      // vega
      { Option::Call,  60.00,  55.00, 0.00, 0.10, 0.750000, 0.30, 18.9358, 0 },
      { Option::Put,   60.00,  55.00, 0.00, 0.10, 0.750000, 0.30, 18.9358, 0 },
      // theta
      { Option::Put,  405.00, 430.00, 0.05, 0.07, 1.0/12.0, 0.20,-31.1924, 0 },
      // theta per day
      { Option::Put,  405.00, 430.00, 0.05, 0.07, 1.0/12.0, 0.20, -0.0855, 0 },
      // rho
      { Option::Call,  75.00,  72.00, 0.00, 0.09, 1.000000, 0.19, 38.7325, 0 },
      // dividendRho
      { Option::Put,  490.00, 500.00, 0.05, 0.08, 0.250000, 0.15, 42.2254, 0 }
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
    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(new
        BlackScholesMertonProcess(Handle<Quote>(spot),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS)));
    boost::shared_ptr<PricingEngine> engine(
                                    new AnalyticEuropeanEngine(stochProcess));

    boost::shared_ptr<StrikedTypePayoff> payoff;
    Date exDate;
    boost::shared_ptr<Exercise> exercise;
    boost::shared_ptr<VanillaOption> option;
    Real calculated;

    Integer i = -1;

    i++;
    payoff = boost::shared_ptr<StrikedTypePayoff>(new
        PlainVanillaPayoff(values[i].type, values[i].strike));
    exDate = today + timeToDays(values[i].t);
    exercise = boost::shared_ptr<Exercise>(new EuropeanExercise(exDate));
    spot ->setValue(values[i].s);
    qRate->setValue(values[i].q);
    rRate->setValue(values[i].r);
    vol  ->setValue(values[i].v);
    option = boost::shared_ptr<VanillaOption>(
                                        new EuropeanOption(payoff, exercise));
    option->setPricingEngine(engine);
    calculated = option->delta();
    Real error = std::fabs(calculated-values[i].result);
    Real tolerance = 1e-4;
    if (error>tolerance)
        REPORT_FAILURE("delta", payoff, exercise, values[i].s,
                       values[i].q, values[i].r, today,
                       values[i].v, values[i].result, calculated,
                       error, tolerance);

    i++;
    payoff = boost::shared_ptr<StrikedTypePayoff>(new
        PlainVanillaPayoff(values[i].type, values[i].strike));
    exDate = today + timeToDays(values[i].t);
    exercise = boost::shared_ptr<Exercise>(new EuropeanExercise(exDate));
    spot ->setValue(values[i].s);
    qRate->setValue(values[i].q);
    rRate->setValue(values[i].r);
    vol  ->setValue(values[i].v);
    option = boost::shared_ptr<VanillaOption>(
                                        new EuropeanOption(payoff, exercise));
    option->setPricingEngine(engine);
    calculated = option->delta();
    error = std::fabs(calculated-values[i].result);
    if (error>tolerance)
        REPORT_FAILURE("delta", payoff, exercise, values[i].s,
                       values[i].q, values[i].r, today,
                       values[i].v, values[i].result, calculated,
                       error, tolerance);

    i++;
    payoff = boost::shared_ptr<StrikedTypePayoff>(new
        PlainVanillaPayoff(values[i].type, values[i].strike));
    exDate = today + timeToDays(values[i].t);
    exercise = boost::shared_ptr<Exercise>(new EuropeanExercise(exDate));
    spot ->setValue(values[i].s);
    qRate->setValue(values[i].q);
    rRate->setValue(values[i].r);
    vol  ->setValue(values[i].v);
    option = boost::shared_ptr<VanillaOption>(
                                        new EuropeanOption(payoff, exercise));
    option->setPricingEngine(engine);
    calculated = option->elasticity();
    error = std::fabs(calculated-values[i].result);
    if (error>tolerance)
        REPORT_FAILURE("elasticity", payoff, exercise, values[i].s,
                       values[i].q, values[i].r, today,
                       values[i].v, values[i].result, calculated,
                       error, tolerance);


    i++;
    payoff = boost::shared_ptr<StrikedTypePayoff>(new
        PlainVanillaPayoff(values[i].type, values[i].strike));
    exDate = today + timeToDays(values[i].t);
    exercise = boost::shared_ptr<Exercise>(new EuropeanExercise(exDate));
    spot ->setValue(values[i].s);
    qRate->setValue(values[i].q);
    rRate->setValue(values[i].r);
    vol  ->setValue(values[i].v);
    option = boost::shared_ptr<VanillaOption>(
                                        new EuropeanOption(payoff, exercise));
    option->setPricingEngine(engine);
    calculated = option->gamma();
    error = std::fabs(calculated-values[i].result);
    if (error>tolerance)
        REPORT_FAILURE("gamma", payoff, exercise, values[i].s,
                       values[i].q, values[i].r, today,
                       values[i].v, values[i].result, calculated,
                       error, tolerance);

    i++;
    payoff = boost::shared_ptr<StrikedTypePayoff>(new
        PlainVanillaPayoff(values[i].type, values[i].strike));
    exDate = today + timeToDays(values[i].t);
    exercise = boost::shared_ptr<Exercise>(new EuropeanExercise(exDate));
    spot ->setValue(values[i].s);
    qRate->setValue(values[i].q);
    rRate->setValue(values[i].r);
    vol  ->setValue(values[i].v);
    option = boost::shared_ptr<VanillaOption>(
                                        new EuropeanOption(payoff, exercise));
    option->setPricingEngine(engine);
    calculated = option->gamma();
    error = std::fabs(calculated-values[i].result);
    if (error>tolerance)
        REPORT_FAILURE("gamma", payoff, exercise, values[i].s,
                       values[i].q, values[i].r, today,
                       values[i].v, values[i].result, calculated,
                       error, tolerance);


    i++;
    payoff = boost::shared_ptr<StrikedTypePayoff>(new
        PlainVanillaPayoff(values[i].type, values[i].strike));
    exDate = today + timeToDays(values[i].t);
    exercise = boost::shared_ptr<Exercise>(new EuropeanExercise(exDate));
    spot ->setValue(values[i].s);
    qRate->setValue(values[i].q);
    rRate->setValue(values[i].r);
    vol  ->setValue(values[i].v);
    option = boost::shared_ptr<VanillaOption>(
                                        new EuropeanOption(payoff, exercise));
    option->setPricingEngine(engine);
    calculated = option->vega();
    error = std::fabs(calculated-values[i].result);
    if (error>tolerance)
        REPORT_FAILURE("vega", payoff, exercise, values[i].s,
                       values[i].q, values[i].r, today,
                       values[i].v, values[i].result, calculated,
                       error, tolerance);


    i++;
    payoff = boost::shared_ptr<StrikedTypePayoff>(new
        PlainVanillaPayoff(values[i].type, values[i].strike));
    exDate = today + timeToDays(values[i].t);
    exercise = boost::shared_ptr<Exercise>(new EuropeanExercise(exDate));
    spot ->setValue(values[i].s);
    qRate->setValue(values[i].q);
    rRate->setValue(values[i].r);
    vol  ->setValue(values[i].v);
    option = boost::shared_ptr<VanillaOption>(
                                        new EuropeanOption(payoff, exercise));
    option->setPricingEngine(engine);
    calculated = option->vega();
    error = std::fabs(calculated-values[i].result);
    if (error>tolerance)
        REPORT_FAILURE("vega", payoff, exercise, values[i].s,
                       values[i].q, values[i].r, today,
                       values[i].v, values[i].result, calculated,
                       error, tolerance);


    i++;
    payoff = boost::shared_ptr<StrikedTypePayoff>(new
        PlainVanillaPayoff(values[i].type, values[i].strike));
    exDate = today + timeToDays(values[i].t);
    exercise = boost::shared_ptr<Exercise>(new EuropeanExercise(exDate));
    spot ->setValue(values[i].s);
    qRate->setValue(values[i].q);
    rRate->setValue(values[i].r);
    vol  ->setValue(values[i].v);
    option = boost::shared_ptr<VanillaOption>(
                                        new EuropeanOption(payoff, exercise));
    option->setPricingEngine(engine);
    calculated = option->theta();
    error = std::fabs(calculated-values[i].result);
    if (error>tolerance)
        REPORT_FAILURE("theta", payoff, exercise, values[i].s,
                       values[i].q, values[i].r, today,
                       values[i].v, values[i].result, calculated,
                       error, tolerance);


    i++;
    payoff = boost::shared_ptr<StrikedTypePayoff>(new
        PlainVanillaPayoff(values[i].type, values[i].strike));
    exDate = today + timeToDays(values[i].t);
    exercise = boost::shared_ptr<Exercise>(new EuropeanExercise(exDate));
    spot ->setValue(values[i].s);
    qRate->setValue(values[i].q);
    rRate->setValue(values[i].r);
    vol  ->setValue(values[i].v);
    option = boost::shared_ptr<VanillaOption>(
                                        new EuropeanOption(payoff, exercise));
    option->setPricingEngine(engine);
    calculated = option->thetaPerDay();
    error = std::fabs(calculated-values[i].result);
    if (error>tolerance)
        REPORT_FAILURE("thetaPerDay", payoff, exercise, values[i].s,
                       values[i].q, values[i].r, today,
                       values[i].v, values[i].result, calculated,
                       error, tolerance);


    i++;
    payoff = boost::shared_ptr<StrikedTypePayoff>(new
        PlainVanillaPayoff(values[i].type, values[i].strike));
    exDate = today + timeToDays(values[i].t);
    exercise = boost::shared_ptr<Exercise>(new EuropeanExercise(exDate));
    spot ->setValue(values[i].s);
    qRate->setValue(values[i].q);
    rRate->setValue(values[i].r);
    vol  ->setValue(values[i].v);
    option = boost::shared_ptr<VanillaOption>(
                                        new EuropeanOption(payoff, exercise));
    option->setPricingEngine(engine);
    calculated = option->rho();
    error = std::fabs(calculated-values[i].result);
    if (error>tolerance)
        REPORT_FAILURE("rho", payoff, exercise, values[i].s,
                       values[i].q, values[i].r, today,
                       values[i].v, values[i].result, calculated,
                       error, tolerance);


    i++;
    payoff = boost::shared_ptr<StrikedTypePayoff>(new
        PlainVanillaPayoff(values[i].type, values[i].strike));
    exDate = today + timeToDays(values[i].t);
    exercise = boost::shared_ptr<Exercise>(new EuropeanExercise(exDate));
    spot ->setValue(values[i].s);
    qRate->setValue(values[i].q);
    rRate->setValue(values[i].r);
    vol  ->setValue(values[i].v);
    option = boost::shared_ptr<VanillaOption>(
                                        new EuropeanOption(payoff, exercise));
    option->setPricingEngine(engine);
    calculated = option->dividendRho();
    error = std::fabs(calculated-values[i].result);
    if (error>tolerance)
        REPORT_FAILURE("dividendRho", payoff, exercise, values[i].s,
                       values[i].q, values[i].r, today,
                       values[i].v, values[i].result, calculated,
                       error, tolerance);

}

void EuropeanOptionTest::testGreeks() {

    BOOST_TEST_MESSAGE("Testing analytic European option greeks...");

    SavedSettings backup;

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"]  = 1.0e-5;
    tolerance["gamma"]  = 1.0e-5;
    tolerance["theta"]  = 1.0e-5;
    tolerance["rho"]    = 1.0e-5;
    tolerance["divRho"] = 1.0e-5;
    tolerance["vega"]   = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Time residualTimes[] = { 1.0, 2.0 };
    Volatility vols[] = { 0.11, 0.50, 1.20 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    boost::shared_ptr<StrikedTypePayoff> payoff;

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(residualTimes); k++) {
          Date exDate = today + timeToDays(residualTimes[k]);
          boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
          for (Size kk=0; kk<4; kk++) {
              // option to check
              if (kk==0) {
                  payoff = boost::shared_ptr<StrikedTypePayoff>(new
                    PlainVanillaPayoff(types[i], strikes[j]));
              } else if (kk==1) {
                  payoff = boost::shared_ptr<StrikedTypePayoff>(new
                    CashOrNothingPayoff(types[i], strikes[j],
                    100.0));
              } else if (kk==2) {
                  payoff = boost::shared_ptr<StrikedTypePayoff>(new
                    AssetOrNothingPayoff(types[i], strikes[j]));
              } else if (kk==3) {
                  payoff = boost::shared_ptr<StrikedTypePayoff>(new
                    GapPayoff(types[i], strikes[j], 100.0));
              }

              boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));
              boost::shared_ptr<PricingEngine> engine(
                                    new AnalyticEuropeanEngine(stochProcess));
              EuropeanOption option(payoff, exercise);
              option.setPricingEngine(engine);

              for (Size l=0; l<LENGTH(underlyings); l++) {
                for (Size m=0; m<LENGTH(qRates); m++) {
                  for (Size n=0; n<LENGTH(rRates); n++) {
                    for (Size p=0; p<LENGTH(vols); p++) {
                      Real u = underlyings[l];
                      Rate q = qRates[m],
                           r = rRates[n];
                      Volatility v = vols[p];
                      spot->setValue(u);
                      qRate->setValue(q);
                      rRate->setValue(r);
                      vol->setValue(v);

                      Real value = option.NPV();
                      calculated["delta"]  = option.delta();
                      calculated["gamma"]  = option.gamma();
                      calculated["theta"]  = option.theta();
                      calculated["rho"]    = option.rho();
                      calculated["divRho"] = option.dividendRho();
                      calculated["vega"]   = option.vega();

                      if (value > spot->value()*1.0e-5) {
                          // perturb spot and get delta and gamma
                          Real du = u*1.0e-4;
                          spot->setValue(u+du);
                          Real value_p = option.NPV(),
                               delta_p = option.delta();
                          spot->setValue(u-du);
                          Real value_m = option.NPV(),
                               delta_m = option.delta();
                          spot->setValue(u);
                          expected["delta"] = (value_p - value_m)/(2*du);
                          expected["gamma"] = (delta_p - delta_m)/(2*du);

                          // perturb rates and get rho and dividend rho
                          Spread dr = r*1.0e-4;
                          rRate->setValue(r+dr);
                          value_p = option.NPV();
                          rRate->setValue(r-dr);
                          value_m = option.NPV();
                          rRate->setValue(r);
                          expected["rho"] = (value_p - value_m)/(2*dr);

                          Spread dq = q*1.0e-4;
                          qRate->setValue(q+dq);
                          value_p = option.NPV();
                          qRate->setValue(q-dq);
                          value_m = option.NPV();
                          qRate->setValue(q);
                          expected["divRho"] = (value_p - value_m)/(2*dq);

                          // perturb volatility and get vega
                          Volatility dv = v*1.0e-4;
                          vol->setValue(v+dv);
                          value_p = option.NPV();
                          vol->setValue(v-dv);
                          value_m = option.NPV();
                          vol->setValue(v);
                          expected["vega"] = (value_p - value_m)/(2*dv);

                          // perturb date and get theta
                          Time dT = dc.yearFraction(today-1, today+1);
                          Settings::instance().evaluationDate() = today-1;
                          value_m = option.NPV();
                          Settings::instance().evaluationDate() = today+1;
                          value_p = option.NPV();
                          Settings::instance().evaluationDate() = today;
                          expected["theta"] = (value_p - value_m)/dT;

                          // compare
                          std::map<std::string,Real>::iterator it;
                          for (it = calculated.begin();
                               it != calculated.end(); ++it) {
                              std::string greek = it->first;
                              Real expct = expected  [greek],
                                   calcl = calculated[greek],
                                   tol   = tolerance [greek];
                              Real error = relativeError(expct,calcl,u);
                              if (error>tol) {
                                  REPORT_FAILURE(greek, payoff, exercise,
                                                 u, q, r, today, v,
                                                 expct, calcl, error, tol);
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

void EuropeanOptionTest::testImpliedVol() {

    BOOST_TEST_MESSAGE("Testing European option implied volatility...");

    SavedSettings backup;

    Size maxEvaluations = 100;
    Real tolerance = 1.0e-6;

    // test options
    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 90.0, 99.5, 100.0, 100.5, 110.0 };
    Integer lengths[] = { 36, 180, 360, 1080 };

    // test data
    Real underlyings[] = { 90.0, 95.0, 99.9, 100.0, 100.1, 105.0, 110.0 };
    Rate qRates[] = { 0.01, 0.05, 0.10 };
    Rate rRates[] = { 0.01, 0.05, 0.10 };
    Volatility vols[] = { 0.01, 0.20, 0.30, 0.70, 0.90 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {
          // option to check
          Date exDate = today + lengths[k];
          boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
          boost::shared_ptr<StrikedTypePayoff> payoff(
                                new PlainVanillaPayoff(types[i], strikes[j]));
          boost::shared_ptr<VanillaOption> option =
              makeOption(payoff, exercise, spot, qTS, rTS, volTS,
                         Analytic, Null<Size>(), Null<Size>());

          boost::shared_ptr<GeneralizedBlackScholesProcess> process =
              makeProcess(spot, qTS, rTS,volTS);

          for (Size l=0; l<LENGTH(underlyings); l++) {
            for (Size m=0; m<LENGTH(qRates); m++) {
              for (Size n=0; n<LENGTH(rRates); n++) {
                for (Size p=0; p<LENGTH(vols); p++) {
                  Real u = underlyings[l];
                  Rate q = qRates[m],
                       r = rRates[n];
                  Volatility v = vols[p];
                  spot->setValue(u);
                  qRate->setValue(q);
                  rRate->setValue(r);
                  vol->setValue(v);

                  Real value = option->NPV();
                  Volatility implVol = 0.0; // just to remove a warning...
                  if (value != 0.0) {
                      // shift guess somehow
                      vol->setValue(v*0.5);
                      if (std::fabs(value-option->NPV()) <= 1.0e-12) {
                          // flat price vs vol --- pointless (and
                          // numerically unstable) to solve
                          continue;
                      }
                      try {
                          implVol = option->impliedVolatility(value,
                                                              process,
                                                              tolerance,
                                                              maxEvaluations);
                      } catch (std::exception& e) {
                          BOOST_ERROR(
                              "\nimplied vol calculation failed:" <<
                              "\n   option:         " << types[i] <<
                              "\n   strike:         " << strikes[j] <<
                              "\n   spot value:     " << u <<
                              "\n   dividend yield: " << io::rate(q) <<
                              "\n   risk-free rate: " << io::rate(r) <<
                              "\n   today:          " << today <<
                              "\n   maturity:       " << exDate <<
                              "\n   volatility:     " << io::volatility(v) <<
                              "\n   option value:   " << value <<
                              "\n" << e.what());
                      }
                      if (std::fabs(implVol-v) > tolerance) {
                          // the difference might not matter
                          vol->setValue(implVol);
                          Real value2 = option->NPV();
                          Real error = relativeError(value,value2,u);
                          if (error > tolerance) {
                              BOOST_ERROR(
                                  types[i] << " option :\n"
                                  << "    spot value:          " << u << "\n"
                                  << "    strike:              "
                                  << strikes[j] << "\n"
                                  << "    dividend yield:      "
                                  << io::rate(q) << "\n"
                                  << "    risk-free rate:      "
                                  << io::rate(r) << "\n"
                                  << "    maturity:            "
                                  << exDate << "\n\n"
                                  << "    original volatility: "
                                  << io::volatility(v) << "\n"
                                  << "    price:               "
                                  << value << "\n"
                                  << "    implied volatility:  "
                                  << io::volatility(implVol)
                                  << "\n"
                                  << "    corresponding price: "
                                  << value2 << "\n"
                                  << "    error:               " << error);
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


void EuropeanOptionTest::testImpliedVolContainment() {

    BOOST_TEST_MESSAGE("Testing self-containment of "
                       "implied volatility calculation...");

    SavedSettings backup;

    Size maxEvaluations = 100;
    Real tolerance = 1.0e-6;

    // test options

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    Handle<Quote> underlying(spot);
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.05));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.03));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    Date exerciseDate = today + 1*Years;
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));
    boost::shared_ptr<StrikedTypePayoff> payoff(
                                 new PlainVanillaPayoff(Option::Call, 100.0));

    boost::shared_ptr<BlackScholesMertonProcess> process(
                  new BlackScholesMertonProcess(underlying, qTS, rTS, volTS));
    boost::shared_ptr<PricingEngine> engine(
                                        new AnalyticEuropeanEngine(process));
    // link to the same stochastic process, which shouldn't be changed
    // by calling methods of either option

    boost::shared_ptr<VanillaOption> option1(
                                        new EuropeanOption(payoff, exercise));
    option1->setPricingEngine(engine);
    boost::shared_ptr<VanillaOption> option2(
                                        new EuropeanOption(payoff, exercise));
    option2->setPricingEngine(engine);

    // test

    Real refValue = option2->NPV();

    Flag f;
    f.registerWith(option2);

    option1->impliedVolatility(refValue*1.5, process,
                               tolerance, maxEvaluations);

    if (f.isUp())
        BOOST_ERROR("implied volatility calculation triggered a change "
                    "in another instrument");

    option2->recalculate();
    if (std::fabs(option2->NPV() - refValue) >= 1.0e-8)
        BOOST_ERROR("implied volatility calculation changed the value "
                    << "of another instrument: \n"
                    << std::setprecision(8)
                    << "previous value: " << refValue << "\n"
                    << "current value:  " << option2->NPV());

    vol->setValue(vol->value()*1.5);

    if (!f.isUp())
        BOOST_ERROR("volatility change not notified");

    if (std::fabs(option2->NPV() - refValue) <= 1.0e-8)
        BOOST_ERROR("volatility change did not cause the value to change");

}


// different engines

namespace {

    void testEngineConsistency(EngineType engine,
                               Size binomialSteps,
                               Size samples,
                               std::map<std::string,Real> tolerance,
                               bool testGreeks = false) {

        QL_TEST_START_TIMING

            std::map<std::string,Real> calculated, expected;

        // test options
        Option::Type types[] = { Option::Call, Option::Put };
        Real strikes[] = { 75.0, 100.0, 125.0 };
        Integer lengths[] = { 1 };

        // test data
        Real underlyings[] = { 100.0 };
        Rate qRates[] = { 0.00, 0.05 };
        Rate rRates[] = { 0.01, 0.05, 0.15 };
        Volatility vols[] = { 0.11, 0.50, 1.20 };

        DayCounter dc = Actual360();
        Date today = Date::todaysDate();

        boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
        boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
        boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today,vol,dc);
        boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
        boost::shared_ptr<YieldTermStructure> qTS = flatRate(today,qRate,dc);
        boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
        boost::shared_ptr<YieldTermStructure> rTS = flatRate(today,rRate,dc);

        for (Size i=0; i<LENGTH(types); i++) {
          for (Size j=0; j<LENGTH(strikes); j++) {
            for (Size k=0; k<LENGTH(lengths); k++) {
              Date exDate = today + lengths[k]*360;
              boost::shared_ptr<Exercise> exercise(
                                                new EuropeanExercise(exDate));
              boost::shared_ptr<StrikedTypePayoff> payoff(new
                                    PlainVanillaPayoff(types[i], strikes[j]));
              // reference option
              boost::shared_ptr<VanillaOption> refOption =
                  makeOption(payoff, exercise, spot, qTS, rTS, volTS,
                             Analytic, Null<Size>(), Null<Size>());
              // option to check
              boost::shared_ptr<VanillaOption> option =
                  makeOption(payoff, exercise, spot, qTS, rTS, volTS,
                             engine, binomialSteps, samples);

              for (Size l=0; l<LENGTH(underlyings); l++) {
                for (Size m=0; m<LENGTH(qRates); m++) {
                  for (Size n=0; n<LENGTH(rRates); n++) {
                    for (Size p=0; p<LENGTH(vols); p++) {
                      Real u = underlyings[l];
                      Rate q = qRates[m],
                           r = rRates[n];
                      Volatility v = vols[p];
                      spot->setValue(u);
                      qRate->setValue(q);
                      rRate->setValue(r);
                      vol->setValue(v);

                      expected.clear();
                      calculated.clear();

                      // FLOATING_POINT_EXCEPTION
                      expected["value"] = refOption->NPV();
                      calculated["value"] = option->NPV();

                      if (testGreeks && option->NPV() > spot->value()*1.0e-5) {
                           expected["delta"] = refOption->delta();
                           expected["gamma"] = refOption->gamma();
                           expected["theta"] = refOption->theta();
                           calculated["delta"] = option->delta();
                           calculated["gamma"] = option->gamma();
                           calculated["theta"] = option->theta();
                      }
                      std::map<std::string,Real>::iterator it;
                      for (it = calculated.begin();
                           it != calculated.end(); ++it) {
                          std::string greek = it->first;
                          Real expct = expected  [greek],
                               calcl = calculated[greek],
                               tol   = tolerance [greek];
                          Real error = relativeError(expct,calcl,u);
                          if (error > tol) {
                              REPORT_FAILURE(greek, payoff, exercise,
                                             u, q, r, today, v,
                                             expct, calcl, error, tol);
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


void EuropeanOptionTest::testJRBinomialEngines() {

    BOOST_TEST_MESSAGE("Testing JR binomial European engines "
                       "against analytic results...");

    SavedSettings backup;

    EngineType engine = JR;
    Size steps = 251;
    Size samples = Null<Size>();
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 0.002;
    relativeTol["delta"] = 1.0e-3;
    relativeTol["gamma"] = 1.0e-4;
    relativeTol["theta"] = 0.03;
    testEngineConsistency(engine,steps,samples,relativeTol,true);
}

void EuropeanOptionTest::testCRRBinomialEngines() {

    BOOST_TEST_MESSAGE("Testing CRR binomial European engines "
                       "against analytic results...");

    SavedSettings backup;

    EngineType engine = CRR;
    Size steps = 501;
    Size samples = Null<Size>();
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 0.02;
    relativeTol["delta"] = 1.0e-3;
    relativeTol["gamma"] = 1.0e-4;
    relativeTol["theta"] = 0.03;
    testEngineConsistency(engine,steps,samples,relativeTol,true);
}

void EuropeanOptionTest::testEQPBinomialEngines() {

    BOOST_TEST_MESSAGE("Testing EQP binomial European engines "
                       "against analytic results...");

    SavedSettings backup;

    EngineType engine = EQP;
    Size steps = 501;
    Size samples = Null<Size>();
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 0.02;
    relativeTol["delta"] = 1.0e-3;
    relativeTol["gamma"] = 1.0e-4;
    relativeTol["theta"] = 0.03;
    testEngineConsistency(engine,steps,samples,relativeTol,true);
}

void EuropeanOptionTest::testTGEOBinomialEngines() {

    BOOST_TEST_MESSAGE("Testing TGEO binomial European engines "
                       "against analytic results...");

    SavedSettings backup;

    EngineType engine = TGEO;
    Size steps = 251;
    Size samples = Null<Size>();
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 0.002;
    relativeTol["delta"] = 1.0e-3;
    relativeTol["gamma"] = 1.0e-4;
    relativeTol["theta"] = 0.03;
    testEngineConsistency(engine,steps,samples,relativeTol,true);
}

void EuropeanOptionTest::testTIANBinomialEngines() {

    BOOST_TEST_MESSAGE("Testing TIAN binomial European engines "
                       "against analytic results...");

    SavedSettings backup;

    EngineType engine = TIAN;
    Size steps = 251;
    Size samples = Null<Size>();
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 0.002;
    relativeTol["delta"] = 1.0e-3;
    relativeTol["gamma"] = 1.0e-4;
    relativeTol["theta"] = 0.03;
    testEngineConsistency(engine,steps,samples,relativeTol,true);
}

void EuropeanOptionTest::testLRBinomialEngines() {

    BOOST_TEST_MESSAGE("Testing LR binomial European engines "
                       "against analytic results...");

    SavedSettings backup;

    EngineType engine = LR;
    Size steps = 251;
    Size samples = Null<Size>();
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 1.0e-6;
    relativeTol["delta"] = 1.0e-3;
    relativeTol["gamma"] = 1.0e-4;
    relativeTol["theta"] = 0.03;
    testEngineConsistency(engine,steps,samples,relativeTol,true);
}

void EuropeanOptionTest::testJOSHIBinomialEngines() {

    BOOST_TEST_MESSAGE("Testing Joshi binomial European engines "
                       "against analytic results...");

    SavedSettings backup;

    EngineType engine = JOSHI;
    Size steps = 251;
    Size samples = Null<Size>();
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 1.0e-7;
    relativeTol["delta"] = 1.0e-3;
    relativeTol["gamma"] = 1.0e-4;
    relativeTol["theta"] = 0.03;
    testEngineConsistency(engine,steps,samples,relativeTol,true);
}

void EuropeanOptionTest::testFdEngines() {

    BOOST_TEST_MESSAGE("Testing finite-difference European engines "
                       "against analytic results...");

    SavedSettings backup;

    EngineType engine = FiniteDifferences;
    Size timeSteps = 300;
    Size gridPoints = 300;
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 1.0e-4;
    relativeTol["delta"] = 1.0e-6;
    relativeTol["gamma"] = 1.0e-6;
    relativeTol["theta"] = 1.0e-4;
    testEngineConsistency(engine,timeSteps,gridPoints,relativeTol,true);
}

void EuropeanOptionTest::testIntegralEngines() {

    BOOST_TEST_MESSAGE("Testing integral engines against analytic results...");

    SavedSettings backup;

    EngineType engine = Integral;
    Size timeSteps = 300;
    Size gridPoints = 300;
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 0.0001;
    testEngineConsistency(engine,timeSteps,gridPoints,relativeTol);
}

void EuropeanOptionTest::testMcEngines() {

    BOOST_TEST_MESSAGE("Testing Monte Carlo European engines "
                       "against analytic results...");

    SavedSettings backup;

    EngineType engine = PseudoMonteCarlo;
    Size steps = Null<Size>();
    Size samples = 40000;
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 0.01;
    testEngineConsistency(engine,steps,samples,relativeTol);
}

void EuropeanOptionTest::testQmcEngines() {

    BOOST_TEST_MESSAGE("Testing Quasi Monte Carlo European engines "
                       "against analytic results...");

    SavedSettings backup;

    EngineType engine = QuasiMonteCarlo;
    Size steps = Null<Size>();
    Size samples = 4095; // 2^12-1
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 0.01;
    testEngineConsistency(engine,steps,samples,relativeTol);
}

void EuropeanOptionTest::testFFTEngines() {

    BOOST_TEST_MESSAGE("Testing FFT European engines "
                       "against analytic results...");

    SavedSettings backup;

    EngineType engine = FFT;
    Size steps = Null<Size>();
    Size samples = Null<Size>();
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 0.01;
    testEngineConsistency(engine,steps,samples,relativeTol);
}


void EuropeanOptionTest::testPriceCurve() {

    BOOST_TEST_MESSAGE("Testing European price curves...");

    SavedSettings backup;

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
    */
    EuropeanOptionData values[] = {
      // pag 2-8
      //        type, strike,   spot,    q,    r,    t,  vol,   value
      { Option::Call,  65.00,  60.00, 0.00, 0.08, 0.25, 0.30,  2.1334, 0.0},
      { Option::Put,   95.00, 100.00, 0.05, 0.10, 0.50, 0.20,  2.4648, 0.0},
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Size timeSteps = 300;
    Size gridPoints = 300;

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));
    boost::shared_ptr<PricingEngine> engine(
                             new FDEuropeanEngine<CrankNicolson>(stochProcess,
                                                                 timeSteps,
                                                                 gridPoints));

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));
        // FLOATING_POINT_EXCEPTION
        Date exDate = today + timeToDays(values[i].t);
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        EuropeanOption option(payoff, exercise);
        option.setPricingEngine(engine);
        SampledCurve price_curve = option.result<SampledCurve>("priceCurve");
        if (price_curve.empty()) {
            REPORT_FAILURE("no price curve", payoff, exercise, values[i].s,
                           values[i].q, values[i].r, today,
                           values[i].v, values[i].result, 0.0,
                           0.0, 0.0);
            continue;
        }

        // Ignore the end points
        Size start = price_curve.size() / 4;
        Size end = price_curve.size() * 3 / 4;
        for (Size i=start; i < end; i++) {
            spot->setValue(price_curve.gridValue(i));
            boost::shared_ptr<StochasticProcess> stochProcess1(
                      new BlackScholesMertonProcess(
                                       Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));
            boost::shared_ptr<PricingEngine> engine1(
                             new FDEuropeanEngine<CrankNicolson>(stochProcess,
                                                                 timeSteps,
                                                                 gridPoints));

            option.setPricingEngine(engine1);
            Real calculated = option.NPV();
            Real error = std::fabs(calculated-price_curve.value(i));
            Real tolerance = 1e-3;
            if (error>tolerance) {
                REPORT_FAILURE("price curve error", payoff, exercise,
                               price_curve.gridValue(i),
                               values[i].q, values[i].r, today,
                               values[i].v,
                               price_curve.value(i), calculated,
                               error, tolerance);
                break;
            }
        }
    }

}


void EuropeanOptionTest::testLocalVolatility() {
    BOOST_TEST_MESSAGE("Testing finite-differences with local volatility...");

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
    const boost::shared_ptr<YieldTermStructure> rTS(
                                   new ZeroCurve(dates, rates, dayCounter));
    const boost::shared_ptr<YieldTermStructure> qTS(
                                   flatRate(settlementDate, 0.0, dayCounter));

    const boost::shared_ptr<Quote> s0(new SimpleQuote(4500.00));
    
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
                                 std::vector<Date>(dates.begin()+1, dates.end()),
                                 strikes, blackVolMatrix,
                                 dayCounter));
    volTS->setInterpolation<Bicubic>();
    const boost::shared_ptr<GeneralizedBlackScholesProcess> process =
                                              makeProcess(s0, qTS, rTS,volTS);
    
    for (Size i=2; i < dates.size(); ++i) {
        for (Size j=3; j < strikes.size()-5; j+=5) {
            const Date& exDate = dates[i];
            const boost::shared_ptr<StrikedTypePayoff> payoff(new
                                 PlainVanillaPayoff(Option::Call, strikes[j]));
    
            const boost::shared_ptr<Exercise> exercise(
                                                 new EuropeanExercise(exDate));
    
            EuropeanOption option(payoff, exercise);
            option.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                         new AnalyticEuropeanEngine(process)));
             
            const Real tol = 0.001;
            const Real expectedNPV   = option.NPV();
            const Real expectedDelta = option.delta();
            const Real expectedGamma = option.gamma();
            
            option.setPricingEngine(boost::shared_ptr<PricingEngine>(
                         new FdBlackScholesVanillaEngine(process, 200, 400)));
    
            Real calculatedNPV = option.NPV();
            const Real calculatedDelta = option.delta();
            const Real calculatedGamma = option.gamma();
            
            // check implied pricing first
            if (std::fabs(expectedNPV - calculatedNPV) > tol*expectedNPV) {
                BOOST_FAIL("Failed to reproduce option price for "
                           << "\n    strike:     " << payoff->strike()
                           << "\n    maturity:   " << exDate
                           << "\n    calculated: " << calculatedNPV
                           << "\n    expected:   " << expectedNPV);
            }
            if (std::fabs(expectedDelta - calculatedDelta) >tol*expectedDelta) {
                BOOST_FAIL("Failed to reproduce option delta for "
                           << "\n    strike:     " << payoff->strike()
                           << "\n    maturity:   " << exDate
                           << "\n    calculated: " << calculatedDelta
                           << "\n    expected:   " << expectedDelta);
            }
            if (std::fabs(expectedGamma - calculatedGamma) >tol*expectedGamma) {
                BOOST_FAIL("Failed to reproduce option gamma for "
                           << "\n    strike:     " << payoff->strike()
                           << "\n    maturity:   " << exDate
                           << "\n    calculated: " << calculatedGamma
                           << "\n    expected:   " << expectedGamma);
            }
            
            // check local vol pricing
            // delta/gamma are not the same by definition (model implied greeks)
            option.setPricingEngine(boost::shared_ptr<PricingEngine>(
                    new FdBlackScholesVanillaEngine(process, 25, 400, 0, 
                                                    FdmSchemeDesc::Douglas(), 
                                                    true, 0.35)));
            calculatedNPV = option.NPV();
            if (std::fabs(expectedNPV - calculatedNPV) > tol*expectedNPV) {
                BOOST_FAIL("Failed to reproduce local vol option price for "
                           << "\n    strike:     " << payoff->strike()
                           << "\n    maturity:   " << exDate
                           << "\n    calculated: " << calculatedNPV
                           << "\n    expected:   " << expectedNPV);
            }
        }
    }
}


test_suite* EuropeanOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("European option tests");
    suite->add(QUANTLIB_TEST_CASE(&EuropeanOptionTest::testValues));
    suite->add(QUANTLIB_TEST_CASE(&EuropeanOptionTest::testGreekValues));
    suite->add(QUANTLIB_TEST_CASE(&EuropeanOptionTest::testGreeks));
    suite->add(QUANTLIB_TEST_CASE(&EuropeanOptionTest::testImpliedVol));
    suite->add(QUANTLIB_TEST_CASE(
                           &EuropeanOptionTest::testImpliedVolContainment));
    suite->add(QUANTLIB_TEST_CASE(&EuropeanOptionTest::testJRBinomialEngines));
    suite->add(QUANTLIB_TEST_CASE(&EuropeanOptionTest::testCRRBinomialEngines));
    suite->add(QUANTLIB_TEST_CASE(&EuropeanOptionTest::testEQPBinomialEngines));
    suite->add(QUANTLIB_TEST_CASE(
                               &EuropeanOptionTest::testTGEOBinomialEngines));
    suite->add(QUANTLIB_TEST_CASE(
                               &EuropeanOptionTest::testTIANBinomialEngines));
    suite->add(QUANTLIB_TEST_CASE(&EuropeanOptionTest::testLRBinomialEngines));
    suite->add(QUANTLIB_TEST_CASE(
                              &EuropeanOptionTest::testJOSHIBinomialEngines));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&EuropeanOptionTest::testFdEngines));
    suite->add(QUANTLIB_TEST_CASE(&EuropeanOptionTest::testIntegralEngines));
    suite->add(QUANTLIB_TEST_CASE(&EuropeanOptionTest::testMcEngines));
    suite->add(QUANTLIB_TEST_CASE(&EuropeanOptionTest::testQmcEngines));

    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&EuropeanOptionTest::testPriceCurve));
    suite->add(QUANTLIB_TEST_CASE(&EuropeanOptionTest::testLocalVolatility));

    return suite;
}

test_suite* EuropeanOptionTest::experimental() {
    test_suite* suite = BOOST_TEST_SUITE("European option tests");
    suite->add(QUANTLIB_TEST_CASE(&EuropeanOptionTest::testFFTEngines));
    return suite;
}
