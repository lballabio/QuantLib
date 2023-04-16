/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2007, 2008 StatPro Italia srl

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

#include "cliquetoption.hpp"
#include "utilities.hpp"
#include <ql/instruments/cliquetoption.hpp>
#include <ql/pricingengines/cliquet/analyticcliquetengine.hpp>
#include <ql/pricingengines/cliquet/analyticperformanceengine.hpp>
#include <ql/pricingengines/cliquet/mcperformanceengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/period.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#undef REPORT_FAILURE
#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, v, \
                       expected, calculated, error, tolerance) \
    BOOST_ERROR(payoff->optionType() << " option:\n" \
               << "    spot value:       " << s << "\n" \
               << "    moneyness:        " << payoff->strike() << "\n" \
               << "    dividend yield:   " << io::rate(q) << "\n" \
               << "    risk-free rate:   " << io::rate(r) << "\n" \
               << "    reference date:   " << today << "\n" \
               << "    maturity:         " << exercise->lastDate() << "\n" \
               << "    volatility:       " << io::volatility(v) << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);


void CliquetOptionTest::testValues() {

    BOOST_TEST_MESSAGE("Testing Cliquet option values...");

    Date today = Date::todaysDate();
    DayCounter dc = Actual360();

    std::shared_ptr<SimpleQuote> spot(new SimpleQuote(60.0));
    std::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    std::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    std::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.08));
    std::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    std::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.30));
    std::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    std::shared_ptr<BlackScholesMertonProcess> process(
         new BlackScholesMertonProcess(Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));
    std::shared_ptr<PricingEngine> engine(new AnalyticCliquetEngine(process));

    std::vector<Date> reset;
    reset.push_back(today + 90);
    Date maturity = today + 360;
    Option::Type type = Option::Call;
    Real moneyness = 1.1;

    std::shared_ptr<PercentageStrikePayoff> payoff(
                                 new PercentageStrikePayoff(type, moneyness));
    std::shared_ptr<EuropeanExercise> exercise(
                                              new EuropeanExercise(maturity));

    CliquetOption option(payoff, exercise, reset);
    option.setPricingEngine(engine);

    Real calculated = option.NPV();
    Real expected = 4.4064; // Haug, p.37
    Real error = std::fabs(calculated-expected);
    Real tolerance = 1e-4;
    if (error > tolerance) {
        REPORT_FAILURE("value", payoff, exercise, spot->value(),
                       qRate->value(), rRate->value(), today,
                       vol->value(), expected, calculated,
                       error, tolerance);
    }
}


namespace {

    template <class T>
    void testOptionGreeks() {

        SavedSettings backup;

        std::map<std::string,Real> calculated, expected, tolerance;
        tolerance["delta"]  = 1.0e-5;
        tolerance["gamma"]  = 1.0e-5;
        tolerance["theta"]  = 1.0e-5;
        tolerance["rho"]    = 1.0e-5;
        tolerance["divRho"] = 1.0e-5;
        tolerance["vega"]   = 1.0e-5;

        Option::Type types[] = { Option::Call, Option::Put };
        Real moneyness[] = { 0.9, 1.0, 1.1 };
        Real underlyings[] = { 100.0 };
        Rate qRates[] = { 0.04, 0.05, 0.06 };
        Rate rRates[] = { 0.01, 0.05, 0.15 };
        Integer lengths[] = { 1, 2 };
        Frequency frequencies[] = { Semiannual, Quarterly };
        Volatility vols[] = { 0.11, 0.50, 1.20 };

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

        std::shared_ptr<BlackScholesMertonProcess> process(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

        for (auto& type : types) {
            for (Real moneynes : moneyness) {
                for (int length : lengths) {
                    for (auto& frequencie : frequencies) {

                        std::shared_ptr<EuropeanExercise> maturity(
                            new EuropeanExercise(today + length * Years));

                        std::shared_ptr<PercentageStrikePayoff> payoff(
                            new PercentageStrikePayoff(type, moneynes));

                        std::vector<Date> reset;
                        for (Date d = today + Period(frequencie); d < maturity->lastDate();
                             d += Period(frequencie))
                            reset.push_back(d);

                        std::shared_ptr<PricingEngine> engine(new T(process));

                        CliquetOption option(payoff, maturity, reset);
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
                                            for (it = calculated.begin(); it != calculated.end();
                                                 ++it) {
                                                std::string greek = it->first;
                                                Real expct = expected[greek],
                                                     calcl = calculated[greek],
                                                     tol = tolerance[greek];
                                                Real error = relativeError(expct, calcl, u);
                                                if (error > tol) {
                                                    REPORT_FAILURE(greek, payoff, maturity, u, q, r,
                                                                   today, v, expct, calcl, error,
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


void CliquetOptionTest::testGreeks() {
    BOOST_TEST_MESSAGE("Testing Cliquet option greeks...");
    testOptionGreeks<AnalyticCliquetEngine>();
}


void CliquetOptionTest::testPerformanceGreeks() {
    BOOST_TEST_MESSAGE("Testing performance option greeks...");
    testOptionGreeks<AnalyticPerformanceEngine>();
}


void CliquetOptionTest::testMcPerformance() {
    BOOST_TEST_MESSAGE(
        "Testing Monte Carlo performance engine against analytic results...");

    SavedSettings backup;

    Option::Type types[] = { Option::Call, Option::Put };
    Real moneyness[] = { 0.9, 1.1 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.04, 0.06 };
    Rate rRates[] = { 0.01, 0.10 };
    Integer lengths[] = { 2, 4 };
    Frequency frequencies[] = { Semiannual, Quarterly };
    Volatility vols[] = { 0.10, 0.90 };

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

    std::shared_ptr<BlackScholesMertonProcess> process(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

    for (auto& type : types) {
        for (Real moneynes : moneyness) {
            for (int length : lengths) {
                for (auto& frequencie : frequencies) {

                    Period tenor = Period(frequencie);
                    std::shared_ptr<EuropeanExercise> maturity(
                        new EuropeanExercise(today + length * tenor));

                    std::shared_ptr<PercentageStrikePayoff> payoff(
                        new PercentageStrikePayoff(type, moneynes));

                    std::vector<Date> reset;
                    for (Date d = today + tenor; d < maturity->lastDate(); d += tenor)
                        reset.push_back(d);

                    CliquetOption option(payoff, maturity, reset);

                    std::shared_ptr<PricingEngine> refEngine(
                        new AnalyticPerformanceEngine(process));

                    std::shared_ptr<PricingEngine> mcEngine =
                        MakeMCPerformanceEngine<PseudoRandom>(process)
                            .withBrownianBridge()
                            .withAbsoluteTolerance(5.0e-3)
                            .withSeed(42);

                    for (Real u : underlyings) {
                        for (Real m : qRates) {
                            for (Real n : rRates) {
                                for (Real v : vols) {

                                    Rate q = m, r = n;
                                    spot->setValue(u);
                                    qRate->setValue(q);
                                    rRate->setValue(r);
                                    vol->setValue(v);

                                    option.setPricingEngine(refEngine);
                                    Real refValue = option.NPV();

                                    option.setPricingEngine(mcEngine);
                                    Real value = option.NPV();

                                    Real error = std::fabs(refValue - value);
                                    Real tolerance = 1.5e-2;
                                    if (error > tolerance) {
                                        REPORT_FAILURE("value", payoff, maturity, u, q, r, today, v,
                                                       refValue, value, error, tolerance);
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


test_suite* CliquetOptionTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Cliquet option tests");
    suite->add(QUANTLIB_TEST_CASE(&CliquetOptionTest::testValues));
    suite->add(QUANTLIB_TEST_CASE(&CliquetOptionTest::testGreeks));
    suite->add(QUANTLIB_TEST_CASE(&CliquetOptionTest::testPerformanceGreeks));
    suite->add(QUANTLIB_TEST_CASE(&CliquetOptionTest::testMcPerformance));
    return suite;
}

