/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 StatPro Italia srl

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

#include "schedule.hpp"
#include "utilities.hpp"
#include <ql/time/schedule.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/japan.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    void check_dates(const Schedule& s,
                     const std::vector<Date>& expected) {
        if (s.size() != expected.size()) {
            BOOST_FAIL("expected " << expected.size() << " dates, "
                       << "found " << s.size());
        }
        for (Size i=0; i<expected.size(); ++i) {
            if (s[i] != expected[i]) {
                BOOST_ERROR("expected " << expected[i]
                            << " at index " << i << ", "
                            "found " << s[i]);
            }
        }
    }

}


void ScheduleTest::testDailySchedule() {
    BOOST_MESSAGE("Testing schedule with daily frequency...");

    Date startDate = Date(17,January,2012);

    Schedule s =
        MakeSchedule().from(startDate).to(startDate+7)
                      .withCalendar(TARGET())
                      .withFrequency(Daily)
                      .withConvention(Preceding);

    std::vector<Date> expected(6);
    // The schedule should skip Saturday 21st and Sunday 22rd.
    // Previously, it would adjust them to Friday 20th, resulting
    // in three copies of the same date.
    expected[0] = Date(17,January,2012);
    expected[1] = Date(18,January,2012);
    expected[2] = Date(19,January,2012);
    expected[3] = Date(20,January,2012);
    expected[4] = Date(23,January,2012);
    expected[5] = Date(24,January,2012);

    check_dates(s, expected);
}

void ScheduleTest::testEndDateWithEomAdjustment() {
    BOOST_MESSAGE(
        "Testing end date for schedule with end-of-month adjustment...");

    Schedule s =
        MakeSchedule().from(Date(30,September,2009))
                      .to(Date(15,June,2012))
                      .withCalendar(Japan())
                      .withTenor(6*Months)
                      .withConvention(Following)
                      .withTerminationDateConvention(Following)
                      .forwards()
                      .endOfMonth();

    std::vector<Date> expected(7);
    // The end date is adjusted, so it should also be moved to the end
    // of the month.
    expected[0] = Date(30,September,2009);
    expected[1] = Date(31,March,2010);
    expected[2] = Date(30,September,2010);
    expected[3] = Date(31,March,2011);
    expected[4] = Date(30,September,2011);
    expected[5] = Date(30,March,2012);
    expected[6] = Date(29,June,2012);

    check_dates(s, expected);

    // now with unadjusted termination date...
    s = MakeSchedule().from(Date(30,September,2009))
                      .to(Date(15,June,2012))
                      .withCalendar(Japan())
                      .withTenor(6*Months)
                      .withConvention(Following)
                      .withTerminationDateConvention(Unadjusted)
                      .forwards()
                      .endOfMonth();
    // ...which should leave it alone.
    expected[6] = Date(15,June,2012);

    check_dates(s, expected);
}


void ScheduleTest::testDatesPastEndDateWithEomAdjustment() {
    BOOST_MESSAGE(
        "Testing that no dates are past the end date with EOM adjustment...");

    Schedule s =
        MakeSchedule().from(Date(28,March,2013))
                      .to(Date(30,March,2015))
                      .withCalendar(TARGET())
                      .withTenor(1*Years)
                      .withConvention(Unadjusted)
                      .withTerminationDateConvention(Unadjusted)
                      .forwards()
                      .endOfMonth();

    std::vector<Date> expected(3);
    expected[0] = Date(31,March,2013);
    expected[1] = Date(31,March,2014);
    // March 31st 2015, coming from the EOM adjustment of March 28th,
    // should be discarded as past the end date.
    expected[2] = Date(30,March,2015);

    check_dates(s, expected);
}


test_suite* ScheduleTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Schedule tests");
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testDailySchedule));
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testEndDateWithEomAdjustment));
    suite->add(QUANTLIB_TEST_CASE(
                       &ScheduleTest::testDatesPastEndDateWithEomAdjustment));
    return suite;
}

