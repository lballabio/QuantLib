
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
#include <ql/dataformatters.hpp>
#include <ql/calendar.hpp>
#include <ql/Calendars/germany.hpp>
#include <ql/Calendars/italy.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/Calendars/unitedkingdom.hpp>
#include <ql/Calendars/unitedstates.hpp>
#include <ql/Calendars/tokyo.hpp>
#include <ql/Calendars/jointcalendar.hpp>
#include <ql/Functions/calendars.hpp>
#include <fstream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void CalendarTest::testModifiedCalendars() {

    BOOST_MESSAGE("Testing calendar modification...");

    Calendar c1 = TARGET();
    Calendar c2 = UnitedStates(UnitedStates::Exchange);
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
}


void CalendarTest::testJointCalendars() {

    BOOST_MESSAGE("Testing joint calendars...");

    Calendar c1 = TARGET(),
             c2 = UnitedKingdom(),
             c3 = UnitedStates(UnitedStates::Exchange),
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

void CalendarTest::testUSSettlement() {
    BOOST_MESSAGE("Testing US settlement holiday list...");

    std::vector<Date> expectedHol;
    expectedHol.push_back(Date(1,January,2004));
    expectedHol.push_back(Date(19,January,2004));
    expectedHol.push_back(Date(16,February,2004));
    expectedHol.push_back(Date(9,April,2004));
    expectedHol.push_back(Date(31,May,2004));
    expectedHol.push_back(Date(5,July,2004));
    expectedHol.push_back(Date(6,September,2004));
    expectedHol.push_back(Date(11,October,2004));
    expectedHol.push_back(Date(11,November,2004));
    expectedHol.push_back(Date(25,November,2004));
    expectedHol.push_back(Date(24,December,2004));
    expectedHol.push_back(Date(31,December,2004));

    Calendar c = UnitedStates(UnitedStates::Settlement);
    std::vector<Date> hol = holidayList(c, Date(1,January,2004),
                                           Date(31,December,2004));
    for (Size i=0; i<QL_MIN<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was "
                       + DateFormatter::toString(expectedHol[i]) +
                       " while calculated holiday is "
                       + DateFormatter::toString(hol[i]));
    }
    if (hol.size()!=expectedHol.size())
            BOOST_FAIL("there were "
                + SizeFormatter::toString(expectedHol.size()) +
                " expected holidays, while there are "
                + SizeFormatter::toString(hol.size()) +
                " calculated holidays");
}

void CalendarTest::testUSGovernmentBondMarket() {
    BOOST_MESSAGE("Testing US Government Bond Market holiday list...");

    std::vector<Date> expectedHol;
    expectedHol.push_back(Date(1,January,2004));
    expectedHol.push_back(Date(19,January,2004));
    expectedHol.push_back(Date(16,February,2004));
    expectedHol.push_back(Date(9,April,2004));
    expectedHol.push_back(Date(31,May,2004));
    expectedHol.push_back(Date(5,July,2004));
    expectedHol.push_back(Date(6,September,2004));
    expectedHol.push_back(Date(11,October,2004));
    expectedHol.push_back(Date(11,November,2004));
    expectedHol.push_back(Date(25,November,2004));
    expectedHol.push_back(Date(24,December,2004));

    Calendar c = UnitedStates(UnitedStates::GovernmentBond);
    std::vector<Date> hol = holidayList(c, Date(1,January,2004),
                                           Date(31,December,2004));

    for (Size i=0; i<QL_MIN<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was "
                       + DateFormatter::toString(expectedHol[i]) +
                       " while calculated holiday is "
                       + DateFormatter::toString(hol[i]));
    }
    if (hol.size()!=expectedHol.size())
            BOOST_FAIL("there were "
                + SizeFormatter::toString(expectedHol.size()) +
                " expected holidays, while there are "
                + SizeFormatter::toString(hol.size()) +
                " calculated holidays");
}

void CalendarTest::testUSNewYorkStockExchange() {
    BOOST_MESSAGE("Testing US New York Stock Exchange holiday list...");

    std::vector<Date> expectedHol;
    expectedHol.push_back(Date(1,January,2004));
    expectedHol.push_back(Date(19,January,2004));
    expectedHol.push_back(Date(16,February,2004));
    expectedHol.push_back(Date(9,April,2004));
    expectedHol.push_back(Date(31,May,2004));
    expectedHol.push_back(Date(5,July,2004));
    expectedHol.push_back(Date(6,September,2004));
    expectedHol.push_back(Date(25,November,2004));
    expectedHol.push_back(Date(24,December,2004));

    expectedHol.push_back(Date(17,January,2005));
    expectedHol.push_back(Date(21,February,2005));
    expectedHol.push_back(Date(25,March,2005));
    expectedHol.push_back(Date(30,May,2005));
    expectedHol.push_back(Date(4,July,2005));
    expectedHol.push_back(Date(5,September,2005));
    expectedHol.push_back(Date(24,November,2005));
    expectedHol.push_back(Date(26,December,2005));

    expectedHol.push_back(Date(2,January,2006));
    expectedHol.push_back(Date(16,January,2006));
    expectedHol.push_back(Date(20,February,2006));
    expectedHol.push_back(Date(14,April,2006));
    expectedHol.push_back(Date(29,May,2006));
    expectedHol.push_back(Date(4,July,2006));
    expectedHol.push_back(Date(4,September,2006));
    expectedHol.push_back(Date(23,November,2006));
    expectedHol.push_back(Date(25,December,2006));

    Calendar c = UnitedStates(UnitedStates::Exchange);
    std::vector<Date> hol = holidayList(c, Date(1,January,2004),
                                           Date(31,December,2006));

    for (Size i=0; i<QL_MIN<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was "
                       + DateFormatter::toString(expectedHol[i]) +
                       " while calculated holiday is "
                       + DateFormatter::toString(hol[i]));
    }
    if (hol.size()!=expectedHol.size())
            BOOST_FAIL("there were "
                + SizeFormatter::toString(expectedHol.size()) +
                " expected holidays, while there are "
                + SizeFormatter::toString(hol.size()) +
                " calculated holidays");

}

void CalendarTest::testTARGET() {
    BOOST_MESSAGE("Testing TARGET holiday list...");

    std::vector<Date> expectedHol;
    expectedHol.push_back(Date(1,January,1999));
    expectedHol.push_back(Date(31,December,1999));

    expectedHol.push_back(Date(21,April,2000));
    expectedHol.push_back(Date(24,April,2000));
    expectedHol.push_back(Date(1,May,2000));
    expectedHol.push_back(Date(25,December,2000));
    expectedHol.push_back(Date(26,December,2000));

    expectedHol.push_back(Date(1,January,2001));
    expectedHol.push_back(Date(13,April,2001));
    expectedHol.push_back(Date(16,April,2001));
    expectedHol.push_back(Date(1,May,2001));
    expectedHol.push_back(Date(25,December,2001));
    expectedHol.push_back(Date(26,December,2001));
    expectedHol.push_back(Date(31,December,2001));

    expectedHol.push_back(Date(1,January,2002));
    expectedHol.push_back(Date(29,March,2002));
    expectedHol.push_back(Date(1,April,2002));
    expectedHol.push_back(Date(1,May,2002));
    expectedHol.push_back(Date(25,December,2002));
    expectedHol.push_back(Date(26,December,2002));

    expectedHol.push_back(Date(1,January,2003));
    expectedHol.push_back(Date(18,April,2003));
    expectedHol.push_back(Date(21,April,2003));
    expectedHol.push_back(Date(1,May,2003));
    expectedHol.push_back(Date(25,December,2003));
    expectedHol.push_back(Date(26,December,2003));

    expectedHol.push_back(Date(1,January,2004));
    expectedHol.push_back(Date(9,April,2004));
    expectedHol.push_back(Date(12,April,2004));

    expectedHol.push_back(Date(25,March,2005));
    expectedHol.push_back(Date(28,March,2005));
    expectedHol.push_back(Date(26,December,2005));

    expectedHol.push_back(Date(14,April,2006));
    expectedHol.push_back(Date(17,April,2006));
    expectedHol.push_back(Date(1,May,2006));
    expectedHol.push_back(Date(25,December,2006));
    expectedHol.push_back(Date(26,December,2006));

    Calendar c = TARGET();
    std::vector<Date> hol = holidayList(c, Date(1,January,1999),
                                           Date(31,December,2006));

    for (Size i=0; i<QL_MIN<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was "
                       + DateFormatter::toString(expectedHol[i]) +
                       " while calculated holiday is "
                       + DateFormatter::toString(hol[i]));
    }
    if (hol.size()!=expectedHol.size())
            BOOST_FAIL("there were "
                + SizeFormatter::toString(expectedHol.size()) +
                " expected holidays, while there are "
                + SizeFormatter::toString(hol.size()) +
                " calculated holidays");

}

void CalendarTest::testGermanyFrankfurt() {
    BOOST_MESSAGE("Testing Germany Frankfurt Stock Exchange holiday list...");

    std::vector<Date> expectedHol;

    expectedHol.push_back(Date(1,January,2003));
    expectedHol.push_back(Date(18,April,2003));
    expectedHol.push_back(Date(21,April,2003));
    expectedHol.push_back(Date(1,May,2003));
    expectedHol.push_back(Date(24,December,2003));
    expectedHol.push_back(Date(25,December,2003));
    expectedHol.push_back(Date(26,December,2003));
    expectedHol.push_back(Date(31,December,2003));

    expectedHol.push_back(Date(1,January,2004));
    expectedHol.push_back(Date(9,April,2004));
    expectedHol.push_back(Date(12,April,2004));
    expectedHol.push_back(Date(24,December,2004));
    expectedHol.push_back(Date(31,December,2004));

    Calendar c = Germany(Germany::FrankfurtStockExchange);
    std::vector<Date> hol = holidayList(c, Date(1,January,2003),
                                           Date(31,December,2004));
    for (Size i=0; i<QL_MIN<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was "
                       + DateFormatter::toString(expectedHol[i]) +
                       " while calculated holiday is "
                       + DateFormatter::toString(hol[i]));
    }
    if (hol.size()!=expectedHol.size())
            BOOST_FAIL("there were "
                + SizeFormatter::toString(expectedHol.size()) +
                " expected holidays, while there are "
                + SizeFormatter::toString(hol.size()) +
                " calculated holidays");
}

void CalendarTest::testGermanyEurex() {
    BOOST_MESSAGE("Testing Germany Eurex holiday list...");

    std::vector<Date> expectedHol;

    expectedHol.push_back(Date(1,January,2003));
    expectedHol.push_back(Date(18,April,2003));
    expectedHol.push_back(Date(21,April,2003));
    expectedHol.push_back(Date(1,May,2003));
    expectedHol.push_back(Date(24,December,2003));
    expectedHol.push_back(Date(25,December,2003));
    expectedHol.push_back(Date(26,December,2003));
    expectedHol.push_back(Date(31,December,2003));

    expectedHol.push_back(Date(1,January,2004));
    expectedHol.push_back(Date(9,April,2004));
    expectedHol.push_back(Date(12,April,2004));
    expectedHol.push_back(Date(24,December,2004));
    expectedHol.push_back(Date(31,December,2004));

    Calendar c = Germany(Germany::Eurex);
    std::vector<Date> hol = holidayList(c, Date(1,January,2003),
                                           Date(31,December,2004));
    for (Size i=0; i<QL_MIN<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was "
                       + DateFormatter::toString(expectedHol[i]) +
                       " while calculated holiday is "
                       + DateFormatter::toString(hol[i]));
    }
    if (hol.size()!=expectedHol.size())
            BOOST_FAIL("there were "
                + SizeFormatter::toString(expectedHol.size()) +
                " expected holidays, while there are "
                + SizeFormatter::toString(hol.size()) +
                " calculated holidays");
}

void CalendarTest::testGermanyXetra() {
    BOOST_MESSAGE("Testing Germany Xetra holiday list...");

    std::vector<Date> expectedHol;

    expectedHol.push_back(Date(1,January,2003));
    expectedHol.push_back(Date(18,April,2003));
    expectedHol.push_back(Date(21,April,2003));
    expectedHol.push_back(Date(1,May,2003));
    expectedHol.push_back(Date(24,December,2003));
    expectedHol.push_back(Date(25,December,2003));
    expectedHol.push_back(Date(26,December,2003));
    expectedHol.push_back(Date(31,December,2003));

    expectedHol.push_back(Date(1,January,2004));
    expectedHol.push_back(Date(9,April,2004));
    expectedHol.push_back(Date(12,April,2004));
    expectedHol.push_back(Date(24,December,2004));
    expectedHol.push_back(Date(31,December,2004));

    Calendar c = Germany(Germany::Xetra);
    std::vector<Date> hol = holidayList(c, Date(1,January,2003),
                                           Date(31,December,2004));
    for (Size i=0; i<QL_MIN<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was "
                       + DateFormatter::toString(expectedHol[i]) +
                       " while calculated holiday is "
                       + DateFormatter::toString(hol[i]));
    }
    if (hol.size()!=expectedHol.size())
            BOOST_FAIL("there were "
                + SizeFormatter::toString(expectedHol.size()) +
                " expected holidays, while there are "
                + SizeFormatter::toString(hol.size()) +
                " calculated holidays");
}

test_suite* CalendarTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Calendar tests");

//    suite->add(BOOST_TEST_CASE(&CalendarTest::testGermanySettlement));
    suite->add(BOOST_TEST_CASE(&CalendarTest::testGermanyFrankfurt));
    suite->add(BOOST_TEST_CASE(&CalendarTest::testGermanyXetra));
    suite->add(BOOST_TEST_CASE(&CalendarTest::testGermanyEurex));

    suite->add(BOOST_TEST_CASE(&CalendarTest::testTARGET));

    suite->add(BOOST_TEST_CASE(&CalendarTest::testUSSettlement));
    suite->add(BOOST_TEST_CASE(&CalendarTest::testUSGovernmentBondMarket));
    suite->add(BOOST_TEST_CASE(&CalendarTest::testUSNewYorkStockExchange));

    suite->add(BOOST_TEST_CASE(&CalendarTest::testModifiedCalendars));
    suite->add(BOOST_TEST_CASE(&CalendarTest::testJointCalendars));

    return suite;
}

