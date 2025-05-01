/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 StatPro Italia srl

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
#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/ibor/estr.hpp>
#include <ql/settings.hpp>
#include <ql/time/calendars/all.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/quotes/simplequote.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(OvernightIndexTest)

struct CommonVars {
    Date today;
    ext::shared_ptr<OvernightIndex> estr;
    RelinkableHandle<YieldTermStructure> forecastCurve;

    CommonVars(const Date& evaluationDate) {
        today = evaluationDate;

        Settings::instance().evaluationDate() = today;

        estr = ext::make_shared<Estr>(forecastCurve);

        
        std::vector<Date> pastDates = {
            Date(14, February, 2025), Date(17, February, 2025), Date(18, February, 2025), Date(19, February, 2025), Date(20, February, 2025),
            Date(21, February, 2025), Date(24, February, 2025), Date(25, February, 2025), Date(26, February, 2025), Date(27, February, 2025),
            Date(28, February, 2025), Date(3, March, 2025),     Date(4, March, 2025),     Date(5, March, 2025),     Date(6, March, 2025),
            Date(7, March, 2025),     Date(10, March, 2025),    Date(11, March, 2025),    Date(12, March, 2025),    Date(13, March, 2025),
            Date(14, March, 2025),    Date(17, March, 2025),    Date(18, March, 2025),    Date(19, March, 2025),    Date(20, March, 2025),
            Date(21, March, 2025),    Date(24, March, 2025),    Date(25, March, 2025),    Date(26, March, 2025),    Date(27, March, 2025),
            Date(28, March, 2025),    Date(31, March, 2025),    Date(1, April, 2025),     Date(2, April, 2025),     Date(3, April, 2025),
            Date(4, April, 2025),     Date(7, April, 2025),     Date(8, April, 2025),     Date(9, April, 2025),     Date(10, April, 2025),
            Date(11, April, 2025),    Date(14, April, 2025),    Date(15, April, 2025),    Date(16, April, 2025),    Date(17, April, 2025)
        };

        std::vector<Rate> pastRates = {
            0.02666, 0.02665, 0.02666, 0.02665, 0.02667,
            0.02666, 0.02666, 0.02666, 0.02665, 0.02666,
            0.02658, 0.02663, 0.02664, 0.02664, 0.02666,
            0.02665, 0.02663, 0.02663, 0.02412, 0.02413,
            0.02417, 0.02417, 0.02417, 0.02416, 0.02417,
            0.02417, 0.02418, 0.02418, 0.02417, 0.02417,
            0.02417, 0.02415, 0.02420, 0.02417, 0.02416,
            0.02415, 0.02414, 0.02415, 0.02415, 0.02416,
            0.02416, 0.02417, 0.02416, 0.02418, 0.02417
        };

        TimeSeries<Real> ts(pastDates.begin(), pastDates.end(), pastRates.begin());
        estr->addFixings(ts);
    }

    CommonVars() : CommonVars(Date(23, November, 2021)) {}
};

#define CHECK_OIS_RATE_RESULT(what, calculated, expected, tolerance)   \
    if (std::fabs(calculated-expected) > tolerance) { \
        BOOST_ERROR("Failed to reproduce " what ":" \
                    << "\n    expected:   " << std::setprecision(12) << expected \
                    << "\n    calculated: " << std::setprecision(12) << calculated \
                    << "\n    error:      " << std::setprecision(12) << std::fabs(calculated-expected)); \
    }


BOOST_AUTO_TEST_CASE(testCompoundedIndex1m) {
    BOOST_TEST_MESSAGE("Testing 1m rate for past overnight-indexed coupon...");

    CommonVars vars;
    Date today(22, April, 2025);
    Settings::instance().evaluationDate() = today;

    Calendar calendar = TARGET();
    DayCounter dayCounter = Actual360();

    Date fromFixingDate(14, February, 2025);
    Date toFixingDate(14, March, 2025);

    Rate compoundedRate = vars.estr->compoundedFixings(fromFixingDate, toFixingDate);

    Rate expectedRate = 0.026489361171;

    CHECK_OIS_RATE_RESULT("OIS Rate over a month", compoundedRate, expectedRate, 1e-12);
}

BOOST_AUTO_TEST_CASE(testCompoundedIndex2m) {
    BOOST_TEST_MESSAGE("Testing 2m rate for past overnight-indexed coupon...");

    CommonVars vars;
    Date today(22, April, 2025);
    Settings::instance().evaluationDate() = today;

    Calendar calendar = TARGET();
    DayCounter dayCounter = Actual360();

    Date fromFixingDate(14, February, 2025);
    Date toFixingDate(14, April, 2025);

    Rate compoundedRate = vars.estr->compoundedFixings(fromFixingDate, toFixingDate);

    Rate expectedRate = 0.02530656552467557;

    CHECK_OIS_RATE_RESULT("OIS Rate over two months: ", compoundedRate, expectedRate, 1e-12);
}

BOOST_AUTO_TEST_CASE(testCompoundedIndexOutOfRangeBefore) {
    BOOST_TEST_MESSAGE("Testing compounded fixings for a date range partially before available fixings...");

    CommonVars vars;
    Date today(22, April, 2025);
    Settings::instance().evaluationDate() = today;

    Calendar calendar = TARGET();
    DayCounter dayCounter = Actual360();

    Date fromFixingDate(11, February, 2025);
    Date toFixingDate(11, March, 2025);

    Rate compoundedRate = vars.estr->compoundedFixings(fromFixingDate, toFixingDate);

    // Expected Null<Rate>
    Rate expectedRate = Null<Rate>();

    CHECK_OIS_RATE_RESULT("OIS Rate over a month", compoundedRate, expectedRate, 1e-12);
}

BOOST_AUTO_TEST_CASE(testCompoundedIndexOutOfRangeAfter) {
    BOOST_TEST_MESSAGE("Testing compounded fixings for a date range partially after available fixings...");

    CommonVars vars;
    Date today(22, April, 2025);
    Settings::instance().evaluationDate() = today;

    Calendar calendar = TARGET();
    DayCounter dayCounter = Actual360();

    Date fromFixingDate(22, March, 2025);
    Date toFixingDate(22, April, 2025);

    Rate compoundedRate = vars.estr->compoundedFixings(fromFixingDate, toFixingDate);

    // Expected Null<Rate>
    Rate expectedRate = Null<Rate>();

    CHECK_OIS_RATE_RESULT("OIS Rate over a month", compoundedRate, expectedRate, 1e-12);
}

BOOST_AUTO_TEST_CASE(testCompoundedFactor) {
    BOOST_TEST_MESSAGE("Testing compoundFactor at the end of available fixings...");

    CommonVars vars;
    Date today(22, April, 2025);
    Settings::instance().evaluationDate() = today;

    Calendar calendar = TARGET();
    DayCounter dayCounter = Actual360();

    Date fromFixingDate(14, February, 2025);
    Date toFixingDate(17, March, 2025);

    Real compoundedFactor = vars.estr->compoundedFactor(fromFixingDate, toFixingDate);

    Real expectedCompounedFactor = 1.002262115288;

    CHECK_OIS_RATE_RESULT("OIS Rate over a month", compoundedFactor, expectedCompounedFactor, 1e-12);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()