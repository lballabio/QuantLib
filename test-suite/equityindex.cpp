/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2023 Marcin Rybacki

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/currencies/europe.hpp>
#include <ql/indexes/equityindex.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/quotes/simplequote.hpp>
#include <string>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(EquityIndexTests)

struct CommonVars {

    Date today;
    Calendar calendar;
    DayCounter dayCount;

    ext::shared_ptr<EquityIndex> equityIndex;
    RelinkableHandle<YieldTermStructure> interestHandle;
    RelinkableHandle<YieldTermStructure> dividendHandle;
    ext::shared_ptr<Quote> spot;
    RelinkableHandle<Quote> spotHandle;

    // utilities

    CommonVars(bool addTodaysFixing = true) {
        calendar = TARGET();
        dayCount = Actual365Fixed();

        equityIndex = ext::make_shared<EquityIndex>("eqIndex", calendar, EURCurrency(), interestHandle,
                                                    dividendHandle, spotHandle);

        today = calendar.adjust(Date(27, January, 2023));
            
        if (addTodaysFixing)
            equityIndex->addFixing(today, 8690.0);

        Settings::instance().evaluationDate() = today;

        interestHandle.linkTo(flatRate(0.03, dayCount));
        dividendHandle.linkTo(flatRate(0.01, dayCount));
            
        spot = ext::make_shared<SimpleQuote>(8700.0);
        spotHandle.linkTo(spot);
    }
};


BOOST_AUTO_TEST_CASE(testTodaysFixing) {
    BOOST_TEST_MESSAGE("Testing today's fixing...");

    CommonVars vars;
    const Real tolerance = 1.0e-8;

    const Real historicalIndex = 8690.0;
    Real todaysFixing = vars.equityIndex->fixing(vars.today);

    if ((std::fabs(todaysFixing - historicalIndex) > tolerance))
        BOOST_ERROR("today's fixing should be equal to historical index\n"
                    << "    actual fixing:    " << todaysFixing << "\n"
                    << "    expected fixing:    " << historicalIndex << "\n");

    const Real spot = 8700.0;
    Real forecastedFixing = vars.equityIndex->fixing(vars.today, true);

    if ((std::fabs(forecastedFixing - spot) > tolerance))
        BOOST_ERROR("today's fixing forecast should be equal to spot\n"
                    << "    actual forecast:    " << forecastedFixing << "\n"
                    << "    expected forecast:    " << spot << "\n");
}

BOOST_AUTO_TEST_CASE(testTodaysFixingWithSpotAsProxy) {
    BOOST_TEST_MESSAGE("Testing today's fixing with spot as proxy...");

    CommonVars vars(false);
    const Real tolerance = 1.0e-8;

    const Real spot = 8700.0;
    Real fixing = vars.equityIndex->fixing(vars.today);

    if ((std::fabs(fixing - spot) > tolerance))
        BOOST_ERROR("today's fixing should be equal to spot when historical index not added\n"
                    << "    actual fixing:    " << fixing << "\n"
                    << "    expected fixing:    " << spot << "\n");
}

BOOST_AUTO_TEST_CASE(testFixingForecast) {
    BOOST_TEST_MESSAGE("Testing fixing forecast...");

    CommonVars vars;
    const Real tolerance = 1.0e-8;

    Date forecastedDate(20, May, 2030);
    
    Real forecast = vars.equityIndex->fixing(forecastedDate);
    Real expectedForecast = vars.spotHandle->value() *
                            vars.dividendHandle->discount(forecastedDate) /
                            vars.interestHandle->discount(forecastedDate);

    if ((std::fabs(forecast - expectedForecast) > tolerance))
        BOOST_ERROR("could not replicate index forecast\n"
                    << "    actual forecast:    " << forecast << "\n"
                    << "    expected forecast:    " << expectedForecast << "\n");
}

BOOST_AUTO_TEST_CASE(testFixingForecastWithoutDividend) {
    BOOST_TEST_MESSAGE("Testing fixing forecast without dividend...");

    CommonVars vars;
    const Real tolerance = 1.0e-8;

    Date forecastedDate(20, May, 2030);

    auto equityIndexExDiv =
        vars.equityIndex->clone(vars.interestHandle, Handle<YieldTermStructure>(), vars.spotHandle);

    Real forecast = equityIndexExDiv->fixing(forecastedDate);
    Real expectedForecast =
        vars.spotHandle->value() / vars.interestHandle->discount(forecastedDate);

    if ((std::fabs(forecast - expectedForecast) > tolerance))
        BOOST_ERROR("could not replicate index forecast without dividend\n"
                    << "    actual forecast:    " << forecast << "\n"
                    << "    expected forecast:    " << expectedForecast << "\n");
}

BOOST_AUTO_TEST_CASE(testFixingForecastWithoutSpot) {
    BOOST_TEST_MESSAGE("Testing fixing forecast without spot handle...");

    CommonVars vars;
    const Real tolerance = 1.0e-8;

    Date forecastedDate(20, May, 2030);

    auto equityIndexExSpot =
        vars.equityIndex->clone(vars.interestHandle, vars.dividendHandle, Handle<Quote>());

    Real forecast = equityIndexExSpot->fixing(forecastedDate);
    Real expectedForecast = equityIndexExSpot->pastFixing(vars.today) *
                            vars.dividendHandle->discount(forecastedDate) /
                            vars.interestHandle->discount(forecastedDate);

    if ((std::fabs(forecast - expectedForecast) > tolerance))
        BOOST_ERROR("could not replicate index forecast without spot handle\n"
                    << "    actual forecast:    " << forecast << "\n"
                    << "    expected forecast:    " << expectedForecast << "\n");
}

BOOST_AUTO_TEST_CASE(testFixingForecastWithoutSpotAndHistoricalFixing) {
    BOOST_TEST_MESSAGE("Testing fixing forecast without spot handle and historical fixing...");

    CommonVars vars(false);

    Date forecastedDate(20, May, 2030);

    auto equityIndexExSpot =
        vars.equityIndex->clone(vars.interestHandle, vars.dividendHandle, Handle<Quote>());

    BOOST_CHECK_EXCEPTION(
        equityIndexExSpot->fixing(forecastedDate), Error,
        ExpectedErrorMessage(
            "Cannot forecast equity index, missing both spot and historical index"));
}

BOOST_AUTO_TEST_CASE(testSpotChange) {
    BOOST_TEST_MESSAGE("Testing spot change...");

    CommonVars vars;
    const Real tolerance = 1.0e-8;

    ext::shared_ptr<Quote> newSpot = ext::make_shared<SimpleQuote>(9000.0);
    vars.spotHandle.linkTo(newSpot);

    if ((std::fabs(newSpot->value() - vars.equityIndex->spot()->value()) > tolerance))
        BOOST_ERROR("could not re-link spot quote to new value\n"
                    << "    actual spot:    " << vars.equityIndex->spot()->value() << "\n"
                    << "    expected spot:    " << newSpot->value() << "\n");

    vars.spotHandle.linkTo(vars.spot);

    if ((std::fabs(vars.spot->value() - vars.equityIndex->spot()->value()) > tolerance))
        BOOST_ERROR("could not re-link spot quote back to old value\n"
                    << "    actual spot:    " << vars.equityIndex->spot()->value() << "\n"
                    << "    expected spot:    " << vars.spot->value() << "\n");
}

BOOST_AUTO_TEST_CASE(testErrorWhenInvalidFixingDate) {
    BOOST_TEST_MESSAGE("Testing error when invalid fixing date is used...");

    CommonVars vars;

    BOOST_CHECK_EXCEPTION(
        vars.equityIndex->fixing(Date(1, January, 2023)), Error,
        ExpectedErrorMessage("Fixing date January 1st, 2023 is not valid"));
}

BOOST_AUTO_TEST_CASE(testErrorWhenFixingMissing) {
    BOOST_TEST_MESSAGE("Testing error when required fixing is missing...");

    CommonVars vars;

    BOOST_CHECK_EXCEPTION(
        vars.equityIndex->fixing(Date(2, January, 2023)), Error,
        ExpectedErrorMessage("Missing eqIndex fixing for January 2nd, 2023"));
}

BOOST_AUTO_TEST_CASE(testErrorWhenInterestHandleMissing) {
    BOOST_TEST_MESSAGE("Testing error when interest handle is missing...");

    CommonVars vars;

    Date forecastedDate(20, May, 2030);

    auto equityIndexExDiv =
        vars.equityIndex->clone(
            Handle<YieldTermStructure>(), Handle<YieldTermStructure>(), Handle<Quote>());

    BOOST_CHECK_EXCEPTION(equityIndexExDiv->fixing(forecastedDate), Error,
                          ExpectedErrorMessage(
                              "null interest rate term structure set to this instance of eqIndex"));
}

BOOST_AUTO_TEST_CASE(testFixingObservability) {
    BOOST_TEST_MESSAGE("Testing observability of index fixings...");

    CommonVars vars;

    ext::shared_ptr<EquityIndex> i1 =
        ext::make_shared<EquityIndex>("observableEquityIndex", vars.calendar, EURCurrency());

    Flag flag;
    flag.registerWith(i1);
    flag.lower();

    ext::shared_ptr<Index> i2 =
        ext::make_shared<EquityIndex>("observableEquityIndex", vars.calendar, EURCurrency());

    i2->addFixing(vars.today, 100.0);
    if (!flag.isUp())
        BOOST_FAIL("Observer was not notified of added equity index fixing");
}

BOOST_AUTO_TEST_CASE(testNoErrorIfTodayIsNotBusinessDay) {
    BOOST_TEST_MESSAGE("Testing that no error is thrown if today is not a business day...");

    CommonVars vars;

    Date today(28, January, 2023);
    Date forecastedDate(20, May, 2030);

    Settings::instance().evaluationDate() = today;

    auto equityIndex =
        vars.equityIndex->clone(vars.interestHandle, vars.dividendHandle, Handle<Quote>());

    BOOST_REQUIRE_NO_THROW(vars.equityIndex->fixing(forecastedDate));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
