/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2007 StatPro Italia srl

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

// TODO: Figure out why tests for options with both continuous and discrete
// dividends fail.

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/math/functional.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/vanilla/analyticdividendeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/cashdividendeuropeanengine.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancecurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(DividendOptionTests)

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

BOOST_AUTO_TEST_CASE(testEuropeanValues) {

    BOOST_TEST_MESSAGE(
              "Testing dividend European option values with no dividends...");

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
        for (Real strike : strikes) {
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

                VanillaOption ref_option(payoff, exercise);
                ref_option.setPricingEngine(ref_engine);

                auto engine1 =
                    ext::make_shared<AnalyticDividendEuropeanEngine>(stochProcess, DividendVector(dividendDates, dividends));

                VanillaOption option1(payoff, exercise);
                option1.setPricingEngine(engine1);

                auto engine2 =
                    ext::make_shared<AnalyticDividendEuropeanEngine>(stochProcess, DividendVector({ exDate + 6*Months }, { 1.0 }));

                VanillaOption option2(payoff, exercise);
                option2.setPricingEngine(engine2);

                for (Real u : underlyings) {
                    for (Real m : qRates) {
                        for (Real n : rRates) {
                            for (Real v : vols) {
                                Rate q = m, r = n;
                                spot->setValue(u);
                                qRate->setValue(q);
                                rRate->setValue(r);
                                vol->setValue(v);

                                Real expected = ref_option.NPV();
                                Real calculated1 = option1.NPV();
                                Real calculated2 = option2.NPV();
                                Real error1 = std::fabs(calculated1 - expected);
                                Real error2 = std::fabs(calculated2 - expected);
                                if (error1 > tolerance) {
                                    REPORT_FAILURE("value start limit", payoff, exercise, u, q, r,
                                                   today, v, expected, calculated1, error1,
                                                   tolerance);
                                }
                                if (error2 > tolerance) {
                                    REPORT_FAILURE("value start limit", payoff, exercise, u, q, r,
                                                   today, v, expected, calculated2, error2,
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

BOOST_AUTO_TEST_CASE(testEuropeanKnownValue) {

    BOOST_TEST_MESSAGE("Testing dividend European option against known value...");

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

    auto engine = ext::make_shared<AnalyticDividendEuropeanEngine>(
        stochProcess, DividendVector(dividendDates, dividends));

    VanillaOption option(payoff, exercise);
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

BOOST_AUTO_TEST_CASE(testEuropeanStartLimit) {

    BOOST_TEST_MESSAGE(
       "Testing dividend European option with a dividend on today's date...");

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
        for (Real strike : strikes) {
            for (int length : lengths) {
                Date exDate = today + length * Years;
                ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

                std::vector<Date> dividendDates = {today};
                std::vector<Real> dividends = {dividendValue};

                ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));

                ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
                    new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

                ext::shared_ptr<PricingEngine> ref_engine(new AnalyticEuropeanEngine(stochProcess));

                VanillaOption ref_option(payoff, exercise);
                ref_option.setPricingEngine(ref_engine);

                auto engine = ext::make_shared<AnalyticDividendEuropeanEngine>(
                    stochProcess, DividendVector(dividendDates, dividends));

                VanillaOption option(payoff, exercise);
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

// Doesn't quite work.  Need to use discounted values
//BOOST_AUTO_TEST_CASE(testEuropeanEndLimit) {
//
//    BOOST_TEST_MESSAGE(
//              "Testing dividend European option values with end limits...");
//
//    Real tolerance = 1.0e-5;
//    Real dividendValue = 10.0;
//
//    Option::Type types[] = { Option::Call, Option::Put };
//    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
//    Real underlyings[] = { 100.0 };
//    Rate qRates[] = { 0.00, 0.10, 0.30 };
//    Rate rRates[] = { 0.01, 0.05, 0.15 };
//    Integer lengths[] = { 1, 2 };
//    Volatility vols[] = { 0.05, 0.20, 0.70 };
//
//    DayCounter dc = Actual360();
//    Date today = Date::todaysDate();
//    Settings::instance().evaluationDate() = today;
//
//    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
//    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
//    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
//    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
//    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
//    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
//    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));
//
//    for (auto& type : types) {
//        for (Real strike : strikes) {
//            for (int length : lengths) {
//                Date exDate = today + length * Years;
//                ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
//
//                ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
//                    new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));
//
//                std::vector<Date> dividendDates = {exercise->lastDate()};
//                std::vector<Real> dividends = {dividendValue};
//
//                ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));
//
//                ext::shared_ptr<StrikedTypePayoff> refPayoff(
//                    new PlainVanillaPayoff(type, strike + dividendValue));
//
//                ext::shared_ptr<PricingEngine> ref_engine(new AnalyticEuropeanEngine(stochProcess));
//
//                VanillaOption ref_option(refPayoff, exercise);
//                ref_option.setPricingEngine(ref_engine);
//
//                QL_DEPRECATED_DISABLE_WARNING
//                ext::shared_ptr<PricingEngine> engine1(
//                    new AnalyticDividendEuropeanEngine(stochProcess));
//
//                DividendVanillaOption option1(payoff, exercise, dividendDates, dividends);
//                QL_DEPRECATED_ENABLE_WARNING
//                option1.setPricingEngine(engine1);
//
//                auto engine2 = ext::make_shared<AnalyticDividendEuropeanEngine>(
//                    stochProcess, DividendVector(dividendDates, dividends));
//
//                VanillaOption option2(payoff, exercise);
//                option2.setPricingEngine(engine2);
//
//                for (Real u : underlyings) {
//                    for (Real m : qRates) {
//                        for (Real n : rRates) {
//                            for (Real v : vols) {
//                                Rate q = m, r = n;
//                                spot->setValue(u);
//                                qRate->setValue(q);
//                                rRate->setValue(r);
//                                vol->setValue(v);
//
//                                Real expected = ref_option.NPV();
//                                Real calculated = option1.NPV();
//                                Real error = std::fabs(calculated - expected);
//                                if (error > tolerance) {
//                                    REPORT_FAILURE("value", payoff, exercise, u, q, r, today, v,
//                                                   expected, calculated, error, tolerance);
//                                }
//                                calculated = option2.NPV();
//                                error = std::fabs(calculated - expected);
//                                if (error > tolerance) {
//                                    REPORT_FAILURE("value", payoff, exercise, u, q, r, today, v,
//                                                   expected, calculated, error, tolerance);
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
//}

BOOST_AUTO_TEST_CASE(testEuropeanGreeks) {

    BOOST_TEST_MESSAGE("Testing dividend European option greeks...");

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
        for (Real strike : strikes) {
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

                auto engine = ext::make_shared<AnalyticDividendEuropeanEngine>(
                    stochProcess, DividendVector(dividendDates, dividends));

                VanillaOption option(payoff, exercise);
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

BOOST_AUTO_TEST_CASE(testFdEuropeanValues) {

    BOOST_TEST_MESSAGE(
              "Testing finite-difference dividend European option values...");

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
        for (Real strike : strikes) {
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

                auto ref_engine = ext::make_shared<AnalyticDividendEuropeanEngine>(
                    stochProcess, DividendVector(dividendDates, dividends));

                VanillaOption ref_option(payoff, exercise);
                ref_option.setPricingEngine(ref_engine);

                ext::shared_ptr<PricingEngine> engine =
                    MakeFdBlackScholesVanillaEngine(stochProcess)
                        .withTGrid(timeSteps)
                        .withXGrid(gridPoints)
                        .withCashDividends(dividendDates, dividends)
                        .withCashDividendModel(FdBlackScholesVanillaEngine::Escrowed);

                VanillaOption option(payoff, exercise);
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


void testFdGreeks(const Date& today,
                  const ext::shared_ptr<Exercise>& exercise,
                  FdBlackScholesVanillaEngine::CashDividendModel model) {

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"] = 5.0e-3;
    tolerance["gamma"] = 7.0e-3;

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
        for (Real strike : strikes) {

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
                .withCashDividends(dividendDates, dividends)
                .withCashDividendModel(model);

            VanillaOption option(payoff, exercise);
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


BOOST_AUTO_TEST_CASE(testFdEuropeanGreeks, *precondition(if_speed(Fast))) {
    BOOST_TEST_MESSAGE("Testing finite-differences dividend European option greeks...");

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

BOOST_AUTO_TEST_CASE(testFdAmericanGreeks) {
    BOOST_TEST_MESSAGE(
             "Testing finite-differences dividend American option greeks...");

    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;
    Integer lengths[] = { 1, 2 };

    for (int length : lengths) {
        Date exDate = today + length * Years;
        ext::shared_ptr<Exercise> exercise(new AmericanExercise(today,exDate));
        testFdGreeks(today,exercise,FdBlackScholesVanillaEngine::Spot);
    }
}


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

    ext::shared_ptr<StrikedTypePayoff> payoff(
                                  new PlainVanillaPayoff(Option::Call, 55.0));

    Real tolerance = 1.0e-6;

    VanillaOption option(payoff, exercise);
    ext::shared_ptr<PricingEngine> engine =
        MakeFdBlackScholesVanillaEngine(process)
        .withTGrid(timeSteps)
        .withXGrid(gridPoints)
        .withCashDividendModel(model);
    option.setPricingEngine(engine);

    Real refValue = option.NPV();

    std::vector<Rate> dividends;
    std::vector<Date> dividendDates;

    engine =
        MakeFdBlackScholesVanillaEngine(process)
        .withTGrid(timeSteps)
        .withXGrid(gridPoints)
        .withCashDividends(dividendDates, dividends)
        .withCashDividendModel(model);
    option.setPricingEngine(engine);
    Real value = option.NPV();

    if (std::fabs(refValue-value) > tolerance)
        BOOST_FAIL("NPV changed by empty dividend set:\n"
                   << "    previous value: " << value << "\n"
                   << "    current value:  " << refValue << "\n"
                   << "    change:         " << value-refValue);

    for (Size i=1; i<=6; i++) {

        dividends.push_back(0.0);
        dividendDates.push_back(today+i);

        engine =
            MakeFdBlackScholesVanillaEngine(process)
            .withTGrid(timeSteps)
            .withXGrid(gridPoints)
            .withCashDividends(dividendDates, dividends)
            .withCashDividendModel(model);
        option.setPricingEngine(engine);
        value = option.NPV();

        if (std::fabs(refValue-value) > tolerance)
            BOOST_FAIL("NPV changed by null dividend :\n"
                       << "    previous value: " << value << "\n"
                       << "    current value:  " << refValue << "\n"
                       << "    change:         " << value-refValue);
    }
}


BOOST_AUTO_TEST_CASE(testFdEuropeanDegenerate) {

    BOOST_TEST_MESSAGE(
         "Testing degenerate finite-differences dividend European option...");

    Date today = Date(27,February,2005);
    Settings::instance().evaluationDate() = today;
    Date exDate(13,April,2005);

    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

    testFdDegenerate(today,exercise,FdBlackScholesVanillaEngine::Spot);
    testFdDegenerate(today,exercise,FdBlackScholesVanillaEngine::Escrowed);
}

BOOST_AUTO_TEST_CASE(testFdAmericanDegenerate) {

    BOOST_TEST_MESSAGE(
         "Testing degenerate finite-differences dividend American option...");

    Date today = Date(27,February,2005);
    Settings::instance().evaluationDate() = today;
    Date exDate(13,April,2005);

    ext::shared_ptr<Exercise> exercise(new AmericanExercise(today,exDate));

    testFdDegenerate(today,exercise,FdBlackScholesVanillaEngine::Spot);
    testFdDegenerate(today,exercise,FdBlackScholesVanillaEngine::Escrowed);
}


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

    ext::shared_ptr<StrikedTypePayoff> payoff(
                                  new PlainVanillaPayoff(Option::Call, 55.0));

    // today's dividend must by taken into account
    std::vector<Rate> dividends(1, 1.0);
    std::vector<Date> dividendDates(1, today);

    VanillaOption option(payoff, exercise);
    ext::shared_ptr<PricingEngine> engine =
        MakeFdBlackScholesVanillaEngine(process)
        .withTGrid(timeSteps)
        .withXGrid(gridPoints)
        .withCashDividends(dividendDates, dividends)
        .withCashDividendModel(model);
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
    VanillaOption europeanOption(payoff, europeanExercise);

    europeanOption.setPricingEngine(
            ext::make_shared<AnalyticDividendEuropeanEngine>(process, DividendVector(dividendDates, dividends)));

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


BOOST_AUTO_TEST_CASE(testFdEuropeanWithDividendToday) {

    BOOST_TEST_MESSAGE(
         "Testing finite-differences dividend European option with dividend on today's date...");

    Date today = Date(27,February,2005);
    Settings::instance().evaluationDate() = today;
    Date exDate(13,April,2005);

    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

    testFdDividendAtTZero(today,exercise,FdBlackScholesVanillaEngine::Spot);
    testFdDividendAtTZero(today,exercise,FdBlackScholesVanillaEngine::Escrowed);
}

BOOST_AUTO_TEST_CASE(testFdAmericanWithDividendToday) {

    BOOST_TEST_MESSAGE(
         "Testing finite-differences dividend American option with dividend on today's date...");

    Date today = Date(27,February,2005);
    Settings::instance().evaluationDate() = today;
    Date exDate(13,April,2005);

    ext::shared_ptr<Exercise> exercise(new AmericanExercise(today,exDate));

    testFdDividendAtTZero(today,exercise,FdBlackScholesVanillaEngine::Spot);
}

BOOST_AUTO_TEST_CASE(testEscrowedDividendModel) {
    BOOST_TEST_MESSAGE("Testing finite-difference European engine "
                       "with the escrowed dividend model...");

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

    VanillaOption ref_option(payoff, exercise);

    ref_option.setPricingEngine(
        ext::make_shared<AnalyticDividendEuropeanEngine>(process, DividendVector(dividendDates, dividendAmounts)));

    const Real analyticNPV = ref_option.NPV();
    const Real analyticDelta = ref_option.delta();

    VanillaOption option(payoff, exercise);
    option.setPricingEngine(
        MakeFdBlackScholesVanillaEngine(process)
        .withTGrid(50)
        .withXGrid(200)
        .withDampingSteps(1)
        .withCashDividends(dividendDates, dividendAmounts)
        .withCashDividendModel(FdBlackScholesVanillaEngine::Escrowed)
    );

    Real pdeNPV = option.NPV();
    Real pdeDelta = option.delta();

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

BOOST_AUTO_TEST_CASE(testCashDividendEuropeanEngine) {
    BOOST_TEST_MESSAGE("Testing cash-dividend European engine "
                       "with finite-difference European engine...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(1, January, 2024);

    Settings::instance().evaluationDate() = today;

    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(100.0));

    const Handle<YieldTermStructure> rTS(
        ext::make_shared<ZeroCurve>(
            std::vector<Date>{today, Date(1, May, 2024),
                              Date(1, November, 2024), Date(1, January, 2027)},
            std::vector<Real>{0.3, 0.15, 0.1, 0.15},
            dc
        )
    );
    const Handle<YieldTermStructure> qTS(
        ext::make_shared<ZeroCurve>(
            std::vector<Date>{today, Date(1, May, 2024),
                              Date(1, November, 2025), Date(1, January, 2027)},
            std::vector<Real>{0.05, 0.03, 0.1, 0.05},
            dc
        )
    );
    const Handle<BlackVolTermStructure> vTS(
        ext::make_shared<BlackVarianceCurve>(
            today,
            std::vector<Date>{Date(2, January, 2024), Date(1, July, 2024),
                              Date(1, August, 2024), Date(1, January, 2027)},
            std::vector<Volatility>{0.3, 0.4, 0.42, 0.5},
            dc
        )
    );

    const ext::shared_ptr<BlackScholesMertonProcess> process =
        ext::make_shared<BlackScholesMertonProcess>(spot, qTS, rTS, vTS);

    const std::vector<Date> dividendDates{
    	Date(1, April, 2024), Date(1, November, 2024), Date(1, October, 2024),
    	Date(1, April, 2026), Date(27, March, 2028), Date(1, October, 2023)
    };
    const std::vector<Rate> dividendAmounts{4, 10, 2, 5, 25, 15};

    DividendSchedule dividendSchedule;
    std::transform(
    	dividendAmounts.begin(), dividendAmounts.end(),
		dividendDates.begin(), std::back_inserter(dividendSchedule),
		[](const Rate amount, const Date& date) {
    		return ext::make_shared<FixedDividend>(amount, date);
    	}
    );

    const Real tol = 0.005;
    for (const auto cashDivModel: {
    	CashDividendEuropeanEngine::Spot, CashDividendEuropeanEngine::Escrowed
    }) {
		const ext::shared_ptr<PricingEngine> fdEngine =
			MakeFdBlackScholesVanillaEngine(process)
				.withTGrid(100)
				.withXGrid(800)
				.withCashDividends(dividendDates, dividendAmounts)
				.withCashDividendModel(
					FdBlackScholesVanillaEngine::CashDividendModel(cashDivModel));

		const ext::shared_ptr<PricingEngine> cashDivEngine =
			ext::make_shared<CashDividendEuropeanEngine>(
				process, dividendSchedule, cashDivModel
			);

		for (const auto& optionType: {Option::Call, Option::Put})
			for (const Date maturityDate: {Date(1, April, 2026), Date(1, January, 2027)})
				for (const Real strike: {50, 100, 125, 175}) {
					VanillaOption option(
						ext::make_shared<PlainVanillaPayoff>(optionType, strike),
						ext::make_shared<EuropeanExercise>(maturityDate)
					);

					option.setPricingEngine(fdEngine);
					const Real fdNPV = option.NPV();

					option.setPricingEngine(cashDivEngine);
					const Real cdNPV = option.NPV();

					const Real diff = std::fabs(fdNPV - cdNPV);
					if (diff > tol) {
						BOOST_FAIL("Failed to compare European option prices "
								   "with CashDividendEuropeanEngine and "
								   "FdBlackScholesVanillaEngine"
								   << "\n    Strike         : " << strike
								   << "\n    Option Type    : "
								   << ((optionType == Option::Call)? "Call" : "Put")
								   << "\n    Maturity Date  : " << maturityDate
								   << "\n    Dividends Model: "
								   << ((cashDivModel == CashDividendEuropeanEngine::Spot)? "Spot" : "Escrowed")
								   << "\n    FDM price      : " << fdNPV
								   << "\n    Cash Div price : " << cdNPV
								   << "\n    difference     : " << diff
								   << "\n    tolerance      : " << tol);
					}
				}
    }
}


BOOST_AUTO_TEST_CASE(testZeroStrikeCallWithEscrowedDividends) {
    BOOST_TEST_MESSAGE("Testing zero strike call with escrowed dividend model...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(26, October, 2025);

    Settings::instance().evaluationDate() = today;

    const Date maturityDate = today + Period(1, Years);
    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(100.0));

    const Handle<YieldTermStructure> qTS(flatRate(today, 0.063, dc));
    const Handle<YieldTermStructure> rTS(flatRate(today, 0.094, dc));

    const ext::shared_ptr<BlackScholesMertonProcess> process =
        ext::make_shared<BlackScholesMertonProcess>(
            Handle<Quote>(ext::make_shared<SimpleQuote>(100)),
			qTS, rTS,
			Handle<BlackVolTermStructure>(flatVol(today, 0.3, dc))
		);

	VanillaOption europeanOption(
		ext::make_shared<PlainVanillaPayoff>(Option::Call, 0.0),
		ext::make_shared<EuropeanExercise>(maturityDate)
	);

    const Real dividend = 5.0;

    for (const Date dividendDate: {today, Date(1, January, 2026), maturityDate}) {
		const ext::shared_ptr<PricingEngine> fdEngine =
			MakeFdBlackScholesVanillaEngine(process)
				.withTGrid(100)
				.withXGrid(400)
				.withCashDividends({dividendDate}, {dividend})
				.withCashDividendModel(
					FdBlackScholesVanillaEngine::Escrowed);

		europeanOption.setPricingEngine(fdEngine);
		const Real europeanCalculated = europeanOption.NPV();
		const Real europeanExpected = process->x0() * qTS->discount(maturityDate)
			- dividend * rTS->discount(dividendDate)
					   / qTS->discount(dividendDate) * qTS->discount(maturityDate);

		const Real tol = 1e-3;
		const Real europeanDiff = std::abs(europeanCalculated - europeanExpected);
		if (europeanDiff > tol) {
			BOOST_FAIL("Failed to calculate zero strike Europeasn call prices with "
					   "escrowed dividend model"
					   << "\n    FDM price   : " << europeanCalculated
					   << "\n    expected    : " << europeanExpected
					   << "\n    difference  : " << europeanDiff
					   << "\n    tolerance   : " << tol);
		}

		VanillaOption americanOption(
			ext::make_shared<PlainVanillaPayoff>(Option::Call, 0.0),
			ext::make_shared<AmericanExercise>(maturityDate)
		);
		americanOption.setPricingEngine(fdEngine);

		const Real americanCalculated = americanOption.NPV();
		const Real americanExpected = process->x0();
		const Real americanDiff = std::abs(americanCalculated - americanExpected);

		if (americanDiff > tol) {
			BOOST_FAIL("Failed to calculate zero strike Europeasn call prices with "
					   "escrowed dividend model"
					   << "\n    FDM price   : " << americanCalculated
					   << "\n    expected    : " << americanExpected
					   << "\n    difference  : " << europeanDiff
					   << "\n    tolerance   : " << tol);
		}
    }
}


BOOST_AUTO_TEST_CASE(testAmericanOptionsWithEscrowedDividends) {
    BOOST_TEST_MESSAGE("Testing American option with escrowed dividend model...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(26, October, 2025);

    Settings::instance().evaluationDate() = today;

    const Date maturityDate = today + Period(18, Months);
    const Time maturityTime = dc.yearFraction(today, maturityDate);

    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(100.0));

    const Handle<YieldTermStructure> qTS(flatRate(today, 0.05, dc));
    const Handle<YieldTermStructure> rTS(flatRate(today, 0.15, dc));

    const Volatility v = 0.3;
    const ext::shared_ptr<SimpleQuote> vol = ext::make_shared<SimpleQuote>(v);
    RelinkableHandle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    const ext::shared_ptr<BlackScholesMertonProcess> process =
        ext::make_shared<BlackScholesMertonProcess>(
            Handle<Quote>(ext::make_shared<SimpleQuote>(100)),
			qTS, rTS, volTS
		);

    const Real dividend = 5.0;

    for (const Option::Type optionType: {Option::Call, Option::Put})
		for (const Date dividendDate: {
			today, Date(1, January, 2026), Date(1, January, 2027), maturityDate}) {

			const auto builder = [&](FdBlackScholesVanillaEngine::CashDividendModel model) {
				return MakeFdBlackScholesVanillaEngine(process)
						.withTGrid(100)
						.withXGrid(400)
						.withCashDividends({dividendDate}, {dividend})
						.withCashDividendModel(model);
			};

			const ext::shared_ptr<PricingEngine> escrowedEngine = builder(
				FdBlackScholesVanillaEngine::Escrowed);

			const ext::shared_ptr<PricingEngine> spotEngine = builder(
				FdBlackScholesVanillaEngine::Spot);

			VanillaOption option(
				ext::make_shared<PlainVanillaPayoff>(optionType, 95),
				ext::make_shared<AmericanExercise>(maturityDate)
			);

			vol->setValue(v);
			option.setPricingEngine(escrowedEngine);
			const Real escrowedNPV = option.NPV();

			vol->setValue(
				std::sqrt(
					(  v*v*dc.yearFraction(today, dividendDate)
						*squared((spot->value()-dividend)/spot->value())
					 + v*v*dc.yearFraction(dividendDate, maturityDate))/maturityTime
				)
			);
			option.setPricingEngine(spotEngine);
			const Real spotNPV = option.NPV();

			const Real tol = 0.05;
			const Real diff = std::abs(spotNPV - escrowedNPV);
			if (diff > tol) {
				BOOST_FAIL("Failed to compare American option prices "
						   "with cash- and escrowed dividend model"
						   << "\n    Option Type       : "
						   << ((optionType == Option::Call)? "Call" : "Put")
						   << "\n    dividend date     : " << dividendDate
						   << "\n    escrowed div price: " << escrowedNPV
						   << "\n    cash div price    : " << spotNPV
						   << "\n    difference        : " << diff
						   << "\n    tolerance         : " << tol);
			}
		}
}



BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
