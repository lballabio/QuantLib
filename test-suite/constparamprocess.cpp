/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2022 Ruilong Xu

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

#include "constparamprocess.hpp"
#include "utilities.hpp"
#include <ql/experimental/processes/constparamprocess.hpp>
#include <ql/instruments/asianoption.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/pricingengines/asian/mc_discr_arith_av_price.hpp>
#include <ql/pricingengines/asian/mc_discr_arith_av_price_heston.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/gjrgarchprocess.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <chrono>
#include <utility>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {
    ext::shared_ptr<GeneralizedBlackScholesProcess>
    makeGBSprocess(const ext::shared_ptr<Quote>& u,
                   const ext::shared_ptr<YieldTermStructure>& q,
                   const ext::shared_ptr<YieldTermStructure>& r,
                   const ext::shared_ptr<BlackVolTermStructure>& vol) {
        return ext::make_shared<GeneralizedBlackScholesProcess>(
            Handle<Quote>(u), Handle<YieldTermStructure>(q), Handle<YieldTermStructure>(r),
            Handle<BlackVolTermStructure>(vol));
    }

    ext::shared_ptr<GeneralizedBlackScholesProcess>
    makeCPGBSprocess(const ext::shared_ptr<Quote>& u,
                     const ext::shared_ptr<YieldTermStructure>& q,
                     const ext::shared_ptr<YieldTermStructure>& r,
                     const ext::shared_ptr<BlackVolTermStructure>& vol) {
        return ext::make_shared<ConstParam<GeneralizedBlackScholesProcess> >(
            Handle<Quote>(u), Handle<YieldTermStructure>(q), Handle<YieldTermStructure>(r),
            Handle<BlackVolTermStructure>(vol));
    }

    ext::shared_ptr<GeneralizedBlackScholesProcess>
    makeBSMprocess(const ext::shared_ptr<Quote>& u,
                   const ext::shared_ptr<YieldTermStructure>& q,
                   const ext::shared_ptr<YieldTermStructure>& r,
                   const ext::shared_ptr<BlackVolTermStructure>& vol) {
        return ext::make_shared<BlackScholesMertonProcess>(
            Handle<Quote>(u), Handle<YieldTermStructure>(q), Handle<YieldTermStructure>(r),
            Handle<BlackVolTermStructure>(vol));
    }

    ext::shared_ptr<GeneralizedBlackScholesProcess>
    makeCPBSMprocess(const ext::shared_ptr<Quote>& u,
                     const ext::shared_ptr<YieldTermStructure>& q,
                     const ext::shared_ptr<YieldTermStructure>& r,
                     const ext::shared_ptr<BlackVolTermStructure>& vol) {
        return ext::make_shared<ConstParam<BlackScholesMertonProcess> >(
            Handle<Quote>(u), Handle<YieldTermStructure>(q), Handle<YieldTermStructure>(r),
            Handle<BlackVolTermStructure>(vol));
    }

    ext::shared_ptr<GeneralizedBlackScholesProcess>
    makeBSprocess(const ext::shared_ptr<Quote>& u,
                  const ext::shared_ptr<YieldTermStructure>& r,
                  const ext::shared_ptr<BlackVolTermStructure>& vol) {
        return ext::make_shared<BlackScholesProcess>(
            Handle<Quote>(u), Handle<YieldTermStructure>(r), Handle<BlackVolTermStructure>(vol));
    }

    ext::shared_ptr<GeneralizedBlackScholesProcess>
    makeCPBSprocess(const ext::shared_ptr<Quote>& u,
                    const ext::shared_ptr<YieldTermStructure>& r,
                    const ext::shared_ptr<BlackVolTermStructure>& vol) {
        return ext::make_shared<ConstParam<BlackScholesProcess> >(
            Handle<Quote>(u), Handle<YieldTermStructure>(r), Handle<BlackVolTermStructure>(vol));
    }

    ext::shared_ptr<GeneralizedBlackScholesProcess>
    makeBprocess(const ext::shared_ptr<Quote>& u,
                 const ext::shared_ptr<YieldTermStructure>& r,
                 const ext::shared_ptr<BlackVolTermStructure>& vol) {
        return ext::make_shared<BlackProcess>(Handle<Quote>(u), Handle<YieldTermStructure>(r),
                                              Handle<BlackVolTermStructure>(vol));
    }

    ext::shared_ptr<GeneralizedBlackScholesProcess>
    makeCPBprocess(const ext::shared_ptr<Quote>& u,
                   const ext::shared_ptr<YieldTermStructure>& r,
                   const ext::shared_ptr<BlackVolTermStructure>& vol) {
        return ext::make_shared<ConstParam<BlackProcess> >(
            Handle<Quote>(u), Handle<YieldTermStructure>(r), Handle<BlackVolTermStructure>(vol));
    }

    ext::shared_ptr<GeneralizedBlackScholesProcess>
    makeGKSprocess(const ext::shared_ptr<Quote>& u,
                   const ext::shared_ptr<YieldTermStructure>& f,
                   const ext::shared_ptr<YieldTermStructure>& d,
                   const ext::shared_ptr<BlackVolTermStructure>& vol) {
        return ext::make_shared<GarmanKohlagenProcess>(
            Handle<Quote>(u), Handle<YieldTermStructure>(f), Handle<YieldTermStructure>(d),
            Handle<BlackVolTermStructure>(vol));
    }

    ext::shared_ptr<GeneralizedBlackScholesProcess>
    makeCPGKSprocess(const ext::shared_ptr<Quote>& u,
                     const ext::shared_ptr<YieldTermStructure>& f,
                     const ext::shared_ptr<YieldTermStructure>& d,
                     const ext::shared_ptr<BlackVolTermStructure>& vol) {
        return ext::make_shared<ConstParam<GarmanKohlagenProcess> >(
            Handle<Quote>(u), Handle<YieldTermStructure>(f), Handle<YieldTermStructure>(d),
            Handle<BlackVolTermStructure>(vol));
    }

    ext::shared_ptr<GeneralizedBlackScholesProcess>
    makeVSBSMprocess(const ext::shared_ptr<Quote>& u,
                     const ext::shared_ptr<YieldTermStructure>& q,
                     const ext::shared_ptr<YieldTermStructure>& r,
                     const ext::shared_ptr<BlackVolTermStructure>& vol,
                     Real lt,
                     Real ut,
                     Real la,
                     Real ua,
                     Real stress) {
        return ext::make_shared<VegaStressedBlackScholesProcess>(
            Handle<Quote>(u), Handle<YieldTermStructure>(q), Handle<YieldTermStructure>(r),
            Handle<BlackVolTermStructure>(vol), lt, ut, la, ua, stress);
    }

    ext::shared_ptr<GeneralizedBlackScholesProcess>
    makeCPVSBSMprocess(const ext::shared_ptr<Quote>& u,
                       const ext::shared_ptr<YieldTermStructure>& q,
                       const ext::shared_ptr<YieldTermStructure>& r,
                       const ext::shared_ptr<BlackVolTermStructure>& vol,
                       Real lt,
                       Real ut,
                       Real la,
                       Real ua,
                       Real stress) {
        return ext::make_shared<ConstParam<VegaStressedBlackScholesProcess> >(
            Handle<Quote>(u), Handle<YieldTermStructure>(q), Handle<YieldTermStructure>(r),
            Handle<BlackVolTermStructure>(vol), lt, ut, la, ua, stress);
    }
}

void ConstParamProcessTest::testConstParamGeneralizedBlackScholesProcess() {
    BOOST_TEST_MESSAGE("Testing constant parameter generalized Black-Scholes-Merton process...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    /*
      spot,    q,    r,    t,  vol
    100.00, 0.05, 0.10, 10.0, 0.20
    */
    Real u = 100.0;
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(u));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.05));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.10));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    ext::shared_ptr<GeneralizedBlackScholesProcess> refProcess =
        makeGBSprocess(spot, qTS, rTS, volTS);
    ext::shared_ptr<GeneralizedBlackScholesProcess> process =
        makeCPGBSprocess(spot, qTS, rTS, volTS);

    const Time T = 10.0;
    const Size nTimeSteps = 10000;

    const Time dt = T / nTimeSteps;
    Time t = 0.0;
    Real q = u;
    Real p = u;

    PseudoRandom::rng_type rng(PseudoRandom::urng_type(42U));

    for (Size j = 0; j < nTimeSteps; ++j) {
        const Real dw = rng.next().value;
        q = process->evolve(t, q, dt, dw);
        p = refProcess->evolve(t, p, dt, dw);

        if (std::fabs(q / p - 1.0) > 1.0e-10) {
            BOOST_FAIL("invalid process evaluation at " << j << " " << q - p);
        }
        t += dt;
    }
}

void ConstParamProcessTest::testConstParamBlackScholesMertonProcess() {
    BOOST_TEST_MESSAGE("Testing constant parameter Black-Scholes-Merton process...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    /*
      spot,    q,    r,    t,  vol
    100.00, 0.05, 0.10, 10.0, 0.20
    */
    Real u = 100.0;
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(u));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.05));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.10));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    ext::shared_ptr<GeneralizedBlackScholesProcess> refProcess =
        makeBSMprocess(spot, qTS, rTS, volTS);
    ext::shared_ptr<GeneralizedBlackScholesProcess> process =
        makeCPBSMprocess(spot, qTS, rTS, volTS);

    const Time T = 10.0;
    const Size nTimeSteps = 10000;

    const Time dt = T / nTimeSteps;
    Time t = 0.0;
    Real q = u;
    Real p = u;

    PseudoRandom::rng_type rng(PseudoRandom::urng_type(42U));

    for (Size j = 0; j < nTimeSteps; ++j) {
        const Real dw = rng.next().value;
        q = process->evolve(t, q, dt, dw);
        p = refProcess->evolve(t, p, dt, dw);

        if (std::fabs(q / p - 1.0) > 1.0e-10) {
            BOOST_FAIL("invalid process evaluation at " << j << " " << q - p);
        }
        t += dt;
    }
}

void ConstParamProcessTest::testConstParamBlackScholesProcess() {
    BOOST_TEST_MESSAGE("Testing constant parameter Black-Scholes process...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    /*
      spot,    r,    t,  vol
    100.00, 0.10, 10.0, 0.20
    */
    Real u = 100.0;
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(u));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.10));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    ext::shared_ptr<GeneralizedBlackScholesProcess> refProcess = makeBSprocess(spot, rTS, volTS);
    ext::shared_ptr<GeneralizedBlackScholesProcess> process = makeCPBSprocess(spot, rTS, volTS);

    const Time T = 10.0;
    const Size nTimeSteps = 10000;

    const Time dt = T / nTimeSteps;
    Time t = 0.0;
    Real q = u;
    Real p = u;

    PseudoRandom::rng_type rng(PseudoRandom::urng_type(42U));

    for (Size j = 0; j < nTimeSteps; ++j) {
        const Real dw = rng.next().value;
        q = process->evolve(t, q, dt, dw);
        p = refProcess->evolve(t, p, dt, dw);

        if (std::fabs(q / p - 1.0) > 1.0e-10) {
            BOOST_FAIL("invalid process evaluation at " << j << " " << q - p);
        }
        t += dt;
    }
}

void ConstParamProcessTest::testConstParamBlackProcess() {
    BOOST_TEST_MESSAGE("Testing constant parameter Black process...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    /*
      spot,    r,    t,  vol
    100.00, 0.10, 10.0, 0.20
    */
    Real u = 100.0;
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(u));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.10));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    ext::shared_ptr<GeneralizedBlackScholesProcess> refProcess = makeBprocess(spot, rTS, volTS);
    ext::shared_ptr<GeneralizedBlackScholesProcess> process = makeCPBprocess(spot, rTS, volTS);

    const Time T = 10.0;
    const Size nTimeSteps = 10000;

    const Time dt = T / nTimeSteps;
    Time t = 0.0;
    Real q = u;
    Real p = u;

    PseudoRandom::rng_type rng(PseudoRandom::urng_type(42U));

    for (Size j = 0; j < nTimeSteps; ++j) {
        const Real dw = rng.next().value;
        q = process->evolve(t, q, dt, dw);
        p = refProcess->evolve(t, p, dt, dw);

        if (std::fabs(q / p - 1.0) > 1.0e-10) {
            BOOST_FAIL("invalid process evaluation at " << j << " " << q - p);
        }
        t += dt;
    }
}

void ConstParamProcessTest::testConstParamGarmanKohlagenProcess() {
    BOOST_TEST_MESSAGE("Testing constant parameter Garman-Kohlagen process...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    /*
      spot,    foreign,    domestic,    t,  vol
    100.00,       0.05,        0.10, 10.0, 0.20
    */
    Real u = 100.0;
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(u));
    ext::shared_ptr<SimpleQuote> fRate(new SimpleQuote(0.05));
    ext::shared_ptr<YieldTermStructure> fTS = flatRate(today, fRate, dc);
    ext::shared_ptr<SimpleQuote> dRate(new SimpleQuote(0.10));
    ext::shared_ptr<YieldTermStructure> dTS = flatRate(today, dRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    ext::shared_ptr<GeneralizedBlackScholesProcess> refProcess =
        makeGKSprocess(spot, fTS, dTS, volTS);
    ext::shared_ptr<GeneralizedBlackScholesProcess> process =
        makeCPGKSprocess(spot, fTS, dTS, volTS);

    const Time T = 10.0;
    const Size nTimeSteps = 10000;

    const Time dt = T / nTimeSteps;
    Time t = 0.0;
    Real q = u;
    Real p = u;

    PseudoRandom::rng_type rng(PseudoRandom::urng_type(42U));

    for (Size j = 0; j < nTimeSteps; ++j) {
        const Real dw = rng.next().value;
        q = process->evolve(t, q, dt, dw);
        p = refProcess->evolve(t, p, dt, dw);

        if (std::fabs(q / p - 1.0) > 1.0e-10) {
            BOOST_FAIL("invalid process evaluation at " << j << " " << q - p);
        }
        t += dt;
    }
}

namespace {
    struct DiscreteAverageData {
        Option::Type type;
        Real underlying;
        Real strike;
        Rate dividendYield;
        Rate riskFreeRate;
        Time first;
        Time length;
        Size fixings;
        Volatility volatility;
        bool controlVariate;
        Real result;
    };
}

void ConstParamProcessTest::speedUpBSMProcess() {
    BOOST_TEST_MESSAGE(
        "Speeding up BSM process in discrete arithmetic average-price Asians option MC pricing...");
    // data from "Asian Option", Levy, 1997
    // in "Exotic Options: The State of the Art",
    // edited by Clewlow, Strickland
    DiscreteAverageData asian{Option::Put, 90.0, 87.0, 0.06,  0.025,       1.0 / 12.0,
                              11.0 / 12.0, 12,   0.13, false, 2.1105094397};

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    spot->setValue(asian.underlying);
    qRate->setValue(asian.dividendYield);
    rRate->setValue(asian.riskFreeRate);
    vol->setValue(asian.volatility);

    Average::Type averageType = Average::Arithmetic;
    Real runningSum = 0.0;
    Size pastFixings = 0;
    ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(asian.type, asian.strike));

    Time dt = asian.length / (asian.fixings - 1);
    std::vector<Time> timeIncrements(asian.fixings);
    std::vector<Date> fixingDates(asian.fixings);
    timeIncrements[0] = asian.first;
    fixingDates[0] = today + timeToDays(timeIncrements[0]);
    for (Size i = 1; i < asian.fixings; i++) {
        timeIncrements[i] = i * dt + asian.first;
        fixingDates[i] = today + timeToDays(timeIncrements[i]);
    }
    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(fixingDates[asian.fixings - 1]));
    DiscreteAveragingAsianOption option(averageType, runningSum, pastFixings, fixingDates, payoff,
                                        exercise);

    Size samples = 100000;

    decltype(std::chrono::steady_clock::now()) t0, t1, t2;

    t0 = std::chrono::steady_clock::now();
    {
        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new BlackScholesMertonProcess(
            Handle<Quote>(spot), Handle<YieldTermStructure>(qTS), Handle<YieldTermStructure>(rTS),
            Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engineSlow =
            MakeMCDiscreteArithmeticAPEngine<LowDiscrepancy>(stochProcess)
                .withSamples(samples)
                .withControlVariate(asian.controlVariate);

        option.setPricingEngine(engineSlow);
        Real calculated = option.NPV();
    }
    t1 = std::chrono::steady_clock::now();
    {
        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
            new ConstParam<BlackScholesMertonProcess>(
                Handle<Quote>(spot), Handle<YieldTermStructure>(qTS),
                Handle<YieldTermStructure>(rTS), Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engineFast =
            MakeMCDiscreteArithmeticAPEngine<LowDiscrepancy>(stochProcess)
                .withSamples(samples)
                .withControlVariate(asian.controlVariate);

        option.setPricingEngine(engineFast);
        Real calculated = option.NPV();
    }
    t2 = std::chrono::steady_clock::now();

    Real running1 =
        static_cast<Real>(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());
    Real running2 =
        static_cast<Real>(std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());

    std::cout << "speed up BSM process simulation " << running1 / running2 << " times" << std::endl;
}

void ConstParamProcessTest::speedUpHestonProcess() {

    BOOST_TEST_MESSAGE("Speeding up Heston process in discrete arithmetic average-price Asians "
                       "option MC pricing...");

    // data from "A numerical method to price exotic path-dependent
    // options on an underlying described by the Heston stochastic
    // volatility model", Ballestra, Pacelli and Zirilli, Journal
    // of Banking & Finance, 2007 (section 4 - Numerical Results)

    // nb. for Heston, the volatility param below is ignored
    DiscreteAverageData asian{Option::Call, 120.0, 100.0, 0.0,   0.05, 1.0 / 12.0,
                              11.0 / 12.0,  12,    0.1,   false, 22.50};

    Real vol = 0.3;
    Real v0 = vol * vol;
    Real kappa = 11.35;
    Real theta = 0.022;
    Real sigma = 0.618;
    Real rho = -0.5;

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    Average::Type averageType = Average::Arithmetic;
    Real runningSum = 0.0;
    Size pastFixings = 0;

    ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(asian.type, asian.strike));

    Time dt = asian.length / (asian.fixings - 1);
    std::vector<Time> timeIncrements(asian.fixings);
    std::vector<Date> fixingDates(asian.fixings);
    timeIncrements[0] = asian.first;
    fixingDates[0] = today + Integer(timeIncrements[0] * 365.25);
    for (Size i = 1; i < asian.fixings; i++) {
        timeIncrements[i] = i * dt + asian.first;
        fixingDates[i] = today + Integer(timeIncrements[i] * 365.25);
    }
    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(fixingDates[asian.fixings - 1]));

    spot->setValue(asian.underlying);
    qRate->setValue(asian.dividendYield);
    rRate->setValue(asian.riskFreeRate);

    Size samples = 100000;
    decltype(std::chrono::steady_clock::now()) t0, t1, t2;

    t0 = std::chrono::steady_clock::now();
    {
        ext::shared_ptr<HestonProcess> hestonProcess(
            new HestonProcess(Handle<YieldTermStructure>(rTS), Handle<YieldTermStructure>(qTS),
                              Handle<Quote>(spot), v0, kappa, theta, sigma, rho));

        ext::shared_ptr<PricingEngine> engineSlow =
            MakeMCDiscreteArithmeticAPHestonEngine<LowDiscrepancy>(hestonProcess)
                .withSeed(42)
                .withSamples(samples);

        DiscreteAveragingAsianOption option(averageType, runningSum, pastFixings, fixingDates,
                                            payoff, exercise);
        option.setPricingEngine(engineSlow);

        Real calculated = option.NPV();
    }

    t1 = std::chrono::steady_clock::now();
    {
        ext::shared_ptr<HestonProcess> hestonProcess(new ConstParam<HestonProcess>(
            Handle<YieldTermStructure>(rTS), Handle<YieldTermStructure>(qTS), Handle<Quote>(spot),
            v0, kappa, theta, sigma, rho));

        ext::shared_ptr<PricingEngine> engineSlow =
            MakeMCDiscreteArithmeticAPHestonEngine<LowDiscrepancy>(hestonProcess)
                .withSeed(42)
                .withSamples(samples);

        DiscreteAveragingAsianOption option(averageType, runningSum, pastFixings, fixingDates,
                                            payoff, exercise);
        option.setPricingEngine(engineSlow);

        Real calculated = option.NPV();
    }
    t2 = std::chrono::steady_clock::now();

    Real running1 =
        static_cast<Real>(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());
    Real running2 =
        static_cast<Real>(std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());

    std::cout << "speed up Heston process simulation " << running1 / running2 << " times"
              << std::endl;
}

void ConstParamProcessTest::testFetchValue() {
    BOOST_TEST_MESSAGE("Testing fetchValue method of constant parameter process...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    /*
      spot,    q,    r,    t,  vol
    100.00, 0.05, 0.10, 10.0, 0.20
    */
    Real u = 100.0;
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(u));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.05));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.10));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    ext::shared_ptr<GeneralizedBlackScholesProcess> refProcess =
        makeGBSprocess(spot, qTS, rTS, volTS);
    ext::shared_ptr<GeneralizedBlackScholesProcess> process =
        makeCPGBSprocess(spot, qTS, rTS, volTS);

    const Time T = 10;
    const Size nTimeSteps = 10000;

    const Time dt = T / nTimeSteps;
    Time t = 0.0;
    Real q = spot->value();
    Real p = spot->value();

    PseudoRandom::rng_type rng(PseudoRandom::urng_type(42U));

    for (Size j = 0; j < nTimeSteps; ++j) {
        const Real dw = rng.next().value;
        q = process->evolve(t, q, dt, dw);
        p = refProcess->evolve(t, p, dt, dw);

        if (std::fabs(q / p - 1.0) > 1.0e-10) {
            BOOST_FAIL("invalid process evaluation at " << j << " " << q - p);
        }
        t += dt;
    }

    /*
      spot,    q,    r,    t,  vol
    110.00, 0.06, 0.11, 10.0, 0.30
    */

    spot->setValue(110.0);
    qRate->setValue(0.06);
    rRate->setValue(0.11);
    vol->setValue(0.30);

    t = 0.0;
    q = spot->value();
    p = spot->value();

    for (Size j = 0; j < nTimeSteps; ++j) {
        const Real dw = rng.next().value;
        q = process->evolve(t, q, dt, dw);
        p = refProcess->evolve(t, p, dt, dw);

        if (std::fabs(q / p - 1.0) > 1.0e-10) {
            BOOST_FAIL("invalid process evaluation at " << j << " " << q - p);
        }
        t += dt;
    }
}

void ConstParamProcessTest::testConstParamVegaStressedBlackScholesProcess() {
    BOOST_TEST_MESSAGE("Testing constant parameter Vega stressed Black-Scholes-Merton process...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    /*
      spot,    q,    r,    t,  vol lt, ut, la,  ua, stress
    100.00, 0.05, 0.10, 10.0, 0.20  3,  7, 95, 105,   0.20
    */
    Real u = 100.0;
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(u));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.05));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.10));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    Real lt = 3.0, ut = 7.0, la = 95.0, ua = 105.0, stress = 0.2;

    ext::shared_ptr<GeneralizedBlackScholesProcess> refProcess =
        makeVSBSMprocess(spot, qTS, rTS, volTS, lt, ut, la, ua, stress);
    ext::shared_ptr<GeneralizedBlackScholesProcess> process =
        makeCPVSBSMprocess(spot, qTS, rTS, volTS, lt, ut, la, ua, stress);

    const Time T = 10;
    const Size nTimeSteps = 10000;

    const Time dt = T / nTimeSteps;
    Time t = 0.0;
    Real q = spot->value();
    Real p = spot->value();

    PseudoRandom::rng_type rng(PseudoRandom::urng_type(42U));

    for (Size j = 0; j < nTimeSteps; ++j) {
        const Real dw = rng.next().value;
        q = process->evolve(t, q, dt, dw);
        p = refProcess->evolve(t, p, dt, dw);

        if (std::fabs(q / p - 1.0) > 1.0e-10) {
            BOOST_FAIL("invalid process evaluation at " << j << " " << q - p);
        }
        t += dt;
    }

    /*
      spot,    q,    r,    t,  vol lt, ut,  la,  ua, stress
    110.00, 0.06, 0.11, 10.0, 0.30  2,  6, 100, 120,   0.10
    */

    spot->setValue(110.0);
    qRate->setValue(0.06);
    rRate->setValue(0.11);
    vol->setValue(0.30);

    lt = 2.0, ut = 6.0, la = 100.0, ua = 120.0, stress = 0.1;

    ext::dynamic_pointer_cast<VegaStressedBlackScholesProcess>(process)
        ->setLowerTimeBorderForStressTest(lt);
    ext::dynamic_pointer_cast<VegaStressedBlackScholesProcess>(process)
        ->setUpperTimeBorderForStressTest(ut);
    ext::dynamic_pointer_cast<VegaStressedBlackScholesProcess>(process)
        ->setLowerAssetBorderForStressTest(la);
    ext::dynamic_pointer_cast<VegaStressedBlackScholesProcess>(process)
        ->setUpperAssetBorderForStressTest(ua);
    ext::dynamic_pointer_cast<VegaStressedBlackScholesProcess>(process)->setStressLevel(stress);

    ext::dynamic_pointer_cast<VegaStressedBlackScholesProcess>(refProcess)
        ->setLowerTimeBorderForStressTest(lt);
    ext::dynamic_pointer_cast<VegaStressedBlackScholesProcess>(refProcess)
        ->setUpperTimeBorderForStressTest(ut);
    ext::dynamic_pointer_cast<VegaStressedBlackScholesProcess>(refProcess)
        ->setLowerAssetBorderForStressTest(la);
    ext::dynamic_pointer_cast<VegaStressedBlackScholesProcess>(refProcess)
        ->setUpperAssetBorderForStressTest(ua);
    ext::dynamic_pointer_cast<VegaStressedBlackScholesProcess>(refProcess)->setStressLevel(stress);

    t = 0.0;
    q = spot->value();
    p = spot->value();

    for (Size j = 0; j < nTimeSteps; ++j) {
        const Real dw = rng.next().value;
        q = process->evolve(t, q, dt, dw);
        p = refProcess->evolve(t, p, dt, dw);

        if (std::fabs(q / p - 1.0) > 1.0e-10) {
            BOOST_FAIL("invalid process evaluation at " << j << " " << q - p);
        }
        t += dt;
    }
}

void ConstParamProcessTest::testConstParamHestonProcess() {
    BOOST_TEST_MESSAGE("Testing constant parameter Heston process...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    Real u = 100.0;
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(u));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.08));

    Handle<Quote> s0(spot);
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));

    ext::shared_ptr<HestonProcess> process(ext::make_shared<ConstParam<HestonProcess> >(
        rTS, qTS, s0, 0.25 * 0.25, 1.0, 0.25 * 0.25, 0.001, 0.0));
    ext::shared_ptr<HestonProcess> refProcess(
        ext::make_shared<HestonProcess>(rTS, qTS, s0, 0.25 * 0.25, 1.0, 0.25 * 0.25, 0.001, 0.0));

    const Time T = 10;
    const Size nTimeSteps = 10000;

    const Time dt = T / nTimeSteps;
    Time t = 0.0;
    Array p(2), q(2);
    q[0] = s0->value(), q[1] = refProcess->v0();
    p[0] = s0->value(), p[1] = refProcess->v0();

    PseudoRandom::rsg_type rsg = PseudoRandom::make_sequence_generator(refProcess->factors(), 42U);
    Array dw(refProcess->factors());

    for (Size j = 0; j < nTimeSteps; ++j) {
        for (Size i = 0; i < refProcess->factors(); ++i) {
            dw[i] = rsg.nextSequence().value[i];
        }

        q = process->evolve(t, q, dt, dw);
        p = refProcess->evolve(t, p, dt, dw);

        if (Norm2(q - p) / Norm2(p) > 1.0e-10) {
            BOOST_FAIL("invalid process evaluation at " << j << " " << q - p);
        }
        t += dt;
    }

    /* update */

    spot->setValue(110.0);
    rRate->setValue(0.09);
    qRate->setValue(0.05);

    t = 0.0;
    q[0] = s0->value(), q[1] = refProcess->v0();
    p[0] = s0->value(), p[1] = refProcess->v0();

    for (Size j = 0; j < nTimeSteps; ++j) {
        for (Size i = 0; i < refProcess->factors(); ++i) {
            dw[i] = rsg.nextSequence().value[i];
        }

        q = process->evolve(t, q, dt, dw);
        p = refProcess->evolve(t, p, dt, dw);

        if (Norm2(q - p) / Norm2(p) > 1.0e-10) {
            BOOST_FAIL("invalid process evaluation at " << j << " " << q - p);
        }
        t += dt;
    }
}

void ConstParamProcessTest::testConstParamBatesProcess() {
    BOOST_TEST_MESSAGE("Testing constant parameter Bates process...");

    SavedSettings backup;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    Real u = 100.0;
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(u));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.08));

    Handle<Quote> s0(spot);
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));

    ext::shared_ptr<BatesProcess> process(
        new ConstParam<BatesProcess>(rTS, qTS, s0, 0.07, 2.0, 0.04, 0.55, -0.8, 2.0, -0.2, 0.1));
    ext::shared_ptr<BatesProcess> refProcess(
        new BatesProcess(rTS, qTS, s0, 0.07, 2.0, 0.04, 0.55, -0.8, 2.0, -0.2, 0.1));

    const Time T = 10;
    const Size nTimeSteps = 10000;

    const Time dt = T / nTimeSteps;
    Time t = 0.0;
    Array p(2), q(2);
    q[0] = s0->value(), q[1] = refProcess->v0();
    p[0] = s0->value(), p[1] = refProcess->v0();

    PseudoRandom::rsg_type rsg = PseudoRandom::make_sequence_generator(refProcess->factors(), 42U);
    Array dw(refProcess->factors());

    for (Size j = 0; j < nTimeSteps; ++j) {
        for (Size i = 0; i < refProcess->factors(); ++i) {
            dw[i] = rsg.nextSequence().value[i];
        }

        q = process->evolve(t, q, dt, dw);
        p = refProcess->evolve(t, p, dt, dw);

        if (Norm2(q - p) / Norm2(p) > 1.0e-10) {
            BOOST_FAIL("invalid process evaluation at " << j << " " << q - p);
        }
        t += dt;
    }

    /* update */

    spot->setValue(110.0);
    rRate->setValue(0.09);
    qRate->setValue(0.05);

    t = 0.0;
    q[0] = s0->value(), q[1] = refProcess->v0();
    p[0] = s0->value(), p[1] = refProcess->v0();

    for (Size j = 0; j < nTimeSteps; ++j) {
        for (Size i = 0; i < refProcess->factors(); ++i) {
            dw[i] = rsg.nextSequence().value[i];
        }

        q = process->evolve(t, q, dt, dw);
        p = refProcess->evolve(t, p, dt, dw);

        if (Norm2(q - p) / Norm2(p) > 1.0e-10) {
            BOOST_FAIL("invalid process evaluation at " << j << " " << q - p);
        }
        t += dt;
    }
}

void ConstParamProcessTest::testConstParamGJRGARCHProcess() {
    BOOST_TEST_MESSAGE("Testing constant parameter GJRGARCH process...");

    SavedSettings backup;

    DayCounter dayCounter = ActualActual(ActualActual::ISDA);

    const Date today = Date::todaysDate();

    const Real u = 50.0;
    const Real omega = 2.0e-6;
    const Real alpha = 0.024;
    const Real beta = 0.93;
    const Real gamma = 0.059;
    const Real daysPerYear = 365.0; // number of trading days per year
    const Size maturity = 180;
    const Real strike = 45;
    const Real lambda = 0.1;

    Real m1 = beta +
              (alpha + gamma * CumulativeNormalDistribution()(lambda)) * (1.0 + lambda * lambda) +
              gamma * lambda * std::exp(-lambda * lambda / 2.0) / std::sqrt(2.0 * M_PI);
    Real v0 = omega / (1.0 - m1);

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(u));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.05));

    Handle<Quote> s0(spot);
    Handle<YieldTermStructure> riskFreeTS(flatRate(today, rRate, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(today, qRate, dayCounter));

    ext::shared_ptr<GJRGARCHProcess> process(new ConstParam<GJRGARCHProcess>(
        riskFreeTS, dividendTS, s0, v0, omega, alpha, beta, gamma, lambda, daysPerYear));
    ext::shared_ptr<GJRGARCHProcess> refProcess(new GJRGARCHProcess(
        riskFreeTS, dividendTS, s0, v0, omega, alpha, beta, gamma, lambda, daysPerYear));

    const Time T = 10;
    const Size nTimeSteps = 10000;

    const Time dt = T / nTimeSteps;
    Time t = 0.0;
    Array p(2), q(2);
    q[0] = spot->value(), q[1] = refProcess->v0();
    p[0] = spot->value(), p[1] = refProcess->v0();

    PseudoRandom::rsg_type rsg = PseudoRandom::make_sequence_generator(refProcess->factors(), 42U);
    Array dw(refProcess->factors());

    for (Size j = 0; j < nTimeSteps; ++j) {
        for (Size i = 0; i < refProcess->factors(); ++i) {
            dw[i] = rsg.nextSequence().value[i];
        }

        q = process->evolve(t, q, dt, dw);
        p = refProcess->evolve(t, p, dt, dw);

        if (Norm2(q - p) / Norm2(p) > 1.0e-10) {
            BOOST_FAIL("invalid process evaluation at " << j << " " << q - p);
        }
        t += dt;
    }

    /* update */

    spot->setValue(60.0);
    rRate->setValue(0.09);
    qRate->setValue(0.05);

    t = 0.0;
    q[0] = spot->value(), q[1] = refProcess->v0();
    p[0] = spot->value(), p[1] = refProcess->v0();

    for (Size j = 0; j < nTimeSteps; ++j) {
        for (Size i = 0; i < refProcess->factors(); ++i) {
            dw[i] = rsg.nextSequence().value[i];
        }

        q = process->evolve(t, q, dt, dw);
        p = refProcess->evolve(t, p, dt, dw);

        if (Norm2(q - p) / Norm2(p) > 1.0e-10) {
            BOOST_FAIL("invalid process evaluation at " << j << " " << q - p);
        }
        t += dt;
    }
}

test_suite* ConstParamProcessTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Constant parameter BSM process Test");

    suite->add(
        QUANTLIB_TEST_CASE(&ConstParamProcessTest::testConstParamGeneralizedBlackScholesProcess));
    suite->add(QUANTLIB_TEST_CASE(&ConstParamProcessTest::testConstParamBlackScholesMertonProcess));
    suite->add(QUANTLIB_TEST_CASE(&ConstParamProcessTest::testConstParamBlackScholesProcess));
    suite->add(QUANTLIB_TEST_CASE(&ConstParamProcessTest::testConstParamBlackProcess));
    suite->add(QUANTLIB_TEST_CASE(&ConstParamProcessTest::testConstParamGarmanKohlagenProcess));
    suite->add(
        QUANTLIB_TEST_CASE(&ConstParamProcessTest::testConstParamVegaStressedBlackScholesProcess));
    suite->add(QUANTLIB_TEST_CASE(&ConstParamProcessTest::testConstParamHestonProcess));
    suite->add(QUANTLIB_TEST_CASE(&ConstParamProcessTest::testConstParamBatesProcess));
    suite->add(QUANTLIB_TEST_CASE(&ConstParamProcessTest::testConstParamGJRGARCHProcess));
    //suite->add(QUANTLIB_TEST_CASE(&ConstParamProcessTest::speedUpBSMProcess));
    //suite->add(QUANTLIB_TEST_CASE(&ConstParamProcessTest::speedUpHestonProcess));
    suite->add(QUANTLIB_TEST_CASE(&ConstParamProcessTest::testFetchValue));
    return suite;
}