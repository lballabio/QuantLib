
/*
 Copyright (C) 2003, 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "calendars.hpp"
#include <ql/calendar.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/Calendars/london.hpp>
#include <ql/Calendars/newyork.hpp>
#include <ql/Calendars/tokyo.hpp>
#include <ql/Calendars/jointcalendar.hpp>
#include <ql/dataformatters.hpp>
#include <fstream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void CalendarTest::testModifiedCalendars() {

    BOOST_MESSAGE("Testing calendar modification...");

    Calendar c1 = TARGET();
    Calendar c2 = NewYork();
    Date d1(1,May,2004);      // holiday for both calendars
    Date d2(26,April,2004);   // business day

    QL_REQUIRE(c1.isHoliday(d1), "wrong assumption---correct the test");
    QL_REQUIRE(c1.isBusinessDay(d2), "wrong assumption---correct the test");

    QL_REQUIRE(c2.isHoliday(d1), "wrong assumption---correct the test");
    QL_REQUIRE(c2.isBusinessDay(d2), "wrong assumption---correct the test");

    // modify the TARGET calendar
    c1.removeHoliday(d1);
    c1.addHoliday(d2);

    // test
    if (c1.isHoliday(d1))
        BOOST_FAIL(DateFormatter::toString(d1) + 
                   " still a holiday for original TARGET instance");
    if (c1.isBusinessDay(d2))
        BOOST_FAIL(DateFormatter::toString(d2) + 
                   " still a business day for original TARGET instance");

    // any instance of TARGET should be modified...
    Calendar c3 = TARGET();
    if (c3.isHoliday(d1))
        BOOST_FAIL(DateFormatter::toString(d1) + 
                   " still a holiday for generic TARGET instance");
    if (c3.isBusinessDay(d2))
        BOOST_FAIL(DateFormatter::toString(d2) + 
                   " still a business day for generic TARGET instance");

    // ...but not other calendars
    if (c2.isBusinessDay(d1))
        BOOST_FAIL(DateFormatter::toString(d1) + " business day for New York");
    if (c2.isHoliday(d2))
        BOOST_FAIL(DateFormatter::toString(d2) + " holiday for New York");

    // restore original holiday set---test the other way around
    c3.addHoliday(d1);
    c3.removeHoliday(d2);

    if (c1.isBusinessDay(d1))
        BOOST_FAIL(DateFormatter::toString(d1) + " still a business day");
    if (c1.isHoliday(d2))
        BOOST_FAIL(DateFormatter::toString(d2) + " still a holiday");

    #if !defined(QL_PATCH_MICROSOFT)

    // repeat, loading from file

    std::ofstream f1("calendars.dat");
    f1 << "TARGET: - "
       << DateFormatter::toString(d1, DateFormatter::ISO)
       << "    # a comment"
       << std::endl;
    f1 << std::endl;
    f1 << "TARGET: + " 
       << DateFormatter::toString(d2, DateFormatter::ISO)
       << std::endl;
    f1.close();
    c1.load("calendars.dat");

    if (c3.isHoliday(d1))
        BOOST_FAIL(DateFormatter::toString(d1) + " holiday for TARGET");
    if (c3.isBusinessDay(d2))
        BOOST_FAIL(DateFormatter::toString(d2) + " business day for TARGET");

    if (c2.isBusinessDay(d1))
        BOOST_FAIL(DateFormatter::toString(d1) + " business day for New York");
    if (c2.isHoliday(d2))
        BOOST_FAIL(DateFormatter::toString(d2) + " holiday for New York");

    std::ofstream f2("calendars.dat");
    f2 << "    # another comment" << std::endl;
    f2 << "TARGET: + " 
       << DateFormatter::toString(d1, DateFormatter::ISO)
       << std::endl;
    f2 << "TARGET: - " 
       << DateFormatter::toString(d2, DateFormatter::ISO)
       << std::endl;
    f2.close();
    c3.load("calendars.dat");

    if (c1.isBusinessDay(d1))
        BOOST_FAIL(DateFormatter::toString(d1) + " business day for TARGET");
    if (c1.isHoliday(d2))
        BOOST_FAIL(DateFormatter::toString(d2) + " holiday for TARGET");

    #endif
}


void CalendarTest::testJointCalendars() {

    BOOST_MESSAGE("Testing joint calendars...");

    Calendar c1 = TARGET(),
             c2 = London(),
             c3 = NewYork(),
             c4 = Tokyo();

    Calendar c12h = JointCalendar(c1,c2,JoinHolidays),
             c12b = JointCalendar(c1,c2,JoinBusinessDays),
             c123h = JointCalendar(c1,c2,c3,JoinHolidays),
             c123b = JointCalendar(c1,c2,c3,JoinBusinessDays),
             c1234h = JointCalendar(c1,c2,c3,c4,JoinHolidays),
             c1234b = JointCalendar(c1,c2,c3,c4,JoinBusinessDays);

    // test one year, starting today
    Date firstDate = Date::todaysDate(),
         endDate = firstDate.plusYears(1);

    for (Date d = firstDate; d < endDate; d++) {

        bool b1 = c1.isBusinessDay(d),
             b2 = c2.isBusinessDay(d),
             b3 = c3.isBusinessDay(d),
             b4 = c4.isBusinessDay(d);

        if ((b1 && b2) != c12h.isBusinessDay(d))
            BOOST_FAIL(
                "At date " + DateFormatter::toString(d) + ":\n"
                "    inconsistency between joint calendar "
                + c12h.name() + " (joining holidays)\n"
                "    and its components");

        if ((b1 || b2) != c12b.isBusinessDay(d))
            BOOST_FAIL(
                "At date " + DateFormatter::toString(d) + ":\n"
                "    inconsistency between joint calendar "
                + c12b.name() + " (joining business days)\n"
                "    and its components");

        if ((b1 && b2 && b3) != c123h.isBusinessDay(d))
            BOOST_FAIL(
                "At date " + DateFormatter::toString(d) + ":\n"
                "    inconsistency between joint calendar "
                + c123h.name() + " (joining holidays)\n"
                "    and its components");

        if ((b1 || b2 || b3) != c123b.isBusinessDay(d))
            BOOST_FAIL(
                "At date " + DateFormatter::toString(d) + ":\n"
                "    inconsistency between joint calendar "
                + c123b.name() + " (joining business days)\n"
                "    and its components");

        if ((b1 && b2 && b3 && b4) != c1234h.isBusinessDay(d))
            BOOST_FAIL(
                "At date " + DateFormatter::toString(d) + ":\n"
                "    inconsistency between joint calendar "
                + c1234h.name() + " (joining holidays)\n"
                "    and its components");

        if ((b1 || b2 || b3 || b4) != c1234b.isBusinessDay(d))
            BOOST_FAIL(
                "At date " + DateFormatter::toString(d) + ":\n"
                "    inconsistency between joint calendar "
                + c1234b.name() + " (joining business days)\n"
                "    and its components");

    }
}


test_suite* CalendarTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Calendar tests");
    suite->add(BOOST_TEST_CASE(&CalendarTest::testModifiedCalendars));
    suite->add(BOOST_TEST_CASE(&CalendarTest::testJointCalendars));
    return suite;
}

