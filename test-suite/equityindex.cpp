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

        // cleanup
        SavedSettings backup;
        // utilities

        CommonVars() {
            calendar = TARGET();
            dayCount = Actual365Fixed();

            equityIndex =
                ext::make_shared<EquityIndex>("eqIndex", calendar, interestHandle, dividendHandle);

            equityIndex->addFixing(Date(31, January, 2023), 8690.0);

            today = calendar.adjust(Date(31, January, 2023));
            Settings::instance().evaluationDate() = today;

            interestHandle.linkTo(flatRate(today, 0.03, dayCount));
            dividendHandle.linkTo(flatRate(today, 0.01, dayCount));
        }
    };
}

void EquityIndexTest::testTodaysFixingForecast() {
    BOOST_TEST_MESSAGE("Testing todays fixing forecast...");

    using namespace equityindex_test;

    CommonVars vars;
    const Real tolerance = 1.0e-8;

    Real forecastedFixing = vars.equityIndex->fixing(vars.today, true);
    Real pastFixing = vars.equityIndex->fixing(vars.today, false);

    if ((std::fabs(pastFixing - forecastedFixing) > tolerance))
        BOOST_ERROR("forecasted fixing should be equal to past fixing\n"
                    << "    past fixing:    " << pastFixing << "\n"
                    << "    forecasted fixing:    " << forecastedFixing << "\n");

}

void EquityIndexTest::testFixingForecast() {
    BOOST_TEST_MESSAGE("Testing fixing forecast...");

    using namespace equityindex_test;

    CommonVars vars;
    const Real tolerance = 1.0e-8;

    Date forecastedDate(20, May, 2030);
    
    Real forecast = vars.equityIndex->fixing(forecastedDate);
    Real expectedForecast = vars.equityIndex->pastFixing(vars.today) *
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

    auto equityIndexExDiv = vars.equityIndex->clone(vars.equityIndex->equityInterestRateCurve(),
                                                    Handle<YieldTermStructure>());

    Real forecast = equityIndexExDiv->fixing(forecastedDate);
    Real expectedForecast = equityIndexExDiv->pastFixing(vars.today) /
                            vars.interestHandle->discount(forecastedDate);

    if ((std::fabs(forecast - expectedForecast) > tolerance))
        BOOST_ERROR("could not replicate index forecast without dividend\n"
                    << "    actual forecast:    " << forecast << "\n"
                    << "    expected forecast:    " << expectedForecast << "\n");
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
        vars.equityIndex->clone(Handle<YieldTermStructure>(), Handle<YieldTermStructure>());

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

test_suite* EquityIndexTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Equity index tests");

    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testTodaysFixingForecast));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testFixingForecast));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testFixingForecastWithoutDividend));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testErrorWhenInvalidFixingDate));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testErrorWhenFixingMissing));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testErrorWhenInterestHandleMissing));
    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testFixingObservability));

    return suite;
}
