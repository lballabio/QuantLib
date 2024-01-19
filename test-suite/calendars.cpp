/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2008 StatPro Italia srl
 Copyright (C) 2005 Ferdinando Ametrano
 Copyright (C) 2006 Piter Dias
 Copyright (C) 2008 Charles Chongseok Hyun
 Copyright (C) 2015 Dmitri Nesteruk
 Copyright (C) 2020 Piotr Siejda
 Copyright (C) 2020 Leonardo Arcari
 Copyright (C) 2020 Kline s.r.l.
 Copyright (C) 2022, 2024 Skandinaviska Enskilda Banken AB (publ)
 Copyright (C) 2023 Jonghee Lee

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
#include <ql/errors.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/calendars/bespokecalendar.hpp>
#include <ql/time/calendars/brazil.hpp>
#include <ql/time/calendars/china.hpp>
#include <ql/time/calendars/denmark.hpp>
#include <ql/time/calendars/germany.hpp>
#include <ql/time/calendars/italy.hpp>
#include <ql/time/calendars/japan.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/calendars/mexico.hpp>
#include <ql/time/calendars/russia.hpp>
#include <ql/time/calendars/southkorea.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <fstream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CalendarTests)

BOOST_AUTO_TEST_CASE(testModifiedCalendars) {

    BOOST_TEST_MESSAGE("Testing calendar modification...");

    Calendar c1 = TARGET();
    Calendar c2 = UnitedStates(UnitedStates::NYSE);
    Date d1(1, May, 2004);    // holiday for both calendars
    Date d2(26, April, 2004); // business day

    QL_REQUIRE(c1.isHoliday(d1), "wrong assumption---correct the test");
    QL_REQUIRE(c1.isBusinessDay(d2), "wrong assumption---correct the test");

    QL_REQUIRE(c2.isHoliday(d1), "wrong assumption---correct the test");
    QL_REQUIRE(c2.isBusinessDay(d2), "wrong assumption---correct the test");

    // modify the TARGET calendar
    c1.removeHoliday(d1);
    c1.addHoliday(d2);

    // test
    std::set<Date> addedHolidays(c1.addedHolidays());
    std::set<Date> removedHolidays(c1.removedHolidays());

    QL_REQUIRE(addedHolidays.find(d1) == addedHolidays.end(),
               "did not expect to find date in addedHolidays");
    QL_REQUIRE(addedHolidays.find(d2) != addedHolidays.end(),
               "expected to find date in addedHolidays");
    QL_REQUIRE(removedHolidays.find(d1) != removedHolidays.end(),
               "expected to find date in removedHolidays");
    QL_REQUIRE(removedHolidays.find(d2) == removedHolidays.end(),
               "did not expect to find date in removedHolidays");

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

BOOST_AUTO_TEST_CASE(testJointCalendars) {

    BOOST_TEST_MESSAGE("Testing joint calendars...");

    Calendar c1 = TARGET(), c2 = UnitedKingdom(), c3 = UnitedStates(UnitedStates::NYSE),
             c4 = Japan(), c5 = Germany();

    std::vector<Calendar> calendar_vect;
    calendar_vect.reserve(5);
    calendar_vect.push_back(c1);
    calendar_vect.push_back(c2);
    calendar_vect.push_back(c3);
    calendar_vect.push_back(c4);
    calendar_vect.push_back(c5);

    Calendar c12h = JointCalendar(c1, c2, JoinHolidays),
             c12b = JointCalendar(c1, c2, JoinBusinessDays),
             c123h = JointCalendar(c1, c2, c3, JoinHolidays),
             c123b = JointCalendar(c1, c2, c3, JoinBusinessDays),
             c1234h = JointCalendar(c1, c2, c3, c4, JoinHolidays),
             c1234b = JointCalendar(c1, c2, c3, c4, JoinBusinessDays),
             cvh = JointCalendar(calendar_vect, JoinHolidays);

    // test one year, starting today
    Date firstDate = Date::todaysDate(), endDate = firstDate + 1 * Years;

    for (Date d = firstDate; d < endDate; d++) {

        bool b1 = c1.isBusinessDay(d), b2 = c2.isBusinessDay(d), b3 = c3.isBusinessDay(d),
             b4 = c4.isBusinessDay(d), b5 = c5.isBusinessDay(d);

        if ((b1 && b2) != c12h.isBusinessDay(d))
            BOOST_FAIL("At date " << d << ":\n"
                                  << "    inconsistency between joint calendar " << c12h.name()
                                  << " (joining holidays)\n"
                                  << "    and its components");

        if ((b1 || b2) != c12b.isBusinessDay(d))
            BOOST_FAIL("At date " << d << ":\n"
                                  << "    inconsistency between joint calendar " << c12b.name()
                                  << " (joining business days)\n"
                                  << "    and its components");

        if ((b1 && b2 && b3) != c123h.isBusinessDay(d))
            BOOST_FAIL("At date " << d << ":\n"
                                  << "    inconsistency between joint calendar " << c123h.name()
                                  << " (joining holidays)\n"
                                  << "    and its components");

        if ((b1 || b2 || b3) != c123b.isBusinessDay(d))
            BOOST_FAIL("At date " << d << ":\n"
                                  << "    inconsistency between joint calendar " << c123b.name()
                                  << " (joining business days)\n"
                                  << "    and its components");

        if ((b1 && b2 && b3 && b4) != c1234h.isBusinessDay(d))
            BOOST_FAIL("At date " << d << ":\n"
                                  << "    inconsistency between joint calendar " << c1234h.name()
                                  << " (joining holidays)\n"
                                  << "    and its components");

        if ((b1 || b2 || b3 || b4) != c1234b.isBusinessDay(d))
            BOOST_FAIL("At date " << d << ":\n"
                                  << "    inconsistency between joint calendar " << c1234b.name()
                                  << " (joining business days)\n"
                                  << "    and its components");

        if ((b1 && b2 && b3 && b4 && b5) != cvh.isBusinessDay(d))
            BOOST_FAIL("At date " << d << ":\n"
                                  << "    inconsistency between joint calendar " << cvh.name()
                                  << " (joining holidays)\n"
                                  << "    and its components");
    }
}

BOOST_AUTO_TEST_CASE(testUSSettlement) {
    BOOST_TEST_MESSAGE("Testing US settlement holiday list...");

    std::vector<Date> expectedHol;
    expectedHol.emplace_back(1, January, 2004);
    expectedHol.emplace_back(19, January, 2004);
    expectedHol.emplace_back(16, February, 2004);
    expectedHol.emplace_back(31, May, 2004);
    expectedHol.emplace_back(5, July, 2004);
    expectedHol.emplace_back(6, September, 2004);
    expectedHol.emplace_back(11, October, 2004);
    expectedHol.emplace_back(11, November, 2004);
    expectedHol.emplace_back(25, November, 2004);
    expectedHol.emplace_back(24, December, 2004);

    expectedHol.emplace_back(31, December, 2004);
    expectedHol.emplace_back(17, January, 2005);
    expectedHol.emplace_back(21, February, 2005);
    expectedHol.emplace_back(30, May, 2005);
    expectedHol.emplace_back(4, July, 2005);
    expectedHol.emplace_back(5, September, 2005);
    expectedHol.emplace_back(10, October, 2005);
    expectedHol.emplace_back(11, November, 2005);
    expectedHol.emplace_back(24, November, 2005);
    expectedHol.emplace_back(26, December, 2005);

    Calendar c = UnitedStates(UnitedStates::Settlement);
    std::vector<Date> hol = c.holidayList(Date(1, January, 2004), Date(31, December, 2005));
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
    for (Size i = 0; i < hol.size(); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }

    // before Uniform Monday Holiday Act
    expectedHol = std::vector<Date>();
    expectedHol.emplace_back(2, January, 1961);
    expectedHol.emplace_back(22, February, 1961);
    expectedHol.emplace_back(30, May, 1961);
    expectedHol.emplace_back(4, July, 1961);
    expectedHol.emplace_back(4, September, 1961);
    expectedHol.emplace_back(10, November, 1961);
    expectedHol.emplace_back(23, November, 1961);
    expectedHol.emplace_back(25, December, 1961);

    hol = c.holidayList(Date(1, January, 1961), Date(31, December, 1961));
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
    for (Size i = 0; i < hol.size(); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
}

BOOST_AUTO_TEST_CASE(testUSGovernmentBondMarket) {
    BOOST_TEST_MESSAGE("Testing US government bond market holiday list...");

    std::vector<Date> expectedHol;
    expectedHol.emplace_back(1, January, 2004);
    expectedHol.emplace_back(19, January, 2004);
    expectedHol.emplace_back(16, February, 2004);
    expectedHol.emplace_back(9, April, 2004);
    expectedHol.emplace_back(31, May, 2004);
    expectedHol.emplace_back(11, June, 2004); // Reagan's funeral
    expectedHol.emplace_back(5, July, 2004);
    expectedHol.emplace_back(6, September, 2004);
    expectedHol.emplace_back(11, October, 2004);
    expectedHol.emplace_back(11, November, 2004);
    expectedHol.emplace_back(25, November, 2004);
    expectedHol.emplace_back(24, December, 2004);

    Calendar c = UnitedStates(UnitedStates::GovernmentBond);
    std::vector<Date> hol = c.holidayList(Date(1, January, 2004), Date(31, December, 2004));

    for (Size i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
}

BOOST_AUTO_TEST_CASE(testUSNewYorkStockExchange) {
    BOOST_TEST_MESSAGE("Testing New York Stock Exchange holiday list...");

    std::vector<Date> expectedHol;
    expectedHol.emplace_back(1, January, 2004);
    expectedHol.emplace_back(19, January, 2004);
    expectedHol.emplace_back(16, February, 2004);
    expectedHol.emplace_back(9, April, 2004);
    expectedHol.emplace_back(31, May, 2004);
    expectedHol.emplace_back(11, June, 2004);
    expectedHol.emplace_back(5, July, 2004);
    expectedHol.emplace_back(6, September, 2004);
    expectedHol.emplace_back(25, November, 2004);
    expectedHol.emplace_back(24, December, 2004);

    expectedHol.emplace_back(17, January, 2005);
    expectedHol.emplace_back(21, February, 2005);
    expectedHol.emplace_back(25, March, 2005);
    expectedHol.emplace_back(30, May, 2005);
    expectedHol.emplace_back(4, July, 2005);
    expectedHol.emplace_back(5, September, 2005);
    expectedHol.emplace_back(24, November, 2005);
    expectedHol.emplace_back(26, December, 2005);

    expectedHol.emplace_back(2, January, 2006);
    expectedHol.emplace_back(16, January, 2006);
    expectedHol.emplace_back(20, February, 2006);
    expectedHol.emplace_back(14, April, 2006);
    expectedHol.emplace_back(29, May, 2006);
    expectedHol.emplace_back(4, July, 2006);
    expectedHol.emplace_back(4, September, 2006);
    expectedHol.emplace_back(23, November, 2006);
    expectedHol.emplace_back(25, December, 2006);

    Calendar c = UnitedStates(UnitedStates::NYSE);
    std::vector<Date> hol = c.holidayList(Date(1, January, 2004), Date(31, December, 2006));

    Size i;
    for (i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");

    std::vector<Date> histClose;
    histClose.emplace_back(30, October, 2012);   // Hurricane Sandy
    histClose.emplace_back(29, October, 2012);   // Hurricane Sandy
    histClose.emplace_back(11, June, 2004);      // Reagan's funeral
    histClose.emplace_back(14, September, 2001); // September 11, 2001
    histClose.emplace_back(13, September, 2001); // September 11, 2001
    histClose.emplace_back(12, September, 2001); // September 11, 2001
    histClose.emplace_back(11, September, 2001); // September 11, 2001
    histClose.emplace_back(27, April, 1994);     // Nixon's funeral.
    histClose.emplace_back(27, September, 1985); // Hurricane Gloria
    histClose.emplace_back(14, July, 1977);      // 1977 Blackout
    histClose.emplace_back(25, January, 1973);   // Johnson's funeral.
    histClose.emplace_back(28, December, 1972);  // Truman's funeral
    histClose.emplace_back(21, July, 1969);      // Lunar exploration nat. day
    histClose.emplace_back(31, March, 1969);     // Eisenhower's funeral
    histClose.emplace_back(10, February, 1969);  // heavy snow
    histClose.emplace_back(5, July, 1968);       // Day after Independence Day
    histClose.emplace_back(9, April, 1968);      // Mourning for MLK
    histClose.emplace_back(24, December, 1965);  // Christmas Eve
    histClose.emplace_back(25, November, 1963);  // Kennedy's funeral
    histClose.emplace_back(29, May, 1961);       // Day before Decoration Day
    histClose.emplace_back(26, December, 1958);  // Day after Christmas
    histClose.emplace_back(24, December, 1956);  // Christmas Eve
    histClose.emplace_back(24, December, 1954);  // Christmas Eve
    // June 12-Dec. 31, 1968
    // Four day week (closed on Wednesdays) - Paperwork Crisis
    histClose.emplace_back(12, Jun, 1968);
    histClose.emplace_back(19, Jun, 1968);
    histClose.emplace_back(26, Jun, 1968);
    histClose.emplace_back(3, Jul, 1968);
    histClose.emplace_back(10, Jul, 1968);
    histClose.emplace_back(17, Jul, 1968);
    histClose.emplace_back(20, Nov, 1968);
    histClose.emplace_back(27, Nov, 1968);
    histClose.emplace_back(4, Dec, 1968);
    histClose.emplace_back(11, Dec, 1968);
    histClose.emplace_back(18, Dec, 1968);
    // Presidential election days
    histClose.emplace_back(4, Nov, 1980);
    histClose.emplace_back(2, Nov, 1976);
    histClose.emplace_back(7, Nov, 1972);
    histClose.emplace_back(5, Nov, 1968);
    histClose.emplace_back(3, Nov, 1964);
    for (i = 0; i < histClose.size(); i++) {
        if (!c.isHoliday(histClose[i]))
            BOOST_FAIL(histClose[i] << " should be holiday (historical close)");
    }
}

BOOST_AUTO_TEST_CASE(testSOFR) {
    BOOST_TEST_MESSAGE("Testing extra non-fixing day for SOFR...");

    auto fedCalendar = UnitedStates(UnitedStates::GovernmentBond);
    auto testDate = Date(7, April, 2023); // Good Friday 2023 was only a half close but SOFR didn't fix

    if (fedCalendar.isHoliday(testDate))
        BOOST_ERROR(testDate << " should not be a holiday for " << fedCalendar.name());

    auto sofr = Sofr();
    if (sofr.isValidFixingDate(testDate))
        BOOST_ERROR(testDate << " should not be a fixing date for " << sofr.name());
}

BOOST_AUTO_TEST_CASE(testUSFederalReserveJuneteenth) {
    BOOST_TEST_MESSAGE("Testing holiday occurrence of Juneteenth for US Federal Reserve calendar...");

    auto fedCalendar = UnitedStates(UnitedStates::FederalReserve);

    std::vector<Date> expectedHol;
    // Sunday, moved to Monday 20th: expectedHol.emplace_back(19, June, 2022);
    expectedHol.emplace_back(20, June, 2022);
    expectedHol.emplace_back(19, June, 2023);
    expectedHol.emplace_back(19, June, 2024);
    expectedHol.emplace_back(19, June, 2025);
    // Saturday: expectedHol.emplace_back(19, June, 2026);
    expectedHol.emplace_back(19, June, 2027);
    expectedHol.emplace_back(19, June, 2028);
    expectedHol.emplace_back(19, June, 2029);
    expectedHol.emplace_back(19, June, 2030);
    expectedHol.emplace_back(19, June, 2031);
    // Saturday: expectedHol.emplace_back(19, June, 2032);
    // Sunday, moved to Monday 20th: expectedHol.emplace_back(19, June, 2033);
    expectedHol.emplace_back(20, June, 2033);
    for (Date holiday : expectedHol) {
        if (!fedCalendar.isHoliday(holiday))
            BOOST_ERROR(holiday << " should be a holiday for " << fedCalendar.name());
    }

    Date notMovedToFriday(18, June, 2027);
    if (fedCalendar.isHoliday(notMovedToFriday))
        BOOST_ERROR(notMovedToFriday << " should not be a holiday for " << fedCalendar.name());
}

BOOST_AUTO_TEST_CASE(testTARGET) {
    BOOST_TEST_MESSAGE("Testing TARGET holiday list...");

    std::vector<Date> expectedHol;
    expectedHol.emplace_back(1, January, 1999);
    expectedHol.emplace_back(31, December, 1999);

    expectedHol.emplace_back(21, April, 2000);
    expectedHol.emplace_back(24, April, 2000);
    expectedHol.emplace_back(1, May, 2000);
    expectedHol.emplace_back(25, December, 2000);
    expectedHol.emplace_back(26, December, 2000);

    expectedHol.emplace_back(1, January, 2001);
    expectedHol.emplace_back(13, April, 2001);
    expectedHol.emplace_back(16, April, 2001);
    expectedHol.emplace_back(1, May, 2001);
    expectedHol.emplace_back(25, December, 2001);
    expectedHol.emplace_back(26, December, 2001);
    expectedHol.emplace_back(31, December, 2001);

    expectedHol.emplace_back(1, January, 2002);
    expectedHol.emplace_back(29, March, 2002);
    expectedHol.emplace_back(1, April, 2002);
    expectedHol.emplace_back(1, May, 2002);
    expectedHol.emplace_back(25, December, 2002);
    expectedHol.emplace_back(26, December, 2002);

    expectedHol.emplace_back(1, January, 2003);
    expectedHol.emplace_back(18, April, 2003);
    expectedHol.emplace_back(21, April, 2003);
    expectedHol.emplace_back(1, May, 2003);
    expectedHol.emplace_back(25, December, 2003);
    expectedHol.emplace_back(26, December, 2003);

    expectedHol.emplace_back(1, January, 2004);
    expectedHol.emplace_back(9, April, 2004);
    expectedHol.emplace_back(12, April, 2004);

    expectedHol.emplace_back(25, March, 2005);
    expectedHol.emplace_back(28, March, 2005);
    expectedHol.emplace_back(26, December, 2005);

    expectedHol.emplace_back(14, April, 2006);
    expectedHol.emplace_back(17, April, 2006);
    expectedHol.emplace_back(1, May, 2006);
    expectedHol.emplace_back(25, December, 2006);
    expectedHol.emplace_back(26, December, 2006);

    Calendar c = TARGET();
    std::vector<Date> hol = c.holidayList(Date(1, January, 1999), Date(31, December, 2006));

    for (Size i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
}

BOOST_AUTO_TEST_CASE(testGermanyFrankfurt) {
    BOOST_TEST_MESSAGE("Testing Frankfurt Stock Exchange holiday list...");

    std::vector<Date> expectedHol;

    expectedHol.emplace_back(1, January, 2003);
    expectedHol.emplace_back(18, April, 2003);
    expectedHol.emplace_back(21, April, 2003);
    expectedHol.emplace_back(1, May, 2003);
    expectedHol.emplace_back(24, December, 2003);
    expectedHol.emplace_back(25, December, 2003);
    expectedHol.emplace_back(26, December, 2003);

    expectedHol.emplace_back(1, January, 2004);
    expectedHol.emplace_back(9, April, 2004);
    expectedHol.emplace_back(12, April, 2004);
    expectedHol.emplace_back(24, December, 2004);

    Calendar c = Germany(Germany::FrankfurtStockExchange);
    std::vector<Date> hol = c.holidayList(Date(1, January, 2003), Date(31, December, 2004));
    for (Size i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
}

BOOST_AUTO_TEST_CASE(testGermanyEurex) {
    BOOST_TEST_MESSAGE("Testing Eurex holiday list...");

    std::vector<Date> expectedHol;

    expectedHol.emplace_back(1, January, 2003);
    expectedHol.emplace_back(18, April, 2003);
    expectedHol.emplace_back(21, April, 2003);
    expectedHol.emplace_back(1, May, 2003);
    expectedHol.emplace_back(24, December, 2003);
    expectedHol.emplace_back(25, December, 2003);
    expectedHol.emplace_back(26, December, 2003);
    expectedHol.emplace_back(31, December, 2003);

    expectedHol.emplace_back(1, January, 2004);
    expectedHol.emplace_back(9, April, 2004);
    expectedHol.emplace_back(12, April, 2004);
    expectedHol.emplace_back(24, December, 2004);
    expectedHol.emplace_back(31, December, 2004);

    Calendar c = Germany(Germany::Eurex);
    std::vector<Date> hol = c.holidayList(Date(1, January, 2003), Date(31, December, 2004));
    for (Size i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
}

BOOST_AUTO_TEST_CASE(testGermanyXetra) {
    BOOST_TEST_MESSAGE("Testing Xetra holiday list...");

    std::vector<Date> expectedHol;

    expectedHol.emplace_back(1, January, 2003);
    expectedHol.emplace_back(18, April, 2003);
    expectedHol.emplace_back(21, April, 2003);
    expectedHol.emplace_back(1, May, 2003);
    expectedHol.emplace_back(24, December, 2003);
    expectedHol.emplace_back(25, December, 2003);
    expectedHol.emplace_back(26, December, 2003);

    expectedHol.emplace_back(1, January, 2004);
    expectedHol.emplace_back(9, April, 2004);
    expectedHol.emplace_back(12, April, 2004);
    expectedHol.emplace_back(24, December, 2004);

    Calendar c = Germany(Germany::Xetra);
    std::vector<Date> hol = c.holidayList(Date(1, January, 2003), Date(31, December, 2004));
    for (Size i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
}

BOOST_AUTO_TEST_CASE(testUKSettlement) {
    BOOST_TEST_MESSAGE("Testing UK settlement holiday list...");

    std::vector<Date> expectedHol;

    expectedHol.emplace_back(1, January, 2004);
    expectedHol.emplace_back(9, April, 2004);
    expectedHol.emplace_back(12, April, 2004);
    expectedHol.emplace_back(3, May, 2004);
    expectedHol.emplace_back(31, May, 2004);
    expectedHol.emplace_back(30, August, 2004);
    expectedHol.emplace_back(27, December, 2004);
    expectedHol.emplace_back(28, December, 2004);

    expectedHol.emplace_back(3, January, 2005);
    expectedHol.emplace_back(25, March, 2005);
    expectedHol.emplace_back(28, March, 2005);
    expectedHol.emplace_back(2, May, 2005);
    expectedHol.emplace_back(30, May, 2005);
    expectedHol.emplace_back(29, August, 2005);
    expectedHol.emplace_back(26, December, 2005);
    expectedHol.emplace_back(27, December, 2005);

    expectedHol.emplace_back(2, January, 2006);
    expectedHol.emplace_back(14, April, 2006);
    expectedHol.emplace_back(17, April, 2006);
    expectedHol.emplace_back(1, May, 2006);
    expectedHol.emplace_back(29, May, 2006);
    expectedHol.emplace_back(28, August, 2006);
    expectedHol.emplace_back(25, December, 2006);
    expectedHol.emplace_back(26, December, 2006);

    expectedHol.emplace_back(1, January, 2007);
    expectedHol.emplace_back(6, April, 2007);
    expectedHol.emplace_back(9, April, 2007);
    expectedHol.emplace_back(7, May, 2007);
    expectedHol.emplace_back(28, May, 2007);
    expectedHol.emplace_back(27, August, 2007);
    expectedHol.emplace_back(25, December, 2007);
    expectedHol.emplace_back(26, December, 2007);

    Calendar c = UnitedKingdom(UnitedKingdom::Settlement);
    std::vector<Date> hol = c.holidayList(Date(1, January, 2004), Date(31, December, 2007));
    for (Size i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
}

BOOST_AUTO_TEST_CASE(testUKExchange) {
    BOOST_TEST_MESSAGE("Testing London Stock Exchange holiday list...");

    std::vector<Date> expectedHol;

    expectedHol.emplace_back(1, January, 2004);
    expectedHol.emplace_back(9, April, 2004);
    expectedHol.emplace_back(12, April, 2004);
    expectedHol.emplace_back(3, May, 2004);
    expectedHol.emplace_back(31, May, 2004);
    expectedHol.emplace_back(30, August, 2004);
    expectedHol.emplace_back(27, December, 2004);
    expectedHol.emplace_back(28, December, 2004);

    expectedHol.emplace_back(3, January, 2005);
    expectedHol.emplace_back(25, March, 2005);
    expectedHol.emplace_back(28, March, 2005);
    expectedHol.emplace_back(2, May, 2005);
    expectedHol.emplace_back(30, May, 2005);
    expectedHol.emplace_back(29, August, 2005);
    expectedHol.emplace_back(26, December, 2005);
    expectedHol.emplace_back(27, December, 2005);

    expectedHol.emplace_back(2, January, 2006);
    expectedHol.emplace_back(14, April, 2006);
    expectedHol.emplace_back(17, April, 2006);
    expectedHol.emplace_back(1, May, 2006);
    expectedHol.emplace_back(29, May, 2006);
    expectedHol.emplace_back(28, August, 2006);
    expectedHol.emplace_back(25, December, 2006);
    expectedHol.emplace_back(26, December, 2006);

    expectedHol.emplace_back(1, January, 2007);
    expectedHol.emplace_back(6, April, 2007);
    expectedHol.emplace_back(9, April, 2007);
    expectedHol.emplace_back(7, May, 2007);
    expectedHol.emplace_back(28, May, 2007);
    expectedHol.emplace_back(27, August, 2007);
    expectedHol.emplace_back(25, December, 2007);
    expectedHol.emplace_back(26, December, 2007);

    Calendar c = UnitedKingdom(UnitedKingdom::Exchange);
    std::vector<Date> hol = c.holidayList(Date(1, January, 2004), Date(31, December, 2007));
    for (Size i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
}

BOOST_AUTO_TEST_CASE(testUKMetals) {
    BOOST_TEST_MESSAGE("Testing London Metals Exchange holiday list...");

    std::vector<Date> expectedHol;

    expectedHol.emplace_back(1, January, 2004);
    expectedHol.emplace_back(9, April, 2004);
    expectedHol.emplace_back(12, April, 2004);
    expectedHol.emplace_back(3, May, 2004);
    expectedHol.emplace_back(31, May, 2004);
    expectedHol.emplace_back(30, August, 2004);
    expectedHol.emplace_back(27, December, 2004);
    expectedHol.emplace_back(28, December, 2004);

    expectedHol.emplace_back(3, January, 2005);
    expectedHol.emplace_back(25, March, 2005);
    expectedHol.emplace_back(28, March, 2005);
    expectedHol.emplace_back(2, May, 2005);
    expectedHol.emplace_back(30, May, 2005);
    expectedHol.emplace_back(29, August, 2005);
    expectedHol.emplace_back(26, December, 2005);
    expectedHol.emplace_back(27, December, 2005);

    expectedHol.emplace_back(2, January, 2006);
    expectedHol.emplace_back(14, April, 2006);
    expectedHol.emplace_back(17, April, 2006);
    expectedHol.emplace_back(1, May, 2006);
    expectedHol.emplace_back(29, May, 2006);
    expectedHol.emplace_back(28, August, 2006);
    expectedHol.emplace_back(25, December, 2006);
    expectedHol.emplace_back(26, December, 2006);

    expectedHol.emplace_back(1, January, 2007);
    expectedHol.emplace_back(6, April, 2007);
    expectedHol.emplace_back(9, April, 2007);
    expectedHol.emplace_back(7, May, 2007);
    expectedHol.emplace_back(28, May, 2007);
    expectedHol.emplace_back(27, August, 2007);
    expectedHol.emplace_back(25, December, 2007);
    expectedHol.emplace_back(26, December, 2007);

    Calendar c = UnitedKingdom(UnitedKingdom::Metals);
    std::vector<Date> hol = c.holidayList(Date(1, January, 2004), Date(31, December, 2007));
    for (Size i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
}

BOOST_AUTO_TEST_CASE(testItalyExchange) {
    BOOST_TEST_MESSAGE("Testing Milan Stock Exchange holiday list...");

    std::vector<Date> expectedHol;

    expectedHol.emplace_back(1, January, 2002);
    expectedHol.emplace_back(29, March, 2002);
    expectedHol.emplace_back(1, April, 2002);
    expectedHol.emplace_back(1, May, 2002);
    expectedHol.emplace_back(15, August, 2002);
    expectedHol.emplace_back(24, December, 2002);
    expectedHol.emplace_back(25, December, 2002);
    expectedHol.emplace_back(26, December, 2002);
    expectedHol.emplace_back(31, December, 2002);

    expectedHol.emplace_back(1, January, 2003);
    expectedHol.emplace_back(18, April, 2003);
    expectedHol.emplace_back(21, April, 2003);
    expectedHol.emplace_back(1, May, 2003);
    expectedHol.emplace_back(15, August, 2003);
    expectedHol.emplace_back(24, December, 2003);
    expectedHol.emplace_back(25, December, 2003);
    expectedHol.emplace_back(26, December, 2003);
    expectedHol.emplace_back(31, December, 2003);

    expectedHol.emplace_back(1, January, 2004);
    expectedHol.emplace_back(9, April, 2004);
    expectedHol.emplace_back(12, April, 2004);
    expectedHol.emplace_back(24, December, 2004);
    expectedHol.emplace_back(31, December, 2004);

    Calendar c = Italy(Italy::Exchange);
    std::vector<Date> hol = c.holidayList(Date(1, January, 2002), Date(31, December, 2004));
    for (Size i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
}

BOOST_AUTO_TEST_CASE(testRussia) {
    BOOST_TEST_MESSAGE("Testing Russia holiday list...");

    std::vector<Date> expectedHol;

    // exhaustive holiday list for the year 2012
    expectedHol.emplace_back(1, January, 2012);
    expectedHol.emplace_back(2, January, 2012);
    expectedHol.emplace_back(7, January, 2012);
    expectedHol.emplace_back(8, January, 2012);
    expectedHol.emplace_back(14, January, 2012);
    expectedHol.emplace_back(15, January, 2012);
    expectedHol.emplace_back(21, January, 2012);
    expectedHol.emplace_back(22, January, 2012);
    expectedHol.emplace_back(28, January, 2012);
    expectedHol.emplace_back(29, January, 2012);
    expectedHol.emplace_back(4, February, 2012);
    expectedHol.emplace_back(5, February, 2012);
    expectedHol.emplace_back(11, February, 2012);
    expectedHol.emplace_back(12, February, 2012);
    expectedHol.emplace_back(18, February, 2012);
    expectedHol.emplace_back(19, February, 2012);
    expectedHol.emplace_back(23, February, 2012);
    expectedHol.emplace_back(25, February, 2012);
    expectedHol.emplace_back(26, February, 2012);
    expectedHol.emplace_back(3, March, 2012);
    expectedHol.emplace_back(4, March, 2012);
    expectedHol.emplace_back(8, March, 2012);
    expectedHol.emplace_back(9, March, 2012);
    expectedHol.emplace_back(10, March, 2012);
    expectedHol.emplace_back(17, March, 2012);
    expectedHol.emplace_back(18, March, 2012);
    expectedHol.emplace_back(24, March, 2012);
    expectedHol.emplace_back(25, March, 2012);
    expectedHol.emplace_back(31, March, 2012);
    expectedHol.emplace_back(1, April, 2012);
    expectedHol.emplace_back(7, April, 2012);
    expectedHol.emplace_back(8, April, 2012);
    expectedHol.emplace_back(14, April, 2012);
    expectedHol.emplace_back(15, April, 2012);
    expectedHol.emplace_back(21, April, 2012);
    expectedHol.emplace_back(22, April, 2012);
    expectedHol.emplace_back(29, April, 2012);
    expectedHol.emplace_back(30, April, 2012);
    expectedHol.emplace_back(1, May, 2012);
    expectedHol.emplace_back(6, May, 2012);
    expectedHol.emplace_back(9, May, 2012);
    expectedHol.emplace_back(13, May, 2012);
    expectedHol.emplace_back(19, May, 2012);
    expectedHol.emplace_back(20, May, 2012);
    expectedHol.emplace_back(26, May, 2012);
    expectedHol.emplace_back(27, May, 2012);
    expectedHol.emplace_back(2, June, 2012);
    expectedHol.emplace_back(3, June, 2012);
    expectedHol.emplace_back(10, June, 2012);
    expectedHol.emplace_back(11, June, 2012);
    expectedHol.emplace_back(12, June, 2012);
    expectedHol.emplace_back(16, June, 2012);
    expectedHol.emplace_back(17, June, 2012);
    expectedHol.emplace_back(23, June, 2012);
    expectedHol.emplace_back(24, June, 2012);
    expectedHol.emplace_back(30, June, 2012);
    expectedHol.emplace_back(1, July, 2012);
    expectedHol.emplace_back(7, July, 2012);
    expectedHol.emplace_back(8, July, 2012);
    expectedHol.emplace_back(14, July, 2012);
    expectedHol.emplace_back(15, July, 2012);
    expectedHol.emplace_back(21, July, 2012);
    expectedHol.emplace_back(22, July, 2012);
    expectedHol.emplace_back(28, July, 2012);
    expectedHol.emplace_back(29, July, 2012);
    expectedHol.emplace_back(4, August, 2012);
    expectedHol.emplace_back(5, August, 2012);
    expectedHol.emplace_back(11, August, 2012);
    expectedHol.emplace_back(12, August, 2012);
    expectedHol.emplace_back(18, August, 2012);
    expectedHol.emplace_back(19, August, 2012);
    expectedHol.emplace_back(25, August, 2012);
    expectedHol.emplace_back(26, August, 2012);
    expectedHol.emplace_back(1, September, 2012);
    expectedHol.emplace_back(2, September, 2012);
    expectedHol.emplace_back(8, September, 2012);
    expectedHol.emplace_back(9, September, 2012);
    expectedHol.emplace_back(15, September, 2012);
    expectedHol.emplace_back(16, September, 2012);
    expectedHol.emplace_back(22, September, 2012);
    expectedHol.emplace_back(23, September, 2012);
    expectedHol.emplace_back(29, September, 2012);
    expectedHol.emplace_back(30, September, 2012);
    expectedHol.emplace_back(6, October, 2012);
    expectedHol.emplace_back(7, October, 2012);
    expectedHol.emplace_back(13, October, 2012);
    expectedHol.emplace_back(14, October, 2012);
    expectedHol.emplace_back(20, October, 2012);
    expectedHol.emplace_back(21, October, 2012);
    expectedHol.emplace_back(27, October, 2012);
    expectedHol.emplace_back(28, October, 2012);
    expectedHol.emplace_back(3, November, 2012);
    expectedHol.emplace_back(4, November, 2012);
    expectedHol.emplace_back(5, November, 2012);
    expectedHol.emplace_back(10, November, 2012);
    expectedHol.emplace_back(11, November, 2012);
    expectedHol.emplace_back(17, November, 2012);
    expectedHol.emplace_back(18, November, 2012);
    expectedHol.emplace_back(24, November, 2012);
    expectedHol.emplace_back(25, November, 2012);
    expectedHol.emplace_back(1, December, 2012);
    expectedHol.emplace_back(2, December, 2012);
    expectedHol.emplace_back(8, December, 2012);
    expectedHol.emplace_back(9, December, 2012);
    expectedHol.emplace_back(15, December, 2012);
    expectedHol.emplace_back(16, December, 2012);
    expectedHol.emplace_back(22, December, 2012);
    expectedHol.emplace_back(23, December, 2012);
    expectedHol.emplace_back(29, December, 2012);
    expectedHol.emplace_back(30, December, 2012);
    expectedHol.emplace_back(31, December, 2012);

    // exhaustive holiday list for the year 2013
    expectedHol.emplace_back(1, January, 2013);
    expectedHol.emplace_back(2, January, 2013);
    expectedHol.emplace_back(3, January, 2013);
    expectedHol.emplace_back(4, January, 2013);
    expectedHol.emplace_back(5, January, 2013);
    expectedHol.emplace_back(6, January, 2013);
    expectedHol.emplace_back(7, January, 2013);
    expectedHol.emplace_back(12, January, 2013);
    expectedHol.emplace_back(13, January, 2013);
    expectedHol.emplace_back(19, January, 2013);
    expectedHol.emplace_back(20, January, 2013);
    expectedHol.emplace_back(26, January, 2013);
    expectedHol.emplace_back(27, January, 2013);
    expectedHol.emplace_back(2, February, 2013);
    expectedHol.emplace_back(3, February, 2013);
    expectedHol.emplace_back(9, February, 2013);
    expectedHol.emplace_back(10, February, 2013);
    expectedHol.emplace_back(16, February, 2013);
    expectedHol.emplace_back(17, February, 2013);
    expectedHol.emplace_back(23, February, 2013);
    expectedHol.emplace_back(24, February, 2013);
    expectedHol.emplace_back(2, March, 2013);
    expectedHol.emplace_back(3, March, 2013);
    expectedHol.emplace_back(8, March, 2013);
    expectedHol.emplace_back(9, March, 2013);
    expectedHol.emplace_back(10, March, 2013);
    expectedHol.emplace_back(16, March, 2013);
    expectedHol.emplace_back(17, March, 2013);
    expectedHol.emplace_back(23, March, 2013);
    expectedHol.emplace_back(24, March, 2013);
    expectedHol.emplace_back(30, March, 2013);
    expectedHol.emplace_back(31, March, 2013);
    expectedHol.emplace_back(6, April, 2013);
    expectedHol.emplace_back(7, April, 2013);
    expectedHol.emplace_back(13, April, 2013);
    expectedHol.emplace_back(14, April, 2013);
    expectedHol.emplace_back(20, April, 2013);
    expectedHol.emplace_back(21, April, 2013);
    expectedHol.emplace_back(27, April, 2013);
    expectedHol.emplace_back(28, April, 2013);
    expectedHol.emplace_back(1, May, 2013);
    expectedHol.emplace_back(4, May, 2013);
    expectedHol.emplace_back(5, May, 2013);
    expectedHol.emplace_back(9, May, 2013);
    expectedHol.emplace_back(11, May, 2013);
    expectedHol.emplace_back(12, May, 2013);
    expectedHol.emplace_back(18, May, 2013);
    expectedHol.emplace_back(19, May, 2013);
    expectedHol.emplace_back(25, May, 2013);
    expectedHol.emplace_back(26, May, 2013);
    expectedHol.emplace_back(1, June, 2013);
    expectedHol.emplace_back(2, June, 2013);
    expectedHol.emplace_back(8, June, 2013);
    expectedHol.emplace_back(9, June, 2013);
    expectedHol.emplace_back(12, June, 2013);
    expectedHol.emplace_back(15, June, 2013);
    expectedHol.emplace_back(16, June, 2013);
    expectedHol.emplace_back(22, June, 2013);
    expectedHol.emplace_back(23, June, 2013);
    expectedHol.emplace_back(29, June, 2013);
    expectedHol.emplace_back(30, June, 2013);
    expectedHol.emplace_back(6, July, 2013);
    expectedHol.emplace_back(7, July, 2013);
    expectedHol.emplace_back(13, July, 2013);
    expectedHol.emplace_back(14, July, 2013);
    expectedHol.emplace_back(20, July, 2013);
    expectedHol.emplace_back(21, July, 2013);
    expectedHol.emplace_back(27, July, 2013);
    expectedHol.emplace_back(28, July, 2013);
    expectedHol.emplace_back(3, August, 2013);
    expectedHol.emplace_back(4, August, 2013);
    expectedHol.emplace_back(10, August, 2013);
    expectedHol.emplace_back(11, August, 2013);
    expectedHol.emplace_back(17, August, 2013);
    expectedHol.emplace_back(18, August, 2013);
    expectedHol.emplace_back(24, August, 2013);
    expectedHol.emplace_back(25, August, 2013);
    expectedHol.emplace_back(31, August, 2013);
    expectedHol.emplace_back(1, September, 2013);
    expectedHol.emplace_back(7, September, 2013);
    expectedHol.emplace_back(8, September, 2013);
    expectedHol.emplace_back(14, September, 2013);
    expectedHol.emplace_back(15, September, 2013);
    expectedHol.emplace_back(21, September, 2013);
    expectedHol.emplace_back(22, September, 2013);
    expectedHol.emplace_back(28, September, 2013);
    expectedHol.emplace_back(29, September, 2013);
    expectedHol.emplace_back(5, October, 2013);
    expectedHol.emplace_back(6, October, 2013);
    expectedHol.emplace_back(12, October, 2013);
    expectedHol.emplace_back(13, October, 2013);
    expectedHol.emplace_back(19, October, 2013);
    expectedHol.emplace_back(20, October, 2013);
    expectedHol.emplace_back(26, October, 2013);
    expectedHol.emplace_back(27, October, 2013);
    expectedHol.emplace_back(2, November, 2013);
    expectedHol.emplace_back(3, November, 2013);
    expectedHol.emplace_back(4, November, 2013);
    expectedHol.emplace_back(9, November, 2013);
    expectedHol.emplace_back(10, November, 2013);
    expectedHol.emplace_back(16, November, 2013);
    expectedHol.emplace_back(17, November, 2013);
    expectedHol.emplace_back(23, November, 2013);
    expectedHol.emplace_back(24, November, 2013);
    expectedHol.emplace_back(30, November, 2013);
    expectedHol.emplace_back(1, December, 2013);
    expectedHol.emplace_back(7, December, 2013);
    expectedHol.emplace_back(8, December, 2013);
    expectedHol.emplace_back(14, December, 2013);
    expectedHol.emplace_back(15, December, 2013);
    expectedHol.emplace_back(21, December, 2013);
    expectedHol.emplace_back(22, December, 2013);
    expectedHol.emplace_back(28, December, 2013);
    expectedHol.emplace_back(29, December, 2013);
    expectedHol.emplace_back(31, December, 2013);

    // exhaustive holiday list for the year 2014
    expectedHol.emplace_back(1, January, 2014);
    expectedHol.emplace_back(2, January, 2014);
    expectedHol.emplace_back(3, January, 2014);
    expectedHol.emplace_back(4, January, 2014);
    expectedHol.emplace_back(5, January, 2014);
    expectedHol.emplace_back(7, January, 2014);
    expectedHol.emplace_back(11, January, 2014);
    expectedHol.emplace_back(12, January, 2014);
    expectedHol.emplace_back(18, January, 2014);
    expectedHol.emplace_back(19, January, 2014);
    expectedHol.emplace_back(25, January, 2014);
    expectedHol.emplace_back(26, January, 2014);
    expectedHol.emplace_back(1, February, 2014);
    expectedHol.emplace_back(2, February, 2014);
    expectedHol.emplace_back(8, February, 2014);
    expectedHol.emplace_back(9, February, 2014);
    expectedHol.emplace_back(15, February, 2014);
    expectedHol.emplace_back(16, February, 2014);
    expectedHol.emplace_back(22, February, 2014);
    expectedHol.emplace_back(23, February, 2014);
    expectedHol.emplace_back(1, March, 2014);
    expectedHol.emplace_back(2, March, 2014);
    expectedHol.emplace_back(8, March, 2014);
    expectedHol.emplace_back(9, March, 2014);
    expectedHol.emplace_back(10, March, 2014);
    expectedHol.emplace_back(15, March, 2014);
    expectedHol.emplace_back(16, March, 2014);
    expectedHol.emplace_back(22, March, 2014);
    expectedHol.emplace_back(23, March, 2014);
    expectedHol.emplace_back(29, March, 2014);
    expectedHol.emplace_back(30, March, 2014);
    expectedHol.emplace_back(5, April, 2014);
    expectedHol.emplace_back(6, April, 2014);
    expectedHol.emplace_back(12, April, 2014);
    expectedHol.emplace_back(13, April, 2014);
    expectedHol.emplace_back(19, April, 2014);
    expectedHol.emplace_back(20, April, 2014);
    expectedHol.emplace_back(26, April, 2014);
    expectedHol.emplace_back(27, April, 2014);
    expectedHol.emplace_back(1, May, 2014);
    expectedHol.emplace_back(3, May, 2014);
    expectedHol.emplace_back(4, May, 2014);
    expectedHol.emplace_back(9, May, 2014);
    expectedHol.emplace_back(10, May, 2014);
    expectedHol.emplace_back(11, May, 2014);
    expectedHol.emplace_back(17, May, 2014);
    expectedHol.emplace_back(18, May, 2014);
    expectedHol.emplace_back(24, May, 2014);
    expectedHol.emplace_back(25, May, 2014);
    expectedHol.emplace_back(31, May, 2014);
    expectedHol.emplace_back(1, June, 2014);
    expectedHol.emplace_back(7, June, 2014);
    expectedHol.emplace_back(8, June, 2014);
    expectedHol.emplace_back(12, June, 2014);
    expectedHol.emplace_back(14, June, 2014);
    expectedHol.emplace_back(15, June, 2014);
    expectedHol.emplace_back(21, June, 2014);
    expectedHol.emplace_back(22, June, 2014);
    expectedHol.emplace_back(28, June, 2014);
    expectedHol.emplace_back(29, June, 2014);
    expectedHol.emplace_back(5, July, 2014);
    expectedHol.emplace_back(6, July, 2014);
    expectedHol.emplace_back(12, July, 2014);
    expectedHol.emplace_back(13, July, 2014);
    expectedHol.emplace_back(19, July, 2014);
    expectedHol.emplace_back(20, July, 2014);
    expectedHol.emplace_back(26, July, 2014);
    expectedHol.emplace_back(27, July, 2014);
    expectedHol.emplace_back(2, August, 2014);
    expectedHol.emplace_back(3, August, 2014);
    expectedHol.emplace_back(9, August, 2014);
    expectedHol.emplace_back(10, August, 2014);
    expectedHol.emplace_back(16, August, 2014);
    expectedHol.emplace_back(17, August, 2014);
    expectedHol.emplace_back(23, August, 2014);
    expectedHol.emplace_back(24, August, 2014);
    expectedHol.emplace_back(30, August, 2014);
    expectedHol.emplace_back(31, August, 2014);
    expectedHol.emplace_back(6, September, 2014);
    expectedHol.emplace_back(7, September, 2014);
    expectedHol.emplace_back(13, September, 2014);
    expectedHol.emplace_back(14, September, 2014);
    expectedHol.emplace_back(20, September, 2014);
    expectedHol.emplace_back(21, September, 2014);
    expectedHol.emplace_back(27, September, 2014);
    expectedHol.emplace_back(28, September, 2014);
    expectedHol.emplace_back(4, October, 2014);
    expectedHol.emplace_back(5, October, 2014);
    expectedHol.emplace_back(11, October, 2014);
    expectedHol.emplace_back(12, October, 2014);
    expectedHol.emplace_back(18, October, 2014);
    expectedHol.emplace_back(19, October, 2014);
    expectedHol.emplace_back(25, October, 2014);
    expectedHol.emplace_back(26, October, 2014);
    expectedHol.emplace_back(1, November, 2014);
    expectedHol.emplace_back(2, November, 2014);
    expectedHol.emplace_back(4, November, 2014);
    expectedHol.emplace_back(8, November, 2014);
    expectedHol.emplace_back(9, November, 2014);
    expectedHol.emplace_back(15, November, 2014);
    expectedHol.emplace_back(16, November, 2014);
    expectedHol.emplace_back(22, November, 2014);
    expectedHol.emplace_back(23, November, 2014);
    expectedHol.emplace_back(29, November, 2014);
    expectedHol.emplace_back(30, November, 2014);
    expectedHol.emplace_back(6, December, 2014);
    expectedHol.emplace_back(7, December, 2014);
    expectedHol.emplace_back(13, December, 2014);
    expectedHol.emplace_back(14, December, 2014);
    expectedHol.emplace_back(20, December, 2014);
    expectedHol.emplace_back(21, December, 2014);
    expectedHol.emplace_back(27, December, 2014);
    expectedHol.emplace_back(28, December, 2014);
    expectedHol.emplace_back(31, December, 2014);

    // exhaustive holiday list for the year 2015
    expectedHol.emplace_back(1, January, 2015);
    expectedHol.emplace_back(2, January, 2015);
    expectedHol.emplace_back(3, January, 2015);
    expectedHol.emplace_back(4, January, 2015);
    expectedHol.emplace_back(7, January, 2015);
    expectedHol.emplace_back(10, January, 2015);
    expectedHol.emplace_back(11, January, 2015);
    expectedHol.emplace_back(17, January, 2015);
    expectedHol.emplace_back(18, January, 2015);
    expectedHol.emplace_back(24, January, 2015);
    expectedHol.emplace_back(25, January, 2015);
    expectedHol.emplace_back(31, January, 2015);
    expectedHol.emplace_back(1, February, 2015);
    expectedHol.emplace_back(7, February, 2015);
    expectedHol.emplace_back(8, February, 2015);
    expectedHol.emplace_back(14, February, 2015);
    expectedHol.emplace_back(15, February, 2015);
    expectedHol.emplace_back(21, February, 2015);
    expectedHol.emplace_back(22, February, 2015);
    expectedHol.emplace_back(23, February, 2015);
    expectedHol.emplace_back(28, February, 2015);
    expectedHol.emplace_back(1, March, 2015);
    expectedHol.emplace_back(7, March, 2015);
    expectedHol.emplace_back(8, March, 2015);
    expectedHol.emplace_back(9, March, 2015);
    expectedHol.emplace_back(14, March, 2015);
    expectedHol.emplace_back(15, March, 2015);
    expectedHol.emplace_back(21, March, 2015);
    expectedHol.emplace_back(22, March, 2015);
    expectedHol.emplace_back(28, March, 2015);
    expectedHol.emplace_back(29, March, 2015);
    expectedHol.emplace_back(4, April, 2015);
    expectedHol.emplace_back(5, April, 2015);
    expectedHol.emplace_back(11, April, 2015);
    expectedHol.emplace_back(12, April, 2015);
    expectedHol.emplace_back(18, April, 2015);
    expectedHol.emplace_back(19, April, 2015);
    expectedHol.emplace_back(25, April, 2015);
    expectedHol.emplace_back(26, April, 2015);
    expectedHol.emplace_back(1, May, 2015);
    expectedHol.emplace_back(2, May, 2015);
    expectedHol.emplace_back(3, May, 2015);
    expectedHol.emplace_back(9, May, 2015);
    expectedHol.emplace_back(10, May, 2015);
    expectedHol.emplace_back(11, May, 2015);
    expectedHol.emplace_back(16, May, 2015);
    expectedHol.emplace_back(17, May, 2015);
    expectedHol.emplace_back(23, May, 2015);
    expectedHol.emplace_back(24, May, 2015);
    expectedHol.emplace_back(30, May, 2015);
    expectedHol.emplace_back(31, May, 2015);
    expectedHol.emplace_back(6, June, 2015);
    expectedHol.emplace_back(7, June, 2015);
    expectedHol.emplace_back(12, June, 2015);
    expectedHol.emplace_back(13, June, 2015);
    expectedHol.emplace_back(14, June, 2015);
    expectedHol.emplace_back(20, June, 2015);
    expectedHol.emplace_back(21, June, 2015);
    expectedHol.emplace_back(27, June, 2015);
    expectedHol.emplace_back(28, June, 2015);
    expectedHol.emplace_back(4, July, 2015);
    expectedHol.emplace_back(5, July, 2015);
    expectedHol.emplace_back(11, July, 2015);
    expectedHol.emplace_back(12, July, 2015);
    expectedHol.emplace_back(18, July, 2015);
    expectedHol.emplace_back(19, July, 2015);
    expectedHol.emplace_back(25, July, 2015);
    expectedHol.emplace_back(26, July, 2015);
    expectedHol.emplace_back(1, August, 2015);
    expectedHol.emplace_back(2, August, 2015);
    expectedHol.emplace_back(8, August, 2015);
    expectedHol.emplace_back(9, August, 2015);
    expectedHol.emplace_back(15, August, 2015);
    expectedHol.emplace_back(16, August, 2015);
    expectedHol.emplace_back(22, August, 2015);
    expectedHol.emplace_back(23, August, 2015);
    expectedHol.emplace_back(29, August, 2015);
    expectedHol.emplace_back(30, August, 2015);
    expectedHol.emplace_back(5, September, 2015);
    expectedHol.emplace_back(6, September, 2015);
    expectedHol.emplace_back(12, September, 2015);
    expectedHol.emplace_back(13, September, 2015);
    expectedHol.emplace_back(19, September, 2015);
    expectedHol.emplace_back(20, September, 2015);
    expectedHol.emplace_back(26, September, 2015);
    expectedHol.emplace_back(27, September, 2015);
    expectedHol.emplace_back(3, October, 2015);
    expectedHol.emplace_back(4, October, 2015);
    expectedHol.emplace_back(10, October, 2015);
    expectedHol.emplace_back(11, October, 2015);
    expectedHol.emplace_back(17, October, 2015);
    expectedHol.emplace_back(18, October, 2015);
    expectedHol.emplace_back(24, October, 2015);
    expectedHol.emplace_back(25, October, 2015);
    expectedHol.emplace_back(31, October, 2015);
    expectedHol.emplace_back(1, November, 2015);
    expectedHol.emplace_back(4, November, 2015);
    expectedHol.emplace_back(7, November, 2015);
    expectedHol.emplace_back(8, November, 2015);
    expectedHol.emplace_back(14, November, 2015);
    expectedHol.emplace_back(15, November, 2015);
    expectedHol.emplace_back(21, November, 2015);
    expectedHol.emplace_back(22, November, 2015);
    expectedHol.emplace_back(28, November, 2015);
    expectedHol.emplace_back(29, November, 2015);
    expectedHol.emplace_back(5, December, 2015);
    expectedHol.emplace_back(6, December, 2015);
    expectedHol.emplace_back(12, December, 2015);
    expectedHol.emplace_back(13, December, 2015);
    expectedHol.emplace_back(19, December, 2015);
    expectedHol.emplace_back(20, December, 2015);
    expectedHol.emplace_back(26, December, 2015);
    expectedHol.emplace_back(27, December, 2015);
    expectedHol.emplace_back(31, December, 2015);

    // exhaustive holiday list for the year 2016
    expectedHol.emplace_back(1, January, 2016);
    expectedHol.emplace_back(2, January, 2016);
    expectedHol.emplace_back(3, January, 2016);
    expectedHol.emplace_back(7, January, 2016);
    expectedHol.emplace_back(8, January, 2016);
    expectedHol.emplace_back(9, January, 2016);
    expectedHol.emplace_back(10, January, 2016);
    expectedHol.emplace_back(16, January, 2016);
    expectedHol.emplace_back(17, January, 2016);
    expectedHol.emplace_back(23, January, 2016);
    expectedHol.emplace_back(24, January, 2016);
    expectedHol.emplace_back(30, January, 2016);
    expectedHol.emplace_back(31, January, 2016);
    expectedHol.emplace_back(6, February, 2016);
    expectedHol.emplace_back(7, February, 2016);
    expectedHol.emplace_back(13, February, 2016);
    expectedHol.emplace_back(14, February, 2016);
    expectedHol.emplace_back(21, February, 2016);
    expectedHol.emplace_back(23, February, 2016);
    expectedHol.emplace_back(27, February, 2016);
    expectedHol.emplace_back(28, February, 2016);
    expectedHol.emplace_back(5, March, 2016);
    expectedHol.emplace_back(6, March, 2016);
    expectedHol.emplace_back(8, March, 2016);
    expectedHol.emplace_back(12, March, 2016);
    expectedHol.emplace_back(13, March, 2016);
    expectedHol.emplace_back(19, March, 2016);
    expectedHol.emplace_back(20, March, 2016);
    expectedHol.emplace_back(26, March, 2016);
    expectedHol.emplace_back(27, March, 2016);
    expectedHol.emplace_back(2, April, 2016);
    expectedHol.emplace_back(3, April, 2016);
    expectedHol.emplace_back(9, April, 2016);
    expectedHol.emplace_back(10, April, 2016);
    expectedHol.emplace_back(16, April, 2016);
    expectedHol.emplace_back(17, April, 2016);
    expectedHol.emplace_back(23, April, 2016);
    expectedHol.emplace_back(24, April, 2016);
    expectedHol.emplace_back(30, April, 2016);
    expectedHol.emplace_back(1, May, 2016);
    expectedHol.emplace_back(2, May, 2016);
    expectedHol.emplace_back(3, May, 2016);
    expectedHol.emplace_back(7, May, 2016);
    expectedHol.emplace_back(8, May, 2016);
    expectedHol.emplace_back(9, May, 2016);
    expectedHol.emplace_back(14, May, 2016);
    expectedHol.emplace_back(15, May, 2016);
    expectedHol.emplace_back(21, May, 2016);
    expectedHol.emplace_back(22, May, 2016);
    expectedHol.emplace_back(28, May, 2016);
    expectedHol.emplace_back(29, May, 2016);
    expectedHol.emplace_back(4, June, 2016);
    expectedHol.emplace_back(5, June, 2016);
    expectedHol.emplace_back(11, June, 2016);
    expectedHol.emplace_back(12, June, 2016);
    expectedHol.emplace_back(13, June, 2016);
    expectedHol.emplace_back(18, June, 2016);
    expectedHol.emplace_back(19, June, 2016);
    expectedHol.emplace_back(25, June, 2016);
    expectedHol.emplace_back(26, June, 2016);
    expectedHol.emplace_back(2, July, 2016);
    expectedHol.emplace_back(3, July, 2016);
    expectedHol.emplace_back(9, July, 2016);
    expectedHol.emplace_back(10, July, 2016);
    expectedHol.emplace_back(16, July, 2016);
    expectedHol.emplace_back(17, July, 2016);
    expectedHol.emplace_back(23, July, 2016);
    expectedHol.emplace_back(24, July, 2016);
    expectedHol.emplace_back(30, July, 2016);
    expectedHol.emplace_back(31, July, 2016);
    expectedHol.emplace_back(6, August, 2016);
    expectedHol.emplace_back(7, August, 2016);
    expectedHol.emplace_back(13, August, 2016);
    expectedHol.emplace_back(14, August, 2016);
    expectedHol.emplace_back(20, August, 2016);
    expectedHol.emplace_back(21, August, 2016);
    expectedHol.emplace_back(27, August, 2016);
    expectedHol.emplace_back(28, August, 2016);
    expectedHol.emplace_back(3, September, 2016);
    expectedHol.emplace_back(4, September, 2016);
    expectedHol.emplace_back(10, September, 2016);
    expectedHol.emplace_back(11, September, 2016);
    expectedHol.emplace_back(17, September, 2016);
    expectedHol.emplace_back(18, September, 2016);
    expectedHol.emplace_back(24, September, 2016);
    expectedHol.emplace_back(25, September, 2016);
    expectedHol.emplace_back(1, October, 2016);
    expectedHol.emplace_back(2, October, 2016);
    expectedHol.emplace_back(8, October, 2016);
    expectedHol.emplace_back(9, October, 2016);
    expectedHol.emplace_back(15, October, 2016);
    expectedHol.emplace_back(16, October, 2016);
    expectedHol.emplace_back(22, October, 2016);
    expectedHol.emplace_back(23, October, 2016);
    expectedHol.emplace_back(29, October, 2016);
    expectedHol.emplace_back(30, October, 2016);
    expectedHol.emplace_back(4, November, 2016);
    expectedHol.emplace_back(5, November, 2016);
    expectedHol.emplace_back(6, November, 2016);
    expectedHol.emplace_back(12, November, 2016);
    expectedHol.emplace_back(13, November, 2016);
    expectedHol.emplace_back(19, November, 2016);
    expectedHol.emplace_back(20, November, 2016);
    expectedHol.emplace_back(26, November, 2016);
    expectedHol.emplace_back(27, November, 2016);
    expectedHol.emplace_back(3, December, 2016);
    expectedHol.emplace_back(4, December, 2016);
    expectedHol.emplace_back(10, December, 2016);
    expectedHol.emplace_back(11, December, 2016);
    expectedHol.emplace_back(17, December, 2016);
    expectedHol.emplace_back(18, December, 2016);
    expectedHol.emplace_back(24, December, 2016);
    expectedHol.emplace_back(25, December, 2016);
    expectedHol.emplace_back(30, December, 2016);
    expectedHol.emplace_back(31, December, 2016);

    Calendar c = Russia(Russia::MOEX);


    std::vector<Date> hol =
        c.holidayList(Date(1, January, 2012),
                      Date(31, December, 2016), // only dates for which calendars are available
                      true);                    // include week-ends since lists are exhaustive
    for (Size i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
}

BOOST_AUTO_TEST_CASE(testBrazil) {
    BOOST_TEST_MESSAGE("Testing Brazil holiday list...");

    std::vector<Date> expectedHol;

    // expectedHol.push_back(Date(1,January,2005)); // Saturday
    expectedHol.emplace_back(7, February, 2005);
    expectedHol.emplace_back(8, February, 2005);
    expectedHol.emplace_back(25, March, 2005);
    expectedHol.emplace_back(21, April, 2005);
    // expectedHol.push_back(Date(1,May,2005)); // Sunday
    expectedHol.emplace_back(26, May, 2005);
    expectedHol.emplace_back(7, September, 2005);
    expectedHol.emplace_back(12, October, 2005);
    expectedHol.emplace_back(2, November, 2005);
    expectedHol.emplace_back(15, November, 2005);
    // expectedHol.push_back(Date(25,December,2005)); // Sunday

    // expectedHol.push_back(Date(1,January,2006)); // Sunday
    expectedHol.emplace_back(27, February, 2006);
    expectedHol.emplace_back(28, February, 2006);
    expectedHol.emplace_back(14, April, 2006);
    expectedHol.emplace_back(21, April, 2006);
    expectedHol.emplace_back(1, May, 2006);
    expectedHol.emplace_back(15, June, 2006);
    expectedHol.emplace_back(7, September, 2006);
    expectedHol.emplace_back(12, October, 2006);
    expectedHol.emplace_back(2, November, 2006);
    expectedHol.emplace_back(15, November, 2006);
    expectedHol.emplace_back(25, December, 2006);

    Calendar c = Brazil();
    std::vector<Date> hol = c.holidayList(Date(1, January, 2005), Date(31, December, 2006));
    for (Size i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
}

BOOST_AUTO_TEST_CASE(testDenmark) {

    BOOST_TEST_MESSAGE("Testing Denmark holiday list...");

    std::vector<Date> expectedHol;
    expectedHol.emplace_back(1, January, 2020);
    expectedHol.emplace_back(9, April, 2020);
    expectedHol.emplace_back(10, April, 2020);
    expectedHol.emplace_back(13, April, 2020);
    expectedHol.emplace_back(8, May, 2020);
    expectedHol.emplace_back(21, May, 2020);
    expectedHol.emplace_back(22, May, 2020);
    expectedHol.emplace_back(1, June, 2020);
    expectedHol.emplace_back(5, June, 2020);
    expectedHol.emplace_back(24, December, 2020);
    expectedHol.emplace_back(25, December, 2020);
    // Saturday: expectedHol.emplace_back(26, December, 2020);
    expectedHol.emplace_back(31, December, 2020);

    expectedHol.emplace_back(1, January, 2021);
    expectedHol.emplace_back(1, April, 2021);
    expectedHol.emplace_back(2, April, 2021);
    expectedHol.emplace_back(5, April, 2021);
    expectedHol.emplace_back(30, April, 2021);
    expectedHol.emplace_back(13, May, 2021);
    expectedHol.emplace_back(14, May, 2021);
    expectedHol.emplace_back(24, May, 2021);
    // Saturday: expectedHol.emplace_back(5, June, 2021);
    expectedHol.emplace_back(24, December, 2021);
    // Saturday: expectedHol.emplace_back(25, December, 2021);
    // Sunday: expectedHol.emplace_back(26, December, 2021);
    expectedHol.emplace_back(31, December, 2021);

    // Saturday: expectedHol.emplace_back(1, January, 2022);
    expectedHol.emplace_back(14, April, 2022);
    expectedHol.emplace_back(15, April, 2022);
    expectedHol.emplace_back(18, April, 2022);
    expectedHol.emplace_back(13, May, 2022);
    expectedHol.emplace_back(26, May, 2022);
    expectedHol.emplace_back(27, May, 2022);
    // Sunday: expectedHol.emplace_back(5, June, 2022);
    expectedHol.emplace_back(6, June, 2022);
    // Saturday: expectedHol.emplace_back(24, December, 2022);
    // Sunday: expectedHol.emplace_back(25, December, 2022);
    expectedHol.emplace_back(26, December, 2022);
    // Saturday: expectedHol.emplace_back(31, December, 2022);

    Calendar c = Denmark();
    std::vector<Date> hol = c.holidayList(Date(1, January, 2020), Date(31, December, 2022));

    for (Size i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
}

BOOST_AUTO_TEST_CASE(testSouthKoreanSettlement) {
    BOOST_TEST_MESSAGE("Testing South-Korean settlement holiday list...");

    std::vector<Date> expectedHol;
    expectedHol.emplace_back(1, January, 2004);
    expectedHol.emplace_back(21, January, 2004);
    expectedHol.emplace_back(22, January, 2004);
    expectedHol.emplace_back(23, January, 2004);
    expectedHol.emplace_back(1, March, 2004);
    expectedHol.emplace_back(5, April, 2004);
    expectedHol.emplace_back(15, April, 2004); // election day
    //    expectedHol.push_back(Date(1,May,2004)); // Saturday
    expectedHol.emplace_back(5, May, 2004);
    expectedHol.emplace_back(26, May, 2004);
    //    expectedHol.push_back(Date(6,June,2004)); // Sunday
    //    expectedHol.push_back(Date(17,July,2004)); // Saturday
    //    expectedHol.push_back(Date(15,August,2004)); // Sunday
    expectedHol.emplace_back(27, September, 2004);
    expectedHol.emplace_back(28, September, 2004);
    expectedHol.emplace_back(29, September, 2004);
    //    expectedHol.push_back(Date(3,October,2004)); // Sunday
    //    expectedHol.push_back(Date(25,December,2004)); // Saturday

    //    expectedHol.push_back(Date(1,January,2005)); // Saturday
    expectedHol.emplace_back(8, February, 2005);
    expectedHol.emplace_back(9, February, 2005);
    expectedHol.emplace_back(10, February, 2005);
    expectedHol.emplace_back(1, March, 2005);
    expectedHol.emplace_back(5, April, 2005);
    expectedHol.emplace_back(5, May, 2005);
    //    expectedHol.push_back(Date(15,May,2005)); // Sunday
    expectedHol.emplace_back(6, June, 2005);
    //    expectedHol.push_back(Date(17,July,2005)); // Sunday
    expectedHol.emplace_back(15, August, 2005);
    //    expectedHol.push_back(Date(17,September,2005)); // Saturday
    //    expectedHol.push_back(Date(18,September,2005)); // Sunday
    expectedHol.emplace_back(19, September, 2005);
    expectedHol.emplace_back(3, October, 2005);
    //    expectedHol.push_back(Date(25,December,2005)); // Sunday

    //    expectedHol.push_back(Date(1,January,2006)); // Sunday
    //    expectedHol.push_back(Date(28,January,2006)); // Saturday
    //    expectedHol.push_back(Date(29,January,2006)); // Sunday
    expectedHol.emplace_back(30, January, 2006);
    expectedHol.emplace_back(1, March, 2006);
    expectedHol.emplace_back(1, May, 2006);
    expectedHol.emplace_back(5, May, 2006);
    expectedHol.emplace_back(31, May, 2006); // election
    expectedHol.emplace_back(6, June, 2006);
    expectedHol.emplace_back(17, July, 2006);
    expectedHol.emplace_back(15, August, 2006);
    expectedHol.emplace_back(3, October, 2006);
    expectedHol.emplace_back(5, October, 2006);
    expectedHol.emplace_back(6, October, 2006);
    //    expectedHol.push_back(Date(7,October,2006)); // Saturday
    expectedHol.emplace_back(25, December, 2006);

    expectedHol.emplace_back(1, January, 2007);
    //    expectedHol.push_back(Date(17,February,2007)); // Saturday
    //    expectedHol.push_back(Date(18,February,2007)); // Sunday
    expectedHol.emplace_back(19, February, 2007);
    expectedHol.emplace_back(1, March, 2007);
    expectedHol.emplace_back(1, May, 2007);
    //    expectedHol.push_back(Date(5,May,2007)); // Saturday
    expectedHol.emplace_back(24, May, 2007);
    expectedHol.emplace_back(6, June, 2007);
    expectedHol.emplace_back(17, July, 2007);
    expectedHol.emplace_back(15, August, 2007);
    expectedHol.emplace_back(24, September, 2007);
    expectedHol.emplace_back(25, September, 2007);
    expectedHol.emplace_back(26, September, 2007);
    expectedHol.emplace_back(3, October, 2007);
    expectedHol.emplace_back(19, December, 2007); // election
    expectedHol.emplace_back(25, December, 2007);
    expectedHol.emplace_back(1, January, 2008);
    expectedHol.emplace_back(6, February, 2008);
    expectedHol.emplace_back(7, February, 2008);
    expectedHol.emplace_back(8, February, 2008);
    expectedHol.emplace_back(9, April, 2008);
    expectedHol.emplace_back(1, May, 2008);
    expectedHol.emplace_back(5, May, 2008);
    expectedHol.emplace_back(12, May, 2008);
    expectedHol.emplace_back(6, June, 2008);
    expectedHol.emplace_back(15, August, 2008);
    expectedHol.emplace_back(15, September, 2008);
    expectedHol.emplace_back(3, October, 2008);
    expectedHol.emplace_back(25, December, 2008);
    expectedHol.emplace_back(1, January, 2009);
    expectedHol.emplace_back(26, January, 2009);
    expectedHol.emplace_back(27, January, 2009);
    expectedHol.emplace_back(1, May, 2009);
    expectedHol.emplace_back(5, May, 2009);
    expectedHol.emplace_back(2, October, 2009);
    expectedHol.emplace_back(25, December, 2009);
    expectedHol.emplace_back(1, January, 2010);
    expectedHol.emplace_back(15, February, 2010);
    expectedHol.emplace_back(1, March, 2010);
    expectedHol.emplace_back(5, May, 2010);
    expectedHol.emplace_back(21, May, 2010);
    expectedHol.emplace_back(2, June, 2010);
    expectedHol.emplace_back(21, September, 2010);
    expectedHol.emplace_back(22, September, 2010);
    expectedHol.emplace_back(23, September, 2010);
    expectedHol.emplace_back(2, February, 2011);
    expectedHol.emplace_back(3, February, 2011);
    expectedHol.emplace_back(4, February, 2011);
    expectedHol.emplace_back(1, March, 2011);
    expectedHol.emplace_back(5, May, 2011);
    expectedHol.emplace_back(10, May, 2011);
    expectedHol.emplace_back(6, June, 2011);
    expectedHol.emplace_back(15, August, 2011);
    expectedHol.emplace_back(12, September, 2011);
    expectedHol.emplace_back(13, September, 2011);
    expectedHol.emplace_back(3, October, 2011);
    expectedHol.emplace_back(23, January, 2012);
    expectedHol.emplace_back(24, January, 2012);
    expectedHol.emplace_back(1, March, 2012);
    expectedHol.emplace_back(11, April, 2012);
    expectedHol.emplace_back(1, May, 2012);
    expectedHol.emplace_back(28, May, 2012);
    expectedHol.emplace_back(6, June, 2012);
    expectedHol.emplace_back(15, August, 2012);
    expectedHol.emplace_back(1, October, 2012);
    expectedHol.emplace_back(3, October, 2012);
    expectedHol.emplace_back(19, December, 2012);
    expectedHol.emplace_back(25, December, 2012);
    expectedHol.emplace_back(1, January, 2013);
    expectedHol.emplace_back(11, February, 2013);
    expectedHol.emplace_back(1, March, 2013);
    expectedHol.emplace_back(1, May, 2013);
    expectedHol.emplace_back(17, May, 2013);
    expectedHol.emplace_back(6, June, 2013);
    expectedHol.emplace_back(15, August, 2013);
    expectedHol.emplace_back(18, September, 2013);
    expectedHol.emplace_back(19, September, 2013);
    expectedHol.emplace_back(20, September, 2013);
    expectedHol.emplace_back(3, October, 2013);
    expectedHol.emplace_back(9, October, 2013);
    expectedHol.emplace_back(25, December, 2013);
    expectedHol.emplace_back(1, January, 2014);
    expectedHol.emplace_back(30, January, 2014);
    expectedHol.emplace_back(31, January, 2014);
    expectedHol.emplace_back(1, May, 2014);
    expectedHol.emplace_back(5, May, 2014);
    expectedHol.emplace_back(6, May, 2014);
    expectedHol.emplace_back(4, June, 2014);
    expectedHol.emplace_back(6, June, 2014);
    expectedHol.emplace_back(15, August, 2014);
    expectedHol.emplace_back(8, September, 2014);
    expectedHol.emplace_back(9, September, 2014);
    expectedHol.emplace_back(10, September, 2014);
    expectedHol.emplace_back(3, October, 2014);
    expectedHol.emplace_back(9, October, 2014);
    expectedHol.emplace_back(25, December, 2014);
    expectedHol.emplace_back(1, January, 2015);
    expectedHol.emplace_back(18, February, 2015);
    expectedHol.emplace_back(19, February, 2015);
    expectedHol.emplace_back(20, February, 2015);
    expectedHol.emplace_back(1, May, 2015);
    expectedHol.emplace_back(5, May, 2015);
    expectedHol.emplace_back(25, May, 2015);
    expectedHol.emplace_back(14, August, 2015);
    expectedHol.emplace_back(28, September, 2015);
    expectedHol.emplace_back(29, September, 2015);
    expectedHol.emplace_back(9, October, 2015);
    expectedHol.emplace_back(25, December, 2015);
    expectedHol.emplace_back(1, January, 2016);
    expectedHol.emplace_back(8, February, 2016);
    expectedHol.emplace_back(9, February, 2016);
    expectedHol.emplace_back(10, February, 2016);
    expectedHol.emplace_back(1, March, 2016);
    expectedHol.emplace_back(13, April, 2016);
    expectedHol.emplace_back(5, May, 2016);
    expectedHol.emplace_back(6, June, 2016);
    expectedHol.emplace_back(15, August, 2016);
    expectedHol.emplace_back(14, September, 2016);
    expectedHol.emplace_back(15, September, 2016);
    expectedHol.emplace_back(16, September, 2016);
    expectedHol.emplace_back(3, October, 2016);
    expectedHol.emplace_back(27, January, 2017);
    expectedHol.emplace_back(30, January, 2017);
    expectedHol.emplace_back(1, March, 2017);
    expectedHol.emplace_back(1, May, 2017);
    expectedHol.emplace_back(3, May, 2017);
    expectedHol.emplace_back(5, May, 2017);
    expectedHol.emplace_back(9, May, 2017);
    expectedHol.emplace_back(6, June, 2017);
    expectedHol.emplace_back(15, August, 2017);
    expectedHol.emplace_back(3, October, 2017);
    expectedHol.emplace_back(4, October, 2017);
    expectedHol.emplace_back(5, October, 2017);
    expectedHol.emplace_back(6, October, 2017);
    expectedHol.emplace_back(9, October, 2017);
    expectedHol.emplace_back(25, December, 2017);
    expectedHol.emplace_back(1, January, 2018);
    expectedHol.emplace_back(15, February, 2018);
    expectedHol.emplace_back(16, February, 2018);
    expectedHol.emplace_back(1, March, 2018);
    expectedHol.emplace_back(1, May, 2018);
    expectedHol.emplace_back(7, May, 2018);
    expectedHol.emplace_back(22, May, 2018);
    expectedHol.emplace_back(6, June, 2018);
    expectedHol.emplace_back(13, June, 2018);
    expectedHol.emplace_back(15, August, 2018);
    expectedHol.emplace_back(24, September, 2018);
    expectedHol.emplace_back(25, September, 2018);
    expectedHol.emplace_back(26, September, 2018);
    expectedHol.emplace_back(3, October, 2018);
    expectedHol.emplace_back(9, October, 2018);
    expectedHol.emplace_back(25, December, 2018);
    expectedHol.emplace_back(1, January, 2019);
    expectedHol.emplace_back(4, February, 2019);
    expectedHol.emplace_back(5, February, 2019);
    expectedHol.emplace_back(6, February, 2019);
    expectedHol.emplace_back(1, March, 2019);
    expectedHol.emplace_back(1, May, 2019);
    expectedHol.emplace_back(6, May, 2019);
    expectedHol.emplace_back(6, June, 2019);
    expectedHol.emplace_back(15, August, 2019);
    expectedHol.emplace_back(12, September, 2019);
    expectedHol.emplace_back(13, September, 2019);
    expectedHol.emplace_back(3, October, 2019);
    expectedHol.emplace_back(9, October, 2019);
    expectedHol.emplace_back(25, December, 2019);
    expectedHol.emplace_back(1, January, 2020);
    expectedHol.emplace_back(24, January, 2020);
    expectedHol.emplace_back(27, January, 2020);
    expectedHol.emplace_back(15, April, 2020);
    expectedHol.emplace_back(30, April, 2020);
    expectedHol.emplace_back(1, May, 2020);
    expectedHol.emplace_back(5, May, 2020);
    expectedHol.emplace_back(17, August, 2020);
    expectedHol.emplace_back(30, September, 2020);
    expectedHol.emplace_back(1, October, 2020);
    expectedHol.emplace_back(2, October, 2020);
    expectedHol.emplace_back(9, October, 2020);
    expectedHol.emplace_back(25, December, 2020);
    expectedHol.emplace_back(1, January, 2021);
    expectedHol.emplace_back(11, February, 2021);
    expectedHol.emplace_back(12, February, 2021);
    expectedHol.emplace_back(1, March, 2021);
    expectedHol.emplace_back(5, May, 2021);
    expectedHol.emplace_back(19, May, 2021);
    expectedHol.emplace_back(16, August, 2021);
    expectedHol.emplace_back(20, September, 2021);
    expectedHol.emplace_back(21, September, 2021);
    expectedHol.emplace_back(22, September, 2021);
    expectedHol.emplace_back(4, October, 2021);
    expectedHol.emplace_back(11, October, 2021);
    expectedHol.emplace_back(31, January, 2022);
    expectedHol.emplace_back(1, February, 2022);
    expectedHol.emplace_back(2, February, 2022);
    expectedHol.emplace_back(1, March, 2022);
    expectedHol.emplace_back(9, March, 2022);
    expectedHol.emplace_back(5, May, 2022);
    expectedHol.emplace_back(1, June, 2022);
    expectedHol.emplace_back(6, June, 2022);
    expectedHol.emplace_back(15, August, 2022);
    expectedHol.emplace_back(9, September, 2022);
    expectedHol.emplace_back(12, September, 2022);
    expectedHol.emplace_back(3, October, 2022);
    expectedHol.emplace_back(10, October, 2022);
    // expectedHol.emplace_back(1, January, 2023);    // Sunday
    // expectedHol.emplace_back(21, January, 2023);    // Saturday
    expectedHol.emplace_back(23, January, 2023);
    expectedHol.emplace_back(24, January, 2023);
    expectedHol.emplace_back(1, March, 2023);
    expectedHol.emplace_back(1, May, 2023);
    expectedHol.emplace_back(5, May, 2023);
    expectedHol.emplace_back(29, May, 2023);
    expectedHol.emplace_back(6, June, 2023);
    expectedHol.emplace_back(15, August, 2023);
    expectedHol.emplace_back(28, September, 2023);
    expectedHol.emplace_back(29, September, 2023);
    // expectedHol.emplace_back(30, September, 2023);    // Saturday
    expectedHol.emplace_back(2, October, 2023);
    expectedHol.emplace_back(3, October, 2023);
    expectedHol.emplace_back(9, October, 2023);
    expectedHol.emplace_back(25, December, 2023);
    expectedHol.emplace_back(1, January, 2024);
    expectedHol.emplace_back(9, February, 2024);
    // expectedHol.emplace_back(10, February, 2024);    // Saturday
    expectedHol.emplace_back(12, February, 2024);
    expectedHol.emplace_back(1, March, 2024);
    expectedHol.emplace_back(10, April, 2024);
    expectedHol.emplace_back(1, May, 2024);
    expectedHol.emplace_back(6, May, 2024);
    expectedHol.emplace_back(15, May, 2024);
    expectedHol.emplace_back(6, June, 2024);
    expectedHol.emplace_back(15, August, 2024);
    expectedHol.emplace_back(16, September, 2024);
    expectedHol.emplace_back(17, September, 2024);
    expectedHol.emplace_back(18, September, 2024);
    expectedHol.emplace_back(3, October, 2024);
    expectedHol.emplace_back(9, October, 2024);
    expectedHol.emplace_back(25, December, 2024);
    expectedHol.emplace_back(1, January, 2025);
    expectedHol.emplace_back(28, January, 2025);
    expectedHol.emplace_back(29, January, 2025);
    expectedHol.emplace_back(30, January, 2025);
    expectedHol.emplace_back(3, March, 2025);
    expectedHol.emplace_back(1, May, 2025);
    expectedHol.emplace_back(5, May, 2025);
    expectedHol.emplace_back(6, May, 2025);
    expectedHol.emplace_back(6, June, 2025);
    expectedHol.emplace_back(15, August, 2025);
    expectedHol.emplace_back(3, October, 2025);
    expectedHol.emplace_back(6, October, 2025);
    expectedHol.emplace_back(7, October, 2025);
    expectedHol.emplace_back(8, October, 2025);
    expectedHol.emplace_back(9, October, 2025);
    expectedHol.emplace_back(25, December, 2025);
    expectedHol.emplace_back(1, January, 2026);
    expectedHol.emplace_back(16, February, 2026);
    expectedHol.emplace_back(17, February, 2026);
    expectedHol.emplace_back(18, February, 2026);
    expectedHol.emplace_back(2, March, 2026);
    expectedHol.emplace_back(1, May, 2026);
    expectedHol.emplace_back(5, May, 2026);
    expectedHol.emplace_back(25, May, 2026);
    // expectedHol.emplace_back(6, June, 2026);    // Saturday
    expectedHol.emplace_back(17, August, 2026);
    expectedHol.emplace_back(24, September, 2026);
    expectedHol.emplace_back(25, September, 2026);
    // expectedHol.emplace_back(26, September, 2026);    // Saturday
    expectedHol.emplace_back(5, October, 2026);
    expectedHol.emplace_back(9, October, 2026);
    expectedHol.emplace_back(25, December, 2026);
    expectedHol.emplace_back(1, January, 2027);
    // expectedHol.emplace_back(6, February, 2027);    // Saturday
    expectedHol.emplace_back(8, February, 2027);
    expectedHol.emplace_back(9, February, 2027);
    expectedHol.emplace_back(1, March, 2027);
    // expectedHol.emplace_back(1, May, 2027);    // Saturday
    expectedHol.emplace_back(5, May, 2027);
    expectedHol.emplace_back(13, May, 2027);
    // expectedHol.emplace_back(6, June, 2027);    // Sunday
    expectedHol.emplace_back(16, August, 2027);
    expectedHol.emplace_back(14, September, 2027);
    expectedHol.emplace_back(15, September, 2027);
    expectedHol.emplace_back(16, September, 2027);
    expectedHol.emplace_back(4, October, 2027);
    expectedHol.emplace_back(11, October, 2027);
    expectedHol.emplace_back(27, December, 2027);
    // expectedHol.emplace_back(1, January, 2028);    // Saturday
    expectedHol.emplace_back(26, January, 2028);
    expectedHol.emplace_back(27, January, 2028);
    expectedHol.emplace_back(28, January, 2028);
    expectedHol.emplace_back(1, March, 2028);
    expectedHol.emplace_back(1, May, 2028);
    expectedHol.emplace_back(2, May, 2028);
    expectedHol.emplace_back(5, May, 2028);
    expectedHol.emplace_back(6, June, 2028);
    expectedHol.emplace_back(15, August, 2028);
    expectedHol.emplace_back(2, October, 2028);
    expectedHol.emplace_back(3, October, 2028);
    expectedHol.emplace_back(4, October, 2028);
    expectedHol.emplace_back(5, October, 2028);
    expectedHol.emplace_back(9, October, 2028);
    expectedHol.emplace_back(25, December, 2028);
    expectedHol.emplace_back(1, January, 2029);
    expectedHol.emplace_back(12, February, 2029);
    expectedHol.emplace_back(13, February, 2029);
    expectedHol.emplace_back(14, February, 2029);
    expectedHol.emplace_back(1, March, 2029);
    expectedHol.emplace_back(1, May, 2029);
    expectedHol.emplace_back(7, May, 2029);
    expectedHol.emplace_back(21, May, 2029);
    expectedHol.emplace_back(6, June, 2029);
    expectedHol.emplace_back(15, August, 2029);
    expectedHol.emplace_back(21, September, 2029);
    // expectedHol.emplace_back(22, September, 2029);    // Saturday
    expectedHol.emplace_back(24, September, 2029);
    expectedHol.emplace_back(3, October, 2029);
    expectedHol.emplace_back(9, October, 2029);
    expectedHol.emplace_back(25, December, 2029);
    expectedHol.emplace_back(1, January, 2030);
    // expectedHol.emplace_back(2, February, 2030);    // Saturday
    expectedHol.emplace_back(4, February, 2030);
    expectedHol.emplace_back(5, February, 2030);
    expectedHol.emplace_back(1, March, 2030);
    expectedHol.emplace_back(1, May, 2030);
    expectedHol.emplace_back(6, May, 2030);
    expectedHol.emplace_back(9, May, 2030);
    expectedHol.emplace_back(6, June, 2030);
    expectedHol.emplace_back(15, August, 2030);
    expectedHol.emplace_back(11, September, 2030);
    expectedHol.emplace_back(12, September, 2030);
    expectedHol.emplace_back(13, September, 2030);
    expectedHol.emplace_back(3, October, 2030);
    expectedHol.emplace_back(9, October, 2030);
    expectedHol.emplace_back(25, December, 2030);
    expectedHol.emplace_back(1, January, 2031);
    expectedHol.emplace_back(22, January, 2031);
    expectedHol.emplace_back(23, January, 2031);
    expectedHol.emplace_back(24, January, 2031);
    expectedHol.emplace_back(3, March, 2031);
    expectedHol.emplace_back(1, May, 2031);
    expectedHol.emplace_back(5, May, 2031);
    expectedHol.emplace_back(28, May, 2031);
    expectedHol.emplace_back(6, June, 2031);
    expectedHol.emplace_back(15, August, 2031);
    expectedHol.emplace_back(30, September, 2031);
    expectedHol.emplace_back(1, October, 2031);
    expectedHol.emplace_back(2, October, 2031);
    expectedHol.emplace_back(3, October, 2031);
    expectedHol.emplace_back(9, October, 2031);
    expectedHol.emplace_back(25, December, 2031);
    expectedHol.emplace_back(1, January, 2032);
    expectedHol.emplace_back(10, February, 2032);
    expectedHol.emplace_back(11, February, 2032);
    expectedHol.emplace_back(12, February, 2032);
    expectedHol.emplace_back(1, March, 2032);
    // expectedHol.emplace_back(1, May, 2032);    // Saturday
    expectedHol.emplace_back(5, May, 2032);
    expectedHol.emplace_back(17, May, 2032);
    // expectedHol.emplace_back(6, June, 2032);    // Sunday
    expectedHol.emplace_back(16, August, 2032);
    // expectedHol.emplace_back(18, September, 2032);    // Saturday
    expectedHol.emplace_back(20, September, 2032);
    expectedHol.emplace_back(21, September, 2032);
    expectedHol.emplace_back(4, October, 2032);
    expectedHol.emplace_back(11, October, 2032);
    expectedHol.emplace_back(27, December, 2032);
    // expectedHol.emplace_back(1, January, 2033);    // Saturday
    expectedHol.emplace_back(31, January, 2033);
    expectedHol.emplace_back(1, February, 2033);
    expectedHol.emplace_back(2, February, 2033);
    expectedHol.emplace_back(1, March, 2033);
    // expectedHol.emplace_back(1, May, 2033);    // Sunday
    expectedHol.emplace_back(5, May, 2033);
    expectedHol.emplace_back(6, May, 2033);
    expectedHol.emplace_back(6, June, 2033);
    expectedHol.emplace_back(15, August, 2033);
    expectedHol.emplace_back(7, September, 2033);
    expectedHol.emplace_back(8, September, 2033);
    expectedHol.emplace_back(9, September, 2033);
    expectedHol.emplace_back(3, October, 2033);
    expectedHol.emplace_back(10, October, 2033);
    expectedHol.emplace_back(26, December, 2033);
    // expectedHol.emplace_back(1, January, 2034);    // Sunday
    // expectedHol.emplace_back(18, February, 2034);    // Saturday
    expectedHol.emplace_back(20, February, 2034);
    expectedHol.emplace_back(21, February, 2034);
    expectedHol.emplace_back(1, March, 2034);
    expectedHol.emplace_back(1, May, 2034);
    expectedHol.emplace_back(5, May, 2034);
    expectedHol.emplace_back(25, May, 2034);
    expectedHol.emplace_back(6, June, 2034);
    expectedHol.emplace_back(15, August, 2034);
    expectedHol.emplace_back(26, September, 2034);
    expectedHol.emplace_back(27, September, 2034);
    expectedHol.emplace_back(28, September, 2034);
    expectedHol.emplace_back(3, October, 2034);
    expectedHol.emplace_back(9, October, 2034);
    expectedHol.emplace_back(25, December, 2034);
    expectedHol.emplace_back(1, January, 2035);
    expectedHol.emplace_back(7, February, 2035);
    expectedHol.emplace_back(8, February, 2035);
    expectedHol.emplace_back(9, February, 2035);
    expectedHol.emplace_back(1, March, 2035);
    expectedHol.emplace_back(1, May, 2035);
    expectedHol.emplace_back(7, May, 2035);
    expectedHol.emplace_back(15, May, 2035);
    expectedHol.emplace_back(6, June, 2035);
    expectedHol.emplace_back(15, August, 2035);
    // expectedHol.emplace_back(15, September, 2035);    // Saturday
    expectedHol.emplace_back(17, September, 2035);
    expectedHol.emplace_back(18, September, 2035);
    expectedHol.emplace_back(3, October, 2035);
    expectedHol.emplace_back(9, October, 2035);
    expectedHol.emplace_back(25, December, 2035);
    expectedHol.emplace_back(1, January, 2036);
    expectedHol.emplace_back(28, January, 2036);
    expectedHol.emplace_back(29, January, 2036);
    expectedHol.emplace_back(30, January, 2036);
    expectedHol.emplace_back(3, March, 2036);
    expectedHol.emplace_back(1, May, 2036);
    expectedHol.emplace_back(5, May, 2036);
    expectedHol.emplace_back(6, May, 2036);
    expectedHol.emplace_back(6, June, 2036);
    expectedHol.emplace_back(15, August, 2036);
    expectedHol.emplace_back(3, October, 2036);
    // expectedHol.emplace_back(4, October, 2036);    // Saturday
    expectedHol.emplace_back(6, October, 2036);
    expectedHol.emplace_back(7, October, 2036);
    expectedHol.emplace_back(9, October, 2036);
    expectedHol.emplace_back(25, December, 2036);
    expectedHol.emplace_back(1, January, 2037);
    // expectedHol.emplace_back(14, February, 2037);    // Saturday
    expectedHol.emplace_back(16, February, 2037);
    expectedHol.emplace_back(17, February, 2037);
    expectedHol.emplace_back(2, March, 2037);
    expectedHol.emplace_back(1, May, 2037);
    expectedHol.emplace_back(5, May, 2037);
    expectedHol.emplace_back(22, May, 2037);
    // expectedHol.emplace_back(6, June, 2037);    // Saturday
    expectedHol.emplace_back(17, August, 2037);
    expectedHol.emplace_back(23, September, 2037);
    expectedHol.emplace_back(24, September, 2037);
    expectedHol.emplace_back(25, September, 2037);
    expectedHol.emplace_back(5, October, 2037);
    expectedHol.emplace_back(9, October, 2037);
    expectedHol.emplace_back(25, December, 2037);
    expectedHol.emplace_back(1, January, 2038);
    expectedHol.emplace_back(3, February, 2038);
    expectedHol.emplace_back(4, February, 2038);
    expectedHol.emplace_back(5, February, 2038);
    expectedHol.emplace_back(1, March, 2038);
    // expectedHol.emplace_back(1, May, 2038);    // Saturday
    expectedHol.emplace_back(5, May, 2038);
    expectedHol.emplace_back(11, May, 2038);
    // expectedHol.emplace_back(6, June, 2038);    // Sunday
    expectedHol.emplace_back(16, August, 2038);
    expectedHol.emplace_back(13, September, 2038);
    expectedHol.emplace_back(14, September, 2038);
    expectedHol.emplace_back(15, September, 2038);
    expectedHol.emplace_back(4, October, 2038);
    expectedHol.emplace_back(11, October, 2038);
    expectedHol.emplace_back(27, December, 2038);
    // expectedHol.emplace_back(1, January, 2039);    // Saturday
    expectedHol.emplace_back(24, January, 2039);
    expectedHol.emplace_back(25, January, 2039);
    expectedHol.emplace_back(26, January, 2039);
    expectedHol.emplace_back(1, March, 2039);
    // expectedHol.emplace_back(1, May, 2039);    // Sunday
    expectedHol.emplace_back(2, May, 2039);
    expectedHol.emplace_back(5, May, 2039);
    expectedHol.emplace_back(6, June, 2039);
    expectedHol.emplace_back(15, August, 2039);
    // expectedHol.emplace_back(1, October, 2039);    // Saturday
    expectedHol.emplace_back(3, October, 2039);
    expectedHol.emplace_back(4, October, 2039);
    expectedHol.emplace_back(5, October, 2039);
    expectedHol.emplace_back(10, October, 2039);
    expectedHol.emplace_back(26, December, 2039);
    // expectedHol.emplace_back(1, January, 2040);    // Sunday
    // expectedHol.emplace_back(11, February, 2040);    // Saturday
    expectedHol.emplace_back(13, February, 2040);
    expectedHol.emplace_back(14, February, 2040);
    expectedHol.emplace_back(1, March, 2040);
    expectedHol.emplace_back(1, May, 2040);
    expectedHol.emplace_back(7, May, 2040);
    expectedHol.emplace_back(18, May, 2040);
    expectedHol.emplace_back(6, June, 2040);
    expectedHol.emplace_back(15, August, 2040);
    expectedHol.emplace_back(20, September, 2040);
    expectedHol.emplace_back(21, September, 2040);
    // expectedHol.emplace_back(22, September, 2040);    // Saturday
    expectedHol.emplace_back(3, October, 2040);
    expectedHol.emplace_back(9, October, 2040);
    expectedHol.emplace_back(25, December, 2040);
    expectedHol.emplace_back(1, January, 2041);
    expectedHol.emplace_back(31, January, 2041);
    expectedHol.emplace_back(1, February, 2041);
    // expectedHol.emplace_back(2, February, 2041);    // Saturday
    expectedHol.emplace_back(1, March, 2041);
    expectedHol.emplace_back(1, May, 2041);
    expectedHol.emplace_back(6, May, 2041);
    expectedHol.emplace_back(7, May, 2041);
    expectedHol.emplace_back(6, June, 2041);
    expectedHol.emplace_back(15, August, 2041);
    expectedHol.emplace_back(9, September, 2041);
    expectedHol.emplace_back(10, September, 2041);
    expectedHol.emplace_back(11, September, 2041);
    expectedHol.emplace_back(3, October, 2041);
    expectedHol.emplace_back(9, October, 2041);
    expectedHol.emplace_back(25, December, 2041);
    expectedHol.emplace_back(1, January, 2042);
    expectedHol.emplace_back(21, January, 2042);
    expectedHol.emplace_back(22, January, 2042);
    expectedHol.emplace_back(23, January, 2042);
    expectedHol.emplace_back(3, March, 2042);
    expectedHol.emplace_back(1, May, 2042);
    expectedHol.emplace_back(5, May, 2042);
    expectedHol.emplace_back(26, May, 2042);
    expectedHol.emplace_back(6, June, 2042);
    expectedHol.emplace_back(15, August, 2042);
    // expectedHol.emplace_back(27, September, 2042);    // Saturday
    expectedHol.emplace_back(29, September, 2042);
    expectedHol.emplace_back(30, September, 2042);
    expectedHol.emplace_back(3, October, 2042);
    expectedHol.emplace_back(9, October, 2042);
    expectedHol.emplace_back(25, December, 2042);
    expectedHol.emplace_back(1, January, 2043);
    expectedHol.emplace_back(9, February, 2043);
    expectedHol.emplace_back(10, February, 2043);
    expectedHol.emplace_back(11, February, 2043);
    expectedHol.emplace_back(2, March, 2043);
    expectedHol.emplace_back(1, May, 2043);
    expectedHol.emplace_back(5, May, 2043);
    expectedHol.emplace_back(18, May, 2043);
    // expectedHol.emplace_back(6, June, 2043);    // Saturday
    expectedHol.emplace_back(17, August, 2043);
    expectedHol.emplace_back(16, September, 2043);
    expectedHol.emplace_back(17, September, 2043);
    expectedHol.emplace_back(18, September, 2043);
    expectedHol.emplace_back(5, October, 2043);
    expectedHol.emplace_back(9, October, 2043);
    expectedHol.emplace_back(25, December, 2043);
    expectedHol.emplace_back(1, January, 2044);
    expectedHol.emplace_back(29, January, 2044);
    // expectedHol.emplace_back(30, January, 2044);    // Saturday
    expectedHol.emplace_back(1, February, 2044);
    expectedHol.emplace_back(1, March, 2044);
    // expectedHol.emplace_back(1, May, 2044);    // Sunday
    expectedHol.emplace_back(5, May, 2044);
    expectedHol.emplace_back(6, May, 2044);
    expectedHol.emplace_back(6, June, 2044);
    expectedHol.emplace_back(15, August, 2044);
    expectedHol.emplace_back(3, October, 2044);
    expectedHol.emplace_back(4, October, 2044);
    expectedHol.emplace_back(5, October, 2044);
    expectedHol.emplace_back(6, October, 2044);
    expectedHol.emplace_back(10, October, 2044);
    expectedHol.emplace_back(26, December, 2044);
    // expectedHol.emplace_back(1, January, 2045);    // Sunday
    expectedHol.emplace_back(16, February, 2045);
    expectedHol.emplace_back(17, February, 2045);
    // expectedHol.emplace_back(18, February, 2045);    // Saturday
    expectedHol.emplace_back(1, March, 2045);
    expectedHol.emplace_back(1, May, 2045);
    expectedHol.emplace_back(5, May, 2045);
    expectedHol.emplace_back(24, May, 2045);
    expectedHol.emplace_back(6, June, 2045);
    expectedHol.emplace_back(15, August, 2045);
    expectedHol.emplace_back(25, September, 2045);
    expectedHol.emplace_back(26, September, 2045);
    expectedHol.emplace_back(27, September, 2045);
    expectedHol.emplace_back(3, October, 2045);
    expectedHol.emplace_back(9, October, 2045);
    expectedHol.emplace_back(25, December, 2045);
    expectedHol.emplace_back(1, January, 2046);
    expectedHol.emplace_back(5, February, 2046);
    expectedHol.emplace_back(6, February, 2046);
    expectedHol.emplace_back(7, February, 2046);
    expectedHol.emplace_back(1, March, 2046);
    expectedHol.emplace_back(1, May, 2046);
    expectedHol.emplace_back(7, May, 2046);
    expectedHol.emplace_back(14, May, 2046);
    expectedHol.emplace_back(6, June, 2046);
    expectedHol.emplace_back(15, August, 2046);
    expectedHol.emplace_back(14, September, 2046);
    // expectedHol.emplace_back(15, September, 2046);    // Saturday
    expectedHol.emplace_back(17, September, 2046);
    expectedHol.emplace_back(3, October, 2046);
    expectedHol.emplace_back(9, October, 2046);
    expectedHol.emplace_back(25, December, 2046);
    expectedHol.emplace_back(1, January, 2047);
    expectedHol.emplace_back(25, January, 2047);
    // expectedHol.emplace_back(26, January, 2047);    // Saturday
    expectedHol.emplace_back(28, January, 2047);
    expectedHol.emplace_back(1, March, 2047);
    expectedHol.emplace_back(1, May, 2047);
    expectedHol.emplace_back(2, May, 2047);
    expectedHol.emplace_back(6, May, 2047);
    expectedHol.emplace_back(6, June, 2047);
    expectedHol.emplace_back(15, August, 2047);
    expectedHol.emplace_back(3, October, 2047);
    expectedHol.emplace_back(4, October, 2047);
    // expectedHol.emplace_back(5, October, 2047);    // Saturday
    expectedHol.emplace_back(7, October, 2047);
    expectedHol.emplace_back(9, October, 2047);
    expectedHol.emplace_back(25, December, 2047);
    expectedHol.emplace_back(1, January, 2048);
    expectedHol.emplace_back(13, February, 2048);
    expectedHol.emplace_back(14, February, 2048);
    // expectedHol.emplace_back(15, February, 2048);    // Saturday
    expectedHol.emplace_back(2, March, 2048);
    expectedHol.emplace_back(1, May, 2048);
    expectedHol.emplace_back(5, May, 2048);
    expectedHol.emplace_back(20, May, 2048);
    // expectedHol.emplace_back(6, June, 2048);    // Saturday
    expectedHol.emplace_back(17, August, 2048);
    expectedHol.emplace_back(21, September, 2048);
    expectedHol.emplace_back(22, September, 2048);
    expectedHol.emplace_back(23, September, 2048);
    expectedHol.emplace_back(5, October, 2048);
    expectedHol.emplace_back(9, October, 2048);
    expectedHol.emplace_back(25, December, 2048);
    expectedHol.emplace_back(1, January, 2049);
    expectedHol.emplace_back(1, February, 2049);
    expectedHol.emplace_back(2, February, 2049);
    expectedHol.emplace_back(3, February, 2049);
    expectedHol.emplace_back(1, March, 2049);
    // expectedHol.emplace_back(1, May, 2049);    // Saturday
    expectedHol.emplace_back(5, May, 2049);
    expectedHol.emplace_back(10, May, 2049);
    // expectedHol.emplace_back(6, June, 2049);    // Sunday
    expectedHol.emplace_back(16, August, 2049);
    expectedHol.emplace_back(10, September, 2049);
    // expectedHol.emplace_back(11, September, 2049);    // Saturday
    expectedHol.emplace_back(13, September, 2049);
    expectedHol.emplace_back(4, October, 2049);
    expectedHol.emplace_back(11, October, 2049);
    expectedHol.emplace_back(27, December, 2049);
    // expectedHol.emplace_back(1, January, 2050);    // Saturday
    // expectedHol.emplace_back(22, January, 2050);    // Saturday
    expectedHol.emplace_back(24, January, 2050);
    expectedHol.emplace_back(25, January, 2050);
    expectedHol.emplace_back(1, March, 2050);
    // expectedHol.emplace_back(1, May, 2050);    // Sunday
    expectedHol.emplace_back(5, May, 2050);
    expectedHol.emplace_back(30, May, 2050);
    expectedHol.emplace_back(6, June, 2050);
    expectedHol.emplace_back(15, August, 2050);
    expectedHol.emplace_back(29, September, 2050);
    expectedHol.emplace_back(30, September, 2050);
    // expectedHol.emplace_back(1, October, 2050);    // Saturday
    expectedHol.emplace_back(3, October, 2050);
    expectedHol.emplace_back(10, October, 2050);
    expectedHol.emplace_back(26, December, 2050);

    Calendar c = SouthKorea(SouthKorea::Settlement);
    std::vector<Date> hol = c.holidayList(Date(1, January, 2004), Date(31, December, 2050));

    for (Size i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
}

BOOST_AUTO_TEST_CASE(testKoreaStockExchange) {
    BOOST_TEST_MESSAGE("Testing Korea Stock Exchange holiday list...");

    std::vector<Date> expectedHol;
    expectedHol.emplace_back(1, January, 2004);
    expectedHol.emplace_back(21, January, 2004);
    expectedHol.emplace_back(22, January, 2004);
    expectedHol.emplace_back(23, January, 2004);
    expectedHol.emplace_back(1, March, 2004);
    expectedHol.emplace_back(5, April, 2004);
    expectedHol.emplace_back(15, April, 2004); // election day
    //    expectedHol.push_back(Date(1,May,2004)); // Saturday
    expectedHol.emplace_back(5, May, 2004);
    expectedHol.emplace_back(26, May, 2004);
    //    expectedHol.push_back(Date(6,June,2004)); // Sunday
    //    expectedHol.push_back(Date(17,July,2004)); // Saturday
    //    expectedHol.push_back(Date(15,August,2004)); // Sunday
    expectedHol.emplace_back(27, September, 2004);
    expectedHol.emplace_back(28, September, 2004);
    expectedHol.emplace_back(29, September, 2004);
    //    expectedHol.push_back(Date(3,October,2004)); // Sunday
    //    expectedHol.push_back(Date(25,December,2004)); // Saturday
    expectedHol.emplace_back(31, December, 2004);

    //    expectedHol.push_back(Date(1,January,2005)); // Saturday
    expectedHol.emplace_back(8, February, 2005);
    expectedHol.emplace_back(9, February, 2005);
    expectedHol.emplace_back(10, February, 2005);
    expectedHol.emplace_back(1, March, 2005);
    expectedHol.emplace_back(5, April, 2005);
    expectedHol.emplace_back(5, May, 2005);
    //    expectedHol.push_back(Date(15,May,2005)); // Sunday
    expectedHol.emplace_back(6, June, 2005);
    //    expectedHol.push_back(Date(17,July,2005)); // Sunday
    expectedHol.emplace_back(15, August, 2005);
    //    expectedHol.push_back(Date(17,September,2005)); // Saturday
    //    expectedHol.push_back(Date(18,September,2005)); // Sunday
    expectedHol.emplace_back(19, September, 2005);
    expectedHol.emplace_back(3, October, 2005);
    //    expectedHol.push_back(Date(25,December,2005)); // Sunday
    expectedHol.emplace_back(30, December, 2005);

    //    expectedHol.push_back(Date(1,January,2006)); // Sunday
    //    expectedHol.push_back(Date(28,January,2006)); // Saturday
    //    expectedHol.push_back(Date(29,January,2006)); // Sunday
    expectedHol.emplace_back(30, January, 2006);
    expectedHol.emplace_back(1, March, 2006);
    expectedHol.emplace_back(1, May, 2006);
    expectedHol.emplace_back(5, May, 2006);
    expectedHol.emplace_back(31, May, 2006); // election
    expectedHol.emplace_back(6, June, 2006);
    expectedHol.emplace_back(17, July, 2006);
    expectedHol.emplace_back(15, August, 2006);
    expectedHol.emplace_back(3, October, 2006);
    expectedHol.emplace_back(5, October, 2006);
    expectedHol.emplace_back(6, October, 2006);
    //    expectedHol.push_back(Date(7,October,2006)); // Saturday
    expectedHol.emplace_back(25, December, 2006);
    expectedHol.emplace_back(29, December, 2006);

    expectedHol.emplace_back(1, January, 2007);
    //    expectedHol.push_back(Date(17,February,2007)); // Saturday
    //    expectedHol.push_back(Date(18,February,2007)); // Sunday
    expectedHol.emplace_back(19, February, 2007);
    expectedHol.emplace_back(1, March, 2007);
    expectedHol.emplace_back(1, May, 2007);
    //    expectedHol.push_back(Date(5,May,2007)); // Saturday
    expectedHol.emplace_back(24, May, 2007);
    expectedHol.emplace_back(6, June, 2007);
    expectedHol.emplace_back(17, July, 2007);
    expectedHol.emplace_back(15, August, 2007);
    expectedHol.emplace_back(24, September, 2007);
    expectedHol.emplace_back(25, September, 2007);
    expectedHol.emplace_back(26, September, 2007);
    expectedHol.emplace_back(3, October, 2007);
    expectedHol.emplace_back(19, December, 2007); // election
    expectedHol.emplace_back(25, December, 2007);
    expectedHol.emplace_back(31, December, 2007);
    expectedHol.emplace_back(1, January, 2008);
    expectedHol.emplace_back(6, February, 2008);
    expectedHol.emplace_back(7, February, 2008);
    expectedHol.emplace_back(8, February, 2008);
    expectedHol.emplace_back(9, April, 2008);
    expectedHol.emplace_back(1, May, 2008);
    expectedHol.emplace_back(5, May, 2008);
    expectedHol.emplace_back(12, May, 2008);
    expectedHol.emplace_back(6, June, 2008);
    expectedHol.emplace_back(15, August, 2008);
    expectedHol.emplace_back(15, September, 2008);
    expectedHol.emplace_back(3, October, 2008);
    expectedHol.emplace_back(25, December, 2008);
    expectedHol.emplace_back(31, December, 2008);
    expectedHol.emplace_back(1, January, 2009);
    expectedHol.emplace_back(26, January, 2009);
    expectedHol.emplace_back(27, January, 2009);
    expectedHol.emplace_back(1, May, 2009);
    expectedHol.emplace_back(5, May, 2009);
    expectedHol.emplace_back(2, October, 2009);
    expectedHol.emplace_back(25, December, 2009);
    expectedHol.emplace_back(31, December, 2009);
    expectedHol.emplace_back(1, January, 2010);
    expectedHol.emplace_back(15, February, 2010);
    expectedHol.emplace_back(1, March, 2010);
    expectedHol.emplace_back(5, May, 2010);
    expectedHol.emplace_back(21, May, 2010);
    expectedHol.emplace_back(2, June, 2010);
    expectedHol.emplace_back(21, September, 2010);
    expectedHol.emplace_back(22, September, 2010);
    expectedHol.emplace_back(23, September, 2010);
    expectedHol.emplace_back(31, December, 2010);
    expectedHol.emplace_back(2, February, 2011);
    expectedHol.emplace_back(3, February, 2011);
    expectedHol.emplace_back(4, February, 2011);
    expectedHol.emplace_back(1, March, 2011);
    expectedHol.emplace_back(5, May, 2011);
    expectedHol.emplace_back(10, May, 2011);
    expectedHol.emplace_back(6, June, 2011);
    expectedHol.emplace_back(15, August, 2011);
    expectedHol.emplace_back(12, September, 2011);
    expectedHol.emplace_back(13, September, 2011);
    expectedHol.emplace_back(3, October, 2011);
    expectedHol.emplace_back(30, December, 2011);
    expectedHol.emplace_back(23, January, 2012);
    expectedHol.emplace_back(24, January, 2012);
    expectedHol.emplace_back(1, March, 2012);
    expectedHol.emplace_back(11, April, 2012);
    expectedHol.emplace_back(1, May, 2012);
    expectedHol.emplace_back(28, May, 2012);
    expectedHol.emplace_back(6, June, 2012);
    expectedHol.emplace_back(15, August, 2012);
    expectedHol.emplace_back(1, October, 2012);
    expectedHol.emplace_back(3, October, 2012);
    expectedHol.emplace_back(19, December, 2012);
    expectedHol.emplace_back(25, December, 2012);
    expectedHol.emplace_back(31, December, 2012);
    expectedHol.emplace_back(1, January, 2013);
    expectedHol.emplace_back(11, February, 2013);
    expectedHol.emplace_back(1, March, 2013);
    expectedHol.emplace_back(1, May, 2013);
    expectedHol.emplace_back(17, May, 2013);
    expectedHol.emplace_back(6, June, 2013);
    expectedHol.emplace_back(15, August, 2013);
    expectedHol.emplace_back(18, September, 2013);
    expectedHol.emplace_back(19, September, 2013);
    expectedHol.emplace_back(20, September, 2013);
    expectedHol.emplace_back(3, October, 2013);
    expectedHol.emplace_back(9, October, 2013);
    expectedHol.emplace_back(25, December, 2013);
    expectedHol.emplace_back(31, December, 2013);
    expectedHol.emplace_back(1, January, 2014);
    expectedHol.emplace_back(30, January, 2014);
    expectedHol.emplace_back(31, January, 2014);
    expectedHol.emplace_back(1, May, 2014);
    expectedHol.emplace_back(5, May, 2014);
    expectedHol.emplace_back(6, May, 2014);
    expectedHol.emplace_back(4, June, 2014);
    expectedHol.emplace_back(6, June, 2014);
    expectedHol.emplace_back(15, August, 2014);
    expectedHol.emplace_back(8, September, 2014);
    expectedHol.emplace_back(9, September, 2014);
    expectedHol.emplace_back(10, September, 2014);
    expectedHol.emplace_back(3, October, 2014);
    expectedHol.emplace_back(9, October, 2014);
    expectedHol.emplace_back(25, December, 2014);
    expectedHol.emplace_back(31, December, 2014);
    expectedHol.emplace_back(1, January, 2015);
    expectedHol.emplace_back(18, February, 2015);
    expectedHol.emplace_back(19, February, 2015);
    expectedHol.emplace_back(20, February, 2015);
    expectedHol.emplace_back(1, May, 2015);
    expectedHol.emplace_back(5, May, 2015);
    expectedHol.emplace_back(25, May, 2015);
    expectedHol.emplace_back(14, August, 2015);
    expectedHol.emplace_back(28, September, 2015);
    expectedHol.emplace_back(29, September, 2015);
    expectedHol.emplace_back(9, October, 2015);
    expectedHol.emplace_back(25, December, 2015);
    expectedHol.emplace_back(31, December, 2015);
    expectedHol.emplace_back(1, January, 2016);
    expectedHol.emplace_back(8, February, 2016);
    expectedHol.emplace_back(9, February, 2016);
    expectedHol.emplace_back(10, February, 2016);
    expectedHol.emplace_back(1, March, 2016);
    expectedHol.emplace_back(13, April, 2016);
    expectedHol.emplace_back(5, May, 2016);
    expectedHol.emplace_back(6, May, 2016);
    expectedHol.emplace_back(6, June, 2016);
    expectedHol.emplace_back(15, August, 2016);
    expectedHol.emplace_back(14, September, 2016);
    expectedHol.emplace_back(15, September, 2016);
    expectedHol.emplace_back(16, September, 2016);
    expectedHol.emplace_back(3, October, 2016);
    expectedHol.emplace_back(30, December, 2016);
    expectedHol.emplace_back(27, January, 2017);
    expectedHol.emplace_back(30, January, 2017);
    expectedHol.emplace_back(1, March, 2017);
    expectedHol.emplace_back(1, May, 2017);
    expectedHol.emplace_back(3, May, 2017);
    expectedHol.emplace_back(5, May, 2017);
    expectedHol.emplace_back(9, May, 2017);
    expectedHol.emplace_back(6, June, 2017);
    expectedHol.emplace_back(15, August, 2017);
    expectedHol.emplace_back(2, October, 2017);
    expectedHol.emplace_back(3, October, 2017);
    expectedHol.emplace_back(4, October, 2017);
    expectedHol.emplace_back(5, October, 2017);
    expectedHol.emplace_back(6, October, 2017);
    expectedHol.emplace_back(9, October, 2017);
    expectedHol.emplace_back(25, December, 2017);
    expectedHol.emplace_back(29, December, 2017);
    expectedHol.emplace_back(1, January, 2018);
    expectedHol.emplace_back(15, February, 2018);
    expectedHol.emplace_back(16, February, 2018);
    expectedHol.emplace_back(1, March, 2018);
    expectedHol.emplace_back(1, May, 2018);
    expectedHol.emplace_back(7, May, 2018);
    expectedHol.emplace_back(22, May, 2018);
    expectedHol.emplace_back(6, June, 2018);
    expectedHol.emplace_back(13, June, 2018);
    expectedHol.emplace_back(15, August, 2018);
    expectedHol.emplace_back(24, September, 2018);
    expectedHol.emplace_back(25, September, 2018);
    expectedHol.emplace_back(26, September, 2018);
    expectedHol.emplace_back(3, October, 2018);
    expectedHol.emplace_back(9, October, 2018);
    expectedHol.emplace_back(25, December, 2018);
    expectedHol.emplace_back(31, December, 2018);
    expectedHol.emplace_back(1, January, 2019);
    expectedHol.emplace_back(4, February, 2019);
    expectedHol.emplace_back(5, February, 2019);
    expectedHol.emplace_back(6, February, 2019);
    expectedHol.emplace_back(1, March, 2019);
    expectedHol.emplace_back(1, May, 2019);
    expectedHol.emplace_back(6, May, 2019);
    expectedHol.emplace_back(6, June, 2019);
    expectedHol.emplace_back(15, August, 2019);
    expectedHol.emplace_back(12, September, 2019);
    expectedHol.emplace_back(13, September, 2019);
    expectedHol.emplace_back(3, October, 2019);
    expectedHol.emplace_back(9, October, 2019);
    expectedHol.emplace_back(25, December, 2019);
    expectedHol.emplace_back(31, December, 2019);
    expectedHol.emplace_back(1, January, 2020);
    expectedHol.emplace_back(24, January, 2020);
    expectedHol.emplace_back(27, January, 2020);
    expectedHol.emplace_back(15, April, 2020);
    expectedHol.emplace_back(30, April, 2020);
    expectedHol.emplace_back(1, May, 2020);
    expectedHol.emplace_back(5, May, 2020);
    expectedHol.emplace_back(17, August, 2020);
    expectedHol.emplace_back(30, September, 2020);
    expectedHol.emplace_back(1, October, 2020);
    expectedHol.emplace_back(2, October, 2020);
    expectedHol.emplace_back(9, October, 2020);
    expectedHol.emplace_back(25, December, 2020);
    expectedHol.emplace_back(31, December, 2020);
    expectedHol.emplace_back(1, January, 2021);
    expectedHol.emplace_back(11, February, 2021);
    expectedHol.emplace_back(12, February, 2021);
    expectedHol.emplace_back(1, March, 2021);
    expectedHol.emplace_back(5, May, 2021);
    expectedHol.emplace_back(19, May, 2021);
    expectedHol.emplace_back(16, August, 2021);
    expectedHol.emplace_back(20, September, 2021);
    expectedHol.emplace_back(21, September, 2021);
    expectedHol.emplace_back(22, September, 2021);
    expectedHol.emplace_back( 4, October, 2021);
    expectedHol.emplace_back(11, October, 2021);
    expectedHol.emplace_back(31, December, 2021);
    expectedHol.emplace_back(31, January, 2022);
    expectedHol.emplace_back(1, February, 2022);
    expectedHol.emplace_back(2, February, 2022);
    expectedHol.emplace_back(1, March, 2022);
    expectedHol.emplace_back(9, March, 2022);
    expectedHol.emplace_back(5, May, 2022);
    expectedHol.emplace_back(1, June, 2022);
    expectedHol.emplace_back(6, June, 2022);
    expectedHol.emplace_back(15, August, 2022);
    expectedHol.emplace_back(9, September, 2022);
    expectedHol.emplace_back(12, September, 2022);
    expectedHol.emplace_back(3, October, 2022);
    expectedHol.emplace_back(10, October, 2022);
    expectedHol.emplace_back(30, December, 2022);
    // expectedHol.emplace_back(1, January, 2023);    // Sunday
    // expectedHol.emplace_back(21, January, 2023);    // Saturday
    expectedHol.emplace_back(23, January, 2023);
    expectedHol.emplace_back(24, January, 2023);
    expectedHol.emplace_back(1, March, 2023);
    expectedHol.emplace_back(1, May, 2023);
    expectedHol.emplace_back(5, May, 2023);
    expectedHol.emplace_back(29, May, 2023);
    expectedHol.emplace_back(6, June, 2023);
    expectedHol.emplace_back(15, August, 2023);
    expectedHol.emplace_back(28, September, 2023);
    expectedHol.emplace_back(29, September, 2023);
    // expectedHol.emplace_back(30, September, 2023);    // Saturday
    expectedHol.emplace_back(2, October, 2023);
    expectedHol.emplace_back(3, October, 2023);
    expectedHol.emplace_back(9, October, 2023);
    expectedHol.emplace_back(25, December, 2023);
    expectedHol.emplace_back(29, December, 2023);
    expectedHol.emplace_back(1, January, 2024);
    expectedHol.emplace_back(9, February, 2024);
    // expectedHol.emplace_back(10, February, 2024);    // Saturday
    expectedHol.emplace_back(12, February, 2024);
    expectedHol.emplace_back(1, March, 2024);
    expectedHol.emplace_back(10, April, 2024);
    expectedHol.emplace_back(1, May, 2024);
    expectedHol.emplace_back(6, May, 2024);
    expectedHol.emplace_back(15, May, 2024);
    expectedHol.emplace_back(6, June, 2024);
    expectedHol.emplace_back(15, August, 2024);
    expectedHol.emplace_back(16, September, 2024);
    expectedHol.emplace_back(17, September, 2024);
    expectedHol.emplace_back(18, September, 2024);
    expectedHol.emplace_back(3, October, 2024);
    expectedHol.emplace_back(9, October, 2024);
    expectedHol.emplace_back(25, December, 2024);
    expectedHol.emplace_back(31, December, 2024);
    expectedHol.emplace_back(1, January, 2025);
    expectedHol.emplace_back(28, January, 2025);
    expectedHol.emplace_back(29, January, 2025);
    expectedHol.emplace_back(30, January, 2025);
    expectedHol.emplace_back(3, March, 2025);
    expectedHol.emplace_back(1, May, 2025);
    expectedHol.emplace_back(5, May, 2025);
    expectedHol.emplace_back(6, May, 2025);
    expectedHol.emplace_back(6, June, 2025);
    expectedHol.emplace_back(15, August, 2025);
    expectedHol.emplace_back(3, October, 2025);
    expectedHol.emplace_back(6, October, 2025);
    expectedHol.emplace_back(7, October, 2025);
    expectedHol.emplace_back(8, October, 2025);
    expectedHol.emplace_back(9, October, 2025);
    expectedHol.emplace_back(25, December, 2025);
    expectedHol.emplace_back(31, December, 2025);
    expectedHol.emplace_back(1, January, 2026);
    expectedHol.emplace_back(16, February, 2026);
    expectedHol.emplace_back(17, February, 2026);
    expectedHol.emplace_back(18, February, 2026);
    expectedHol.emplace_back(2, March, 2026);
    expectedHol.emplace_back(1, May, 2026);
    expectedHol.emplace_back(5, May, 2026);
    expectedHol.emplace_back(25, May, 2026);
    // expectedHol.emplace_back(6, June, 2026);    // Saturday
    expectedHol.emplace_back(17, August, 2026);
    expectedHol.emplace_back(24, September, 2026);
    expectedHol.emplace_back(25, September, 2026);
    // expectedHol.emplace_back(26, September, 2026);    // Saturday
    expectedHol.emplace_back(5, October, 2026);
    expectedHol.emplace_back(9, October, 2026);
    expectedHol.emplace_back(25, December, 2026);
    expectedHol.emplace_back(31, December, 2026);
    expectedHol.emplace_back(1, January, 2027);
    // expectedHol.emplace_back(6, February, 2027);    // Saturday
    expectedHol.emplace_back(8, February, 2027);
    expectedHol.emplace_back(9, February, 2027);
    expectedHol.emplace_back(1, March, 2027);
    // expectedHol.emplace_back(1, May, 2027);    // Saturday
    expectedHol.emplace_back(5, May, 2027);
    expectedHol.emplace_back(13, May, 2027);
    // expectedHol.emplace_back(6, June, 2027);    // Sunday
    expectedHol.emplace_back(16, August, 2027);
    expectedHol.emplace_back(14, September, 2027);
    expectedHol.emplace_back(15, September, 2027);
    expectedHol.emplace_back(16, September, 2027);
    expectedHol.emplace_back(4, October, 2027);
    expectedHol.emplace_back(11, October, 2027);
    expectedHol.emplace_back(27, December, 2027);
    expectedHol.emplace_back(31, December, 2027);
    // expectedHol.emplace_back(1, January, 2028);    // Saturday
    expectedHol.emplace_back(26, January, 2028);
    expectedHol.emplace_back(27, January, 2028);
    expectedHol.emplace_back(28, January, 2028);
    expectedHol.emplace_back(1, March, 2028);
    expectedHol.emplace_back(1, May, 2028);
    expectedHol.emplace_back(2, May, 2028);
    expectedHol.emplace_back(5, May, 2028);
    expectedHol.emplace_back(6, June, 2028);
    expectedHol.emplace_back(15, August, 2028);
    expectedHol.emplace_back(2, October, 2028);
    expectedHol.emplace_back(3, October, 2028);
    expectedHol.emplace_back(4, October, 2028);
    expectedHol.emplace_back(5, October, 2028);
    expectedHol.emplace_back(9, October, 2028);
    expectedHol.emplace_back(25, December, 2028);
    expectedHol.emplace_back(29, December, 2028);
    expectedHol.emplace_back(1, January, 2029);
    expectedHol.emplace_back(12, February, 2029);
    expectedHol.emplace_back(13, February, 2029);
    expectedHol.emplace_back(14, February, 2029);
    expectedHol.emplace_back(1, March, 2029);
    expectedHol.emplace_back(1, May, 2029);
    expectedHol.emplace_back(7, May, 2029);
    expectedHol.emplace_back(21, May, 2029);
    expectedHol.emplace_back(6, June, 2029);
    expectedHol.emplace_back(15, August, 2029);
    expectedHol.emplace_back(21, September, 2029);
    // expectedHol.emplace_back(22, September, 2029);    // Saturday
    expectedHol.emplace_back(24, September, 2029);
    expectedHol.emplace_back(3, October, 2029);
    expectedHol.emplace_back(9, October, 2029);
    expectedHol.emplace_back(25, December, 2029);
    expectedHol.emplace_back(31, December, 2029);
    expectedHol.emplace_back(1, January, 2030);
    // expectedHol.emplace_back(2, February, 2030);    // Saturday
    expectedHol.emplace_back(4, February, 2030);
    expectedHol.emplace_back(5, February, 2030);
    expectedHol.emplace_back(1, March, 2030);
    expectedHol.emplace_back(1, May, 2030);
    expectedHol.emplace_back(6, May, 2030);
    expectedHol.emplace_back(9, May, 2030);
    expectedHol.emplace_back(6, June, 2030);
    expectedHol.emplace_back(15, August, 2030);
    expectedHol.emplace_back(11, September, 2030);
    expectedHol.emplace_back(12, September, 2030);
    expectedHol.emplace_back(13, September, 2030);
    expectedHol.emplace_back(3, October, 2030);
    expectedHol.emplace_back(9, October, 2030);
    expectedHol.emplace_back(25, December, 2030);
    expectedHol.emplace_back(31, December, 2030);
    expectedHol.emplace_back(1, January, 2031);
    expectedHol.emplace_back(22, January, 2031);
    expectedHol.emplace_back(23, January, 2031);
    expectedHol.emplace_back(24, January, 2031);
    expectedHol.emplace_back(3, March, 2031);
    expectedHol.emplace_back(1, May, 2031);
    expectedHol.emplace_back(5, May, 2031);
    expectedHol.emplace_back(28, May, 2031);
    expectedHol.emplace_back(6, June, 2031);
    expectedHol.emplace_back(15, August, 2031);
    expectedHol.emplace_back(30, September, 2031);
    expectedHol.emplace_back(1, October, 2031);
    expectedHol.emplace_back(2, October, 2031);
    expectedHol.emplace_back(3, October, 2031);
    expectedHol.emplace_back(9, October, 2031);
    expectedHol.emplace_back(25, December, 2031);
    expectedHol.emplace_back(31, December, 2031);
    expectedHol.emplace_back(1, January, 2032);
    expectedHol.emplace_back(10, February, 2032);
    expectedHol.emplace_back(11, February, 2032);
    expectedHol.emplace_back(12, February, 2032);
    expectedHol.emplace_back(1, March, 2032);
    // expectedHol.emplace_back(1, May, 2032);    // Saturday
    expectedHol.emplace_back(5, May, 2032);
    expectedHol.emplace_back(17, May, 2032);
    // expectedHol.emplace_back(6, June, 2032);    // Sunday
    expectedHol.emplace_back(16, August, 2032);
    // expectedHol.emplace_back(18, September, 2032);    // Saturday
    expectedHol.emplace_back(20, September, 2032);
    expectedHol.emplace_back(21, September, 2032);
    expectedHol.emplace_back(4, October, 2032);
    expectedHol.emplace_back(11, October, 2032);
    expectedHol.emplace_back(27, December, 2032);
    expectedHol.emplace_back(31, December, 2032);
    // expectedHol.emplace_back(1, January, 2033);    // Saturday
    expectedHol.emplace_back(31, January, 2033);
    expectedHol.emplace_back(1, February, 2033);
    expectedHol.emplace_back(2, February, 2033);
    expectedHol.emplace_back(1, March, 2033);
    // expectedHol.emplace_back(1, May, 2033);    // Sunday
    expectedHol.emplace_back(5, May, 2033);
    expectedHol.emplace_back(6, May, 2033);
    expectedHol.emplace_back(6, June, 2033);
    expectedHol.emplace_back(15, August, 2033);
    expectedHol.emplace_back(7, September, 2033);
    expectedHol.emplace_back(8, September, 2033);
    expectedHol.emplace_back(9, September, 2033);
    expectedHol.emplace_back(3, October, 2033);
    expectedHol.emplace_back(10, October, 2033);
    expectedHol.emplace_back(26, December, 2033);
    expectedHol.emplace_back(30, December, 2033);
    // expectedHol.emplace_back(1, January, 2034);    // Sunday
    // expectedHol.emplace_back(18, February, 2034);    // Saturday
    expectedHol.emplace_back(20, February, 2034);
    expectedHol.emplace_back(21, February, 2034);
    expectedHol.emplace_back(1, March, 2034);
    expectedHol.emplace_back(1, May, 2034);
    expectedHol.emplace_back(5, May, 2034);
    expectedHol.emplace_back(25, May, 2034);
    expectedHol.emplace_back(6, June, 2034);
    expectedHol.emplace_back(15, August, 2034);
    expectedHol.emplace_back(26, September, 2034);
    expectedHol.emplace_back(27, September, 2034);
    expectedHol.emplace_back(28, September, 2034);
    expectedHol.emplace_back(3, October, 2034);
    expectedHol.emplace_back(9, October, 2034);
    expectedHol.emplace_back(25, December, 2034);
    expectedHol.emplace_back(29, December, 2034);
    expectedHol.emplace_back(1, January, 2035);
    expectedHol.emplace_back(7, February, 2035);
    expectedHol.emplace_back(8, February, 2035);
    expectedHol.emplace_back(9, February, 2035);
    expectedHol.emplace_back(1, March, 2035);
    expectedHol.emplace_back(1, May, 2035);
    expectedHol.emplace_back(7, May, 2035);
    expectedHol.emplace_back(15, May, 2035);
    expectedHol.emplace_back(6, June, 2035);
    expectedHol.emplace_back(15, August, 2035);
    // expectedHol.emplace_back(15, September, 2035);    // Saturday
    expectedHol.emplace_back(17, September, 2035);
    expectedHol.emplace_back(18, September, 2035);
    expectedHol.emplace_back(3, October, 2035);
    expectedHol.emplace_back(9, October, 2035);
    expectedHol.emplace_back(25, December, 2035);
    expectedHol.emplace_back(31, December, 2035);
    expectedHol.emplace_back(1, January, 2036);
    expectedHol.emplace_back(28, January, 2036);
    expectedHol.emplace_back(29, January, 2036);
    expectedHol.emplace_back(30, January, 2036);
    expectedHol.emplace_back(3, March, 2036);
    expectedHol.emplace_back(1, May, 2036);
    expectedHol.emplace_back(5, May, 2036);
    expectedHol.emplace_back(6, May, 2036);
    expectedHol.emplace_back(6, June, 2036);
    expectedHol.emplace_back(15, August, 2036);
    expectedHol.emplace_back(3, October, 2036);
    // expectedHol.emplace_back(4, October, 2036);    // Saturday
    expectedHol.emplace_back(6, October, 2036);
    expectedHol.emplace_back(7, October, 2036);
    expectedHol.emplace_back(9, October, 2036);
    expectedHol.emplace_back(25, December, 2036);
    expectedHol.emplace_back(31, December, 2036);
    expectedHol.emplace_back(1, January, 2037);
    // expectedHol.emplace_back(14, February, 2037);    // Saturday
    expectedHol.emplace_back(16, February, 2037);
    expectedHol.emplace_back(17, February, 2037);
    expectedHol.emplace_back(2, March, 2037);
    expectedHol.emplace_back(1, May, 2037);
    expectedHol.emplace_back(5, May, 2037);
    expectedHol.emplace_back(22, May, 2037);
    // expectedHol.emplace_back(6, June, 2037);    // Saturday
    expectedHol.emplace_back(17, August, 2037);
    expectedHol.emplace_back(23, September, 2037);
    expectedHol.emplace_back(24, September, 2037);
    expectedHol.emplace_back(25, September, 2037);
    expectedHol.emplace_back(5, October, 2037);
    expectedHol.emplace_back(9, October, 2037);
    expectedHol.emplace_back(25, December, 2037);
    expectedHol.emplace_back(31, December, 2037);
    expectedHol.emplace_back(1, January, 2038);
    expectedHol.emplace_back(3, February, 2038);
    expectedHol.emplace_back(4, February, 2038);
    expectedHol.emplace_back(5, February, 2038);
    expectedHol.emplace_back(1, March, 2038);
    // expectedHol.emplace_back(1, May, 2038);    // Saturday
    expectedHol.emplace_back(5, May, 2038);
    expectedHol.emplace_back(11, May, 2038);
    // expectedHol.emplace_back(6, June, 2038);    // Sunday
    expectedHol.emplace_back(16, August, 2038);
    expectedHol.emplace_back(13, September, 2038);
    expectedHol.emplace_back(14, September, 2038);
    expectedHol.emplace_back(15, September, 2038);
    expectedHol.emplace_back(4, October, 2038);
    expectedHol.emplace_back(11, October, 2038);
    expectedHol.emplace_back(27, December, 2038);
    expectedHol.emplace_back(31, December, 2038);
    // expectedHol.emplace_back(1, January, 2039);    // Saturday
    expectedHol.emplace_back(24, January, 2039);
    expectedHol.emplace_back(25, January, 2039);
    expectedHol.emplace_back(26, January, 2039);
    expectedHol.emplace_back(1, March, 2039);
    // expectedHol.emplace_back(1, May, 2039);    // Sunday
    expectedHol.emplace_back(2, May, 2039);
    expectedHol.emplace_back(5, May, 2039);
    expectedHol.emplace_back(6, June, 2039);
    expectedHol.emplace_back(15, August, 2039);
    // expectedHol.emplace_back(1, October, 2039);    // Saturday
    expectedHol.emplace_back(3, October, 2039);
    expectedHol.emplace_back(4, October, 2039);
    expectedHol.emplace_back(5, October, 2039);
    expectedHol.emplace_back(10, October, 2039);
    expectedHol.emplace_back(26, December, 2039);
    expectedHol.emplace_back(30, December, 2039);
    // expectedHol.emplace_back(1, January, 2040);    // Sunday
    // expectedHol.emplace_back(11, February, 2040);    // Saturday
    expectedHol.emplace_back(13, February, 2040);
    expectedHol.emplace_back(14, February, 2040);
    expectedHol.emplace_back(1, March, 2040);
    expectedHol.emplace_back(1, May, 2040);
    expectedHol.emplace_back(7, May, 2040);
    expectedHol.emplace_back(18, May, 2040);
    expectedHol.emplace_back(6, June, 2040);
    expectedHol.emplace_back(15, August, 2040);
    expectedHol.emplace_back(20, September, 2040);
    expectedHol.emplace_back(21, September, 2040);
    // expectedHol.emplace_back(22, September, 2040);    // Saturday
    expectedHol.emplace_back(3, October, 2040);
    expectedHol.emplace_back(9, October, 2040);
    expectedHol.emplace_back(25, December, 2040);
    expectedHol.emplace_back(31, December, 2040);
    expectedHol.emplace_back(1, January, 2041);
    expectedHol.emplace_back(31, January, 2041);
    expectedHol.emplace_back(1, February, 2041);
    // expectedHol.emplace_back(2, February, 2041);    // Saturday
    expectedHol.emplace_back(1, March, 2041);
    expectedHol.emplace_back(1, May, 2041);
    expectedHol.emplace_back(6, May, 2041);
    expectedHol.emplace_back(7, May, 2041);
    expectedHol.emplace_back(6, June, 2041);
    expectedHol.emplace_back(15, August, 2041);
    expectedHol.emplace_back(9, September, 2041);
    expectedHol.emplace_back(10, September, 2041);
    expectedHol.emplace_back(11, September, 2041);
    expectedHol.emplace_back(3, October, 2041);
    expectedHol.emplace_back(9, October, 2041);
    expectedHol.emplace_back(25, December, 2041);
    expectedHol.emplace_back(31, December, 2041);
    expectedHol.emplace_back(1, January, 2042);
    expectedHol.emplace_back(21, January, 2042);
    expectedHol.emplace_back(22, January, 2042);
    expectedHol.emplace_back(23, January, 2042);
    expectedHol.emplace_back(3, March, 2042);
    expectedHol.emplace_back(1, May, 2042);
    expectedHol.emplace_back(5, May, 2042);
    expectedHol.emplace_back(26, May, 2042);
    expectedHol.emplace_back(6, June, 2042);
    expectedHol.emplace_back(15, August, 2042);
    // expectedHol.emplace_back(27, September, 2042);    // Saturday
    expectedHol.emplace_back(29, September, 2042);
    expectedHol.emplace_back(30, September, 2042);
    expectedHol.emplace_back(3, October, 2042);
    expectedHol.emplace_back(9, October, 2042);
    expectedHol.emplace_back(25, December, 2042);
    expectedHol.emplace_back(31, December, 2042);
    expectedHol.emplace_back(1, January, 2043);
    expectedHol.emplace_back(9, February, 2043);
    expectedHol.emplace_back(10, February, 2043);
    expectedHol.emplace_back(11, February, 2043);
    expectedHol.emplace_back(2, March, 2043);
    expectedHol.emplace_back(1, May, 2043);
    expectedHol.emplace_back(5, May, 2043);
    expectedHol.emplace_back(18, May, 2043);
    // expectedHol.emplace_back(6, June, 2043);    // Saturday
    expectedHol.emplace_back(17, August, 2043);
    expectedHol.emplace_back(16, September, 2043);
    expectedHol.emplace_back(17, September, 2043);
    expectedHol.emplace_back(18, September, 2043);
    expectedHol.emplace_back(5, October, 2043);
    expectedHol.emplace_back(9, October, 2043);
    expectedHol.emplace_back(25, December, 2043);
    expectedHol.emplace_back(31, December, 2043);
    expectedHol.emplace_back(1, January, 2044);
    expectedHol.emplace_back(29, January, 2044);
    // expectedHol.emplace_back(30, January, 2044);    // Saturday
    expectedHol.emplace_back(1, February, 2044);
    expectedHol.emplace_back(1, March, 2044);
    // expectedHol.emplace_back(1, May, 2044);    // Sunday
    expectedHol.emplace_back(5, May, 2044);
    expectedHol.emplace_back(6, May, 2044);
    expectedHol.emplace_back(6, June, 2044);
    expectedHol.emplace_back(15, August, 2044);
    expectedHol.emplace_back(3, October, 2044);
    expectedHol.emplace_back(4, October, 2044);
    expectedHol.emplace_back(5, October, 2044);
    expectedHol.emplace_back(6, October, 2044);
    expectedHol.emplace_back(10, October, 2044);
    expectedHol.emplace_back(26, December, 2044);
    expectedHol.emplace_back(30, December, 2044);
    // expectedHol.emplace_back(1, January, 2045);    // Sunday
    expectedHol.emplace_back(16, February, 2045);
    expectedHol.emplace_back(17, February, 2045);
    // expectedHol.emplace_back(18, February, 2045);    // Saturday
    expectedHol.emplace_back(1, March, 2045);
    expectedHol.emplace_back(1, May, 2045);
    expectedHol.emplace_back(5, May, 2045);
    expectedHol.emplace_back(24, May, 2045);
    expectedHol.emplace_back(6, June, 2045);
    expectedHol.emplace_back(15, August, 2045);
    expectedHol.emplace_back(25, September, 2045);
    expectedHol.emplace_back(26, September, 2045);
    expectedHol.emplace_back(27, September, 2045);
    expectedHol.emplace_back(3, October, 2045);
    expectedHol.emplace_back(9, October, 2045);
    expectedHol.emplace_back(25, December, 2045);
    expectedHol.emplace_back(29, December, 2045);
    expectedHol.emplace_back(1, January, 2046);
    expectedHol.emplace_back(5, February, 2046);
    expectedHol.emplace_back(6, February, 2046);
    expectedHol.emplace_back(7, February, 2046);
    expectedHol.emplace_back(1, March, 2046);
    expectedHol.emplace_back(1, May, 2046);
    expectedHol.emplace_back(7, May, 2046);
    expectedHol.emplace_back(14, May, 2046);
    expectedHol.emplace_back(6, June, 2046);
    expectedHol.emplace_back(15, August, 2046);
    expectedHol.emplace_back(14, September, 2046);
    // expectedHol.emplace_back(15, September, 2046);    // Saturday
    expectedHol.emplace_back(17, September, 2046);
    expectedHol.emplace_back(3, October, 2046);
    expectedHol.emplace_back(9, October, 2046);
    expectedHol.emplace_back(25, December, 2046);
    expectedHol.emplace_back(31, December, 2046);
    expectedHol.emplace_back(1, January, 2047);
    expectedHol.emplace_back(25, January, 2047);
    // expectedHol.emplace_back(26, January, 2047);    // Saturday
    expectedHol.emplace_back(28, January, 2047);
    expectedHol.emplace_back(1, March, 2047);
    expectedHol.emplace_back(1, May, 2047);
    expectedHol.emplace_back(2, May, 2047);
    expectedHol.emplace_back(6, May, 2047);
    expectedHol.emplace_back(6, June, 2047);
    expectedHol.emplace_back(15, August, 2047);
    expectedHol.emplace_back(3, October, 2047);
    expectedHol.emplace_back(4, October, 2047);
    // expectedHol.emplace_back(5, October, 2047);    // Saturday
    expectedHol.emplace_back(7, October, 2047);
    expectedHol.emplace_back(9, October, 2047);
    expectedHol.emplace_back(25, December, 2047);
    expectedHol.emplace_back(31, December, 2047);
    expectedHol.emplace_back(1, January, 2048);
    expectedHol.emplace_back(13, February, 2048);
    expectedHol.emplace_back(14, February, 2048);
    // expectedHol.emplace_back(15, February, 2048);    // Saturday
    expectedHol.emplace_back(2, March, 2048);
    expectedHol.emplace_back(1, May, 2048);
    expectedHol.emplace_back(5, May, 2048);
    expectedHol.emplace_back(20, May, 2048);
    // expectedHol.emplace_back(6, June, 2048);    // Saturday
    expectedHol.emplace_back(17, August, 2048);
    expectedHol.emplace_back(21, September, 2048);
    expectedHol.emplace_back(22, September, 2048);
    expectedHol.emplace_back(23, September, 2048);
    expectedHol.emplace_back(5, October, 2048);
    expectedHol.emplace_back(9, October, 2048);
    expectedHol.emplace_back(25, December, 2048);
    expectedHol.emplace_back(31, December, 2048);
    expectedHol.emplace_back(1, January, 2049);
    expectedHol.emplace_back(1, February, 2049);
    expectedHol.emplace_back(2, February, 2049);
    expectedHol.emplace_back(3, February, 2049);
    expectedHol.emplace_back(1, March, 2049);
    // expectedHol.emplace_back(1, May, 2049);    // Saturday
    expectedHol.emplace_back(5, May, 2049);
    expectedHol.emplace_back(10, May, 2049);
    // expectedHol.emplace_back(6, June, 2049);    // Sunday
    expectedHol.emplace_back(16, August, 2049);
    expectedHol.emplace_back(10, September, 2049);
    // expectedHol.emplace_back(11, September, 2049);    // Saturday
    expectedHol.emplace_back(13, September, 2049);
    expectedHol.emplace_back(4, October, 2049);
    expectedHol.emplace_back(11, October, 2049);
    expectedHol.emplace_back(27, December, 2049);
    expectedHol.emplace_back(31, December, 2049);
    // expectedHol.emplace_back(1, January, 2050);    // Saturday
    // expectedHol.emplace_back(22, January, 2050);    // Saturday
    expectedHol.emplace_back(24, January, 2050);
    expectedHol.emplace_back(25, January, 2050);
    expectedHol.emplace_back(1, March, 2050);
    // expectedHol.emplace_back(1, May, 2050);    // Sunday
    expectedHol.emplace_back(5, May, 2050);
    expectedHol.emplace_back(30, May, 2050);
    expectedHol.emplace_back(6, June, 2050);
    expectedHol.emplace_back(15, August, 2050);
    expectedHol.emplace_back(29, September, 2050);
    expectedHol.emplace_back(30, September, 2050);
    // expectedHol.emplace_back(1, October, 2050);    // Saturday
    expectedHol.emplace_back(3, October, 2050);
    expectedHol.emplace_back(10, October, 2050);
    expectedHol.emplace_back(26, December, 2050);
    expectedHol.emplace_back(30, December, 2050);

    Calendar c = SouthKorea(SouthKorea::KRX);
    std::vector<Date> hol = c.holidayList(Date(1, January, 2004), Date(31, December, 2050));

    for (Size i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
}

BOOST_AUTO_TEST_CASE(testChinaSSE) {
    BOOST_TEST_MESSAGE("Testing China Shanghai Stock Exchange holiday list...");

    std::vector<Date> expectedHol;

    // China Shanghai Securities Exchange holiday list in the year 2014
    expectedHol.emplace_back(1, Jan, 2014);
    expectedHol.emplace_back(31, Jan, 2014);
    expectedHol.emplace_back(3, Feb, 2014);
    expectedHol.emplace_back(4, Feb, 2014);
    expectedHol.emplace_back(5, Feb, 2014);
    expectedHol.emplace_back(6, Feb, 2014);
    expectedHol.emplace_back(7, Apr, 2014);
    expectedHol.emplace_back(1, May, 2014);
    expectedHol.emplace_back(2, May, 2014);
    expectedHol.emplace_back(2, Jun, 2014);
    expectedHol.emplace_back(8, Sep, 2014);
    expectedHol.emplace_back(1, Oct, 2014);
    expectedHol.emplace_back(2, Oct, 2014);
    expectedHol.emplace_back(3, Oct, 2014);
    expectedHol.emplace_back(6, Oct, 2014);
    expectedHol.emplace_back(7, Oct, 2014);

    // China Shanghai Securities Exchange holiday list in the year 2015
    expectedHol.emplace_back(1, Jan, 2015);
    expectedHol.emplace_back(2, Jan, 2015);
    expectedHol.emplace_back(18, Feb, 2015);
    expectedHol.emplace_back(19, Feb, 2015);
    expectedHol.emplace_back(20, Feb, 2015);
    expectedHol.emplace_back(23, Feb, 2015);
    expectedHol.emplace_back(24, Feb, 2015);
    expectedHol.emplace_back(6, Apr, 2015);
    expectedHol.emplace_back(1, May, 2015);
    expectedHol.emplace_back(22, Jun, 2015);
    expectedHol.emplace_back(3, Sep, 2015);
    expectedHol.emplace_back(4, Sep, 2015);
    expectedHol.emplace_back(1, Oct, 2015);
    expectedHol.emplace_back(2, Oct, 2015);
    expectedHol.emplace_back(5, Oct, 2015);
    expectedHol.emplace_back(6, Oct, 2015);
    expectedHol.emplace_back(7, Oct, 2015);

    // China Shanghai Securities Exchange holiday list in the year 2016
    expectedHol.emplace_back(1, Jan, 2016);
    expectedHol.emplace_back(8, Feb, 2016);
    expectedHol.emplace_back(9, Feb, 2016);
    expectedHol.emplace_back(10, Feb, 2016);
    expectedHol.emplace_back(11, Feb, 2016);
    expectedHol.emplace_back(12, Feb, 2016);
    expectedHol.emplace_back(4, Apr, 2016);
    expectedHol.emplace_back(2, May, 2016);
    expectedHol.emplace_back(9, Jun, 2016);
    expectedHol.emplace_back(10, Jun, 2016);
    expectedHol.emplace_back(15, Sep, 2016);
    expectedHol.emplace_back(16, Sep, 2016);
    expectedHol.emplace_back(3, Oct, 2016);
    expectedHol.emplace_back(4, Oct, 2016);
    expectedHol.emplace_back(5, Oct, 2016);
    expectedHol.emplace_back(6, Oct, 2016);
    expectedHol.emplace_back(7, Oct, 2016);

    // China Shanghai Securities Exchange holiday list in the year 2017
    expectedHol.emplace_back(2, Jan, 2017);
    expectedHol.emplace_back(27, Jan, 2017);
    expectedHol.emplace_back(30, Jan, 2017);
    expectedHol.emplace_back(31, Jan, 2017);
    expectedHol.emplace_back(1, Feb, 2017);
    expectedHol.emplace_back(2, Feb, 2017);
    expectedHol.emplace_back(3, April, 2017);
    expectedHol.emplace_back(4, April, 2017);
    expectedHol.emplace_back(1, May, 2017);
    expectedHol.emplace_back(29, May, 2017);
    expectedHol.emplace_back(30, May, 2017);
    expectedHol.emplace_back(2, Oct, 2017);
    expectedHol.emplace_back(3, Oct, 2017);
    expectedHol.emplace_back(4, Oct, 2017);
    expectedHol.emplace_back(5, Oct, 2017);
    expectedHol.emplace_back(6, Oct, 2017);

    // China Shanghai Securities Exchange holiday list in the year 2018
    expectedHol.emplace_back(1, Jan, 2018);
    expectedHol.emplace_back(15, Feb, 2018);
    expectedHol.emplace_back(16, Feb, 2018);
    expectedHol.emplace_back(19, Feb, 2018);
    expectedHol.emplace_back(20, Feb, 2018);
    expectedHol.emplace_back(21, Feb, 2018);
    expectedHol.emplace_back(5, April, 2018);
    expectedHol.emplace_back(6, April, 2018);
    expectedHol.emplace_back(30, April, 2018);
    expectedHol.emplace_back(1, May, 2018);
    expectedHol.emplace_back(18, June, 2018);
    expectedHol.emplace_back(24, September, 2018);
    expectedHol.emplace_back(1, Oct, 2018);
    expectedHol.emplace_back(2, Oct, 2018);
    expectedHol.emplace_back(3, Oct, 2018);
    expectedHol.emplace_back(4, Oct, 2018);
    expectedHol.emplace_back(5, Oct, 2018);
    expectedHol.emplace_back(31, December, 2018);

    // China Shanghai Securities Exchange holiday list in the year 2019
    expectedHol.emplace_back(1, Jan, 2019);
    expectedHol.emplace_back(4, Feb, 2019);
    expectedHol.emplace_back(5, Feb, 2019);
    expectedHol.emplace_back(6, Feb, 2019);
    expectedHol.emplace_back(7, Feb, 2019);
    expectedHol.emplace_back(8, Feb, 2019);
    expectedHol.emplace_back(5, April, 2019);
    expectedHol.emplace_back(1, May, 2019);
    expectedHol.emplace_back(2, May, 2019);
    expectedHol.emplace_back(3, May, 2019);
    expectedHol.emplace_back(7, June, 2019);
    expectedHol.emplace_back(13, September, 2019);
    expectedHol.emplace_back(1, October, 2019);
    expectedHol.emplace_back(2, October, 2019);
    expectedHol.emplace_back(3, October, 2019);
    expectedHol.emplace_back(4, October, 2019);
    expectedHol.emplace_back(7, October, 2019);

    // China Shanghai Securities Exchange holiday list in the year 2020
    expectedHol.emplace_back(1, Jan, 2020);
    expectedHol.emplace_back(24, Jan, 2020);
    expectedHol.emplace_back(27, Jan, 2020);
    expectedHol.emplace_back(28, Jan, 2020);
    expectedHol.emplace_back(29, Jan, 2020);
    expectedHol.emplace_back(30, Jan, 2020);
    expectedHol.emplace_back(31, Jan, 2020);
    expectedHol.emplace_back(6, April, 2020);
    expectedHol.emplace_back(1, May, 2020);
    expectedHol.emplace_back(4, May, 2020);
    expectedHol.emplace_back(5, May, 2020);
    expectedHol.emplace_back(25, June, 2020);
    expectedHol.emplace_back(26, June, 2020);
    expectedHol.emplace_back(1, October, 2020);
    expectedHol.emplace_back(2, October, 2020);
    expectedHol.emplace_back(5, October, 2020);
    expectedHol.emplace_back(6, October, 2020);
    expectedHol.emplace_back(7, October, 2020);
    expectedHol.emplace_back(8, October, 2020);

    // China Shanghai Securities Exchange holiday list in the year 2021
    expectedHol.emplace_back(1, Jan, 2021);
    expectedHol.emplace_back(11, Feb, 2021);
    expectedHol.emplace_back(12, Feb, 2021);
    expectedHol.emplace_back(15, Feb, 2021);
    expectedHol.emplace_back(16, Feb, 2021);
    expectedHol.emplace_back(17, Feb, 2021);
    expectedHol.emplace_back(5, April, 2021);
    expectedHol.emplace_back(3, May, 2021);
    expectedHol.emplace_back(4, May, 2021);
    expectedHol.emplace_back(5, May, 2021);
    expectedHol.emplace_back(14, June, 2021);
    expectedHol.emplace_back(20, September, 2021);
    expectedHol.emplace_back(21, September, 2021);
    expectedHol.emplace_back(1, October, 2021);
    expectedHol.emplace_back(4, October, 2021);
    expectedHol.emplace_back(5, October, 2021);
    expectedHol.emplace_back(6, October, 2021);
    expectedHol.emplace_back(7, October, 2021);

    // China Shanghai Securities Exchange holiday list in the year 2022
    expectedHol.emplace_back(3, Jan, 2022);
    expectedHol.emplace_back(31, Jan, 2022);
    expectedHol.emplace_back(1, Feb, 2022);
    expectedHol.emplace_back(2, Feb, 2022);
    expectedHol.emplace_back(3, Feb, 2022);
    expectedHol.emplace_back(4, Feb, 2022);
    expectedHol.emplace_back(4, April, 2022);
    expectedHol.emplace_back(5, April, 2022);
    expectedHol.emplace_back(2, May, 2022);
    expectedHol.emplace_back(3, May, 2022);
    expectedHol.emplace_back(4, May, 2022);
    expectedHol.emplace_back(3, June, 2022);
    expectedHol.emplace_back(12, September, 2022);
    expectedHol.emplace_back(3, October, 2022);
    expectedHol.emplace_back(4, October, 2022);
    expectedHol.emplace_back(5, October, 2022);
    expectedHol.emplace_back(6, October, 2022);
    expectedHol.emplace_back(7, October, 2022);

    // China Shanghai Securities Exchange holiday list in the year 2023
    expectedHol.emplace_back(2, Jan, 2023);
    expectedHol.emplace_back(23, Jan, 2023);
    expectedHol.emplace_back(24, Jan, 2023);
    expectedHol.emplace_back(25, Jan, 2023);
    expectedHol.emplace_back(26, Jan, 2023);
    expectedHol.emplace_back(27, Jan, 2023);
    expectedHol.emplace_back(5, April, 2023);
    expectedHol.emplace_back(1, May, 2023);
    expectedHol.emplace_back(2, May, 2023);
    expectedHol.emplace_back(3, May, 2023);
    expectedHol.emplace_back(22, June, 2023);
    expectedHol.emplace_back(23, June, 2023);
    expectedHol.emplace_back(29, September, 2023);
    expectedHol.emplace_back(2, October, 2023);
    expectedHol.emplace_back(3, October, 2023);
    expectedHol.emplace_back(4, October, 2023);
    expectedHol.emplace_back(5, October, 2023);
    expectedHol.emplace_back(6, October, 2023);

    // China Shanghai Securities Exchange holiday list in the year 2024
    expectedHol.emplace_back(1, Jan, 2024);
    expectedHol.emplace_back(9, Feb, 2024);
    expectedHol.emplace_back(12, Feb, 2024);
    expectedHol.emplace_back(13, Feb, 2024);
    expectedHol.emplace_back(14, Feb, 2024);
    expectedHol.emplace_back(15, Feb, 2024);
    expectedHol.emplace_back(16, Feb, 2024);
    expectedHol.emplace_back(4, April, 2024);
    expectedHol.emplace_back(5, April, 2024);
    expectedHol.emplace_back(1, May, 2024);
    expectedHol.emplace_back(2, May, 2024);
    expectedHol.emplace_back(3, May, 2024);
    expectedHol.emplace_back(10, Jun, 2024);
    expectedHol.emplace_back(16, Sep, 2024);
    expectedHol.emplace_back(17, Sep, 2024);
    expectedHol.emplace_back(1, Oct, 2024);
    expectedHol.emplace_back(2, Oct, 2024);
    expectedHol.emplace_back(3, Oct, 2024);
    expectedHol.emplace_back(4, Oct, 2024);
    expectedHol.emplace_back(7, Oct, 2024);

    Calendar c = China(China::SSE);
    std::vector<Date> hol = c.holidayList(Date(1, January, 2014), Date(31, December, 2024));

    for (Size i = 0; i < std::min<Size>(hol.size(), expectedHol.size()); i++) {
        if (hol[i] != expectedHol[i])
            BOOST_FAIL("expected holiday was " << expectedHol[i] << " while calculated holiday is "
                                               << hol[i]);
    }
    if (hol.size() != expectedHol.size())
        BOOST_FAIL("there were " << expectedHol.size() << " expected holidays, while there are "
                                 << hol.size() << " calculated holidays");
}

BOOST_AUTO_TEST_CASE(testChinaIB) {
    BOOST_TEST_MESSAGE("Testing China Inter Bank working weekends list...");

    std::vector<Date> expectedWorkingWeekEnds;

    // China Inter Bank working weekends list in the year 2014
    expectedWorkingWeekEnds.emplace_back(26, Jan, 2014);
    expectedWorkingWeekEnds.emplace_back(8, Feb, 2014);
    expectedWorkingWeekEnds.emplace_back(4, May, 2014);
    expectedWorkingWeekEnds.emplace_back(28, Sep, 2014);
    expectedWorkingWeekEnds.emplace_back(11, Oct, 2014);

    // China Inter Bank working weekends list in the year 2015
    expectedWorkingWeekEnds.emplace_back(4, Jan, 2015);
    expectedWorkingWeekEnds.emplace_back(15, Feb, 2015);
    expectedWorkingWeekEnds.emplace_back(28, Feb, 2015);
    expectedWorkingWeekEnds.emplace_back(6, Sep, 2015);
    expectedWorkingWeekEnds.emplace_back(10, Oct, 2015);

    // China Inter Bank working weekends list in the year 2016
    expectedWorkingWeekEnds.emplace_back(6, Feb, 2016);
    expectedWorkingWeekEnds.emplace_back(14, Feb, 2016);
    expectedWorkingWeekEnds.emplace_back(12, Jun, 2016);
    expectedWorkingWeekEnds.emplace_back(18, Sep, 2016);
    expectedWorkingWeekEnds.emplace_back(8, Oct, 2016);
    expectedWorkingWeekEnds.emplace_back(9, Oct, 2016);

    // China Inter Bank working weekends list in the year 2017
    expectedWorkingWeekEnds.emplace_back(22, Jan, 2017);
    expectedWorkingWeekEnds.emplace_back(4, Feb, 2017);
    expectedWorkingWeekEnds.emplace_back(1, April, 2017);
    expectedWorkingWeekEnds.emplace_back(27, May, 2017);
    expectedWorkingWeekEnds.emplace_back(30, Sep, 2017);

    // China Inter Bank working weekends list in the year 2018
    expectedWorkingWeekEnds.emplace_back(11, Feb, 2018);
    expectedWorkingWeekEnds.emplace_back(24, Feb, 2018);
    expectedWorkingWeekEnds.emplace_back(8, April, 2018);
    expectedWorkingWeekEnds.emplace_back(28, April, 2018);
    expectedWorkingWeekEnds.emplace_back(29, Sep, 2018);
    expectedWorkingWeekEnds.emplace_back(30, Sep, 2018);
    expectedWorkingWeekEnds.emplace_back(29, December, 2018);

    // China Inter Bank working weekends list in the year 2019
    expectedWorkingWeekEnds.emplace_back(2, Feb, 2019);
    expectedWorkingWeekEnds.emplace_back(3, Feb, 2019);
    expectedWorkingWeekEnds.emplace_back(28, April, 2019);
    expectedWorkingWeekEnds.emplace_back(5, May, 2019);
    expectedWorkingWeekEnds.emplace_back(29, September, 2019);
    expectedWorkingWeekEnds.emplace_back(12, October, 2019);

    // China Inter Bank working weekends list in the year 2020
    expectedWorkingWeekEnds.emplace_back(19, January, 2020);
    expectedWorkingWeekEnds.emplace_back(26, April, 2020);
    expectedWorkingWeekEnds.emplace_back(9, May, 2020);
    expectedWorkingWeekEnds.emplace_back(28, June, 2020);
    expectedWorkingWeekEnds.emplace_back(27, September, 2020);
    expectedWorkingWeekEnds.emplace_back(10, October, 2020);

    // China Inter Bank working weekends list in the year 2021
    expectedWorkingWeekEnds.emplace_back(7, Feb, 2021);
    expectedWorkingWeekEnds.emplace_back(20, Feb, 2021);
    expectedWorkingWeekEnds.emplace_back(25, April, 2021);
    expectedWorkingWeekEnds.emplace_back(8, May, 2021);
    expectedWorkingWeekEnds.emplace_back(18, September, 2021);
    expectedWorkingWeekEnds.emplace_back(26, September, 2021);
    expectedWorkingWeekEnds.emplace_back(9, October, 2021);

    // China Inter Bank working weekends list in the year 2022
    expectedWorkingWeekEnds.emplace_back(29, Jan, 2022);
    expectedWorkingWeekEnds.emplace_back(30, Jan, 2022);
    expectedWorkingWeekEnds.emplace_back(2, April, 2022);
    expectedWorkingWeekEnds.emplace_back(24, April, 2022);
    expectedWorkingWeekEnds.emplace_back(7, May, 2022);
    expectedWorkingWeekEnds.emplace_back(8, October, 2022);
    expectedWorkingWeekEnds.emplace_back(9, October, 2022);

    // China Inter Bank working weekends list in the year 2023
    expectedWorkingWeekEnds.emplace_back(28, Jan, 2023);
    expectedWorkingWeekEnds.emplace_back(29, Jan, 2023);
    expectedWorkingWeekEnds.emplace_back(23, April, 2023);
    expectedWorkingWeekEnds.emplace_back(6, May, 2023);
    expectedWorkingWeekEnds.emplace_back(25, June, 2023);
    expectedWorkingWeekEnds.emplace_back(7, October, 2023);
    expectedWorkingWeekEnds.emplace_back(8, October, 2023);

    // China Inter Bank working weekends list in the year 2024
    expectedWorkingWeekEnds.emplace_back(4, Feb, 2024);
    expectedWorkingWeekEnds.emplace_back(18, Feb, 2024);
    expectedWorkingWeekEnds.emplace_back(7, April, 2024);
    expectedWorkingWeekEnds.emplace_back(28, April, 2024);
    expectedWorkingWeekEnds.emplace_back(11, May, 2024);
    expectedWorkingWeekEnds.emplace_back(14, Sep, 2024);
    expectedWorkingWeekEnds.emplace_back(29, Sep, 2024);
    expectedWorkingWeekEnds.emplace_back(12, October, 2024);

    Calendar c = China(China::IB);
    Date start(1, Jan, 2014);
    Date end(31, Dec, 2024);

    Size k = 0;

    while (start <= end) {
        if (c.isBusinessDay(start) && c.isWeekend(start.weekday())) {
            if (expectedWorkingWeekEnds[k] != start)
                BOOST_FAIL("expected working weekend was "
                           << expectedWorkingWeekEnds[k] << " while calculated working weekend is "
                           << start);
            ++k;
        }
        ++start;
    }

    if (k != (expectedWorkingWeekEnds.size()))
        BOOST_FAIL("there were " << expectedWorkingWeekEnds.size()
                                 << " expected working weekends, while there are " << k
                                 << " calculated working weekends");
}

BOOST_AUTO_TEST_CASE(testMexicoInaugurationDay) {
    BOOST_TEST_MESSAGE("Testing Mexican Inauguration Day holiday...");

    // The first five Inauguration Days 2024 and later
    std::vector<Date> expectedHol;
    expectedHol.emplace_back(1, Oct, 2024);
    expectedHol.emplace_back(1, Oct, 2030);
    expectedHol.emplace_back(1, Oct, 2036);
    expectedHol.emplace_back(1, Oct, 2042);
    expectedHol.emplace_back(1, Oct, 2048);

    // Some years of non-Inaugurations
    std::vector<Date> expectedWorkingDays;
    expectedWorkingDays.emplace_back(1, Oct, 2018);
    expectedWorkingDays.emplace_back(1, Oct, 2025);
    expectedWorkingDays.emplace_back(1, Oct, 2026);
    expectedWorkingDays.emplace_back(1, Oct, 2027);
    // 2028 falls on a weekend
    expectedWorkingDays.emplace_back(1, Oct, 2029);
    expectedWorkingDays.emplace_back(1, Oct, 2031);
    expectedWorkingDays.emplace_back(1, Oct, 2032);
    // 2033 and 2034 fall on weekends
    expectedWorkingDays.emplace_back(1, Oct, 2035);

    Calendar mexico = Mexico();
    for (auto holiday : expectedHol) {
        if (!mexico.isHoliday(holiday)) {
            BOOST_FAIL("Expected to have an Inauguration Day holiday in the Mexican calendar for date " << holiday);
        }
    }
    for (auto workingDay : expectedWorkingDays) {
        if (!mexico.isBusinessDay(workingDay)) {
            BOOST_FAIL("Did not expect to have a holiday in the Mexican calendar for date "
                       << workingDay);
        }
    }
}


BOOST_AUTO_TEST_CASE(testEndOfMonth) {
    BOOST_TEST_MESSAGE("Testing end-of-month calculation...");

    Calendar c = TARGET(); // any calendar would be OK

    Date eom, counter = Date::minDate();
    Date last = Date::maxDate() - 2 * Months;

    while (counter <= last) {
        eom = c.endOfMonth(counter);
        // check that eom is eom
        if (!c.isEndOfMonth(eom))
            BOOST_FAIL("\n  " << eom.weekday() << " " << eom << " is not the last business day in "
                              << eom.month() << " " << eom.year() << " according to " << c.name());
        // check that eom is in the same month as counter
        if (eom.month() != counter.month())
            BOOST_FAIL("\n  " << eom << " is not in the same month as " << counter);
        counter = counter + 1;
    }
}

BOOST_AUTO_TEST_CASE(testBusinessDaysBetween) {

    BOOST_TEST_MESSAGE("Testing calculation of business days between dates...");

    std::vector<Date> testDates;
    testDates.emplace_back(1, February, 2002);  // isBusinessDay = true
    testDates.emplace_back(4, February, 2002);  // isBusinessDay = true
    testDates.emplace_back(16, May, 2003);      // isBusinessDay = true
    testDates.emplace_back(17, December, 2003); // isBusinessDay = true
    testDates.emplace_back(17, December, 2004); // isBusinessDay = true
    testDates.emplace_back(19, December, 2005); // isBusinessDay = true
    testDates.emplace_back(2, January, 2006);   // isBusinessDay = true
    testDates.emplace_back(13, March, 2006);    // isBusinessDay = true
    testDates.emplace_back(15, May, 2006);      // isBusinessDay = true
    testDates.emplace_back(17, March, 2006);    // isBusinessDay = true
    testDates.emplace_back(15, May, 2006);      // isBusinessDay = true
    testDates.emplace_back(26, July, 2006);     // isBusinessDay = true
    testDates.emplace_back(26, July, 2006);     // isBusinessDay = true
    testDates.emplace_back(27, July, 2006);     // isBusinessDay = true
    testDates.emplace_back(29, July, 2006);     // isBusinessDay = false
    testDates.emplace_back(29, July, 2006);     // isBusinessDay = false

    // default params: from date included, to excluded
    Date::serial_type expected[] = {1, 321, 152, 251, 252, 10, 48, 42, -38, 38, 51, 0, 1, 2, 0};

    // exclude from, include to
    Date::serial_type expected_include_to[] = {1,   321, 152, 251, 252, 10, 48, 42,
                                               -38, 38,  51,  0,   1,   1,  0};

    // include both from and to
    Date::serial_type expected_include_all[] = {2,   322, 153, 252, 253, 11, 49, 43,
                                                -39, 39,  52,  1,   2,   2,  0};

    // exclude both from and to
    Date::serial_type expected_exclude_all[] = {0,   320, 151, 250, 251, 9, 47, 41,
                                                -37, 37,  50,  0,   0,   1, 0};

    Calendar calendar = Brazil();

    for (Size i = 1; i < testDates.size(); i++) {
        Integer calculated =
            calendar.businessDaysBetween(testDates[i - 1], testDates[i], true, false);
        if (calculated != expected[i - 1]) {
            BOOST_ERROR("from " << testDates[i - 1] << " included"
                                << " to " << testDates[i] << " excluded:\n"
                                << "    calculated: " << calculated << "\n"
                                << "    expected:   " << expected[i - 1]);
        }

        calculated = calendar.businessDaysBetween(testDates[i - 1], testDates[i], false, true);
        if (calculated != expected_include_to[i - 1]) {
            BOOST_ERROR("from " << testDates[i - 1] << " excluded"
                                << " to " << testDates[i] << " included:\n"
                                << "    calculated: " << calculated << "\n"
                                << "    expected:   " << expected_include_to[i - 1]);
        }

        calculated = calendar.businessDaysBetween(testDates[i - 1], testDates[i], true, true);
        if (calculated != expected_include_all[i - 1]) {
            BOOST_ERROR("from " << testDates[i - 1] << " included"
                                << " to " << testDates[i] << " included:\n"
                                << "    calculated: " << calculated << "\n"
                                << "    expected:   " << expected_include_all[i - 1]);
        }

        calculated = calendar.businessDaysBetween(testDates[i - 1], testDates[i], false, false);
        if (calculated != expected_exclude_all[i - 1]) {
            BOOST_ERROR("from " << testDates[i - 1] << " excluded"
                                << " to " << testDates[i] << " excluded:\n"
                                << "    calculated: " << calculated << "\n"
                                << "    expected:   " << expected_exclude_all[i - 1]);
        }
    }
}

BOOST_AUTO_TEST_CASE(testBespokeCalendars) {

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

    BespokeCalendar a2 = a1; // linked to a1

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

#ifdef QL_HIGH_RESOLUTION_DATE
BOOST_AUTO_TEST_CASE(testIntradayAddHolidays) {

    BOOST_TEST_MESSAGE("Testing addHolidays with enable-intraday...");

    // test cases taken from testModifiedCalendars

    Calendar c1 = TARGET();
    Calendar c2 = UnitedStates(UnitedStates::NYSE);
    Date d1(1, May, 2004);                // holiday for both calendars
    Date d2(26, April, 2004, 0, 0, 1, 1); // business day

    Date d1Mock(1, May, 2004, 1, 1, 0, 0); // holiday for both calendars
    Date d2Mock(26, April, 2004);          // business day

    // this works anyhow because implementation uses day() month() etc
    QL_REQUIRE(c1.isHoliday(d1), "wrong assumption---correct the test");
    QL_REQUIRE(c1.isBusinessDay(d2), "wrong assumption---correct the test");

    QL_REQUIRE(c2.isHoliday(d1), "wrong assumption---correct the test");
    QL_REQUIRE(c2.isBusinessDay(d2), "wrong assumption---correct the test");

    // now with different hourly/min/sec
    QL_REQUIRE(c1.isHoliday(d1Mock), "wrong assumption---correct the test");
    QL_REQUIRE(c1.isBusinessDay(d2Mock), "wrong assumption---correct the test");

    QL_REQUIRE(c2.isHoliday(d1Mock), "wrong assumption---correct the test");
    QL_REQUIRE(c2.isBusinessDay(d2Mock), "wrong assumption---correct the test");


    // modify the TARGET calendar
    c1.removeHoliday(d1);
    c1.addHoliday(d2);

    // test
    if (c1.isHoliday(d1))
        BOOST_FAIL(d1 << " still a holiday for original TARGET instance");
    if (c1.isBusinessDay(d2))
        BOOST_FAIL(d2 << " still a business day for original TARGET instance");

    if (c1.isHoliday(d1Mock))
        BOOST_FAIL(d1Mock << " still a holiday for original TARGET instance"
                          << " and different hours/min/secs");
    if (c1.isBusinessDay(d2Mock))
        BOOST_FAIL(d2Mock << " still a business day for generic TARGET instance"
                          << " and different hours/min/secs");

    // any instance of TARGET should be modified...
    Calendar c3 = TARGET();
    if (c3.isHoliday(d1))
        BOOST_FAIL(d1 << " still a holiday for generic TARGET instance");
    if (c3.isBusinessDay(d2))
        BOOST_FAIL(d2 << " still a business day for generic TARGET instance");

    if (c3.isHoliday(d1Mock))
        BOOST_FAIL(d1Mock << " still a holiday for original TARGET instance"
                          << " and different hours/min/secs");
    if (c3.isBusinessDay(d2Mock))
        BOOST_FAIL(d2Mock << " still a business day for generic TARGET instance"
                          << " and different hours/min/secs");

    // ...but not other calendars
    if (c2.isBusinessDay(d1))
        BOOST_FAIL(d1 << " business day for New York");
    if (c2.isHoliday(d2))
        BOOST_FAIL(d2 << " holiday for New York");

    if (c2.isBusinessDay(d1Mock))
        BOOST_FAIL(d1Mock << " business day for New York"
                          << " and different hours/min/secs");
    if (c2.isHoliday(d2Mock))
        BOOST_FAIL(d2Mock << " holiday for New York"
                          << " and different hours/min/secs");

    // restore original holiday set---test the other way around
    c3.addHoliday(d1Mock);
    c3.removeHoliday(d2Mock);

    if (c1.isBusinessDay(d1))
        BOOST_FAIL(d1 << " still a business day");
    if (c1.isHoliday(d2))
        BOOST_FAIL(d2 << " still a holiday");

    if (c1.isBusinessDay(d1Mock))
        BOOST_FAIL(d1Mock << " still a business day"
                          << " and different hours/min/secs");
    if (c1.isHoliday(d2Mock))
        BOOST_FAIL(d2Mock << " still a holiday and different hours/min/secs");
}
#endif

BOOST_AUTO_TEST_CASE(testDayLists) {

    BOOST_TEST_MESSAGE("Testing holidayList and businessDaysList...");
    Calendar germany = Germany();
    Date firstDate = Settings::instance().evaluationDate(), endDate = firstDate + 1 * Years;

    // Test that same day holidayList and businessDayList does not throw an error
    germany.holidayList(firstDate, firstDate, true);
    germany.businessDayList(firstDate, firstDate);

    std::vector<Date> holidays = germany.holidayList(firstDate, endDate, true);
    std::vector<Date> businessDays = germany.businessDayList(firstDate, endDate);

    auto it_holidays = holidays.begin(), it_businessDays = businessDays.begin();
    for (Date d = firstDate; d < endDate; d++) {
        if (it_holidays != holidays.end() && it_businessDays != businessDays.end() &&
            d == *it_holidays && d == *it_businessDays) {
            BOOST_FAIL("Date " << d << "is both holiday and business day.");
            ++it_holidays;
            ++it_businessDays;
        } else if (it_holidays != holidays.end() && d == *it_holidays) {
            ++it_holidays;
        } else if (it_businessDays != businessDays.end() && d == *it_businessDays) {
            ++it_businessDays;
        } else {
            BOOST_FAIL("Date " << d << "is neither holiday nor business day.");
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
