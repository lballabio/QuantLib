/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2004, 2007 StatPro Italia srl
 Copyright (C) 2008 Paul Farrington
 Copyright (C) 2014 Thema Consulting SA
 Copyright (C) 2019 Klaus Spanderen

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

#include "quantooption.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/instruments/quantovanillaoption.hpp>
#include <ql/instruments/quantoforwardvanillaoption.hpp>
#include <ql/instruments/quantobarrieroption.hpp>
#include <ql/experimental/barrieroption/quantodoublebarrieroption.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/fdhestonvanillaengine.hpp>
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <ql/experimental/barrieroption/analyticdoublebarrierengine.hpp>
#include <ql/pricingengines/quanto/quantoengine.hpp>
#include <ql/pricingengines/forward/forwardperformanceengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/volatility/equityfx/localconstantvol.hpp>
#include <ql/methods/finitedifferences/utilities/fdmquantohelper.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#undef QUANTO_REPORT_FAILURE
#define QUANTO_REPORT_FAILURE(greekName, payoff, exercise, s, q, r, \
                        today, v, fxr, fxv, corr, expected, \
                        calculated, error, tolerance) \
    BOOST_FAIL("Quanto " << exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    spot value:        " << s << "\n" \
               << "    strike:            " << payoff->strike() << "\n" \
               << "    dividend yield:    " << io::rate(q) << "\n" \
               << "    risk-free rate:    " << io::rate(r) << "\n" \
               << "    fx risk-free rate: " << io::rate(fxr) << "\n" \
               << "    reference date:    " << today << "\n" \
               << "    maturity:          " << exercise->lastDate() << "\n" \
               << "    volatility:        " << io::volatility(v) << "\n" \
               << "    fx volatility:     " << io::volatility(fxv) << "\n" \
               << "    correlation:       " << corr << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);

#undef QUANTO_FORWARD_REPORT_FAILURE
#define QUANTO_FORWARD_REPORT_FAILURE(greekName, payoff, moneyness, \
                        exercise, s, q, r, \
                        today, reset, v, fxr, fxv, corr, expected, \
                        calculated, error, tolerance) \
    BOOST_FAIL("Quanto " << exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    spot value:        " << s << "\n" \
               << "    strike:            " << payoff->strike() << "\n" \
               << "    moneyness:         " << io::percent(moneyness) << "\n" \
               << "    dividend yield:    " << io::rate(q) << "\n" \
               << "    risk-free rate:    " << io::rate(r) << "\n" \
               << "    fx risk-free rate: " << io::rate(fxr) << "\n" \
               << "    reference date:    " << today << "\n" \
               << "    reset date:        " << reset << "\n" \
               << "    maturity:          " << exercise->lastDate() << "\n" \
               << "    volatility:        " << io::volatility(v) << "\n" \
               << "    fx volatility:     " << io::volatility(fxv) << "\n" \
               << "    correlation:       " << corr << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);

#undef QUANTO_BARRIER_REPORT_FAILURE
#define QUANTO_BARRIER_REPORT_FAILURE(greekName, payoff, \
                        barrierType, barrier, rebate, \
                        exercise, s, q, r, \
                        today, v, fxr, fxv, corr, expected, \
                        calculated, error, tolerance) \
    BOOST_FAIL("Quanto Barrier" << exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << "    barrier type:        " << barrierType << "\n" \
               << "    barrier:             " << barrier << "\n" \
               << "    rebate:              " << rebate << "\n" \
               << "    payoff:              " << payoffTypeToString(payoff) << "\n" \
               << "    spot value:          " << s << "\n" \
               << "    strike:              " << payoff->strike() << "\n" \
               << "    dividend yield:      " << io::rate(q) << "\n" \
               << "    risk-free rate:      " << io::rate(r) << "\n" \
               << "    fx risk-free rate:   " << io::rate(fxr) << "\n" \
               << "    reference date:      " << today << "\n" \
               << "    maturity:            " << exercise->lastDate() << "\n" \
               << "    volatility:          " << io::volatility(v) << "\n" \
               << "    fx volatility:       " << io::volatility(fxv) << "\n" \
               << "    correlation:         " << corr << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);

#undef QUANTO_DOUBLE_BARRIER_REPORT_FAILURE
#define QUANTO_DOUBLE_BARRIER_REPORT_FAILURE(greekName, payoff, \
                        barrierType, barrier_lo, barrier_hi, rebate, \
                        exercise, s, q, r, \
                        today, v, fxr, fxv, corr, expected, \
                        calculated, error, tolerance) \
    BOOST_ERROR("Quanto Double Barrier" << exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << "    barrier type:        " << barrierType << "\n" \
               << "    barrier_lo:          " << barrier_lo << "\n" \
               << "    barrier_hi:          " << barrier_hi << "\n" \
               << "    rebate:              " << rebate << "\n" \
               << "    payoff:              " << payoffTypeToString(payoff) << "\n" \
               << "    spot value:          " << s << "\n" \
               << "    strike:              " << payoff->strike() << "\n" \
               << "    dividend yield:      " << io::rate(q) << "\n" \
               << "    risk-free rate:      " << io::rate(r) << "\n" \
               << "    fx risk-free rate:   " << io::rate(fxr) << "\n" \
               << "    reference date:      " << today << "\n" \
               << "    maturity:            " << exercise->lastDate() << "\n" \
               << "    volatility:          " << io::volatility(v) << "\n" \
               << "    fx volatility:       " << io::volatility(fxv) << "\n" \
               << "    correlation:         " << corr << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);

namespace {

    struct QuantoOptionData {
        Option::Type type;
        Real strike;
        Real s;          // spot
        Rate q;          // dividend
        Rate r;          // risk-free rate
        Time t;          // time to maturity
        Volatility v;    // volatility
        Rate fxr;        // fx risk-free rate
        Volatility fxv;  // fx volatility
        Real corr;       // correlation
        Real result;     // expected result
        Real tol;        // tolerance
    };

    struct QuantoForwardOptionData {
        Option::Type type;
        Real moneyness;
        Real s;          // spot
        Rate q;          // dividend
        Rate r;          // risk-free rate
        Time start;      // time to reset
        Time t;          // time to maturity
        Volatility v;    // volatility
        Rate fxr;        // fx risk-free rate
        Volatility fxv;  // fx volatility
        Real corr;       // correlation
        Real result;     // expected result
        Real tol;        // tolerance
    };

    struct QuantoBarrierOptionData {
        Barrier::Type barrierType;
        Real barrier;
        Real rebate;
        Option::Type type;
        Real s;          // spot
        Real strike;
        Rate q;          // dividend
        Rate r;          // risk-free rate
        Time t;          // time to maturity
        Volatility v;    // volatility
        Rate fxr;        // fx risk-free rate
        Volatility fxv;  // fx volatility
        Real corr;       // correlation
        Real result;     // expected result
        Real tol;        // tolerance
    };

    struct QuantoDoubleBarrierOptionData {
        DoubleBarrier::Type barrierType;
        Real barrier_lo;
        Real barrier_hi;
        Real rebate;
        Option::Type type;
        Real s;          // spot
        Real strike;
        Rate q;          // dividend
        Rate r;          // risk-free rate
        Time t;          // time to maturity
        Volatility v;    // volatility
        Rate fxr;        // fx risk-free rate
        Volatility fxv;  // fx volatility
        Real corr;       // correlation
        Real result;     // expected result
        Real tol;        // tolerance
    };
}


void QuantoOptionTest::testValues() {

    BOOST_TEST_MESSAGE("Testing quanto option values...");

    SavedSettings backup;

    /* The data below are from
       from "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
    */
    QuantoOptionData values[] = {
        //       type, strike,  spot,  div, rate,   t, vol, fx risk-free rate, fx volatility, correlation,     result, tol
        // "Option pricing formulas", pag 105-106
        { Option::Call, 105.0, 100.0, 0.04, 0.08, 0.5, 0.2,              0.05,          0.10,         0.3, 5.3280/1.5, 1.0e-4 },
        // "Option pricing formulas", VBA code
        {  Option::Put, 105.0, 100.0, 0.04, 0.08, 0.5, 0.2,              0.05,          0.10,         0.3,     8.1636, 1.0e-4 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    ext::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> fxrTS(flatRate(today, fxRate, dc));
    ext::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> fxVolTS(flatVol(today, fxVol, dc));
    ext::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
         new BlackScholesMertonProcess(Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));
    ext::shared_ptr<PricingEngine> engine(
        new QuantoEngine<VanillaOption, AnalyticEuropeanEngine>(
                                                 stochProcess, fxrTS, fxVolTS,
                                                 Handle<Quote>(correlation)));

    for (auto& value : values) {

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(value.type, value.strike));
        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        fxRate->setValue(value.fxr);
        fxVol->setValue(value.fxv);
        correlation->setValue(value.corr);

        QuantoVanillaOption option(payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated - value.result);
        Real tolerance = 1e-4;
        if (error>tolerance) {
            QUANTO_REPORT_FAILURE("value", payoff, exercise, value.s, value.q, value.r, today,
                                  value.v, value.fxr, value.fxv, value.corr, value.result,
                                  calculated, error, tolerance);
        }
    }
}


void QuantoOptionTest::testGreeks() {

    BOOST_TEST_MESSAGE("Testing quanto option greeks...");

    SavedSettings backup;

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"]   = 1.0e-5;
    tolerance["gamma"]   = 1.0e-5;
    tolerance["theta"]   = 1.0e-5;
    tolerance["rho"]     = 1.0e-5;
    tolerance["divRho"]  = 1.0e-5;
    tolerance["vega"]    = 1.0e-5;
    tolerance["qrho"]    = 1.0e-5;
    tolerance["qvega"]   = 1.0e-5;
    tolerance["qlambda"] = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.04, 0.05 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 2 };
    Volatility vols[] = { 0.11, 1.20 };
    Real correlations[] = { 0.10, 0.90 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));
    ext::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> fxrTS(flatRate(fxRate, dc));
    ext::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> fxVolTS(flatVol(fxVol, dc));
    ext::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
         new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

    ext::shared_ptr<PricingEngine> engine(
        new QuantoEngine<VanillaOption,AnalyticEuropeanEngine>(
                                                  stochProcess,fxrTS, fxVolTS,
                                                  Handle<Quote>(correlation)));

    for (auto& type : types) {
        for (Real strike : strikes) {
            for (int length : lengths) {

                Date exDate = today + length * Years;
                ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

                ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));

                QuantoVanillaOption option(payoff, exercise);
                option.setPricingEngine(engine);

                for (Real u : underlyings) {
                    for (Real m : qRates) {
                        for (Real n : rRates) {
                            for (Real v : vols) {
                                for (Real fxr : rRates) {
                                    for (Real fxv : vols) {
                                        for (Real corr : correlations) {

                                            Rate q = m, r = n;
                                            spot->setValue(u);
                                            qRate->setValue(q);
                                            rRate->setValue(r);
                                            vol->setValue(v);
                                            fxRate->setValue(fxr);
                                            fxVol->setValue(fxv);
                                            correlation->setValue(corr);

                                            Real value = option.NPV();
                                            calculated["delta"] = option.delta();
                                            calculated["gamma"] = option.gamma();
                                            calculated["theta"] = option.theta();
                                            calculated["rho"] = option.rho();
                                            calculated["divRho"] = option.dividendRho();
                                            calculated["vega"] = option.vega();
                                            calculated["qrho"] = option.qrho();
                                            calculated["qvega"] = option.qvega();
                                            calculated["qlambda"] = option.qlambda();

                                            if (value > spot->value() * 1.0e-5) {
                                                // perturb spot and get delta and gamma
                                                Real du = u * 1.0e-4;
                                                spot->setValue(u + du);
                                                Real value_p = option.NPV(),
                                                     delta_p = option.delta();
                                                spot->setValue(u - du);
                                                Real value_m = option.NPV(),
                                                     delta_m = option.delta();
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

                                                // perturb fx rate and get qrho
                                                Spread dfxr = fxr * 1.0e-4;
                                                fxRate->setValue(fxr + dfxr);
                                                value_p = option.NPV();
                                                fxRate->setValue(fxr - dfxr);
                                                value_m = option.NPV();
                                                fxRate->setValue(fxr);
                                                expected["qrho"] = (value_p - value_m) / (2 * dfxr);

                                                // perturb fx volatility and get qvega
                                                Volatility dfxv = fxv * 1.0e-4;
                                                fxVol->setValue(fxv + dfxv);
                                                value_p = option.NPV();
                                                fxVol->setValue(fxv - dfxv);
                                                value_m = option.NPV();
                                                fxVol->setValue(fxv);
                                                expected["qvega"] =
                                                    (value_p - value_m) / (2 * dfxv);

                                                // perturb correlation and get qlambda
                                                Real dcorr = corr * 1.0e-4;
                                                correlation->setValue(corr + dcorr);
                                                value_p = option.NPV();
                                                correlation->setValue(corr - dcorr);
                                                value_m = option.NPV();
                                                correlation->setValue(corr);
                                                expected["qlambda"] =
                                                    (value_p - value_m) / (2 * dcorr);

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
                                                for (it = calculated.begin();
                                                     it != calculated.end(); ++it) {
                                                    std::string greek = it->first;
                                                    Real expct = expected[greek],
                                                         calcl = calculated[greek],
                                                         tol = tolerance[greek];
                                                    Real error = relativeError(expct, calcl, u);
                                                    if (error > tol) {
                                                        QUANTO_REPORT_FAILURE(
                                                            greek, payoff, exercise, u, q, r, today,
                                                            v, fxr, fxv, corr, expct, calcl, error,
                                                            tol);
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
    }
}



void QuantoOptionTest::testForwardValues() {

    BOOST_TEST_MESSAGE("Testing quanto-forward option values...");

    SavedSettings backup;

    QuantoForwardOptionData values[] = {
        //   type, moneyness,  spot,  div, risk-free rate, reset, maturity,  vol, fx risk-free rate, fx vol, corr,     result, tol
        // reset=0.0, quanto (not-forward) options
        { Option::Call, 1.05, 100.0, 0.04,           0.08,  0.00,      0.5, 0.20,              0.05,   0.10,  0.3, 5.3280/1.5, 1.0e-4 },
        {  Option::Put, 1.05, 100.0, 0.04,           0.08,  0.00,      0.5, 0.20,              0.05,   0.10,  0.3,     8.1636, 1.0e-4 },
        // reset!=0.0, quanto-forward options (cursory checked against FinCAD 7)
        { Option::Call, 1.05, 100.0, 0.04,           0.08,  0.25,      0.5, 0.20,              0.05,   0.10,  0.3,     2.0171, 1.0e-4 },
        {  Option::Put, 1.05, 100.0, 0.04,           0.08,  0.25,      0.5, 0.20,              0.05,   0.10,  0.3,     6.7296, 1.0e-4 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    ext::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> fxrTS(flatRate(today, fxRate, dc));
    ext::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> fxVolTS(flatVol(today, fxVol, dc));
    ext::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
         new BlackScholesMertonProcess(Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

    ext::shared_ptr<PricingEngine> engine(
        new QuantoEngine<ForwardVanillaOption,
                         ForwardVanillaEngine<AnalyticEuropeanEngine> >(
                                                 stochProcess, fxrTS, fxVolTS,
                                                 Handle<Quote>(correlation)));

    for (auto& value : values) {

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(value.type, 0.0));
        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
        Date reset = today + timeToDays(value.start);

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        fxRate->setValue(value.fxr);
        fxVol->setValue(value.fxv);
        correlation->setValue(value.corr);

        QuantoForwardVanillaOption option(value.moneyness, reset, payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated - value.result);
        Real tolerance = 1e-4;
        if (error>tolerance) {
            QUANTO_FORWARD_REPORT_FAILURE("value", payoff, value.moneyness, exercise, value.s,
                                          value.q, value.r, today, reset, value.v, value.fxr,
                                          value.fxv, value.corr, value.result, calculated, error,
                                          tolerance);
        }
    }
}


void QuantoOptionTest::testForwardGreeks() {

    BOOST_TEST_MESSAGE("Testing quanto-forward option greeks...");

    SavedSettings backup;

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"]   = 1.0e-5;
    tolerance["gamma"]   = 1.0e-5;
    tolerance["theta"]   = 1.0e-5;
    tolerance["rho"]     = 1.0e-5;
    tolerance["divRho"]  = 1.0e-5;
    tolerance["vega"]    = 1.0e-5;
    tolerance["qrho"]    = 1.0e-5;
    tolerance["qvega"]   = 1.0e-5;
    tolerance["qlambda"] = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real moneyness[] = { 0.9, 1.0, 1.1 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.04, 0.05 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 2 };
    Integer startMonths[] = { 6, 9 };
    Volatility vols[] = { 0.11, 1.20 };
    Real correlations[] = { 0.10, 0.90 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));
    ext::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> fxrTS(flatRate(fxRate, dc));
    ext::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> fxVolTS(flatVol(fxVol, dc));
    ext::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
         new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

    ext::shared_ptr<PricingEngine> engine(
        new QuantoEngine<ForwardVanillaOption,
                         ForwardVanillaEngine<AnalyticEuropeanEngine> >(
                                                 stochProcess, fxrTS, fxVolTS,
                                                 Handle<Quote>(correlation)));

    for (auto& type : types) {
        for (Real moneynes : moneyness) {
            for (int length : lengths) {
                for (int startMonth : startMonths) {

                    Date exDate = today + length * Years;
                    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

                    Date reset = today + startMonth * Months;

                    ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, 0.0));

                    QuantoForwardVanillaOption option(moneynes, reset, payoff, exercise);
                    option.setPricingEngine(engine);

                    for (Real u : underlyings) {
                        for (Real m : qRates) {
                            for (Real n : rRates) {
                                for (Real v : vols) {
                                    for (Real fxr : rRates) {
                                        for (Real fxv : vols) {
                                            for (Real corr : correlations) {

                                                Rate q = m, r = n;
                                                spot->setValue(u);
                                                qRate->setValue(q);
                                                rRate->setValue(r);
                                                vol->setValue(v);
                                                fxRate->setValue(fxr);
                                                fxVol->setValue(fxv);
                                                correlation->setValue(corr);

                                                Real value = option.NPV();
                                                calculated["delta"] = option.delta();
                                                calculated["gamma"] = option.gamma();
                                                calculated["theta"] = option.theta();
                                                calculated["rho"] = option.rho();
                                                calculated["divRho"] = option.dividendRho();
                                                calculated["vega"] = option.vega();
                                                calculated["qrho"] = option.qrho();
                                                calculated["qvega"] = option.qvega();
                                                calculated["qlambda"] = option.qlambda();

                                                if (value > spot->value() * 1.0e-5) {
                                                    // perturb spot and get delta and gamma
                                                    Real du = u * 1.0e-4;
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
                                                    Spread dr = r * 1.0e-4;
                                                    rRate->setValue(r + dr);
                                                    value_p = option.NPV();
                                                    rRate->setValue(r - dr);
                                                    value_m = option.NPV();
                                                    rRate->setValue(r);
                                                    expected["rho"] =
                                                        (value_p - value_m) / (2 * dr);

                                                    Spread dq = q * 1.0e-4;
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

                                                    // perturb fx rate and get qrho
                                                    Spread dfxr = fxr * 1.0e-4;
                                                    fxRate->setValue(fxr + dfxr);
                                                    value_p = option.NPV();
                                                    fxRate->setValue(fxr - dfxr);
                                                    value_m = option.NPV();
                                                    fxRate->setValue(fxr);
                                                    expected["qrho"] =
                                                        (value_p - value_m) / (2 * dfxr);

                                                    // perturb fx volatility and get qvega
                                                    Volatility dfxv = fxv * 1.0e-4;
                                                    fxVol->setValue(fxv + dfxv);
                                                    value_p = option.NPV();
                                                    fxVol->setValue(fxv - dfxv);
                                                    value_m = option.NPV();
                                                    fxVol->setValue(fxv);
                                                    expected["qvega"] =
                                                        (value_p - value_m) / (2 * dfxv);

                                                    // perturb correlation and get qlambda
                                                    Real dcorr = corr * 1.0e-4;
                                                    correlation->setValue(corr + dcorr);
                                                    value_p = option.NPV();
                                                    correlation->setValue(corr - dcorr);
                                                    value_m = option.NPV();
                                                    correlation->setValue(corr);
                                                    expected["qlambda"] =
                                                        (value_p - value_m) / (2 * dcorr);

                                                    // perturb date and get theta
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
                                                    for (it = calculated.begin();
                                                         it != calculated.end(); ++it) {
                                                        std::string greek = it->first;
                                                        Real expct = expected[greek],
                                                             calcl = calculated[greek],
                                                             tol = tolerance[greek];
                                                        Real error = relativeError(expct, calcl, u);
                                                        if (error > tol) {
                                                            QUANTO_FORWARD_REPORT_FAILURE(
                                                                greek, payoff, moneynes, exercise,
                                                                u, q, r, today, reset, v, fxr, fxv,
                                                                corr, expct, calcl, error, tol);
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
        }
    }
}


void QuantoOptionTest::testForwardPerformanceValues() {

    BOOST_TEST_MESSAGE("Testing quanto-forward-performance option values...");

    SavedSettings backup;

    QuantoForwardOptionData values[] = {
        //   type, moneyness,  spot,  div, risk-free rate, reset, maturity,  vol, fx risk-free rate, fx vol, corr,     result, tol
        // reset=0.0, quanto-(not-forward)-performance options
        // exactly one hundredth of the non-performance version
        { Option::Call, 1.05, 100.0, 0.04,           0.08,  0.00,      0.5, 0.20,              0.05,   0.10,  0.3, 5.3280/150, 1.0e-4 },
        {  Option::Put, 1.05, 100.0, 0.04,           0.08,  0.00,      0.5, 0.20,              0.05,   0.10,  0.3,     0.0816, 1.0e-4 },
        // reset!=0.0, quanto-forward-performance options (roughly one hundredth of the non-performance version)
        { Option::Call, 1.05, 100.0, 0.04,           0.08,  0.25,      0.5, 0.20,              0.05,   0.10,  0.3,     0.0201, 1.0e-4 },
        {  Option::Put, 1.05, 100.0, 0.04,           0.08,  0.25,      0.5, 0.20,              0.05,   0.10,  0.3,     0.0672, 1.0e-4 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    ext::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> fxrTS(flatRate(today, fxRate, dc));
    ext::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> fxVolTS(flatVol(today, fxVol, dc));
    ext::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
         new BlackScholesMertonProcess(Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

    ext::shared_ptr<PricingEngine> engine(
        new QuantoEngine<ForwardVanillaOption,
                         ForwardPerformanceVanillaEngine<AnalyticEuropeanEngine> >(
                                                 stochProcess, fxrTS, fxVolTS,
                                                 Handle<Quote>(correlation)));

    for (auto& value : values) {

        ext::shared_ptr<StrikedTypePayoff> payoff(
            //                               new PercentageStrikePayoff(values[i].type,
            //                               values[i].moneyness));
            new PlainVanillaPayoff(value.type, 0.0));
        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
        Date reset = today + timeToDays(value.start);

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        fxRate->setValue(value.fxr);
        fxVol->setValue(value.fxv);
        correlation->setValue(value.corr);

        QuantoForwardVanillaOption option(value.moneyness, reset, payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated - value.result);
        Real tolerance = 1e-4;
        if (error>tolerance) {
            QUANTO_FORWARD_REPORT_FAILURE("value", payoff, value.moneyness, exercise, value.s,
                                          value.q, value.r, today, reset, value.v, value.fxr,
                                          value.fxv, value.corr, value.result, calculated, error,
                                          tolerance);
        }
    }
}

void QuantoOptionTest::testBarrierValues()  {

    BOOST_TEST_MESSAGE("Testing quanto-barrier option values...");

    SavedSettings backup;

    QuantoBarrierOptionData values[] = {
         // TODO:  Bench results against an existing prop calculator
         // barrierType, barrier, rebate, type, spot, strike,
         // q, r, T, vol, fx risk-free rate, fx vol, corr, result, tol
        { Barrier::DownOut, 95.0, 3.0, Option::Call, 100, 90,
          0.04, 0.0212, 0.50, 0.25, 0.05, 0.2, 0.3, 8.247, 0.5 },
        { Barrier::DownOut, 95.0, 3.0, Option::Put, 100, 90,
          0.04, 0.0212, 0.50, 0.25, 0.05, 0.2, 0.3, 2.274, 0.5 },
        { Barrier::DownIn, 95.0, 0, Option::Put, 100, 90,
          0.04, 0.0212, 0.50, 0.25, 0.05, 0.2, 0.3, 2.85, 0.5 },
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    ext::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> fxrTS(flatRate(today, fxRate, dc));
    ext::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> fxVolTS(flatVol(today, fxVol, dc));
    ext::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
         new BlackScholesMertonProcess(Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

    ext::shared_ptr<PricingEngine> engine(
        new QuantoEngine<BarrierOption, AnalyticBarrierEngine>(
                                                 stochProcess, fxrTS, fxVolTS,
                                                 Handle<Quote>(correlation)));

    for (auto& value : values) {

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(value.type, value.strike));

        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        fxRate->setValue(value.fxr);
        fxVol->setValue(value.fxv);
        correlation->setValue(value.corr);

        QuantoBarrierOption option(value.barrierType, value.barrier, value.rebate, payoff,
                                   exercise);

        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated - value.result);
        Real tolerance = value.tol;

        if (error>tolerance) {
            QUANTO_BARRIER_REPORT_FAILURE("value", payoff, value.barrierType, value.barrier,
                                          value.rebate, exercise, value.s, value.q, value.r, today,
                                          value.v, value.fxr, value.fxv, value.corr, value.result,
                                          calculated, error, tolerance);
        }
    }
}

void QuantoOptionTest::testDoubleBarrierValues()  {

    BOOST_TEST_MESSAGE("Testing quanto-double-barrier option values...");

    SavedSettings backup;

    QuantoDoubleBarrierOptionData values[] = {
         // barrierType,           bar.lo, bar.hi, rebate,         type, spot,  strk,    q,   r,    T,  vol, fx rate, fx vol, corr, result, tol
        { DoubleBarrier::KnockOut,   50.0,  150.0,      0, Option::Call,  100, 100.0, 0.00, 0.1, 0.25, 0.15,    0.05,    0.2,  0.3,  3.4623, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,  110.0,      0, Option::Call,  100, 100.0, 0.00, 0.1, 0.50, 0.15,    0.05,    0.2,  0.3,  0.5236, 1.0e-4},
        { DoubleBarrier::KnockOut,   90.0,  110.0,      0, Option::Put,   100, 100.0, 0.00, 0.1, 0.25, 0.15,    0.05,    0.2,  0.3,  1.1320, 1.0e-4},
        { DoubleBarrier::KnockIn,    80.0,  120.0,      0, Option::Call,  100, 102.0, 0.00, 0.1, 0.25, 0.25,    0.05,    0.2,  0.3,  2.6313, 1.0e-4},
        { DoubleBarrier::KnockIn,    80.0,  120.0,      0, Option::Call,  100, 102.0, 0.00, 0.1, 0.50, 0.15,    0.05,    0.2,  0.3,  1.9305, 1.0e-4},
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    ext::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> fxrTS(flatRate(today, fxRate, dc));
    ext::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> fxVolTS(flatVol(today, fxVol, dc));
    ext::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
         new BlackScholesMertonProcess(Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

    ext::shared_ptr<PricingEngine> engine(
        new QuantoEngine<DoubleBarrierOption, AnalyticDoubleBarrierEngine>(
                                                 stochProcess, fxrTS, fxVolTS,
                                                 Handle<Quote>(correlation)));

    for (auto& value : values) {

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(value.type, value.strike));

        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        fxRate->setValue(value.fxr);
        fxVol->setValue(value.fxv);
        correlation->setValue(value.corr);

        QuantoDoubleBarrierOption option(value.barrierType, value.barrier_lo, value.barrier_hi,
                                         value.rebate, payoff, exercise);

        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated - value.result);
        Real tolerance = value.tol;

        if (error>tolerance) {
            QUANTO_DOUBLE_BARRIER_REPORT_FAILURE(
                "value", payoff, value.barrierType, value.barrier_lo, value.barrier_hi,
                value.rebate, exercise, value.s, value.q, value.r, today, value.v, value.fxr,
                value.fxv, value.corr, value.result, calculated, error, tolerance);
        }
    }
}

void QuantoOptionTest::testFDMQuantoHelper()  {

    BOOST_TEST_MESSAGE("Testing FDM quanto helper...");

    SavedSettings backup;

    const DayCounter dc = Actual360();
    const Date today = Date(22, April, 2019);

    const Real s = 100;
    const Rate domesticR = 0.1;
    const Rate foreignR = 0.2;
    const Rate q = 0.3;
    const Volatility vol = 0.3;
    const Volatility fxVol = 0.2;

    const Real exchRateATMlevel = 1.0;
    const Real equityFxCorrelation = -0.75;

    const Handle<YieldTermStructure> domesticTS(
        flatRate(today, domesticR, dc));

    const Handle<YieldTermStructure> divTS(
        flatRate(today, q, dc));

    const Handle<BlackVolTermStructure> volTS(
        flatVol(today, vol, dc));

    const Handle<Quote> spot(
        ext::make_shared<SimpleQuote>(s));

    const ext::shared_ptr<BlackScholesMertonProcess> bsmProcess
        = ext::make_shared<BlackScholesMertonProcess>(
            spot, divTS, domesticTS, volTS);

    const ext::shared_ptr<YieldTermStructure> foreignTS
        = flatRate(today, foreignR, dc);

    const ext::shared_ptr<BlackVolTermStructure> fxVolTS
        = flatVol(today, fxVol, dc);

    const ext::shared_ptr<FdmQuantoHelper> fdmQuantoHelper
        = ext::make_shared<FdmQuantoHelper>(
              domesticTS.currentLink(),
              foreignTS, fxVolTS,
              equityFxCorrelation, exchRateATMlevel);

    const Real calculatedQuantoAdj
        = fdmQuantoHelper->quantoAdjustment(vol, 0.0, 1.0);

    const Real expectedQuantoAdj
        = domesticR - foreignR + equityFxCorrelation*vol*fxVol;

    const Real tol = 1e-10;
    if (std::fabs(calculatedQuantoAdj - expectedQuantoAdj) > tol) {
        BOOST_ERROR("failed to reproduce quanto drift rate"
                    << std::setprecision(10)
                    << "\n    calculated: " << calculatedQuantoAdj
                    << "\n    expected:   " << expectedQuantoAdj);
    }

    const Date maturityDate = today + Period(6, Months);
    const Time maturityTime = dc.yearFraction(today, maturityDate);

    const Real eps = 0.0002;
    const Real scalingFactor = 1.25;

    const ext::shared_ptr<FdmBlackScholesMesher> mesher(
        new FdmBlackScholesMesher(
            3, bsmProcess, maturityTime, s,
            Null<Real>(), Null<Real>(), eps, scalingFactor,
            std::pair<Real, Real>(Null<Real>(), Null<Real>()),
            DividendSchedule(),
            fdmQuantoHelper));

    const Real normInvEps = InverseCumulativeNormal()(1-eps);
    const Real sigmaSqrtT = vol * std::sqrt(maturityTime);

    const Real qQuanto = q + expectedQuantoAdj;
    const Real expectedDriftRate = domesticR - qQuanto;

    const Real logFwd = std::log(s) + expectedDriftRate*maturityTime;
    const Real xMin = logFwd - sigmaSqrtT*normInvEps*scalingFactor;
    const Real xMax = std::log(s) + sigmaSqrtT*normInvEps*scalingFactor;

    const std::vector<Real> loc = mesher->locations();

    if (std::fabs(loc.front()-xMin) > tol || std::fabs(loc.back()-xMax) > tol) {
        BOOST_ERROR("failed to reproduce FDM grid boundaries"
                    << "\n    calculated: (" << std::setprecision(10)
                        << loc.front() << ", " << loc.back() << ")"
                    << "\n    expected:   (" << xMin << ", " << xMax << ")");
    }
}

void QuantoOptionTest::testPDEOptionValues()  {

    BOOST_TEST_MESSAGE("Testing quanto-option values with PDEs...");

    SavedSettings backup;

    const DayCounter dc = Actual360();
    const Date today = Date(21, April, 2019);

    QuantoOptionData values[] = {
        //    type,    strike,  spot,   div, domestic rate,  t,   vol, foreign rate, fx vol, correlation, result,     tol
        { Option::Call, 105.0, 100.0, 0.04,     0.08,      0.5,  0.2,     0.05,      0.10,     0.3,  Null<Real>(), Null<Real>() },
        { Option::Call, 100.0, 100.0, 0.16,     0.08,      0.25, 0.15,    0.05,      0.20,    -0.3,  Null<Real>(), Null<Real>() },
        { Option::Call, 105.0, 100.0, 0.04,     0.08,      0.5,  0.2,     0.05,      0.10,     0.3,  Null<Real>(), Null<Real>() },
        {  Option::Put, 105.0, 100.0, 0.04,     0.08,      0.5,  0.2,     0.05,      0.10,     0.3,  Null<Real>(), Null<Real>() },
        { Option::Call, 0.0,   100.0, 0.04,     0.08,      0.3,  0.3,     0.05,      0.10,     0.75, Null<Real>(), Null<Real>() },
    };

    for (auto& value : values) {

        std::map<std::string,Real> calculated, expected, tolerance;
        tolerance["npv"]   = 2e-4;
        tolerance["delta"] = 1e-4;
        tolerance["gamma"] = 1e-4;
        tolerance["theta"] = 1e-4;

        const Handle<Quote> spot(ext::make_shared<SimpleQuote>(value.s));

        const Real strike = value.strike;

        const Handle<YieldTermStructure> domesticTS(flatRate(today, value.r, dc));

        const Handle<YieldTermStructure> divTS(flatRate(today, value.q, dc));

        const Handle<BlackVolTermStructure> volTS(flatVol(today, value.v, dc));

        const ext::shared_ptr<BlackScholesMertonProcess> bsmProcess
            = ext::make_shared<BlackScholesMertonProcess>(
                spot, divTS, domesticTS, volTS);

        const Handle<YieldTermStructure> foreignTS(flatRate(today, value.fxr, dc));

        const Handle<BlackVolTermStructure> fxVolTS(flatVol(today, value.fxv, dc));

        const Real exchRateATMlevel = 1.0;
        const Real equityFxCorrelation = value.corr;

        const ext::shared_ptr<FdmQuantoHelper> quantoHelper
            = ext::make_shared<FdmQuantoHelper>(
                  domesticTS.currentLink(),
                  foreignTS.currentLink(),
                  fxVolTS.currentLink(),
                  equityFxCorrelation, exchRateATMlevel);

        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::make_shared<PlainVanillaPayoff>(value.type, strike);
        const Date exDate = today + timeToDays(value.t);
        const ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        VanillaOption option(payoff, exercise);

        const ext::shared_ptr<PricingEngine> pdeEngine =
            ext::make_shared<FdBlackScholesVanillaEngine>(bsmProcess, quantoHelper,
                                                          Size(value.t * 200), 500, 1);

        option.setPricingEngine(pdeEngine);

        calculated["npv"]   = option.NPV();
        calculated["delta"] = option.delta();
        calculated["gamma"] = option.delta();
        calculated["theta"] = option.delta();

        const ext::shared_ptr<PricingEngine> analyticEngine
            = ext::make_shared<QuantoEngine<
                VanillaOption, AnalyticEuropeanEngine> >(
                     bsmProcess, foreignTS, fxVolTS,
                     Handle<Quote>(
                         ext::make_shared<SimpleQuote>(equityFxCorrelation)));

        option.setPricingEngine(analyticEngine);

        expected["npv"]   = option.NPV();
        expected["delta"] = option.delta();
        expected["gamma"] = option.delta();
        expected["theta"] = option.delta();

        for (std::map<std::string,Real>::const_iterator it = calculated.begin();
             it != calculated.end(); ++it) {

            const std::string greek = it->first;

            const Real expct = expected[greek];
            const Real calcl = calculated[greek];
            const Real error = std::fabs(expct - calcl);
            const Real tol = tolerance[greek];

            if (error > tol) {
                QUANTO_REPORT_FAILURE(greek, payoff, exercise, value.s, value.q, value.r, today,
                                      value.v, value.fxr, value.fxv, value.corr, expct, calcl,
                                      error, tol)
            }
        }
    }
}

void QuantoOptionTest::testAmericanQuantoOption()  {

    BOOST_TEST_MESSAGE("Testing American quanto-option values with PDEs...");

    SavedSettings backup;

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(21, April, 2019);
    const Date maturity = today + Period(9, Months);

    const Real s = 100;
    const Rate domesticR = 0.025;
    const Rate foreignR  = 0.075;
    const Rate q = 0.03;
    const Volatility vol = 0.3;
    const Volatility fxVol = 0.15;

    const Real exchRateATMlevel    =  1.0;
    const Real equityFxCorrelation = -0.75;

    const Handle<YieldTermStructure> domesticTS(
        flatRate(today, domesticR, dc));

    const Handle<YieldTermStructure> divTS(
        flatRate(today, q, dc));

    const Handle<BlackVolTermStructure> volTS(
        flatVol(today, vol, dc));

    const Handle<Quote> spot(
        ext::make_shared<SimpleQuote>(s));

    const ext::shared_ptr<BlackScholesMertonProcess> bsmProcess
        = ext::make_shared<BlackScholesMertonProcess>(
            spot, divTS, domesticTS, volTS);

    const ext::shared_ptr<YieldTermStructure> foreignTS
        = flatRate(today, foreignR, dc);

    const ext::shared_ptr<BlackVolTermStructure> fxVolTS
        = flatVol(today, fxVol, dc);

    const ext::shared_ptr<FdmQuantoHelper> quantoHelper
        = ext::make_shared<FdmQuantoHelper>(
              domesticTS.currentLink(),
              foreignTS,
              fxVolTS,
              equityFxCorrelation, exchRateATMlevel);

    const Real strike = 105.0;

    std::vector<Date> dividendDates = { today + Period(6, Months) };
    std::vector<Real> dividendAmounts = { 8.0 };
    auto dividends = DividendVector(dividendDates, dividendAmounts);

    VanillaOption option(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
        ext::make_shared<AmericanExercise>(maturity));

    option.setPricingEngine(
        ext::make_shared<FdBlackScholesVanillaEngine>(
            bsmProcess, dividends, quantoHelper, 100, 400, 1));

    const Real tol = 1e-4;
    const Real expected = 8.90611734;
    const Real bsCalculated = option.NPV();

    if (std::fabs(expected - bsCalculated) > tol) {
        BOOST_ERROR("failed to reproduce American quanto option prices "
                    "with the Black-Scholes-Merton model"
                    << "\n    calculated: " << bsCalculated
                    << "\n    expected:   " << expected);
    }

    option.setPricingEngine(
        ext::make_shared<FdBlackScholesVanillaEngine>(
            bsmProcess, dividends, quantoHelper, 100, 400, 1));

    const Real localVolCalculated = option.NPV();
    if (std::fabs(expected - localVolCalculated) > tol) {
        BOOST_ERROR("failed to reproduce American quanto option prices "
                    "with the Local Volatility model"
                    << "\n    calculated: " << localVolCalculated
                    << "\n    expected:   " << expected);
    }

    const Real tolBetweenBSandLocalVol = 1e-6;
    if (std::fabs(bsCalculated - localVolCalculated) > tolBetweenBSandLocalVol) {
        BOOST_ERROR("difference between American quanto option prices "
                    "for Local Volatility and Black-Scholes model"
                    << "\n    calculated Local Vol    : " << localVolCalculated
                    << "\n    calculated Black-Scholes: " << bsCalculated);
    }

    VanillaOption divOption(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
        ext::make_shared<AmericanExercise>(maturity));

    const Real v0    = vol*vol;
    const Real kappa = 1.0;
    const Real theta = v0;
    const Real sigma = 1e-4;
    const Real rho   = 0.0;

    const ext::shared_ptr<HestonModel> hestonModel =
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                domesticTS, divTS, spot, v0, kappa, theta, sigma, rho));

    divOption.setPricingEngine(
        ext::make_shared<FdHestonVanillaEngine>(
            hestonModel, dividends, quantoHelper, 100, 400, 3, 1));

    const Real hestonCalculated = divOption.NPV();

    if (std::fabs(expected - hestonCalculated) > tol) {
        BOOST_ERROR("failed to reproduce American quanto option prices "
                    "with the Heston model"
                    << "\n    calculated: " << hestonCalculated
                    << "\n    expected:   " << expected);
    }

    const ext::shared_ptr<LocalVolTermStructure> localConstVol =
        ext::make_shared<LocalConstantVol>(today, 2.0, dc);

    const ext::shared_ptr<HestonModel> hestonModel05 =
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                domesticTS, divTS, spot, 0.25*v0, kappa, 0.25*theta, sigma, rho));

    divOption.setPricingEngine(
        ext::make_shared<FdHestonVanillaEngine>(
            hestonModel05, dividends, quantoHelper, 100, 400, 3, 1,
            FdmSchemeDesc::Hundsdorfer(), localConstVol));

    const Real hestoSlvCalculated = divOption.NPV();

    if (std::fabs(expected - hestoSlvCalculated) > tol) {
        BOOST_ERROR("failed to reproduce American quanto option prices "
                    "with the Heston Local Volatility model"
                    << "\n    calculated: " << hestoSlvCalculated
                    << "\n    expected:   " << expected);
    }
}

test_suite* QuantoOptionTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Quanto option tests");
    suite->add(QUANTLIB_TEST_CASE(&QuantoOptionTest::testValues));
    suite->add(QUANTLIB_TEST_CASE(&QuantoOptionTest::testGreeks));
    suite->add(QUANTLIB_TEST_CASE(&QuantoOptionTest::testForwardValues));
    suite->add(QUANTLIB_TEST_CASE(&QuantoOptionTest::testForwardGreeks));
    suite->add(QUANTLIB_TEST_CASE(
                            &QuantoOptionTest::testForwardPerformanceValues));
    suite->add(QUANTLIB_TEST_CASE(&QuantoOptionTest::testBarrierValues));
    suite->add(QUANTLIB_TEST_CASE(&QuantoOptionTest::testFDMQuantoHelper));
    suite->add(QUANTLIB_TEST_CASE(&QuantoOptionTest::testPDEOptionValues));

    suite->add(QUANTLIB_TEST_CASE(&QuantoOptionTest::testAmericanQuantoOption));
    return suite;
}

test_suite* QuantoOptionTest::experimental() {
    auto* suite = BOOST_TEST_SUITE("Experimental quanto option tests");
    suite->add(QUANTLIB_TEST_CASE(&QuantoOptionTest::testDoubleBarrierValues));
    return suite;
}

