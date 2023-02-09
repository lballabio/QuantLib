/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2023 Marcin Rybacki

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

#include "equityindex.hpp"
#include "utilities.hpp"
#include <ql/indexes/equityindex.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/quotes/simplequote.hpp>
#include <string>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace equityindex_test {

    // Used to check that the exception message contains the expected message string, expMsg.
    struct ExpErrorPred {

        explicit ExpErrorPred(std::string msg) : expMsg(std::move(msg)) {}

        bool operator()(const Error& ex) const {
            std::string errMsg(ex.what());
            if (errMsg.find(expMsg) == std::string::npos) {
                BOOST_TEST_MESSAGE("Error expected to contain: '" << expMsg << "'.");
                BOOST_TEST_MESSAGE("Actual error is: '" << errMsg << "'.");
                return false;
            } else {
                return true;
            }
        }

        std::string expMsg;
    };

    struct CommonVars {

        Date today;
        Calendar calendar;
        DayCounter dayCount;

        ext::shared_ptr<EquityIndex> equityIndex;
        RelinkableHandle<YieldTermStructure> interestHandle;
        RelinkableHandle<YieldTermStructure> dividendHandle;
        ext::shared_ptr<Quote> spot;
        RelinkableHandle<Quote> spotHandle;

        // cleanup
        SavedSettings backup;
        // utilities

        CommonVars(bool addTodaysFixing = true) {
            calendar = TARGET();
            dayCount = Actual365Fixed();

            equityIndex = ext::make_shared<EquityIndex>("eqIndex", calendar, interestHandle,
                                                        dividendHandle, spotHandle);
            IndexManager::instance().clearHistory(equityIndex->name());

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
}

void EquityIndexTest::testTodaysFixing() {
    BOOST_TEST_MESSAGE("Testing todays fixing...");

    using namespace equityindex_test;

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

void EquityIndexTest::testTodaysFixingWithSpotAsProxy() {
    BOOST_TEST_MESSAGE("Testing todays fixing with spot as proxy...");

    using namespace equityindex_test;

    CommonVars vars(false);
    const Real tolerance = 1.0e-8;

    const Real spot = 8700.0;
    Real fixing = vars.equityIndex->fixing(vars.today);

    if ((std::fabs(fixing - spot) > tolerance))
        BOOST_ERROR("today's fixing should be equal to spot when historical index not added\n"
                    << "    actual fixing:    " << fixing << "\n"
                    << "    expected fixing:    " << spot << "\n");
}

void EquityIndexTest::testFixingForecast() {
    BOOST_TEST_MESSAGE("Testing fixing forecast...");

    using namespace equityindex_test;

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

void EquityIndexTest::testFixingForecastWithoutDividend() {
    BOOST_TEST_MESSAGE("Testing fixing forecast without dividend...");

    using namespace equityindex_test;

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

void EquityIndexTest::testFixingForecastWithoutSpot() {
    BOOST_TEST_MESSAGE("Testing fixing forecast without spot handle...");

    using namespace equityindex_test;

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

void EquityIndexTest::testFixingForecastWithoutSpotAndHistoricalFixing() {
    BOOST_TEST_MESSAGE("Testing fixing forecast without spot handle and historical fixing...");

    using namespace equityindex_test;

    CommonVars vars(false);

    Date forecastedDate(20, May, 2030);

    auto equityIndexExSpot =
        vars.equityIndex->clone(vars.interestHandle, vars.dividendHandle, Handle<Quote>());

    BOOST_CHECK_EXCEPTION(
        equityIndexExSpot->fixing(forecastedDate), Error,
        equityindex_test::ExpErrorPred(
            "Cannot forecast equity index, missing both spot and historical index"));
}

void EquityIndexTest::testSpotChange() {
    BOOST_TEST_MESSAGE("Testing spot change...");

    using namespace equityindex_test;

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

void EquityIndexTest::testErrorWhenInvalidFixingDate() {
    BOOST_TEST_MESSAGE("Testing error when invalid fixing date is used...");

    using namespace equityindex_test;

    CommonVars vars;

    BOOST_CHECK_EXCEPTION(
        vars.equityIndex->fixing(Date(1, January, 2023)), Error,
        equityindex_test::ExpErrorPred("Fixing date January 1st, 2023 is not valid"));
}

void EquityIndexTest::testErrorWhenFixingMissing() {
    BOOST_TEST_MESSAGE("Testing error when required fixing is missing...");

    using namespace equityindex_test;

    CommonVars vars;

    BOOST_CHECK_EXCEPTION(
        vars.equityIndex->fixing(Date(2, January, 2023)), Error,
        equityindex_test::ExpErrorPred("Missing eqIndex fixing for January 2nd, 2023"));
}

void EquityIndexTest::testErrorWhenInterestHandleMissing() {
    BOOST_TEST_MESSAGE("Testing error when interest handle is missing...");

    using namespace equityindex_test;

    CommonVars vars;

    Date forecastedDate(20, May, 2030);

    auto equityIndexExDiv =
        vars.equityIndex->clone(
            Handle<YieldTermStructure>(), Handle<YieldTermStructure>(), Handle<Quote>());

    BOOST_CHECK_EXCEPTION(equityIndexExDiv->fixing(forecastedDate), Error,
                          equityindex_test::ExpErrorPred(
                              "null interest rate term structure set to this instance of eqIndex"));
}

void EquityIndexTest::testFixingObservability() {
    BOOST_TEST_MESSAGE("Testing observability of index fixings...");

    using namespace equityindex_test;

    CommonVars vars;

    ext::shared_ptr<EquityIndex> i1 =
        ext::make_shared<EquityIndex>("observableEquityIndex", vars.calendar);

    Flag flag;
    flag.registerWith(i1);
    flag.lower();

    ext::shared_ptr<Index> i2 =
        ext::make_shared<EquityIndex>("observableEquityIndex", vars.calendar);

    i2->addFixing(vars.today, 100.0);
    if (!flag.isUp())
        BOOST_FAIL("Observer was not notified of added equity index fixing");
}

void EquityIndexTest::testNoErrorIfTodayIsNotBusinessDay() {
    BOOST_TEST_MESSAGE("Testing no error if today is not a business day...");

    using namespace equityindex_test;

    CommonVars vars;

    Date today(28, January, 2023);
    Date forecastedDate(20, May, 2030);

    Settings::instance().evaluationDate() = today;

    auto equityIndex =
        vars.equityIndex->clone(vars.interestHandle, vars.dividendHandle, Handle<Quote>());

    BOOST_REQUIRE_NO_THROW(vars.equityIndex->fixing(forecastedDate));
}

test_suite* EquityIndexTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Equity index tests");

    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testTodaysFixing));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testTodaysFixingWithSpotAsProxy));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testFixingForecast));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testFixingForecastWithoutDividend));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testFixingForecastWithoutSpot));
    suite->add(
        QUANTLIB_TEST_CASE(&EquityIndexTest::testFixingForecastWithoutSpotAndHistoricalFixing));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testSpotChange));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testErrorWhenInvalidFixingDate));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testErrorWhenFixingMissing));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testErrorWhenInterestHandleMissing));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testFixingObservability));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testNoErrorIfTodayIsNotBusinessDay));

    return suite;
}
