/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2003 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "europeanoption.hpp"
#include "utilities.hpp"
#include <ql/DayCounters/actual360.hpp>
#include <ql/Instruments/europeanoption.hpp>
#include <ql/RandomNumbers/rngtraits.hpp>
#include <ql/PricingEngines/Vanilla/analyticeuropeanengine.hpp>
#include <ql/PricingEngines/Vanilla/binomialengine.hpp>
#include <ql/PricingEngines/Vanilla/fdeuropeanengine.hpp>
#include <ql/PricingEngines/Vanilla/mceuropeanengine.hpp>
#include <ql/PricingEngines/Vanilla/integralengine.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Utilities/dataformatters.hpp>
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
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);

QL_BEGIN_TEST_LOCALS(EuropeanOptionTest)

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
                  JR, CRR, EQP, TGEO, TIAN, LR,
                  FiniteDifferences,
                  Integral,
                  PseudoMonteCarlo, QuasiMonteCarlo };

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

    boost::shared_ptr<PricingEngine> engine;
    switch (engineType) {
      case Analytic:
        engine = boost::shared_ptr<PricingEngine>(new AnalyticEuropeanEngine);
        break;
      case JR:
        engine = boost::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<JarrowRudd>(binomialSteps));
        break;
      case CRR:
        engine = boost::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<CoxRossRubinstein>(binomialSteps));
      case EQP:
        engine = boost::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<AdditiveEQPBinomialTree>(
                                                              binomialSteps));
        break;
      case TGEO:
        engine = boost::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<Trigeorgis>(binomialSteps));
        break;
      case TIAN:
        engine = boost::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<Tian>(binomialSteps));
        break;
      case LR:
        engine = boost::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<LeisenReimer>(binomialSteps));
        break;
      case FiniteDifferences:
        engine = boost::shared_ptr<PricingEngine>(
                new FDEuropeanEngine(binomialSteps,samples));
        break;
      case Integral:
          engine = boost::shared_ptr<PricingEngine>(
                new IntegralEngine());
          break;
      case PseudoMonteCarlo:
        engine = MakeMCEuropeanEngine<PseudoRandom>().withSteps(1)
                                                     .withSamples(samples)
                                                     .withSeed(42);
        break;
      case QuasiMonteCarlo:
        engine = MakeMCEuropeanEngine<LowDiscrepancy>().withSteps(1)
                                                       .withSamples(samples);
        break;
      default:
        QL_FAIL("unknown engine type");
    }

    boost::shared_ptr<StochasticProcess> stochProcess(
           new BlackScholesMertonProcess(Handle<Quote>(u),
                                         Handle<YieldTermStructure>(q),
                                         Handle<YieldTermStructure>(r),
                                         Handle<BlackVolTermStructure>(vol)));

    return boost::shared_ptr<VanillaOption>(
                  new EuropeanOption(stochProcess, payoff, exercise, engine));
}

std::string engineTypeToString(EngineType type) {
    switch (type) {
      case Analytic:
        return "analytic";
      case JR:
        return "Jarrow-Rudd";
      case CRR:
        return "Cox-Ross-Rubinstein";
      case EQP:
        return "EQP";
      case TGEO:
        return "Trigeorgis";
      case TIAN:
        return "Tian";
      case LR:
        return "LeisenReimer";
      case FiniteDifferences:
        return "FiniteDifferences";
    case Integral:
        return "Integral";
      case PseudoMonteCarlo:
        return "MonteCarlo";
      case QuasiMonteCarlo:
        return "Quasi-MonteCarlo";
      default:
        QL_FAIL("unknown engine type");
    }
}

Integer timeToDays(Time t) {
    return Integer(t*360+0.5);
}

void teardown() {
    Settings::instance().evaluationDate() = Date();
}

QL_END_TEST_LOCALS(EuropeanOptionTest)


void EuropeanOptionTest::testValues() {

    BOOST_MESSAGE("Testing European option values...");

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
    */
    EuropeanOptionData values[] = {
      // pag 2-8
      //        type, strike,   spot,    q,    r,    t,  vol,   value
      { Option::Call,  65.00,  60.00, 0.00, 0.08, 0.25, 0.30,  2.1334},
      { Option::Put,   95.00, 100.00, 0.05, 0.10, 0.50, 0.20,  2.4648},
      { Option::Put,   19.00,  19.00, 0.10, 0.10, 0.75, 0.28,  1.7011},
      { Option::Call,  19.00,  19.00, 0.10, 0.10, 0.75, 0.28,  1.7011},
      { Option::Call,   1.60,   1.56, 0.08, 0.06, 0.50, 0.12,  0.0291},
      { Option::Put,   70.00,  75.00, 0.05, 0.10, 0.50, 0.35,  4.0870},
      // pag 24
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.15,  0.0205},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.15,  1.8734},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.15,  9.9413},
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.25,  0.3150},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.25,  3.1217},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.25, 10.3556},
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.35,  0.9474},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.35,  4.3693},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.35, 11.1381},
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.15,  0.8069},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.15,  4.0232},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.15, 10.5769},
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.25,  2.7026},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.25,  6.6997},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.25, 12.7857},
      { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.35,  4.9329},
      { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.35,  9.3679},
      { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.35, 15.3086},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.15,  9.9210},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.15,  1.8734},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.15,  0.0408},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.25, 10.2155},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.25,  3.1217},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.25,  0.4551},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.35, 10.8479},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.35,  4.3693},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.35,  1.2376},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.15, 10.3192},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.15,  4.0232},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.15,  1.0646},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.25, 12.2149},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.25,  6.6997},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.25,  3.2734},
      { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.35, 14.4452},
      { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.35,  9.3679},
      { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.35,  5.7963},
      // pag27
      { Option::Call,  40.00,  42.00, 0.08, 0.04, 0.75, 0.35,  5.0975}
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
    boost::shared_ptr<PricingEngine> engine(new AnalyticEuropeanEngine);

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));
        Date exDate = today + timeToDays(values[i].t);
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<StochasticProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

        EuropeanOption option(stochProcess, payoff, exercise, engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated-values[i].result);
        Real tolerance = 1e-4;
        if (error>tolerance) {
            REPORT_FAILURE("value", payoff, exercise, values[i].s,
                           values[i].q, values[i].r, today,
                           values[i].v, values[i].result, calculated,
                           error, tolerance);
        }
    }

}



void EuropeanOptionTest::testGreekValues() {

    BOOST_MESSAGE("Testing European option greek values...");

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
       pag 11-16
    */
    EuropeanOptionData values[] = {
        //        type, strike,   spot,    q,    r,        t,  vol,  value
        // delta
        { Option::Call, 100.00, 105.00, 0.10, 0.10, 0.500000, 0.36,  0.5946 },
        { Option::Put,  100.00, 105.00, 0.10, 0.10, 0.500000, 0.36, -0.3566 },
        // elasticity
        { Option::Put,  100.00, 105.00, 0.10, 0.10, 0.500000, 0.36, -4.8775 },
        // gamma
        { Option::Call,  60.00,  55.00, 0.00, 0.10, 0.750000, 0.30,  0.0278 },
        { Option::Put,   60.00,  55.00, 0.00, 0.10, 0.750000, 0.30,  0.0278 },
        // vega
        { Option::Call,  60.00,  55.00, 0.00, 0.10, 0.750000, 0.30, 18.9358 },
        { Option::Put,   60.00,  55.00, 0.00, 0.10, 0.750000, 0.30, 18.9358 },
        // theta
        { Option::Put,  405.00, 430.00, 0.05, 0.07, 1.0/12.0, 0.20,-31.1924 },
        // theta per day
        { Option::Put,  405.00, 430.00, 0.05, 0.07, 1.0/12.0, 0.20, -0.0855 },
        // rho
        { Option::Call,  75.00,  72.00, 0.00, 0.09, 1.000000, 0.19, 38.7325 },
        // dividendRho
        { Option::Put,  490.00, 500.00, 0.05, 0.08, 0.250000, 0.15, 42.2254 }
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
    boost::shared_ptr<PricingEngine> engine(new AnalyticEuropeanEngine);
    boost::shared_ptr<StochasticProcess> stochProcess(new
        BlackScholesMertonProcess(Handle<Quote>(spot),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS)));

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
    option = boost::shared_ptr<VanillaOption>(new EuropeanOption(
        stochProcess, payoff, exercise, engine));
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
    option = boost::shared_ptr<VanillaOption>(new EuropeanOption(
        stochProcess, payoff, exercise, engine));
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
    option = boost::shared_ptr<VanillaOption>(new EuropeanOption(
        stochProcess, payoff, exercise, engine));
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
    option = boost::shared_ptr<VanillaOption>(new EuropeanOption(
        stochProcess, payoff, exercise, engine));
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
    option = boost::shared_ptr<VanillaOption>(new EuropeanOption(
        stochProcess, payoff, exercise, engine));
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
    option = boost::shared_ptr<VanillaOption>(new EuropeanOption(
        stochProcess, payoff, exercise, engine));
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
    option = boost::shared_ptr<VanillaOption>(new EuropeanOption(
        stochProcess, payoff, exercise, engine));
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
    option = boost::shared_ptr<VanillaOption>(new EuropeanOption(
        stochProcess, payoff, exercise, engine));
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
    option = boost::shared_ptr<VanillaOption>(new EuropeanOption(
        stochProcess, payoff, exercise, engine));
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
    option = boost::shared_ptr<VanillaOption>(new EuropeanOption(
        stochProcess, payoff, exercise, engine));
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
    option = boost::shared_ptr<VanillaOption>(new EuropeanOption(
        stochProcess, payoff, exercise, engine));
    calculated = option->dividendRho();
    error = std::fabs(calculated-values[i].result);
    if (error>tolerance)
        REPORT_FAILURE("dividendRho", payoff, exercise, values[i].s,
                       values[i].q, values[i].r, today,
                       values[i].v, values[i].result, calculated,
                       error, tolerance);

}

void EuropeanOptionTest::testGreeks() {

    BOOST_MESSAGE("Testing European option greeks...");

    QL_TEST_BEGIN

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

              boost::shared_ptr<StochasticProcess> stochProcess(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

              EuropeanOption option(stochProcess, payoff, exercise);

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

    QL_TEST_TEARDOWN
}

void EuropeanOptionTest::testImpliedVol() {

    BOOST_MESSAGE("Testing European option implied volatility...");

    Size maxEvaluations = 100;
    Real tolerance = 1.0e-6;

    // test options
    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Integer lengths[] = { 36, 180, 360, 1080 };

    // test data
    Real underlyings[] = { 80.0, 95.0, 99.9, 100.0, 100.1, 105.0, 120.0 };
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
                      vol->setValue(v*1.5);
                      try {
                          implVol = option->impliedVolatility(value,
                                                              tolerance,
                                                              maxEvaluations);
                      } catch (std::exception& e) {
                          BOOST_FAIL(
                              types[i] << " option :\n"
                              << "    spot value:     " << u << "\n"
                              << "    strike:         " << strikes[j] << "\n"
                              << "    dividend yield: " << io::rate(q) << "\n"
                              << "    risk-free rate: " << io::rate(r) << "\n"
                              << "    maturity:       " << exDate << "\n"
                              << "    volatility:     " << io::volatility(v)
                              << "\n\n"
                              << e.what());
                      }
                      if (std::fabs(implVol-v) > tolerance) {
                          // the difference might not matter
                          vol->setValue(implVol);
                          Real value2 = option->NPV();
                          Real error = relativeError(value,value2,u);
                          if (error > tolerance) {
                              BOOST_FAIL(
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

    BOOST_MESSAGE("Testing self-containment of "
                  "implied volatility calculation...");

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

    boost::shared_ptr<StochasticProcess> process(
                  new BlackScholesMertonProcess(underlying, qTS, rTS, volTS));

    // link to the same stochastic process, which shouldn't be changed
    // by calling methods of either option

    boost::shared_ptr<VanillaOption> option1(
                               new EuropeanOption(process, payoff, exercise));
    boost::shared_ptr<VanillaOption> option2(
                               new EuropeanOption(process, payoff, exercise));

    // test

    Real refValue = option2->NPV();

    Flag f;
    f.registerWith(option2);

    option1->impliedVolatility(refValue*1.5, tolerance, maxEvaluations);

    if (f.isUp())
        BOOST_FAIL("implied volatility calculation triggered a change "
                   "in another instrument");

    option2->recalculate();
    if (std::fabs(option2->NPV() - refValue) >= 1.0e-8)
        BOOST_FAIL("implied volatility calculation changed the value "
                   << "of another instrument: \n"
                   << std::setprecision(8)
                   << "previous value: " << refValue << "\n"
                   << "current value:  " << option2->NPV());

    vol->setValue(vol->value()*1.5);

    if (!f.isUp())
        BOOST_FAIL("volatility change not notified");

    if (std::fabs(option2->NPV() - refValue) <= 1.0e-8)
        BOOST_FAIL("volatility change did not cause the value to change");

}


// different engines

QL_BEGIN_TEST_LOCALS(EuropeanOptionTest)

void testEngineConsistency(EngineType *engines,
                           Size N,
                           Size binomialSteps,
                           Size samples,
                           Real tolerance) {

    QL_TEST_START_TIMING

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
              // options to check
              std::map<EngineType,boost::shared_ptr<VanillaOption> > options;
              for (Size ii=0; ii<N; ii++) {
                  options[engines[ii]] =
                      makeOption(payoff, exercise, spot,
                                 qTS, rTS, volTS, engines[ii],
                                 binomialSteps, samples);
              }

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

                      Real refValue = refOption->NPV();
                      for (Size ii=0; ii<N; ii++) {
                          Real value = options[engines[ii]]->NPV();
                          Real relErr = relativeError(value,refValue,u);
                          if (relErr > tolerance) {
                              BOOST_FAIL(
                                  "European " << types[i] << " option :\n"
                                  << "    spot value: " << u << "\n"
                                  << "    strike:           "
                                  << strikes[j] << "\n"
                                  << "    dividend yield:   "
                                  << io::rate(q) << "\n"
                                  << "    risk-free rate:   "
                                  << io::rate(r) << "\n"
                                  << "    reference date:   " << today << "\n"
                                  << "    maturity:         " << exDate << "\n"
                                  << "    volatility:       "
                                  << io::volatility(v) << "\n\n"
                                  << "    analytic value: "
                                  << refValue << "\n"
                                  << "    "
                                  << engineTypeToString(engines[ii]) << ":  "
                                  << value << "\n"
                                  << "    relative error: " \
                                  << relErr << "\n"
                                  << "         tolerance: " \
                                  << tolerance);
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

QL_END_TEST_LOCALS(EuropeanOptionTest)


void EuropeanOptionTest::testJRBinomialEngines() {

    BOOST_MESSAGE("Testing JR binomial European engines "
                  "against analytic results...");

    EngineType engines[] = { JR };
    Size steps = 251;
    Size samples = Null<Size>();
    Real relativeTol = 0.01;
    testEngineConsistency(engines,LENGTH(engines),steps,samples,relativeTol);
}

void EuropeanOptionTest::testCRRBinomialEngines() {

    BOOST_MESSAGE("Testing CRR binomial European engines "
                  "against analytic results...");

    EngineType engines[] = { CRR };
    Size steps = 501;
    Size samples = Null<Size>();
    Real relativeTol = 0.02;
    testEngineConsistency(engines,LENGTH(engines),steps,samples,relativeTol);
}

void EuropeanOptionTest::testEQPBinomialEngines() {

    BOOST_MESSAGE("Testing EQP binomial European engines "
                  "against analytic results...");

    EngineType engines[] = { EQP };
    Size steps = 501;
    Size samples = Null<Size>();
    Real relativeTol = 0.02;
    testEngineConsistency(engines,LENGTH(engines),steps,samples,relativeTol);
}

void EuropeanOptionTest::testTGEOBinomialEngines() {

    BOOST_MESSAGE("Testing TGEO binomial European engines "
                  "against analytic results...");

    EngineType engines[] = { TGEO };
    Size steps = 251;
    Size samples = Null<Size>();
    Real relativeTol = 0.01;
    testEngineConsistency(engines,LENGTH(engines),steps,samples,relativeTol);
}

void EuropeanOptionTest::testTIANBinomialEngines() {

    BOOST_MESSAGE("Testing TIAN binomial European engines "
                  "against analytic results...");

    EngineType engines[] = { TIAN };
    Size steps = 251;
    Size samples = Null<Size>();
    Real relativeTol = 0.01;
    testEngineConsistency(engines,LENGTH(engines),steps,samples,relativeTol);
}

void EuropeanOptionTest::testLRBinomialEngines() {

    BOOST_MESSAGE("Testing LR binomial European engines "
                  "against analytic results...");

    EngineType engines[] = { LR };
    Size steps = 251;
    Size samples = Null<Size>();
    Real relativeTol = 0.01;
    testEngineConsistency(engines,LENGTH(engines),steps,samples,relativeTol);
}

void EuropeanOptionTest::testFdEngines() {

    BOOST_MESSAGE("Testing finite-difference European engines "
                  "against analytic results...");

    EngineType engines[] = { FiniteDifferences };
    Size timeSteps = 300;
    Size gridPoints = 300;
    Real relativeTol = 0.0001;
    testEngineConsistency(engines,LENGTH(engines),
                          timeSteps,gridPoints,relativeTol);
}

void EuropeanOptionTest::testIntegralEngines() {

    BOOST_MESSAGE("Testing integral engines "
                  "against analytic results...");

    EngineType engines[] = { Integral };
    Size timeSteps = 300;
    Size gridPoints = 300;
    Real relativeTol = 0.0001;
    testEngineConsistency(engines,LENGTH(engines),
                          timeSteps,gridPoints,relativeTol);
}

void EuropeanOptionTest::testMcEngines() {

    BOOST_MESSAGE("Testing Monte Carlo European engines "
                  "against analytic results...");

    EngineType engines[] = { PseudoMonteCarlo };
    Size steps = Null<Size>();
    Size samples = 40000;
    Real relativeTol = 0.01;
    testEngineConsistency(engines,LENGTH(engines),steps,samples,relativeTol);
}

void EuropeanOptionTest::testQmcEngines() {

    BOOST_MESSAGE("Testing Quasi Monte Carlo European engines "
                  "against analytic results...");

    EngineType engines[] = { QuasiMonteCarlo };
    Size steps = Null<Size>();
    Size samples = 4095; // 2^12-1
    Real relativeTol = 0.01;
    testEngineConsistency(engines,LENGTH(engines),steps,samples,relativeTol);
}

void EuropeanOptionTest::testFdGreeks() {

    BOOST_MESSAGE("Testing finite-difference European option greeks...");

    QL_TEST_BEGIN

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"]  = 1.0e-3;
    tolerance["gamma"]  = 1.0e-5;
    tolerance["theta"]  = 2.0e-3;

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

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(residualTimes); k++) {
          Date exDate = today + timeToDays(residualTimes[k]);
          boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

          boost::shared_ptr<StrikedTypePayoff> payoff(
                                new PlainVanillaPayoff(types[i], strikes[j]));

          boost::shared_ptr<StochasticProcess> stochProcess(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

          boost::shared_ptr<PricingEngine> engine(new FDEuropeanEngine);
          EuropeanOption option(stochProcess, payoff, exercise, engine);

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

    QL_TEST_TEARDOWN
}

void EuropeanOptionTest::testPriceCurve() {

    BOOST_MESSAGE("Testing European price curves...");

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
    */
    EuropeanOptionData values[] = {
      // pag 2-8
      //        type, strike,   spot,    q,    r,    t,  vol,   value
      { Option::Call,  65.00,  60.00, 0.00, 0.08, 0.25, 0.30,  2.1334},
      { Option::Put,   95.00, 100.00, 0.05, 0.10, 0.50, 0.20,  2.4648},
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
    boost::shared_ptr<PricingEngine>
        engine(new FDEuropeanEngine(timeSteps, gridPoints));

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));
        Date exDate = today + timeToDays(values[i].t);
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<StochasticProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

        EuropeanOption option(stochProcess, payoff, exercise, engine);
        SampledCurve price_curve = option.priceCurve();
        if (price_curve.size() == 0) {
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

            EuropeanOption option1(stochProcess, payoff, exercise, engine);
            Real calculated = option1.NPV();
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


test_suite* EuropeanOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("European option tests");

    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testValues));
    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testGreekValues));
    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testGreeks));
    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testImpliedVol));
    suite->add(BOOST_TEST_CASE(
                           &EuropeanOptionTest::testImpliedVolContainment));
    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testJRBinomialEngines));
    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testCRRBinomialEngines));
    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testEQPBinomialEngines));
    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testTGEOBinomialEngines));
    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testTIANBinomialEngines));
    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testLRBinomialEngines));
    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testFdEngines));
    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testIntegralEngines));
    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testFdGreeks));
    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testMcEngines));
    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testQmcEngines));
    suite->add(BOOST_TEST_CASE(&EuropeanOptionTest::testPriceCurve));

    return suite;
}

