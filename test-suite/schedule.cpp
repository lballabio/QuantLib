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

using namespace QuantLib;
using namespace boost::unit_test_framework;

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

    if (s.size() != expected.size()) {
        BOOST_FAIL("expected " << expected.size() << " dates, "
                   << "found " << s.size());
    }
    for (Size i=0; i<expected.size(); ++i) {
        if (s[i] != expected[i]) {
            BOOST_ERROR("expected " << expected[i] << " at index " << i << ", "
                        "found " << s[i]);
        }
    }
}


test_suite* ScheduleTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Schedule tests");
    suite->add(QUANTLIB_TEST_CASE(&ScheduleTest::testDailySchedule));
    return suite;
}

