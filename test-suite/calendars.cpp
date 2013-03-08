/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2008 StatPro Italia srl
 Copyright (C) 2005 Ferdinando Ametrano
 Copyright (C) 2006 Piter Dias
 Copyright (C) 2008 Charles Chongseok Hyun

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

#include "calendars.hpp"
#include "utilities.hpp"
#include <ql/time/calendar.hpp>
#include <ql/time/calendars/brazil.hpp>
#include <ql/time/calendars/germany.hpp>
#include <ql/time/calendars/italy.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/calendars/japan.hpp>
#include <ql/time/calendars/southkorea.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/calendars/bespokecalendar.hpp>
#include <ql/errors.hpp>
#include <fstream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void CalendarTest::testModifiedCalendars() {

    BOOST_TEST_MESSAGE("Testing calendar modification...");

    Calendar c1 = TARGET();
    Calendar c2 = UnitedStates(UnitedStates::NYSE);
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
        BOOST_FAIL(d1 << " still a holiday for original TARGET instance");
    if (c1.isBusinessDay(d2))
        BOOST_FAIL(d2 << " still a business day for original TARGET instance");

    // any instance of TARGET should be modified...
    Calendar c3 = TARGET();
    if (c3.isHoliday(d1))
        BOOST_FAIL(d1 << " still a holiday for generic TARGET instance");
    if (c3.isBusinessDay(d2))
        BOOST_FAIL(d2 << " still a business day for generic TARGET instance");

    // ...but not other calendars
    if (c2.isBusinessDay(d1))
        BOOST_FAIL(d1 << " business day for New York");
    if (c2.isHoliday(d2))
        BOOST_FAIL(d2 << " holiday for New York");

    // restore original holiday set---test the other way around
    c3.addHoliday(d1);
    c3.removeHoliday(d2);

    if (c1.isBusinessDay(d1))
        BOOST_FAIL(d1 << " still a business day");
    if (c1.isHoliday(d2))
        BOOST_FAIL(d2 << " still a holiday");
}


void CalendarTest::testJointCalendars() {

    BOOST_TEST_MESSAGE("Testing joint calendars...");

    Calendar c1 = TARGET(),
             c2 = UnitedKingdom(),
             c3 = UnitedStates(UnitedStates::NYSE),
             c4 = Japan();

    Calendar c12h = JointCalendar(c1,c2,JoinHolidays),
             c12b = JointCalendar(c1,c2,JoinBusinessDays),
             c123h = JointCalendar(c1,c2,c3,JoinHolidays),
             c123b = JointCalendar(c1,c2,c3,JoinBusinessDays),
             c1234h = JointCalendar(c1,c2,c3,c4,JoinHolidays),
             c1234b = JointCalendar(c1,c2,c3,c4,JoinBusinessDays);

    // test one year, starting today
    Date firstDate = Date::todaysDate(),
         endDate = firstDate + 1*Years;

    for (Date d = firstDate; d < endDate; d++) {

        bool b1 = c1.isBusinessDay(d),
             b2 = c2.isBusinessDay(d),
             b3 = c3.isBusinessDay(d),
             b4 = c4.isBusinessDay(d);

        if ((b1 && b2) != c12h.isBusinessDay(d))
            BOOST_FAIL("At date " << d << ":\n"
                       << "    inconsistency between joint calendar "
                       << c12h.name() << " (joining holidays)\n"
                       << "    and its components");

        if ((b1 || b2) != c12b.isBusinessDay(d))
            BOOST_FAIL("At date " << d << ":\n"
                       << "    inconsistency between joint calendar "
                       << c12b.name() << " (joining business days)\n"
                       << "    and its components");

        if ((b1 && b2 && b3) != c123h.isBusinessDay(d))
            BOOST_FAIL("At date " << d << ":\n"
                       << "    inconsistency between joint calendar "
                       << c123h.name() << " (joining holidays)\n"
                       << "    and its components");

        if ((b1 || b2 || b3) != c123b.isBusinessDay(d))
            BOOST_FAIL("At date " << d << ":\n"
                       << "    inconsistency between joint calendar "
                       << c123b.name() << " (joining business days)\n"
                       << "    and its components");

        if ((b1 && b2 && b3 && b4) != c1234h.isBusinessDay(d))
            BOOST_FAIL("At date " << d << ":\n"
                       << "    inconsistency between joint calendar "
                       << c1234h.name() << " (joining holidays)\n"
                       << "    and its components");

        if ((b1 || b2 || b3 || b4) != c1234b.isBusinessDay(d))
            BOOST_FAIL("At date " << d << ":\n"
                       << "    inconsistency between joint calendar "
                       << c1234b.name() << " (joining business days)\n"
                       << "    and its components");

    }
}

void CalendarTest::testUSSettlement() {
    BOOST_TEST_MESSAGE("Testing US settlement holiday list...");

    std::vector<Date> expectedHol;
    expectedHol.push_back(Date(1,January,2004));
    expectedHol.push_back(Date(19,January,2004));
    expectedHol.push_back(Date(16,February,2004));
    expectedHol.push_back(Date(31,May,2004));
    expectedHol.push_back(Date(5,July,2004));
    expectedHol.push_back(Date(6,September,2004));
    expectedHol.push_back(Date(11,October,2004));
    expectedHol.push_back(Date(11,November,2004));
    expectedHol.push_back(Date(25,November,2004));
    expectedHol.push_back(Date(24,December,2004));

    expectedHol.push_back(Date(31,December,2004));
    expectedHol.push_back(Date(17,January,2005));
    expectedHol.push_back(Date(21,February,2005));
    expectedHol.push_back(Date(30,May,2005));
    expectedHol.push_back(Date(4,July,2005));
    expectedHol.push_back(Date(5,September,2005));
    expectedHol.push_back(Date(10,October,2005));
    expectedHol.push_back(Date(11,November,2005));
    expectedHol.push_back(Date(24,November,2005));
    expectedHol.push_back(Date(26,December,2005));

    Calendar c = UnitedStates(UnitedStates::Settlement);
    std::vector<Date> hol = Calendar::holidayList(c, Date( 1, January, 2004),
                                                     Date(31,December, 2005));
    for (Size i=0; i<std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i]
                       << " while calculated holiday is " << hol[i]);
    }
    if (hol.size()!=expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size()
                   << " expected holidays, while there are " << hol.size()
                   << " calculated holidays");
}

void CalendarTest::testUSGovernmentBondMarket() {
    BOOST_TEST_MESSAGE("Testing US government bond market holiday list...");

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
    std::vector<Date> hol = Calendar::holidayList(c, Date(1,January,2004),
                                                     Date(31,December,2004));

    for (Size i=0; i<std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i]
                       << " while calculated holiday is " << hol[i]);
    }
    if (hol.size()!=expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size()
                   << " expected holidays, while there are " << hol.size()
                   << " calculated holidays");
}

void CalendarTest::testUSNewYorkStockExchange() {
    BOOST_TEST_MESSAGE("Testing New York Stock Exchange holiday list...");

    std::vector<Date> expectedHol;
    expectedHol.push_back(Date(1,January,2004));
    expectedHol.push_back(Date(19,January,2004));
    expectedHol.push_back(Date(16,February,2004));
    expectedHol.push_back(Date(9,April,2004));
    expectedHol.push_back(Date(31,May,2004));
    expectedHol.push_back(Date(11,June,2004));
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

    Calendar c = UnitedStates(UnitedStates::NYSE);
    std::vector<Date> hol = Calendar::holidayList(c, Date(1,January,2004),
                                                     Date(31,December,2006));

    Size i;
    for (i=0; i<std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i]
                       << " while calculated holiday is " << hol[i]);
    }
    if (hol.size()!=expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size()
                   << " expected holidays, while there are " << hol.size()
                   << " calculated holidays");

    std::vector<Date> histClose;
    histClose.push_back(Date(11,June,2004));     // Reagan's funeral
    histClose.push_back(Date(14,September,2001));// September 11, 2001
    histClose.push_back(Date(13,September,2001));// September 11, 2001
    histClose.push_back(Date(12,September,2001));// September 11, 2001
    histClose.push_back(Date(11,September,2001));// September 11, 2001
    histClose.push_back(Date(14,July,1977));     // 1977 Blackout
    histClose.push_back(Date(25,January,1973));  // Johnson's funeral.
    histClose.push_back(Date(28,December,1972)); // Truman's funeral
    histClose.push_back(Date(21,July,1969));     // Lunar exploration nat. day
    histClose.push_back(Date(31,March,1969));    // Eisenhower's funeral
    histClose.push_back(Date(10,February,1969)); // heavy snow
    histClose.push_back(Date(5,July,1968));      // Day after Independence Day
    // June 12-Dec. 31, 1968
    // Four day week (closed on Wednesdays) - Paperwork Crisis
    histClose.push_back(Date(12,Jun,1968));
    histClose.push_back(Date(19,Jun,1968));
    histClose.push_back(Date(26,Jun,1968));
    histClose.push_back(Date(3,Jul,1968 ));
    histClose.push_back(Date(10,Jul,1968));
    histClose.push_back(Date(17,Jul,1968));
    histClose.push_back(Date(20,Nov,1968));
    histClose.push_back(Date(27,Nov,1968));
    histClose.push_back(Date(4,Dec,1968 ));
    histClose.push_back(Date(11,Dec,1968));
    histClose.push_back(Date(18,Dec,1968));
    // Presidential election days
    histClose.push_back(Date(4,Nov,1980));
    histClose.push_back(Date(2,Nov,1976));
    histClose.push_back(Date(7,Nov,1972));
    histClose.push_back(Date(5,Nov,1968));
    histClose.push_back(Date(3,Nov,1964));
    for (i=0; i<histClose.size(); i++) {
        if (!c.isHoliday(histClose[i]))
            BOOST_FAIL(histClose[i]
                       << " should be holiday (historical close)");
    }


}

void CalendarTest::testTARGET() {
    BOOST_TEST_MESSAGE("Testing TARGET holiday list...");

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
    std::vector<Date> hol = Calendar::holidayList(c, Date(1,January,1999),
                                                     Date(31,December,2006));

    for (Size i=0; i<std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i]
                       << " while calculated holiday is " << hol[i]);
    }
    if (hol.size()!=expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size()
                   << " expected holidays, while there are " << hol.size()
                   << " calculated holidays");

}

void CalendarTest::testGermanyFrankfurt() {
    BOOST_TEST_MESSAGE("Testing Frankfurt Stock Exchange holiday list...");

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
    std::vector<Date> hol = Calendar::holidayList(c, Date(1,January,2003),
                                                     Date(31,December,2004));
    for (Size i=0; i<std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i]
                       << " while calculated holiday is " << hol[i]);
    }
    if (hol.size()!=expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size()
                   << " expected holidays, while there are " << hol.size()
                   << " calculated holidays");
}

void CalendarTest::testGermanyEurex() {
    BOOST_TEST_MESSAGE("Testing Eurex holiday list...");

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
    std::vector<Date> hol = Calendar::holidayList(c, Date(1,January,2003),
                                                     Date(31,December,2004));
    for (Size i=0; i<std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i]
                       << " while calculated holiday is " << hol[i]);
    }
    if (hol.size()!=expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size()
                   << " expected holidays, while there are " << hol.size()
                   << " calculated holidays");
}

void CalendarTest::testGermanyXetra() {
    BOOST_TEST_MESSAGE("Testing Xetra holiday list...");

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
    std::vector<Date> hol = Calendar::holidayList(c, Date(1,January,2003),
                                                     Date(31,December,2004));
    for (Size i=0; i<std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i]
                       << " while calculated holiday is " << hol[i]);
    }
    if (hol.size()!=expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size()
                   << " expected holidays, while there are " << hol.size()
                   << " calculated holidays");
}

void CalendarTest::testUKSettlement() {
    BOOST_TEST_MESSAGE("Testing UK settlement holiday list...");

    std::vector<Date> expectedHol;

    expectedHol.push_back(Date(1,January,2004));
    expectedHol.push_back(Date(9,April,2004));
    expectedHol.push_back(Date(12,April,2004));
    expectedHol.push_back(Date(3,May,2004));
    expectedHol.push_back(Date(31,May,2004));
    expectedHol.push_back(Date(30,August,2004));
    expectedHol.push_back(Date(27,December,2004));
    expectedHol.push_back(Date(28,December,2004));

    expectedHol.push_back(Date(3,January,2005));
    expectedHol.push_back(Date(25,March,2005));
    expectedHol.push_back(Date(28,March,2005));
    expectedHol.push_back(Date(2,May,2005));
    expectedHol.push_back(Date(30,May,2005));
    expectedHol.push_back(Date(29,August,2005));
    expectedHol.push_back(Date(26,December,2005));
    expectedHol.push_back(Date(27,December,2005));

    expectedHol.push_back(Date(2,January,2006));
    expectedHol.push_back(Date(14,April,2006));
    expectedHol.push_back(Date(17,April,2006));
    expectedHol.push_back(Date(1,May,2006));
    expectedHol.push_back(Date(29,May,2006));
    expectedHol.push_back(Date(28,August,2006));
    expectedHol.push_back(Date(25,December,2006));
    expectedHol.push_back(Date(26,December,2006));

    expectedHol.push_back(Date(1,January,2007));
    expectedHol.push_back(Date(6,April,2007));
    expectedHol.push_back(Date(9,April,2007));
    expectedHol.push_back(Date(7,May,2007));
    expectedHol.push_back(Date(28,May,2007));
    expectedHol.push_back(Date(27,August,2007));
    expectedHol.push_back(Date(25,December,2007));
    expectedHol.push_back(Date(26,December,2007));

    Calendar c = UnitedKingdom(UnitedKingdom::Settlement);
    std::vector<Date> hol = Calendar::holidayList(c, Date(1,January,2004),
                                                     Date(31,December,2007));
    for (Size i=0; i<std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i]
                       << " while calculated holiday is " << hol[i]);
    }
    if (hol.size()!=expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size()
                   << " expected holidays, while there are " << hol.size()
                   << " calculated holidays");
}

void CalendarTest::testUKExchange() {
    BOOST_TEST_MESSAGE("Testing London Stock Exchange holiday list...");

    std::vector<Date> expectedHol;

    expectedHol.push_back(Date(1,January,2004));
    expectedHol.push_back(Date(9,April,2004));
    expectedHol.push_back(Date(12,April,2004));
    expectedHol.push_back(Date(3,May,2004));
    expectedHol.push_back(Date(31,May,2004));
    expectedHol.push_back(Date(30,August,2004));
    expectedHol.push_back(Date(27,December,2004));
    expectedHol.push_back(Date(28,December,2004));

    expectedHol.push_back(Date(3,January,2005));
    expectedHol.push_back(Date(25,March,2005));
    expectedHol.push_back(Date(28,March,2005));
    expectedHol.push_back(Date(2,May,2005));
    expectedHol.push_back(Date(30,May,2005));
    expectedHol.push_back(Date(29,August,2005));
    expectedHol.push_back(Date(26,December,2005));
    expectedHol.push_back(Date(27,December,2005));

    expectedHol.push_back(Date(2,January,2006));
    expectedHol.push_back(Date(14,April,2006));
    expectedHol.push_back(Date(17,April,2006));
    expectedHol.push_back(Date(1,May,2006));
    expectedHol.push_back(Date(29,May,2006));
    expectedHol.push_back(Date(28,August,2006));
    expectedHol.push_back(Date(25,December,2006));
    expectedHol.push_back(Date(26,December,2006));

    expectedHol.push_back(Date(1,January,2007));
    expectedHol.push_back(Date(6,April,2007));
    expectedHol.push_back(Date(9,April,2007));
    expectedHol.push_back(Date(7,May,2007));
    expectedHol.push_back(Date(28,May,2007));
    expectedHol.push_back(Date(27,August,2007));
    expectedHol.push_back(Date(25,December,2007));
    expectedHol.push_back(Date(26,December,2007));

    Calendar c = UnitedKingdom(UnitedKingdom::Exchange);
    std::vector<Date> hol = Calendar::holidayList(c, Date(1,January,2004),
                                                     Date(31,December,2007));
    for (Size i=0; i<std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i]
                       << " while calculated holiday is " << hol[i]);
    }
    if (hol.size()!=expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size()
                   << " expected holidays, while there are " << hol.size()
                   << " calculated holidays");
}

void CalendarTest::testUKMetals() {
    BOOST_TEST_MESSAGE("Testing London Metals Exchange holiday list...");

    std::vector<Date> expectedHol;

    expectedHol.push_back(Date(1,January,2004));
    expectedHol.push_back(Date(9,April,2004));
    expectedHol.push_back(Date(12,April,2004));
    expectedHol.push_back(Date(3,May,2004));
    expectedHol.push_back(Date(31,May,2004));
    expectedHol.push_back(Date(30,August,2004));
    expectedHol.push_back(Date(27,December,2004));
    expectedHol.push_back(Date(28,December,2004));

    expectedHol.push_back(Date(3,January,2005));
    expectedHol.push_back(Date(25,March,2005));
    expectedHol.push_back(Date(28,March,2005));
    expectedHol.push_back(Date(2,May,2005));
    expectedHol.push_back(Date(30,May,2005));
    expectedHol.push_back(Date(29,August,2005));
    expectedHol.push_back(Date(26,December,2005));
    expectedHol.push_back(Date(27,December,2005));

    expectedHol.push_back(Date(2,January,2006));
    expectedHol.push_back(Date(14,April,2006));
    expectedHol.push_back(Date(17,April,2006));
    expectedHol.push_back(Date(1,May,2006));
    expectedHol.push_back(Date(29,May,2006));
    expectedHol.push_back(Date(28,August,2006));
    expectedHol.push_back(Date(25,December,2006));
    expectedHol.push_back(Date(26,December,2006));

    expectedHol.push_back(Date(1,January,2007));
    expectedHol.push_back(Date(6,April,2007));
    expectedHol.push_back(Date(9,April,2007));
    expectedHol.push_back(Date(7,May,2007));
    expectedHol.push_back(Date(28,May,2007));
    expectedHol.push_back(Date(27,August,2007));
    expectedHol.push_back(Date(25,December,2007));
    expectedHol.push_back(Date(26,December,2007));

    Calendar c = UnitedKingdom(UnitedKingdom::Metals);
    std::vector<Date> hol = Calendar::holidayList(c, Date(1,January,2004),
                                                     Date(31,December,2007));
    for (Size i=0; i<std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i]
                       << " while calculated holiday is " << hol[i]);
    }
    if (hol.size()!=expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size()
                   << " expected holidays, while there are " << hol.size()
                   << " calculated holidays");
}

void CalendarTest::testItalyExchange() {
    BOOST_TEST_MESSAGE("Testing Milan Stock Exchange holiday list...");

    std::vector<Date> expectedHol;

    expectedHol.push_back(Date(1,January,2002));
    expectedHol.push_back(Date(29,March,2002));
    expectedHol.push_back(Date(1,April,2002));
    expectedHol.push_back(Date(1,May,2002));
    expectedHol.push_back(Date(15,August,2002));
    expectedHol.push_back(Date(24,December,2002));
    expectedHol.push_back(Date(25,December,2002));
    expectedHol.push_back(Date(26,December,2002));
    expectedHol.push_back(Date(31,December,2002));

    expectedHol.push_back(Date(1,January,2003));
    expectedHol.push_back(Date(18,April,2003));
    expectedHol.push_back(Date(21,April,2003));
    expectedHol.push_back(Date(1,May,2003));
    expectedHol.push_back(Date(15,August,2003));
    expectedHol.push_back(Date(24,December,2003));
    expectedHol.push_back(Date(25,December,2003));
    expectedHol.push_back(Date(26,December,2003));
    expectedHol.push_back(Date(31,December,2003));

    expectedHol.push_back(Date(1,January,2004));
    expectedHol.push_back(Date(9,April,2004));
    expectedHol.push_back(Date(12,April,2004));
    expectedHol.push_back(Date(24,December,2004));
    expectedHol.push_back(Date(31,December,2004));

    Calendar c = Italy(Italy::Exchange);
    std::vector<Date> hol = Calendar::holidayList(c, Date(1,January,2002),
                                                     Date(31,December,2004));
    for (Size i=0; i<std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i]
                       << " while calculated holiday is " << hol[i]);
    }
    if (hol.size()!=expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size()
                   << " expected holidays, while there are " << hol.size()
                   << " calculated holidays");
}

void CalendarTest::testBrazil() {
    BOOST_TEST_MESSAGE("Testing Brazil holiday list...");

    std::vector<Date> expectedHol;

    //expectedHol.push_back(Date(1,January,2005)); // Saturday
    expectedHol.push_back(Date(7,February,2005));
    expectedHol.push_back(Date(8,February,2005));
    expectedHol.push_back(Date(25,March,2005));
    expectedHol.push_back(Date(21,April,2005));
    //expectedHol.push_back(Date(1,May,2005)); // Sunday
    expectedHol.push_back(Date(26,May,2005));
    expectedHol.push_back(Date(7,September,2005));
    expectedHol.push_back(Date(12,October,2005));
    expectedHol.push_back(Date(2,November,2005));
    expectedHol.push_back(Date(15,November,2005));
    //expectedHol.push_back(Date(25,December,2005)); // Sunday

    //expectedHol.push_back(Date(1,January,2006)); // Sunday
    expectedHol.push_back(Date(27,February,2006));
    expectedHol.push_back(Date(28,February,2006));
    expectedHol.push_back(Date(14,April,2006));
    expectedHol.push_back(Date(21,April,2006));
    expectedHol.push_back(Date(1,May,2006));
    expectedHol.push_back(Date(15,June,2006));
    expectedHol.push_back(Date(7,September,2006));
    expectedHol.push_back(Date(12,October,2006));
    expectedHol.push_back(Date(2,November,2006));
    expectedHol.push_back(Date(15,November,2006));
    expectedHol.push_back(Date(25,December,2006));

    Calendar c = Brazil();
    std::vector<Date> hol = Calendar::holidayList(c, Date(1,January,2005),
                                                     Date(31,December,2006));
    for (Size i=0; i<std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i]
                       << " while calculated holiday is " << hol[i]);
    }
    if (hol.size()!=expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size()
                   << " expected holidays, while there are " << hol.size()
                   << " calculated holidays");
}


void CalendarTest::testSouthKoreanSettlement() {
    BOOST_TEST_MESSAGE("Testing South-Korean settlement holiday list...");

    std::vector<Date> expectedHol;
    expectedHol.push_back(Date(1,January,2004));
    expectedHol.push_back(Date(21,January,2004));
    expectedHol.push_back(Date(22,January,2004));
    expectedHol.push_back(Date(23,January,2004));
    expectedHol.push_back(Date(1,March,2004));
    expectedHol.push_back(Date(5,April,2004));
    expectedHol.push_back(Date(15,April,2004)); // election day
//    expectedHol.push_back(Date(1,May,2004)); // Saturday
    expectedHol.push_back(Date(5,May,2004));
    expectedHol.push_back(Date(26,May,2004));
//    expectedHol.push_back(Date(6,June,2004)); // Sunday
//    expectedHol.push_back(Date(17,July,2004)); // Saturday
//    expectedHol.push_back(Date(15,August,2004)); // Sunday
    expectedHol.push_back(Date(27,September,2004));
    expectedHol.push_back(Date(28,September,2004));
    expectedHol.push_back(Date(29,September,2004));
//    expectedHol.push_back(Date(3,October,2004)); // Sunday
//    expectedHol.push_back(Date(25,December,2004)); // Saturday

//    expectedHol.push_back(Date(1,January,2005)); // Saturday
    expectedHol.push_back(Date(8,February,2005));
    expectedHol.push_back(Date(9,February,2005));
    expectedHol.push_back(Date(10,February,2005));
    expectedHol.push_back(Date(1,March,2005));
    expectedHol.push_back(Date(5,April,2005));
    expectedHol.push_back(Date(5,May,2005));
//    expectedHol.push_back(Date(15,May,2005)); // Sunday
    expectedHol.push_back(Date(6,June,2005));
//    expectedHol.push_back(Date(17,July,2005)); // Sunday
    expectedHol.push_back(Date(15,August,2005));
//    expectedHol.push_back(Date(17,September,2005)); // Saturday
//    expectedHol.push_back(Date(18,September,2005)); // Sunday
    expectedHol.push_back(Date(19,September,2005));
    expectedHol.push_back(Date(3,October,2005));
//    expectedHol.push_back(Date(25,December,2005)); // Sunday

//    expectedHol.push_back(Date(1,January,2006)); // Sunday
//    expectedHol.push_back(Date(28,January,2006)); // Saturday
//    expectedHol.push_back(Date(29,January,2006)); // Sunday
    expectedHol.push_back(Date(30,January,2006));
    expectedHol.push_back(Date(1,March,2006));
    expectedHol.push_back(Date(1,May,2006));
    expectedHol.push_back(Date(5,May,2006));
    expectedHol.push_back(Date(31,May,2006)); // election
    expectedHol.push_back(Date(6,June,2006));
    expectedHol.push_back(Date(17,July,2006));
    expectedHol.push_back(Date(15,August,2006));
    expectedHol.push_back(Date(3,October,2006));
    expectedHol.push_back(Date(5,October,2006));
    expectedHol.push_back(Date(6,October,2006));
//    expectedHol.push_back(Date(7,October,2006)); // Saturday
    expectedHol.push_back(Date(25,December,2006));

    expectedHol.push_back(Date(1,January,2007));
//    expectedHol.push_back(Date(17,February,2007)); // Saturday
//    expectedHol.push_back(Date(18,February,2007)); // Sunday
    expectedHol.push_back(Date(19,February,2007));
    expectedHol.push_back(Date(1,March,2007));
    expectedHol.push_back(Date(1,May,2007));
//    expectedHol.push_back(Date(5,May,2007)); // Saturday
    expectedHol.push_back(Date(24,May,2007));
    expectedHol.push_back(Date(6,June,2007));
    expectedHol.push_back(Date(17,July,2007));
    expectedHol.push_back(Date(15,August,2007));
    expectedHol.push_back(Date(24,September,2007));
    expectedHol.push_back(Date(25,September,2007));
    expectedHol.push_back(Date(26,September,2007));
    expectedHol.push_back(Date(3,October,2007));
    expectedHol.push_back(Date(19,December,2007)); // election
    expectedHol.push_back(Date(25,December,2007));

    Calendar c = SouthKorea(SouthKorea::Settlement);
    std::vector<Date> hol = Calendar::holidayList(c, Date(1,January,2004),
                                                     Date(31,December,2007));
    for (Size i=0; i<std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i]
                       << " while calculated holiday is " << hol[i]);
    }
    if (hol.size()!=expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size()
                   << " expected holidays, while there are " << hol.size()
                   << " calculated holidays");
}

void CalendarTest::testKoreaStockExchange() {
    BOOST_TEST_MESSAGE("Testing Korea Stock Exchange holiday list...");

    std::vector<Date> expectedHol;
    expectedHol.push_back(Date(1,January,2004));
    expectedHol.push_back(Date(21,January,2004));
    expectedHol.push_back(Date(22,January,2004));
    expectedHol.push_back(Date(23,January,2004));
    expectedHol.push_back(Date(1,March,2004));
    expectedHol.push_back(Date(5,April,2004));
    expectedHol.push_back(Date(15,April,2004)); //election day
//    expectedHol.push_back(Date(1,May,2004)); // Saturday
    expectedHol.push_back(Date(5,May,2004));
    expectedHol.push_back(Date(26,May,2004));
//    expectedHol.push_back(Date(6,June,2004)); // Sunday
//    expectedHol.push_back(Date(17,July,2004)); // Saturday
//    expectedHol.push_back(Date(15,August,2004)); // Sunday
    expectedHol.push_back(Date(27,September,2004));
    expectedHol.push_back(Date(28,September,2004));
    expectedHol.push_back(Date(29,September,2004));
//    expectedHol.push_back(Date(3,October,2004)); // Sunday
//    expectedHol.push_back(Date(25,December,2004)); // Saturday
    expectedHol.push_back(Date(31,December,2004));

//    expectedHol.push_back(Date(1,January,2005)); // Saturday
    expectedHol.push_back(Date(8,February,2005));
    expectedHol.push_back(Date(9,February,2005));
    expectedHol.push_back(Date(10,February,2005));
    expectedHol.push_back(Date(1,March,2005));
    expectedHol.push_back(Date(5,April,2005));
    expectedHol.push_back(Date(5,May,2005));
//    expectedHol.push_back(Date(15,May,2005)); // Sunday
    expectedHol.push_back(Date(6,June,2005));
//    expectedHol.push_back(Date(17,July,2005)); // Sunday
    expectedHol.push_back(Date(15,August,2005));
//    expectedHol.push_back(Date(17,September,2005)); // Saturday
//    expectedHol.push_back(Date(18,September,2005)); // Sunday
    expectedHol.push_back(Date(19,September,2005));
    expectedHol.push_back(Date(3,October,2005));
//    expectedHol.push_back(Date(25,December,2005)); // Sunday
    expectedHol.push_back(Date(30,December,2005));

//    expectedHol.push_back(Date(1,January,2006)); // Sunday
//    expectedHol.push_back(Date(28,January,2006)); // Saturday
//    expectedHol.push_back(Date(29,January,2006)); // Sunday
    expectedHol.push_back(Date(30,January,2006));
    expectedHol.push_back(Date(1,March,2006));
    expectedHol.push_back(Date(1,May,2006));
    expectedHol.push_back(Date(5,May,2006));
    expectedHol.push_back(Date(31,May,2006)); // election
    expectedHol.push_back(Date(6,June,2006));
    expectedHol.push_back(Date(17,July,2006));
    expectedHol.push_back(Date(15,August,2006));
    expectedHol.push_back(Date(3,October,2006));
    expectedHol.push_back(Date(5,October,2006));
    expectedHol.push_back(Date(6,October,2006));
//    expectedHol.push_back(Date(7,October,2006)); // Saturday
    expectedHol.push_back(Date(25,December,2006));
    expectedHol.push_back(Date(29,December,2006));

    expectedHol.push_back(Date(1,January,2007));
//    expectedHol.push_back(Date(17,February,2007)); // Saturday
//    expectedHol.push_back(Date(18,February,2007)); // Sunday
    expectedHol.push_back(Date(19,February,2007));
    expectedHol.push_back(Date(1,March,2007));
    expectedHol.push_back(Date(1,May,2007));
//    expectedHol.push_back(Date(5,May,2007)); // Saturday
    expectedHol.push_back(Date(24,May,2007));
    expectedHol.push_back(Date(6,June,2007));
    expectedHol.push_back(Date(17,July,2007));
    expectedHol.push_back(Date(15,August,2007));
    expectedHol.push_back(Date(24,September,2007));
    expectedHol.push_back(Date(25,September,2007));
    expectedHol.push_back(Date(26,September,2007));
    expectedHol.push_back(Date(3,October,2007));
    expectedHol.push_back(Date(19,December,2007)); // election
    expectedHol.push_back(Date(25,December,2007));
    expectedHol.push_back(Date(31,December,2007));

    Calendar c = SouthKorea(SouthKorea::KRX);
    std::vector<Date> hol = Calendar::holidayList(c, Date(1,January,2004),
                                                     Date(31,December,2007));

    for (Size i=0; i<std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i]!=expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i]
                       << " while calculated holiday is " << hol[i]);
    }
    if (hol.size()!=expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size()
                   << " expected holidays, while there are " << hol.size()
                   << " calculated holidays");
}

void CalendarTest::testEndOfMonth() {
    BOOST_TEST_MESSAGE("Testing end-of-month calculation...");

    Calendar c = TARGET(); // any calendar would be OK

    Date eom, counter = Date::minDate();
    Date last = Date::maxDate() - 2*Months;

    while (counter<=last) {
        eom = c.endOfMonth(counter);
        // check that eom is eom
        if (!c.isEndOfMonth(eom))
            BOOST_FAIL("\n  "
                       << eom.weekday() << " " << eom
                       << " is not the last business day in "
                       << eom.month() << " " << eom.year()
                       << " according to " << c.name());
        // check that eom is in the same month as counter
        if (eom.month()!=counter.month())
            BOOST_FAIL("\n  "
                       << eom
                       << " is not in the same month as "
                       << counter);
        counter = counter + 1;
    }
}

void CalendarTest::testBusinessDaysBetween() {

    BOOST_TEST_MESSAGE("Testing calculation of business days between dates...");

    std::vector<Date> testDates;
    testDates.push_back(Date(1,February,2002));
    testDates.push_back(Date(4,February,2002));
    testDates.push_back(Date(16,May,2003));
    testDates.push_back(Date(17,December,2003));
    testDates.push_back(Date(17,December,2004));
    testDates.push_back(Date(19,December,2005));
    testDates.push_back(Date(2,January,2006));
    testDates.push_back(Date(13,March,2006));
    testDates.push_back(Date(15,May,2006));
    testDates.push_back(Date(17,March,2006));
    testDates.push_back(Date(15,May,2006));
    testDates.push_back(Date(26,July,2006));

    BigInteger expected[] = {
        1,
        321,
        152,
        251,
        252,
        10,
        48,
        42,
        -38,
        38,
        51
    };

    Calendar calendar = Brazil();

    for (Size i=1; i<testDates.size(); i++) {
        Integer calculated = calendar.businessDaysBetween(testDates[i-1],
                                                          testDates[i]);
        if (calculated != expected[i-1]) {
            BOOST_ERROR("from " << testDates[i-1]
                        << " to " << testDates[i] << ":\n"
                        << "    calculated: " << calculated << "\n"
                        << "    expected:   " << expected[i-1]);
        }
    }
}


void CalendarTest::testBespokeCalendars() {

    BOOST_TEST_MESSAGE("Testing bespoke calendars...");

    BespokeCalendar a1;
    BespokeCalendar b1;

    Date testDate1 = Date(4, October, 2008); // Saturday
    Date testDate2 = Date(5, October, 2008); // Sunday
    Date testDate3 = Date(6, October, 2008); // Monday
    Date testDate4 = Date(7, October, 2008); // Tuesday

    if (!a1.isBusinessDay(testDate1))
        BOOST_ERROR(testDate1 << " erroneously detected as holiday");
    if (!a1.isBusinessDay(testDate2))
        BOOST_ERROR(testDate2 << " erroneously detected as holiday");
    if (!a1.isBusinessDay(testDate3))
        BOOST_ERROR(testDate3 << " erroneously detected as holiday");
    if (!a1.isBusinessDay(testDate4))
        BOOST_ERROR(testDate4 << " erroneously detected as holiday");

    if (!b1.isBusinessDay(testDate1))
        BOOST_ERROR(testDate1 << " erroneously detected as holiday");
    if (!b1.isBusinessDay(testDate2))
        BOOST_ERROR(testDate2 << " erroneously detected as holiday");
    if (!b1.isBusinessDay(testDate3))
        BOOST_ERROR(testDate3 << " erroneously detected as holiday");
    if (!b1.isBusinessDay(testDate4))
        BOOST_ERROR(testDate4 << " erroneously detected as holiday");

    a1.addWeekend(Sunday);

    if (!a1.isBusinessDay(testDate1))
        BOOST_ERROR(testDate1 << " erroneously detected as holiday");
    if (a1.isBusinessDay(testDate2))
        BOOST_ERROR(testDate2 << " (Sunday) not detected as weekend");
    if (!a1.isBusinessDay(testDate3))
        BOOST_ERROR(testDate3 << " erroneously detected as holiday");
    if (!a1.isBusinessDay(testDate4))
        BOOST_ERROR(testDate4 << " erroneously detected as holiday");

    if (!b1.isBusinessDay(testDate1))
        BOOST_ERROR(testDate1 << " erroneously detected as holiday");
    if (!b1.isBusinessDay(testDate2))
        BOOST_ERROR(testDate2 << " erroneously detected as holiday");
    if (!b1.isBusinessDay(testDate3))
        BOOST_ERROR(testDate3 << " erroneously detected as holiday");
    if (!b1.isBusinessDay(testDate4))
        BOOST_ERROR(testDate4 << " erroneously detected as holiday");

    a1.addHoliday(testDate3);

    if (!a1.isBusinessDay(testDate1))
        BOOST_ERROR(testDate1 << " erroneously detected as holiday");
    if (a1.isBusinessDay(testDate2))
        BOOST_ERROR(testDate2 << " (Sunday) not detected as weekend");
    if (a1.isBusinessDay(testDate3))
        BOOST_ERROR(testDate3 << " (marked as holiday) not detected");
    if (!a1.isBusinessDay(testDate4))
        BOOST_ERROR(testDate4 << " erroneously detected as holiday");

    if (!b1.isBusinessDay(testDate1))
        BOOST_ERROR(testDate1 << " erroneously detected as holiday");
    if (!b1.isBusinessDay(testDate2))
        BOOST_ERROR(testDate2 << " erroneously detected as holiday");
    if (!b1.isBusinessDay(testDate3))
        BOOST_ERROR(testDate3 << " erroneously detected as holiday");
    if (!b1.isBusinessDay(testDate4))
        BOOST_ERROR(testDate4 << " erroneously detected as holiday");

    BespokeCalendar a2 = a1;  // linked to a1

    a2.addWeekend(Saturday);

    if (a1.isBusinessDay(testDate1))
        BOOST_ERROR(testDate1 << " (Saturday) not detected as weekend");
    if (a1.isBusinessDay(testDate2))
        BOOST_ERROR(testDate2 << " (Sunday) not detected as weekend");
    if (a1.isBusinessDay(testDate3))
        BOOST_ERROR(testDate3 << " (marked as holiday) not detected");
    if (!a1.isBusinessDay(testDate4))
        BOOST_ERROR(testDate4 << " erroneously detected as holiday");

    if (a2.isBusinessDay(testDate1))
        BOOST_ERROR(testDate1 << " (Saturday) not detected as weekend");
    if (a2.isBusinessDay(testDate2))
        BOOST_ERROR(testDate2 << " (Sunday) not detected as weekend");
    if (a2.isBusinessDay(testDate3))
        BOOST_ERROR(testDate3 << " (marked as holiday) not detected");
    if (!a2.isBusinessDay(testDate4))
        BOOST_ERROR(testDate4 << " erroneously detected as holiday");

    a2.addHoliday(testDate4);

    if (a1.isBusinessDay(testDate1))
        BOOST_ERROR(testDate1 << " (Saturday) not detected as weekend");
    if (a1.isBusinessDay(testDate2))
        BOOST_ERROR(testDate2 << " (Sunday) not detected as weekend");
    if (a1.isBusinessDay(testDate3))
        BOOST_ERROR(testDate3 << " (marked as holiday) not detected");
    if (a1.isBusinessDay(testDate4))
        BOOST_ERROR(testDate4 << " (marked as holiday) not detected");

    if (a2.isBusinessDay(testDate1))
        BOOST_ERROR(testDate1 << " (Saturday) not detected as weekend");
    if (a2.isBusinessDay(testDate2))
        BOOST_ERROR(testDate2 << " (Sunday) not detected as weekend");
    if (a2.isBusinessDay(testDate3))
        BOOST_ERROR(testDate3 << " (marked as holiday) not detected");
    if (a2.isBusinessDay(testDate4))
        BOOST_ERROR(testDate4 << " (marked as holiday) not detected");

}

test_suite* CalendarTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Calendar tests");

    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testBrazil));

//    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testItalySettlement));
    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testItalyExchange));

    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testUKSettlement));
    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testUKExchange));
    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testUKMetals));

//    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testGermanySettlement));
    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testGermanyFrankfurt));
    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testGermanyXetra));
    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testGermanyEurex));

    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testTARGET));

    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testUSSettlement));
    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testUSGovernmentBondMarket));
    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testUSNewYorkStockExchange));

    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testSouthKoreanSettlement));
    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testKoreaStockExchange));

    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testModifiedCalendars));
    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testJointCalendars));
    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testBespokeCalendars));

    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testEndOfMonth));
    suite->add(QUANTLIB_TEST_CASE(&CalendarTest::testBusinessDaysBetween));

    return suite;
}

