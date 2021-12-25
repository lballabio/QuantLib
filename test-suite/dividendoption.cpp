/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2007 StatPro Italia srl

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

// TODO: Figure out why tests for options with both continuous and discrete
// dividends fail.

#include "dividendoption.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/instruments/dividendvanillaoption.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/vanilla/fddividendshoutengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/analyticdividendeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#undef REPORT_FAILURE
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

// tests

void DividendOptionTest::testEuropeanValues() {

    BOOST_TEST_MESSAGE(
              "Testing dividend European option values with no dividends...");

    SavedSettings backup;

    Real tolerance = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.00, 0.10, 0.30 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 1, 2 };
    Volatility vols[] = { 0.05, 0.20, 0.70 };

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

    for (auto& type : types) {
        for (double strike : strikes) {
            for (int length : lengths) {
                Date exDate = today + length * Years;
                ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

                std::vector<Date> dividendDates;
                std::vector<Real> dividends;
                for (Date d = today + 3 * Months; d < exercise->lastDate(); d += 6 * Months) {
                    dividendDates.push_back(d);
                    dividends.push_back(0.0);
                }

                ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));

                ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                    new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

                ext::shared_ptr<PricingEngine> ref_engine(new AnalyticEuropeanEngine(stochProcess));

                ext::shared_ptr<PricingEngine> engine(
                    new AnalyticDividendEuropeanEngine(stochProcess));

                DividendVanillaOption option(payoff, exercise, dividendDates, dividends);
                option.setPricingEngine(engine);

                VanillaOption ref_option(payoff, exercise);
                ref_option.setPricingEngine(ref_engine);

                for (double u : underlyings) {
                    for (double m : qRates) {
                        for (double n : rRates) {
                            for (double v : vols) {
                                Rate q = m, r = n;
                                spot->setValue(u);
                                qRate->setValue(q);
                                rRate->setValue(r);
                                vol->setValue(v);

                                Real calculated = option.NPV();
                                Real expected = ref_option.NPV();
                                Real error = std::fabs(calculated - expected);
                                if (error > tolerance) {
                                    REPORT_FAILURE("value start limit", payoff, exercise, u, q, r,
                                                   today, v, expected, calculated, error,
                                                   tolerance);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// Reference pg. 253 - Hull - Options, Futures, and Other Derivatives 5th ed
// Exercise 12.8

void DividendOptionTest::testEuropeanKnownValue() {

    BOOST_TEST_MESSAGE(
              "Testing dividend European option values with known value...");

    SavedSettings backup;

    Real tolerance = 1.0e-2;
    Real expected = 3.67;

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

    Date exDate = today + 180 * Days;
    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

    std::vector<Date> dividendDates = {today + 2 * 30 * Days, today + 5 * 30 * Days};
    std::vector<Real> dividends = {0.50, 0.50};

    ext::shared_ptr<StrikedTypePayoff> payoff(
            new PlainVanillaPayoff(Option::Call, 40.0));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

    ext::shared_ptr<PricingEngine> engine(
                            new AnalyticDividendEuropeanEngine(stochProcess));

    DividendVanillaOption option(payoff, exercise,
                                 dividendDates, dividends);
    option.setPricingEngine(engine);

    Real u = 40.0;
    Rate q = 0.0, r = 0.09;
    Volatility v = 0.30;
    spot->setValue(u);
    qRate->setValue(q);
    rRate->setValue(r);
    vol->setValue(v);

    Real calculated = option.NPV();
    Real error = std::fabs(calculated-expected);
    if (error > tolerance) {
        REPORT_FAILURE("value start limit",
                       payoff, exercise,
                       u, q, r, today, v,
                       expected, calculated,
                       error, tolerance);
    }
}


void DividendOptionTest::testEuropeanStartLimit() {

    BOOST_TEST_MESSAGE(
       "Testing dividend European option with a dividend on today's date...");

    SavedSettings backup;

    Real tolerance = 1.0e-5;
    Real dividendValue = 10.0;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.00, 0.10, 0.30 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 1, 2 };
    Volatility vols[] = { 0.05, 0.20, 0.70 };

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

    for (auto& type : types) {
        for (double strike : strikes) {
            for (int length : lengths) {
                Date exDate = today + length * Years;
                ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

                std::vector<Date> dividendDates = {today};
                std::vector<Real> dividends = {dividendValue};

                ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));

                ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                    new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

                ext::shared_ptr<PricingEngine> engine(
                    new AnalyticDividendEuropeanEngine(stochProcess));

                ext::shared_ptr<PricingEngine> ref_engine(new AnalyticEuropeanEngine(stochProcess));

                DividendVanillaOption option(payoff, exercise, dividendDates, dividends);
                option.setPricingEngine(engine);

                VanillaOption ref_option(payoff, exercise);
                ref_option.setPricingEngine(ref_engine);

                for (double u : underlyings) {
                    for (double m : qRates) {
                        for (double n : rRates) {
                            for (double v : vols) {
                                Rate q = m, r = n;
                                spot->setValue(u);
                                qRate->setValue(q);
                                rRate->setValue(r);
                                vol->setValue(v);

                                Real calculated = option.NPV();
                                spot->setValue(u - dividendValue);
                                Real expected = ref_option.NPV();
                                Real error = std::fabs(calculated - expected);
                                if (error > tolerance) {
                                    REPORT_FAILURE("value", payoff, exercise, u, q, r, today, v,
                                                   expected, calculated, error, tolerance);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void DividendOptionTest::testEuropeanEndLimit() {

    BOOST_TEST_MESSAGE(
              "Testing dividend European option values with end limits...");

    SavedSettings backup;

    Real tolerance = 1.0e-5;
    Real dividendValue = 10.0;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.00, 0.10, 0.30 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 1, 2 };
    Volatility vols[] = { 0.05, 0.20, 0.70 };

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

    for (auto& type : types) {
        for (double strike : strikes) {
            for (int length : lengths) {
                Date exDate = today + length * Years;
                ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

                std::vector<Date> dividendDates = {exercise->lastDate()};
                std::vector<Real> dividends = {dividendValue};

                ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));


                ext::shared_ptr<StrikedTypePayoff> refPayoff(
                    new PlainVanillaPayoff(type, strike + dividendValue));

                ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                    new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

                ext::shared_ptr<PricingEngine> engine(
                    new AnalyticDividendEuropeanEngine(stochProcess));

                ext::shared_ptr<PricingEngine> ref_engine(new AnalyticEuropeanEngine(stochProcess));

                DividendVanillaOption option(payoff, exercise, dividendDates, dividends);
                option.setPricingEngine(engine);

                VanillaOption ref_option(refPayoff, exercise);
                ref_option.setPricingEngine(ref_engine);

                for (double u : underlyings) {
                    for (double m : qRates) {
                        for (double n : rRates) {
                            for (double v : vols) {
                                Rate q = m, r = n;
                                spot->setValue(u);
                                qRate->setValue(q);
                                rRate->setValue(r);
                                vol->setValue(v);

                                Real calculated = option.NPV();
                                Real expected = ref_option.NPV();
                                Real error = std::fabs(calculated - expected);
                                if (error > tolerance) {
                                    REPORT_FAILURE("value", payoff, exercise, u, q, r, today, v,
                                                   expected, calculated, error, tolerance);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


void DividendOptionTest::testEuropeanGreeks() {

    BOOST_TEST_MESSAGE("Testing dividend European option greeks...");

    SavedSettings backup;

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"] = 1.0e-5;
    tolerance["gamma"] = 1.0e-5;
    tolerance["theta"] = 1.0e-5;
    tolerance["rho"]   = 1.0e-5;
    tolerance["vega"]  = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.00, 0.10, 0.30 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 1, 2 };
    Volatility vols[] = { 0.05, 0.20, 0.40 };

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

    for (auto& type : types) {
        for (double strike : strikes) {
            for (int length : lengths) {
                Date exDate = today + length * Years;
                ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

                std::vector<Date> dividendDates;
                std::vector<Real> dividends;
                for (Date d = today + 3 * Months; d < exercise->lastDate(); d += 6 * Months) {
                    dividendDates.push_back(d);
                    dividends.push_back(5.0);
                }

                ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));

                ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                    new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

                ext::shared_ptr<PricingEngine> engine(
                    new AnalyticDividendEuropeanEngine(stochProcess));

                DividendVanillaOption option(payoff, exercise, dividendDates, dividends);
                option.setPricingEngine(engine);

                for (double u : underlyings) {
                    for (double m : qRates) {
                        for (double n : rRates) {
                            for (double v : vols) {
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

                                    // perturb risk-free rate and get rho
                                    Spread dr = r * 1.0e-4;
                                    rRate->setValue(r + dr);
                                    value_p = option.NPV();
                                    rRate->setValue(r - dr);
                                    value_m = option.NPV();
                                    rRate->setValue(r);
                                    expected["rho"] = (value_p - value_m) / (2 * dr);

                                    // perturb volatility and get vega
                                    Spread dv = v * 1.0e-4;
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
                                        Real expct = expected[greek], calcl = calculated[greek],
                                             tol = tolerance[greek];
                                        Real error = relativeError(expct, calcl, u);
                                        if (error > tol) {
                                            REPORT_FAILURE(greek, payoff, exercise, u, q, r, today,
                                                           v, expct, calcl, error, tol);
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


void DividendOptionTest::testFdEuropeanValues() {

    BOOST_TEST_MESSAGE(
              "Testing finite-difference dividend European option values...");

    SavedSettings backup;

    Real tolerance = 1.0e-2;
    Size gridPoints = 400;
    Size timeSteps = 40;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.00, 0.10, 0.30 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 1, 2 };
    Volatility vols[] = { 0.05, 0.20, 0.40 };

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

    for (auto& type : types) {
        for (double strike : strikes) {
            for (int length : lengths) {
                Date exDate = today + length * Years;
                ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

                std::vector<Date> dividendDates;
                std::vector<Real> dividends;
                for (Date d = today + 3 * Months; d < exercise->lastDate(); d += 6 * Months) {
                    dividendDates.push_back(d);
                    dividends.push_back(5.0);
                }

                ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));

                ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                    new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

                ext::shared_ptr<PricingEngine> engine =
                    MakeFdBlackScholesVanillaEngine(stochProcess)
                        .withTGrid(timeSteps)
                        .withXGrid(gridPoints)
                        .withCashDividendModel(FdBlackScholesVanillaEngine::Escrowed);

                ext::shared_ptr<PricingEngine> ref_engine(
                    new AnalyticDividendEuropeanEngine(stochProcess));

                DividendVanillaOption option(payoff, exercise, dividendDates, dividends);
                option.setPricingEngine(engine);

                DividendVanillaOption ref_option(payoff, exercise, dividendDates, dividends);
                ref_option.setPricingEngine(ref_engine);

                for (double u : underlyings) {
                    for (double m : qRates) {
                        for (double n : rRates) {
                            for (double v : vols) {
                                Rate q = m, r = n;
                                spot->setValue(u);
                                qRate->setValue(q);
                                rRate->setValue(r);
                                vol->setValue(v);
                                // FLOATING_POINT_EXCEPTION
                                Real calculated = option.NPV();
                                if (calculated > spot->value() * 1.0e-5) {
                                    Real expected = ref_option.NPV();
                                    Real error = std::fabs(calculated - expected);
                                    if (error > tolerance) {
                                        REPORT_FAILURE("value", payoff, exercise, u, q, r, today, v,
                                                       expected, calculated, error, tolerance);
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


namespace {

    void testFdGreeks(const Date& today,
                      const ext::shared_ptr<Exercise>& exercise,
                      FdBlackScholesVanillaEngine::CashDividendModel model) {

        std::map<std::string,Real> calculated, expected, tolerance;
        tolerance["delta"] = 5.0e-3;
        tolerance["gamma"] = 7.0e-3;
        // tolerance["theta"] = 1.0e-2;

        Option::Type types[] = { Option::Call, Option::Put };
        Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
        Real underlyings[] = { 100.0 };
        Rate qRates[] = { 0.00, 0.10, 0.20 };
        Rate rRates[] = { 0.01, 0.05, 0.15 };
        Volatility vols[] = { 0.05, 0.20, 0.50 };

        DayCounter dc = Actual365Fixed();

        ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
        ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
        Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
        ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
        Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
        ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
        Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

        for (auto& type : types) {
            for (double strike : strikes) {

                std::vector<Date> dividendDates;
                std::vector<Real> dividends;
                for (Date d = today + 3*Months;
                     d < exercise->lastDate();
                     d += 6*Months) {
                    dividendDates.push_back(d);
                    dividends.push_back(5.0);
                }

                ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));

                ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

                ext::shared_ptr<PricingEngine> engine =
                    MakeFdBlackScholesVanillaEngine(stochProcess)
                    .withCashDividendModel(model);

                DividendVanillaOption option(payoff, exercise,
                                             dividendDates, dividends);
                option.setPricingEngine(engine);

                for (double u : underlyings) {
                    for (double m : qRates) {
                        for (double n : rRates) {
                            for (double v : vols) {
                                Rate q = m, r = n;
                                spot->setValue(u);
                                qRate->setValue(q);
                                rRate->setValue(r);
                                vol->setValue(v);

                                // FLOATING_POINT_EXCEPTION
                                Real value = option.NPV();
                                calculated["delta"] = option.delta();
                                calculated["gamma"] = option.gamma();

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

                                    // perturb date and get theta
                                    /*
                                      Time dT = dc.yearFraction(today-1, today+1);
                                      Settings::instance().evaluationDate() = today-1;
                                      value_m = option.NPV();
                                      Settings::instance().evaluationDate() = today+1;
                                      value_p = option.NPV();
                                      Settings::instance().evaluationDate() = today;
                                      expected["theta"] = (value_p - value_m)/dT;
                                    */

                                    // compare
                                    std::map<std::string, Real>::iterator it;
                                    for (it = calculated.begin(); it != calculated.end(); ++it) {
                                        std::string greek = it->first;
                                        Real expct = expected[greek], calcl = calculated[greek],
                                             tol = tolerance[greek];
                                        Real error = relativeError(expct, calcl, u);
                                        if (error > tol) {
                                            REPORT_FAILURE(greek, payoff, exercise, u, q, r, today,
                                                           v, expct, calcl, error, tol);
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


void DividendOptionTest::testFdEuropeanGreeks() {

    BOOST_TEST_MESSAGE(
             "Testing finite-differences dividend European option greeks...");

    SavedSettings backup;

    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;
    Integer lengths[] = { 1, 2 };

    for (int length : lengths) {
        Date exDate = today + length * Years;
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
        testFdGreeks(today,exercise,FdBlackScholesVanillaEngine::Spot);
        testFdGreeks(today,exercise,FdBlackScholesVanillaEngine::Escrowed);
    }
}

void DividendOptionTest::testFdAmericanGreeks() {
    BOOST_TEST_MESSAGE(
             "Testing finite-differences dividend American option greeks...");

    SavedSettings backup;

    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;
    Integer lengths[] = { 1, 2 };

    for (int length : lengths) {
        Date exDate = today + length * Years;
        ext::shared_ptr<Exercise> exercise(new AmericanExercise(today,exDate));
        testFdGreeks(today,exercise,FdBlackScholesVanillaEngine::Spot);
    }
}


namespace {

    void testFdDegenerate(const Date& today,
                          const ext::shared_ptr<Exercise>& exercise,
                          FdBlackScholesVanillaEngine::CashDividendModel model) {

        DayCounter dc = Actual360();
        ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(54.625));
        Handle<YieldTermStructure> rTS(flatRate(0.052706, dc));
        Handle<YieldTermStructure> qTS(flatRate(0.0, dc));
        Handle<BlackVolTermStructure> volTS(flatVol(0.282922, dc));

        ext::shared_ptr<BlackScholesMertonProcess> process(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

        Size timeSteps = 100;
        Size gridPoints = 300;

        ext::shared_ptr<PricingEngine> engine =
              MakeFdBlackScholesVanillaEngine(process)
              .withTGrid(timeSteps)
              .withXGrid(gridPoints)
              .withCashDividendModel(model);

        ext::shared_ptr<StrikedTypePayoff> payoff(
                                  new PlainVanillaPayoff(Option::Call, 55.0));

        Real tolerance = 3.0e-3;

        std::vector<Rate> dividends;
        std::vector<Date> dividendDates;

        DividendVanillaOption option1(payoff, exercise,
                                      dividendDates, dividends);
        option1.setPricingEngine(engine);

        Real refValue = option1.NPV();

        for (Size i=1; i<=6; i++) {

            dividends.push_back(0.0);
            dividendDates.push_back(today+i);

            DividendVanillaOption option(payoff, exercise,
                                         dividendDates, dividends);
            option.setPricingEngine(engine);
            Real value = option.NPV();

            if (std::fabs(refValue-value) > tolerance)
                BOOST_FAIL("NPV changed by null dividend :\n"
                           << "    previous value: " << value << "\n"
                           << "    current value:  " << refValue << "\n"
                           << "    change:         " << value-refValue);
        }
    }

}


void DividendOptionTest::testFdEuropeanDegenerate() {

    BOOST_TEST_MESSAGE(
         "Testing degenerate finite-differences dividend European option...");

    SavedSettings backup;

    Date today = Date(27,February,2005);
    Settings::instance().evaluationDate() = today;
    Date exDate(13,April,2005);

    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

    testFdDegenerate(today,exercise,FdBlackScholesVanillaEngine::Spot);
    testFdDegenerate(today,exercise,FdBlackScholesVanillaEngine::Escrowed);
}

void DividendOptionTest::testFdAmericanDegenerate() {

    BOOST_TEST_MESSAGE(
         "Testing degenerate finite-differences dividend American option...");

    SavedSettings backup;

    Date today = Date(27,February,2005);
    Settings::instance().evaluationDate() = today;
    Date exDate(13,April,2005);

    ext::shared_ptr<Exercise> exercise(new AmericanExercise(today,exDate));

    testFdDegenerate(today,exercise,FdBlackScholesVanillaEngine::Spot);
    testFdDegenerate(today,exercise,FdBlackScholesVanillaEngine::Escrowed);
}


namespace {

    void testFdDividendAtTZero(const Date& today,
                               const ext::shared_ptr<Exercise>& exercise,
                               FdBlackScholesVanillaEngine::CashDividendModel model) {

        DayCounter dc = Actual360();
        ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(54.625));
        Handle<YieldTermStructure> rTS(flatRate(0.0, dc));
        Handle<BlackVolTermStructure> volTS(flatVol(0.282922, dc));

        ext::shared_ptr<BlackScholesMertonProcess> process(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          rTS, rTS, volTS));

        Size timeSteps = 50;
        Size gridPoints = 400;

        ext::shared_ptr<PricingEngine> engine =
              MakeFdBlackScholesVanillaEngine(process)
              .withTGrid(timeSteps)
              .withXGrid(gridPoints)
              .withCashDividendModel(model);

        ext::shared_ptr<StrikedTypePayoff> payoff(
                                  new PlainVanillaPayoff(Option::Call, 55.0));

        // today's dividend must by taken into account
        std::vector<Rate> dividends(1, 1.0);
        std::vector<Date> dividendDates(1, today);

        DividendVanillaOption option(payoff, exercise,
                                     dividendDates, dividends);
        option.setPricingEngine(engine);
        Real calculated = option.NPV();

        switch(model) {
          case FdBlackScholesVanillaEngine::Spot:
            BOOST_CHECK_THROW(option.theta(), QuantLib::Error);
            break;
          case FdBlackScholesVanillaEngine::Escrowed:
            BOOST_CHECK_NO_THROW(option.theta());
            break;
          default:
            QL_FAIL("unknown dividend model type");
        }

        ext::shared_ptr<Exercise> europeanExercise =
            ext::make_shared<EuropeanExercise>(exercise->lastDate());
        DividendVanillaOption europeanOption(
            payoff, europeanExercise, dividendDates, dividends);

        europeanOption.setPricingEngine(
            ext::make_shared<AnalyticDividendEuropeanEngine>(process));

        Real expected = europeanOption.NPV();

        const Real tol = 1e-4;

        if (std::fabs(calculated-expected) > tol) {
            BOOST_ERROR("Can not reproduce reference values "
                        "from analytic dividend engine :\n"
                       << "    calculated: " << calculated << "\n"
                       << "    expected  : " << expected << "\n"
                       << "    diff:       " << tol);
        }
    }
}


void DividendOptionTest::testFdEuropeanWithDividendToday() {

    BOOST_TEST_MESSAGE(
         "Testing finite-differences dividend European option with dividend on today's date...");

    SavedSettings backup;

    Date today = Date(27,February,2005);
    Settings::instance().evaluationDate() = today;
    Date exDate(13,April,2005);

    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

    testFdDividendAtTZero(today,exercise,FdBlackScholesVanillaEngine::Spot);
    testFdDividendAtTZero(today,exercise,FdBlackScholesVanillaEngine::Escrowed);
}

void DividendOptionTest::testFdAmericanWithDividendToday() {

    BOOST_TEST_MESSAGE(
         "Testing finite-differences dividend American option with dividend on today's date...");

    SavedSettings backup;

    Date today = Date(27,February,2005);
    Settings::instance().evaluationDate() = today;
    Date exDate(13,April,2005);

    ext::shared_ptr<Exercise> exercise(new AmericanExercise(today,exDate));

    testFdDividendAtTZero(today,exercise,FdBlackScholesVanillaEngine::Spot);
}


void DividendOptionTest::testEscrowedDividendModel() {
    BOOST_TEST_MESSAGE("Testing finite-difference European engine "
                       "with the escrowed dividend model...");

    SavedSettings backup;

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(12, October, 2019);

    Settings::instance().evaluationDate() = today;

    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(100.0));
    const Handle<YieldTermStructure> qTS(flatRate(today, 0.063, dc));
    const Handle<YieldTermStructure> rTS(flatRate(today, 0.094, dc));
    const Handle<BlackVolTermStructure> volTS(flatVol(today, 0.3, dc));

    const Date maturity = today + Period(1, Years);

    const ext::shared_ptr<BlackScholesMertonProcess> process =
        ext::make_shared<BlackScholesMertonProcess>(
            spot, qTS, rTS, volTS);

    const ext::shared_ptr<PlainVanillaPayoff> payoff(
        ext::make_shared<PlainVanillaPayoff>(Option::Put, spot->value()));

    const ext::shared_ptr<Exercise> exercise(
        ext::make_shared<EuropeanExercise>(maturity));

    std::vector<Date> dividendDates = {today + Period(3, Months), today + Period(9, Months)};
    std::vector<Real> dividendAmounts = {8.3, 6.8};

    DividendVanillaOption option(
        payoff, exercise, dividendDates, dividendAmounts);

    option.setPricingEngine(
        ext::make_shared<AnalyticDividendEuropeanEngine>(process));

    const Real analyticNPV = option.NPV();
    const Real analyticDelta = option.delta();

    option.setPricingEngine(
        MakeFdBlackScholesVanillaEngine(process)
            .withTGrid(50)
            .withXGrid(200)
            .withDampingSteps(1)
            .withCashDividendModel(FdBlackScholesVanillaEngine::Escrowed)
        );

    const Real pdeNPV = option.NPV();
    const Real pdeDelta = option.delta();

    const Real tol = 0.0025;
    if (std::fabs(pdeNPV - analyticNPV) > tol) {
        BOOST_FAIL("Failed to reproduce European option values "
                "with the escrowed dividend model and the "
                "FdBlackScholesVanillaEngine engine"
                   << "\n    calculated: " << pdeNPV
                   << "\n    expected:   " << analyticNPV
                   << "\n    difference: " << std::fabs(pdeNPV - analyticNPV)
                   << "\n    tolerance:  " << tol);
    }

    if (std::fabs(pdeDelta - analyticDelta) > tol) {
        BOOST_FAIL("Failed to reproduce European option deltas "
                "with the escrowed dividend model and the "
                "FdBlackScholesVanillaEngine engine"
                   << "\n    calculated: " << pdeNPV
                   << "\n    expected:   " << analyticNPV
                   << "\n    difference: " << std::fabs(pdeNPV - analyticNPV)
                   << "\n    tolerance:  " << tol);
    }
}

test_suite* DividendOptionTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Dividend European option tests");
    suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testEuropeanValues));
    suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testEuropeanKnownValue));
    suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testEuropeanStartLimit));
    // Doesn't quite work.  Need to use discounted values
    //suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testEuropeanEndLimit));
    suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testEuropeanGreeks));
    suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testFdEuropeanValues));
    suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testFdEuropeanGreeks));
    suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testFdAmericanGreeks));
    suite->add(QUANTLIB_TEST_CASE(
                              &DividendOptionTest::testFdEuropeanDegenerate));
    suite->add(QUANTLIB_TEST_CASE(
                              &DividendOptionTest::testFdAmericanDegenerate));
    suite->add(QUANTLIB_TEST_CASE(
                              &DividendOptionTest::testFdEuropeanWithDividendToday));
    suite->add(QUANTLIB_TEST_CASE(
                              &DividendOptionTest::testFdAmericanWithDividendToday));
    suite->add(QUANTLIB_TEST_CASE(
                 &DividendOptionTest::testEscrowedDividendModel));

    return suite;
}

