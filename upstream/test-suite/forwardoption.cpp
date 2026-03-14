/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/experimental/forward/analytichestonforwardeuropeanengine.hpp>
#include <ql/instruments/forwardvanillaoption.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/forward/forwardengine.hpp>
#include <ql/pricingengines/forward/forwardperformanceengine.hpp>
#include <ql/pricingengines/forward/mcforwardeuropeanbsengine.hpp>
#include <ql/pricingengines/forward/mcforwardeuropeanhestonengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/binomialengine.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(ForwardOptionTests)

#undef REPORT_FAILURE
#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, \
                       v, moneyness, reset, expected, calculated, \
                       error, tolerance) \
    BOOST_ERROR("Forward " << exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    spot value:        " << s << "\n" \
               << "    strike:            " << payoff->strike() <<"\n" \
               << "    moneyness:         " << moneyness << "\n" \
               << "    dividend yield:    " << io::rate(q) << "\n" \
               << "    risk-free rate:    " << io::rate(r) << "\n" \
               << "    reference date:    " << today << "\n" \
               << "    reset date:        " << reset << "\n" \
               << "    maturity:          " << exercise->lastDate() << "\n" \
               << "    volatility:        " << io::volatility(v) << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);

struct ForwardOptionData {
    Option::Type type;
    Real moneyness;
    Real s;          // spot
    Rate q;          // dividend
    Rate r;          // risk-free rate
    Time start;      // time to reset
    Time t;          // time to maturity
    Volatility v;    // volatility
    Real result;     // expected result
    Real tol;        // tolerance
};

template <template <class> class Engine>
void testForwardGreeks() {

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"]   = 1.0e-5;
    tolerance["gamma"]   = 1.0e-5;
    tolerance["theta"]   = 1.0e-5;
    tolerance["rho"]     = 1.0e-5;
    tolerance["divRho"]  = 1.0e-5;
    tolerance["vega"]    = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real moneyness[] = { 0.9, 1.0, 1.1 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 1, 2 };
    Integer startMonths[] = { 6, 9 };
    Volatility vols[] = { 0.11, 0.50, 1.20 };

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
            new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

    ext::shared_ptr<PricingEngine> engine(
            new Engine<AnalyticEuropeanEngine>(stochProcess));

    for (auto& type : types) {
        for (Real& moneynes : moneyness) {
            for (int length : lengths) {
                for (int startMonth : startMonths) {

                    Date exDate = today + length * Years;
                    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

                    Date reset = today + startMonth * Months;

                    ext::shared_ptr<StrikedTypePayoff> payoff(
                            new PlainVanillaPayoff(type, 0.0));

                    ForwardVanillaOption option(moneynes, reset, payoff, exercise);
                    option.setPricingEngine(engine);

                    for (Real u : underlyings) {
                        for (Real m : qRates) {
                            for (Real n : rRates) {
                                for (Real v : vols) {

                                    Rate q = m, r = n;
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
                                        Real du = u * 1.0e-4;
                                        spot->setValue(u + du);
                                        Real value_p = option.NPV(), delta_p = option.delta();
                                        spot->setValue(u - du);
                                        Real value_m = option.NPV(), delta_m = option.delta();
                                        spot->setValue(u);
                                        expected["delta"] = (value_p - value_m) / (2 * du);
                                        expected["gamma"] = (delta_p - delta_m) / (2 * du);

                                        // perturb rates and get rho and dividend rho
                                        Spread dr = r * 1.0e-4;
                                        rRate->setValue(r + dr);
                                        value_p = option.NPV();
                                        rRate->setValue(r - dr);
                                        value_m = option.NPV();
                                        rRate->setValue(r);
                                        expected["rho"] = (value_p - value_m) / (2 * dr);

                                        Spread dq = q * 1.0e-4;
                                        qRate->setValue(q + dq);
                                        value_p = option.NPV();
                                        qRate->setValue(q - dq);
                                        value_m = option.NPV();
                                        qRate->setValue(q);
                                        expected["divRho"] = (value_p - value_m) / (2 * dq);

                                        // perturb volatility and get vega
                                        Volatility dv = v * 1.0e-4;
                                        vol->setValue(v + dv);
                                        value_p = option.NPV();
                                        vol->setValue(v - dv);
                                        value_m = option.NPV();
                                        vol->setValue(v);
                                        expected["vega"] = (value_p - value_m) / (2 * dv);

                                        // perturb date and get theta
                                        Time dT = dc.yearFraction(today - 1, today + 1);
                                        Settings::instance().evaluationDate() = today - 1;
                                        value_m = option.NPV();
                                        Settings::instance().evaluationDate() = today + 1;
                                        value_p = option.NPV();
                                        Settings::instance().evaluationDate() = today;
                                        expected["theta"] = (value_p - value_m) / dT;

                                        // compare
                                        std::map<std::string, Real>::iterator it;
                                        for (it = calculated.begin(); it != calculated.end(); ++it) {
                                            std::string greek = it->first;
                                            Real expct = expected[greek],
                                                calcl = calculated[greek],
                                                tol = tolerance[greek];
                                            Real error = relativeError(expct, calcl, u);
                                            if (error > tol) {
                                                REPORT_FAILURE(greek, payoff, exercise, u, q, r,
                                                               today, v, moneynes, reset, expct,
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
        }
    }
}


BOOST_AUTO_TEST_CASE(testValues) {

    BOOST_TEST_MESSAGE("Testing forward option values...");

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
    */
    ForwardOptionData values[] = {
        //  type, moneyness, spot,  div, rate,start,   t,  vol, result, tol
        // "Option pricing formulas", pag. 37
        { Option::Call, 1.1, 60.0, 0.04, 0.08, 0.25, 1.0, 0.30, 4.4064, 1.0e-4 },
        // "Option pricing formulas", VBA code
        {  Option::Put, 1.1, 60.0, 0.04, 0.08, 0.25, 1.0, 0.30, 8.2971, 1.0e-4 }
    };

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
         new BlackScholesMertonProcess(Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

    ext::shared_ptr<PricingEngine> engine(
              new ForwardVanillaEngine<AnalyticEuropeanEngine>(stochProcess));

    for (auto& value : values) {

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(value.type, 0.0));
        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
        Date reset = today + timeToDays(value.start);

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        ForwardVanillaOption option(value.moneyness, reset, payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated - value.result);
        Real tolerance = 1e-4;
        if (error>tolerance) {
            REPORT_FAILURE("value", payoff, exercise, value.s, value.q, value.r, today, value.v,
                           value.moneyness, reset, value.result, calculated, error, tolerance);
        }
    }
}

BOOST_AUTO_TEST_CASE(testPerformanceValues) {

    BOOST_TEST_MESSAGE("Testing forward performance option values...");

    /* The data below are the performance equivalent of the
       forward options tested above and taken from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
    */
    ForwardOptionData values[] = {
        //  type, moneyness, spot,  div, rate,start, maturity,  vol,                       result, tol
        { Option::Call, 1.1, 60.0, 0.04, 0.08, 0.25,      1.0, 0.30, 4.4064/60*std::exp(-0.04*0.25), 1.0e-4 },
        {  Option::Put, 1.1, 60.0, 0.04, 0.08, 0.25,      1.0, 0.30, 8.2971/60*std::exp(-0.04*0.25), 1.0e-4 }
    };

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
         new BlackScholesMertonProcess(Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

    ext::shared_ptr<PricingEngine> engine(
        new ForwardPerformanceVanillaEngine<AnalyticEuropeanEngine>(
                                                               stochProcess));

    for (auto& value : values) {

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(value.type, 0.0));
        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
        Date reset = today + timeToDays(value.start);

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        ForwardVanillaOption option(value.moneyness, reset, payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated - value.result);
        Real tolerance = 1e-4;
        if (error>tolerance) {
            REPORT_FAILURE("value", payoff, exercise, value.s, value.q, value.r, today, value.v,
                           value.moneyness, reset, value.result, calculated, error, tolerance);
        }
    }
}

BOOST_AUTO_TEST_CASE(testGreeks) {

    BOOST_TEST_MESSAGE("Testing forward option greeks...");

    testForwardGreeks<ForwardVanillaEngine>();
}

BOOST_AUTO_TEST_CASE(testPerformanceGreeks) {

    BOOST_TEST_MESSAGE("Testing forward performance option greeks...");

    testForwardGreeks<ForwardPerformanceVanillaEngine>();
}


class TestBinomialEngine : public BinomialVanillaEngine<CoxRossRubinstein>
{
private:
public:
   explicit TestBinomialEngine(
           const ext::shared_ptr<GeneralizedBlackScholesProcess > &process)
   : BinomialVanillaEngine<CoxRossRubinstein>(process, 300) // fixed steps
    {}
};

BOOST_AUTO_TEST_CASE(testGreeksInitialization) {
   BOOST_TEST_MESSAGE("Testing forward option greeks initialization...");

   DayCounter dc = Actual360();
   Date today = Settings::instance().evaluationDate();

   ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
   ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
   Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
   ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.01));
   Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
   ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.11));
   Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

   ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
      new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

   ext::shared_ptr<PricingEngine> engine(
                        new ForwardVanillaEngine<TestBinomialEngine>(stochProcess));
   Date exDate = today + 1*Years;
   ext::shared_ptr<Exercise> exercise(
                                 new EuropeanExercise(exDate));
   Date reset = today + 6*Months;
   ext::shared_ptr<StrikedTypePayoff> payoff(
                        new PlainVanillaPayoff(Option::Call, 0.0));

   ForwardVanillaOption option(0.9, reset, payoff, exercise);
   option.setPricingEngine(engine);

   ext::shared_ptr<PricingEngine> ctrlengine(
                        new TestBinomialEngine(stochProcess));
   VanillaOption ctrloption(payoff, exercise);
   ctrloption.setPricingEngine(ctrlengine);

   Real delta = 0;
   try
   {
      delta = ctrloption.delta();
   }
   catch (const QuantLib::Error &) {
      // if normal option can't calculate delta,
      // nor should forward
      try
      {
         delta   = option.delta();
      }
      catch (const QuantLib::Error &) {
         delta = Null<Real>();
      }
      QL_REQUIRE(delta == Null<Real>(), "Forward delta invalid");
   }

   Real rho  = 0;
   try
   {
      rho = ctrloption.rho();
   }
   catch (const QuantLib::Error &) {
      // if normal option can't calculate rho,
      // nor should forward
      try
      {
         rho = option.rho();
      }
      catch (const QuantLib::Error &) {
         rho = Null<Real>();
      }
      QL_REQUIRE(rho == Null<Real>(), "Forward rho invalid");
   }

   Real divRho = 0;
   try
   {
      divRho = ctrloption.dividendRho();
   }
   catch (const QuantLib::Error &) {
      // if normal option can't calculate divRho,
      // nor should forward
      try
      {
         divRho = option.dividendRho();
      }
      catch (const QuantLib::Error &) {
         divRho = Null<Real>();
      }
      QL_REQUIRE(divRho == Null<Real>(), "Forward dividendRho invalid");
   }

   Real vega = 0;
   try
   {
      vega = ctrloption.vega();
   }
   catch (const QuantLib::Error &) {
      // if normal option can't calculate vega,
      // nor should forward
      try
      {
         vega = option.vega();
      }
      catch (const QuantLib::Error &) {
         vega = Null<Real>();
      }
      QL_REQUIRE(vega == Null<Real>(), "Forward vega invalid");
   }
}

BOOST_AUTO_TEST_CASE(testMCPrices) {
   BOOST_TEST_MESSAGE("Testing forward option MC prices...");

   Real tol[] = {0.002, 0.001, 0.0006, 5e-4, 5e-4};

   Size timeSteps = 100;
   Size numberOfSamples = 5000;
   Size mcSeed = 42;

   Real q = 0.04;
   Real r = 0.01;
   Real sigma = 0.11;
   Real s = 100;

   DayCounter dc = Actual360();
   Date today = Settings::instance().evaluationDate();

   ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(s));
   ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(q));
   Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
   ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(r));
   Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
   ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(sigma));
   Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

   ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
      new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

   ext::shared_ptr<PricingEngine> analyticEngine(
                        new ForwardVanillaEngine<AnalyticEuropeanEngine>(stochProcess));

   ext::shared_ptr<PricingEngine> mcEngine 
      = MakeMCForwardEuropeanBSEngine<PseudoRandom>(stochProcess)
            .withSteps(timeSteps)
            .withSamples(numberOfSamples)
            .withSeed(mcSeed);

   Date exDate = today + 1*Years;
   ext::shared_ptr<Exercise> exercise(
                                 new EuropeanExercise(exDate));
   Date reset = today + 6*Months;
   ext::shared_ptr<StrikedTypePayoff> payoff(
                        new PlainVanillaPayoff(Option::Call, 0.0));

   Real moneyness[] = { 0.8, 0.9, 1.0, 1.1, 1.2 };

   for (Size moneyness_index = 0; moneyness_index < std::size(moneyness); ++moneyness_index) {

       ForwardVanillaOption option(moneyness[moneyness_index], reset, payoff, exercise);

       option.setPricingEngine(analyticEngine);
       Real analyticPrice = option.NPV();

       option.setPricingEngine(mcEngine);
       Real mcPrice = option.NPV();

       Real error = relativeError(analyticPrice, mcPrice, s);
       if (error > tol[moneyness_index]) {
           REPORT_FAILURE("testMCPrices", payoff, exercise, s, q, r, today, sigma, moneyness[moneyness_index], reset,
                          analyticPrice, mcPrice, error, tol[moneyness_index]);
       }
   }
}

BOOST_AUTO_TEST_CASE(testHestonMCPrices) {
   BOOST_TEST_MESSAGE("Testing forward option Heston MC prices...");

   Option::Type optionTypes[] = { Option::Call, Option::Put };
   Real mcForwardStartTolerance[][6] = {{7e-4,    // Call, moneyness=0.8
                                         8e-4,    // Call, moneyness=0.9
                                         6e-4,    // Call, moneyness=1.0
                                         5e-4,    // Call, moneyness=1.1
                                         5e-4},   // Call, moneyness=1.2
                                        {6e-4,    // Put, moneyness=0.8
                                         5e-4,    // Put, moneyness=0.9
                                         6e-4,    // Put, moneyness=1.0
                                         0.001,   // Put, moneyness=1.1
                                         0.001}}; // Put, moneyness=1.2

   Real tol[][6] = {{9e-4,    // Call, moneyness=0.8
                     9e-4,    // Call, moneyness=0.9
                     6e-4,    // Call, moneyness=1.0
                     5e-4,    // Call, moneyness=1.1
                     5e-4},   // Call, moneyness=1.2
                    {6e-4,    // Put, moneyness=0.8
                     5e-4,    // Put, moneyness=0.9
                     8e-4,    // Put, moneyness=1.0
                     0.002,   // Put, moneyness=1.1
                     0.002}}; // Put, moneyness=1.2

   for (Size type_index = 0; type_index < std::size(optionTypes); ++type_index) {

       Real analyticTolerance = 5e-4;

       Size timeSteps = 50;
       Size numberOfSamples = 4095;
       Size mcSeed = 42;

       Real q = 0.04;
       Real r = 0.01;
       Real sigma_bs = 0.245;
       Real s = 100;

       // Test 1: Set up an equivalent flat Heston and compare to analytical BS pricing
       Real v0 = sigma_bs * sigma_bs;
       Real kappa = 1e-8;
       Real theta = sigma_bs * sigma_bs;
       Real sigma = 1e-8;
       Real rho = -0.93;

       DayCounter dc = Actual360();
       Date today = Settings::instance().evaluationDate();

       Date exDate = today + 1 * Years;
       ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
       Date reset = today + 6 * Months;
       ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(optionTypes[type_index], 0.0));

       ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(s));
       ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(q));
       Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
       ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(r));
       Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
       ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(sigma_bs));
       Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

       ext::shared_ptr<BlackScholesMertonProcess> bsProcess(
           new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

       ext::shared_ptr<PricingEngine> analyticEngine(
           new ForwardVanillaEngine<AnalyticEuropeanEngine>(bsProcess));

       ext::shared_ptr<HestonProcess> hestonProcess(
           new HestonProcess(rTS, qTS, Handle<Quote>(spot), v0, kappa, theta, sigma, rho));

       ext::shared_ptr<PricingEngine> mcEngine =
           MakeMCForwardEuropeanHestonEngine<LowDiscrepancy>(hestonProcess)
               .withSteps(timeSteps)
               .withSamples(numberOfSamples)
               .withSeed(mcSeed);

       Real moneyness[] = {0.8, 0.9, 1.0, 1.1, 1.2};

       for (Size moneyness_index = 0; moneyness_index < std::size(moneyness); ++moneyness_index) {

           ForwardVanillaOption option(moneyness[moneyness_index], reset, payoff, exercise);

           option.setPricingEngine(analyticEngine);
           Real analyticPrice = option.NPV();

           option.setPricingEngine(mcEngine);
           Real mcPrice = option.NPV();

           Real mcError = relativeError(analyticPrice, mcPrice, s);

           if (mcError > mcForwardStartTolerance[type_index][moneyness_index]) {
               REPORT_FAILURE("testHestonMCForwardStartPrices", payoff, exercise, s, q, r, today,
                              sigma_bs, moneyness[moneyness_index], reset, analyticPrice, mcPrice, mcError, mcForwardStartTolerance[type_index][moneyness_index]);
           }
       }

       // Test 2: Using an arbitrary Heston model, check that prices match semi-analytical
       // Heston prices when reset date is t=0
       v0 = sigma_bs * sigma_bs;
       kappa = 1.0;
       theta = 0.08;
       sigma = 0.39;
       rho = -0.93;

       reset = today;

       ext::shared_ptr<HestonProcess> hestonProcessSmile(
           new HestonProcess(rTS, qTS, Handle<Quote>(spot), v0, kappa, theta, sigma, rho));

       ext::shared_ptr<HestonModel> hestonModel(ext::make_shared<HestonModel>(hestonProcessSmile));

       ext::shared_ptr<PricingEngine> analyticHestonEngine(
           ext::make_shared<AnalyticHestonEngine>(hestonModel, 96));

       ext::shared_ptr<PricingEngine> mcEngineSmile =
           MakeMCForwardEuropeanHestonEngine<LowDiscrepancy>(hestonProcessSmile)
               .withSteps(timeSteps)
               .withSamples(numberOfSamples)
               .withSeed(mcSeed);

       ext::shared_ptr<AnalyticHestonForwardEuropeanEngine> analyticForwardHestonEngine(
           new AnalyticHestonForwardEuropeanEngine(hestonProcessSmile));

       for (Size moneyness_index = 0; moneyness_index < std::size(moneyness); ++moneyness_index) {

           Real strike = s * moneyness[moneyness_index];
           ext::shared_ptr<StrikedTypePayoff> vanillaPayoff(new PlainVanillaPayoff(optionTypes[type_index], strike));

           VanillaOption vanillaOption(vanillaPayoff, exercise);
           ForwardVanillaOption forwardOption(moneyness[moneyness_index], reset, payoff, exercise);

           vanillaOption.setPricingEngine(analyticHestonEngine);
           Real analyticPrice = vanillaOption.NPV();

           forwardOption.setPricingEngine(mcEngineSmile);
           Real mcPrice = forwardOption.NPV();

           Real mcError = relativeError(analyticPrice, mcPrice, s);
           auto tolerance = tol[type_index][moneyness_index];

           if (mcError > tolerance) {
               REPORT_FAILURE("testHestonMCPrices", vanillaPayoff, exercise, s, q, r, today,
                              sigma_bs, moneyness[moneyness_index], reset, analyticPrice, mcPrice, mcError,
                              tolerance);
           }

           // T=0, testing the Analytic Pricer's T=0 analytical solution
           forwardOption.setPricingEngine(analyticForwardHestonEngine);
           Real hestonAnalyticPrice = forwardOption.NPV();

           Real analyticError = relativeError(analyticPrice, hestonAnalyticPrice, s);
           if (analyticError > analyticTolerance) {
               REPORT_FAILURE("testHestonAnalyticForwardStartPrices", vanillaPayoff, exercise, s, q,
                              r, today, sigma_bs, moneyness[moneyness_index], reset, analyticPrice,
                              hestonAnalyticPrice, analyticError, analyticTolerance);
           }
       }
   }
}

BOOST_AUTO_TEST_CASE(testHestonAnalyticalVsMCPrices, *precondition(if_speed(Fast))) {
   BOOST_TEST_MESSAGE("Testing Heston analytic vs MC prices...");

   Option::Type optionTypes[] = { Option::Call, Option::Put };
   Real tol[][6] = {{0.002,   // Call, moneyness=0.8, CV:false
                     0.002,   // Call, moneyness=0.8, CV:true
                     0.001,   // Call, moneyness=1.0, CV:false
                     0.001,   // Call, moneyness=1.8, CV:true
                     0.001,   // Call, moneyness=1.2, CV:false
                     0.001},  // Call, moneyness=1.2, CV:true
                    {0.001,   // Put, moneyness=0.8, CV:false
                     0.001,   // Put, moneyness=0.8, CV:true
                     0.003,   // Put, moneyness=1.0, CV:false
                     0.003,   // Put, moneyness=1.0, CV:true
                     0.003,   // Put, moneyness=1.2, CV:false
                     0.003}}; // Put, moneyness=1.2, CV:true

   for (Size option_type_index = 0; option_type_index < std::size(optionTypes); ++option_type_index) {

       Size timeSteps = 50;
       Size numberOfSamples = 5000;
       Size mcSeed = 42;

       Real q = 0.03;
       Real r = 0.005;
       Real s = 100;

       Real vol = 0.3;
       Real v0 = vol * vol;
       Real kappa = 11.35;
       Real theta = 0.022;
       Real sigma = 0.618;
       Real rho = -0.5;

       DayCounter dc = Actual360();
       Date today = Settings::instance().evaluationDate();

       Date exDate = today + 1 * Years;
       ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
       Date reset = today + 6 * Months;
       ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(optionTypes[option_type_index], 0.0));

       ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(s));
       ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(q));
       Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
       ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(r));
       Handle<YieldTermStructure> rTS(flatRate(rRate, dc));

       ext::shared_ptr<HestonProcess> hestonProcess(
           new HestonProcess(rTS, qTS, Handle<Quote>(spot), v0, kappa, theta, sigma, rho));

       ext::shared_ptr<PricingEngine> mcEngine =
           MakeMCForwardEuropeanHestonEngine<PseudoRandom>(hestonProcess)
               .withSteps(timeSteps)
               .withSamples(numberOfSamples)
               .withSeed(mcSeed);

       ext::shared_ptr<PricingEngine> mcEngineCv =
           MakeMCForwardEuropeanHestonEngine<PseudoRandom>(hestonProcess)
               .withSteps(timeSteps)
               .withSamples(numberOfSamples)
               .withSeed(mcSeed)
               .withControlVariate(true);

       ext::shared_ptr<AnalyticHestonForwardEuropeanEngine> analyticEngine(
           new AnalyticHestonForwardEuropeanEngine(hestonProcess));

      Real moneyness[] = { 0.8, 1.0, 1.2 };

      for (Size tol_2nd_index = 0; tol_2nd_index < std::size(moneyness); ++tol_2nd_index) {

          auto m = moneyness[tol_2nd_index];
          ForwardVanillaOption option(m, reset, payoff, exercise);

          option.setPricingEngine(analyticEngine);
          Real analyticPrice = option.NPV();

          option.setPricingEngine(mcEngine);
          Real mcPrice = option.NPV();
          Real error = relativeError(analyticPrice, mcPrice, s);

          auto tolerance = tol[option_type_index][tol_2nd_index];
          if (error > tolerance) {
              REPORT_FAILURE("testHestonMCVsAnalyticPrices", payoff, exercise, s, q, r, today, vol,
                             m, reset, analyticPrice, mcPrice, error, tolerance);
          }

          option.setPricingEngine(mcEngineCv);
          Real mcPriceCv = option.NPV();

          Real errorCv = relativeError(analyticPrice, mcPriceCv, s);
          tolerance = tol[option_type_index][++tol_2nd_index];
          if (errorCv > tolerance) {
              REPORT_FAILURE("testHestonMCControlVariateVsAnalyticPrices", payoff, exercise, s, q,
                             r, today, vol, m, reset, analyticPrice, mcPrice, errorCv,
                             tolerance);
          }
      }
   }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
