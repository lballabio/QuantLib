/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/instruments/simplechooseroption.hpp>
#include <ql/instruments/complexchooseroption.hpp>
#include <ql/pricingengines/exotic/analyticsimplechooserengine.hpp>
#include <ql/pricingengines/exotic/analyticcomplexchooserengine.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(ChooserOptionTests)

#undef REPORT_FAILURE
#define REPORT_FAILURE(greekName, choosingDate, \
                       exercise, s, q, r, today, v, \
                       expected, calculated, tolerance) \
    BOOST_ERROR( \
        exerciseTypeToString(exercise) \
        << " Chooser option with " \
        << "    spot value: " << s << "\n" \
        << "    dividend yield:   " << io::rate(q) << "\n" \
        << "    risk-free rate:   " << io::rate(r) << "\n" \
        << "    reference date:   " << today << "\n" \
        << "    maturity:         " << exercise->lastDate() << "\n" \
        << "    volatility:       " << io::volatility(v) << "\n\n" \
        << "    expected   " << greekName << ": " << expected << "\n" \
        << "    calculated " << greekName << ": " << calculated << "\n"\
        << "    error:            " << std::fabs(expected-calculated) \
        << "\n" \
        << "    tolerance:        " << tolerance);

BOOST_AUTO_TEST_CASE(testAnalyticSimpleChooserEngine){

    BOOST_TEST_MESSAGE("Testing analytic simple chooser option...");

    /* The data below are from
       "Complete Guide to Option Pricing Formulas", Espen Gaarder Haug
       pages 39-40
    */
    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot = ext::make_shared<SimpleQuote>(50.0);
    ext::shared_ptr<SimpleQuote> qRate = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate = ext::make_shared<SimpleQuote>(0.08);
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol = ext::make_shared<SimpleQuote>(0.25);
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess =
        ext::make_shared<BlackScholesMertonProcess>(
                                  Handle<Quote>(spot),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS));

    ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<AnalyticSimpleChooserEngine>(stochProcess);

    Real strike = 50.0;

    Date exerciseDate = today + 180;
    ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(exerciseDate);

    Date choosingDate = today + 90;
    SimpleChooserOption option(choosingDate,strike,exercise);
    option.setPricingEngine(engine);

    Real calculated = option.NPV();
    Real expected = 6.1071;
    Real tolerance = 3e-5;
    if (std::fabs(calculated-expected) > tolerance) {
        REPORT_FAILURE("value", choosingDate,
                       exercise, spot->value(),
                       qRate->value(), rRate->value(), today,
                       vol->value(), expected, calculated, tolerance);
    }

}

BOOST_AUTO_TEST_CASE(testAnalyticComplexChooserEngine){
    BOOST_TEST_MESSAGE("Testing analytic complex chooser option...");

    /* The example below is from
       "Complete Guide to Option Pricing Formulas", Espen Gaarder Haug
    */
    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot = ext::make_shared<SimpleQuote>(50.0);
    ext::shared_ptr<SimpleQuote> qRate = ext::make_shared<SimpleQuote>(0.05);
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate = ext::make_shared<SimpleQuote>(0.10);
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol = ext::make_shared<SimpleQuote>(0.35);
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess =
        ext::make_shared<BlackScholesMertonProcess>(
                                  Handle<Quote>(spot),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS));

    ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<AnalyticComplexChooserEngine>(stochProcess);

    Real callStrike = 55.0;
    Real putStrike = 48.0;

    Date choosingDate = today + 90;
    Date callExerciseDate = choosingDate + 180;
    Date putExerciseDate = choosingDate + 210;
    ext::shared_ptr<Exercise> callExercise =
        ext::make_shared<EuropeanExercise>(callExerciseDate);
    ext::shared_ptr<Exercise> putExercise =
        ext::make_shared<EuropeanExercise>(putExerciseDate);

    ComplexChooserOption option(choosingDate,callStrike,putStrike,
                                callExercise,putExercise);
    option.setPricingEngine(engine);

    Real calculated = option.NPV();
    Real expected = 6.0508;
    Real error = std::fabs(calculated-expected);
    Real tolerance = 1e-4;
    if (error > tolerance) {
        BOOST_ERROR("Failed to reproduce complex chooser option value"
                    << "\n    expected:   " << expected
                    << "\n    calculated: " << calculated
                    << "\n    error:      " << error);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
