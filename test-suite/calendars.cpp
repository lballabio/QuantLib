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
 Copyright (C) 2026 Lawrenz Law

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
#include <ql/errors.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/calendars/bespokecalendar.hpp>
#include <ql/time/calendars/brazil.hpp>
#include <ql/time/calendars/china.hpp>
#include <ql/time/calendars/croatia.hpp>
#include <ql/time/calendars/denmark.hpp>
#include <ql/time/calendars/germany.hpp>
#include <ql/time/calendars/india.hpp>
#include <ql/time/calendars/israel.hpp>
#include <ql/time/calendars/italy.hpp>
#include <ql/time/calendars/japan.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/calendars/malaysia.hpp>
#include <ql/time/calendars/malta.hpp>
#include <ql/time/calendars/mexico.hpp>
#include <ql/time/calendars/montenegro.hpp>
#include <ql/time/calendars/newzealand.hpp>
#include <ql/time/calendars/northmacedonia.hpp>
#include <ql/time/calendars/russia.hpp>
#include <ql/time/calendars/saudiarabia.hpp>
#include <ql/time/calendars/serbia.hpp>
#include <ql/time/calendars/slovenia.hpp>
#include <ql/time/calendars/southkorea.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/calendars/uzbekistan.hpp>
#include <fstream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CalendarTests)

void checkHolidays(const std::vector<Date>& calculated, const std::vector<Date>& expected) {
    std::set<Date> calc(calculated.begin(), calculated.end());
    std::set<Date> exp(expected.begin(), expected.end());

    for (Date d : calculated) {
        if (exp.find(d) == exp.end())
            BOOST_ERROR(d << " calculated but not expected");
    }
    for (Date d : expected) {
        if (calc.find(d) == calc.end())
            BOOST_ERROR(d << " expected but not calculated");
    }
}

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

    std::vector<Date> expectedHol = {
        {1, January, 2004},
        {19, January, 2004},
        {16, February, 2004},
        {31, May, 2004},
        {5, July, 2004},
        {6, September, 2004},
        {11, October, 2004},
        {11, November, 2004},
        {25, November, 2004},
        {24, December, 2004},

        {31, December, 2004},
        {17, January, 2005},
        {21, February, 2005},
        {30, May, 2005},
        {4, July, 2005},
        {5, September, 2005},
        {10, October, 2005},
        {11, November, 2005},
        {24, November, 2005},
        {26, December, 2005},
    };
    
    Calendar c = UnitedStates(UnitedStates::Settlement);
    checkHolidays(c.holidayList(Date(1, January, 2004), Date(31, December, 2005)), expectedHol);

    // before Uniform Monday Holiday Act
    expectedHol = {
        {2, January, 1961},
        {22, February, 1961},
        {30, May, 1961},
        {4, July, 1961},
        {4, September, 1961},
        {10, November, 1961},
        {23, November, 1961},
        {25, December, 1961},
    };

    checkHolidays(c.holidayList(Date(1, January, 1961), Date(31, December, 1961)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testUSGovernmentBondMarket) {
    BOOST_TEST_MESSAGE("Testing US government bond market holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2004},
        {19, January, 2004},
        {16, February, 2004},
        {9, April, 2004},
        {31, May, 2004},
        {11, June, 2004}, // Reagan's funeral
        {5, July, 2004},
        {6, September, 2004},
        {11, October, 2004},
        {11, November, 2004},
        {25, November, 2004},
        {24, December, 2004},
    };

    Calendar c = UnitedStates(UnitedStates::GovernmentBond);
    checkHolidays(c.holidayList(Date(1, January, 2004), Date(31, December, 2004)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testUSNewYorkStockExchange) {
    BOOST_TEST_MESSAGE("Testing New York Stock Exchange holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2004},
        {19, January, 2004},
        {16, February, 2004},
        {9, April, 2004},
        {31, May, 2004},
        {11, June, 2004},
        {5, July, 2004},
        {6, September, 2004},
        {25, November, 2004},
        {24, December, 2004},

        {17, January, 2005},
        {21, February, 2005},
        {25, March, 2005},
        {30, May, 2005},
        {4, July, 2005},
        {5, September, 2005},
        {24, November, 2005},
        {26, December, 2005},

        {2, January, 2006},
        {16, January, 2006},
        {20, February, 2006},
        {14, April, 2006},
        {29, May, 2006},
        {4, July, 2006},
        {4, September, 2006},
        {23, November, 2006},
        {25, December, 2006},
    };

    Calendar c = UnitedStates(UnitedStates::NYSE);
    checkHolidays(c.holidayList(Date(1, January, 2004), Date(31, December, 2006)), expectedHol);

    std::vector<Date> histClose {
        {30, October, 2012},   // Hurricane Sandy
        {29, October, 2012},   // Hurricane Sandy
        {11, June, 2004},      // Reagan's funeral
        {14, September, 2001}, // September 11, 2001
        {13, September, 2001}, // September 11, 2001
        {12, September, 2001}, // September 11, 2001
        {11, September, 2001}, // September 11, 2001
        {27, April, 1994},     // Nixon's funeral.
        {27, September, 1985}, // Hurricane Gloria
        {14, July, 1977},      // 1977 Blackout
        {25, January, 1973},   // Johnson's funeral.
        {28, December, 1972},  // Truman's funeral
        {21, July, 1969},      // Lunar exploration nat. day
        {31, March, 1969},     // Eisenhower's funeral
        {10, February, 1969},  // heavy snow
        {5, July, 1968},       // Day after Independence Day
        {9, April, 1968},      // Mourning for MLK
        {24, December, 1965},  // Christmas Eve
        {25, November, 1963},  // Kennedy's funeral
        {29, May, 1961},       // Day before Decoration Day
        {26, December, 1958},  // Day after Christmas
        {24, December, 1956},  // Christmas Eve
        {24, December, 1954},  // Christmas Eve
        // June 12-Dec. 31, 1968
        // Four day week (closed on Wednesdays) - Paperwork Crisis
        {12, Jun, 1968},
        {19, Jun, 1968},
        {26, Jun, 1968},
        {3, Jul, 1968},
        {10, Jul, 1968},
        {17, Jul, 1968},
        {20, Nov, 1968},
        {27, Nov, 1968},
        {4, Dec, 1968},
        {11, Dec, 1968},
        {18, Dec, 1968},
        // Presidential election days
        {4, Nov, 1980},
        {2, Nov, 1976},
        {7, Nov, 1972},
        {5, Nov, 1968},
        {3, Nov, 1964},
    };

    for (auto i : histClose) {
        if (!c.isHoliday(i))
            BOOST_FAIL(i << " should be holiday (historical close)");
    }
}

BOOST_AUTO_TEST_CASE(testSOFR) {
    BOOST_TEST_MESSAGE("Testing holidays for SOFR...");

    // Good Friday
    for (const Date goodFriday :
         {Date(14, April, 2017), Date(30, March, 2018), Date(19, April, 2019),
          Date(10, April, 2020), Date(2, April, 2021), Date(15, April, 2022), Date(7, April, 2023),
          Date(29, March, 2024), Date(18, April, 2025), Date(3, April, 2026), Date(26, March, 2027),
          Date(14, April, 2028), Date(30, March, 2029), Date(19, April, 2030),
          Date(11, April, 2031)})
        BOOST_TEST(UnitedStates(UnitedStates::SOFR).isHoliday(goodFriday));
}

BOOST_AUTO_TEST_CASE(testUSFederalReserveJuneteenth) {
    BOOST_TEST_MESSAGE(
        "Testing holiday occurrence of Juneteenth for US Federal Reserve calendar...");

    auto fedCalendar = UnitedStates(UnitedStates::FederalReserve);

    std::vector<Date> expectedHol = {
        {20, June, 2022},
        {19, June, 2023},
        {19, June, 2024},
        {19, June, 2025},
        {19, June, 2027},
        {19, June, 2028},
        {19, June, 2029},
        {19, June, 2030},
        {19, June, 2031},
        {20, June, 2033},
    };

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

    std::vector<Date> expectedHol = {
        {1, January, 1999},
        {31, December, 1999},

        {21, April, 2000},
        {24, April, 2000},
        {1, May, 2000},
        {25, December, 2000},
        {26, December, 2000},

        {1, January, 2001},
        {13, April, 2001},
        {16, April, 2001},
        {1, May, 2001},
        {25, December, 2001},
        {26, December, 2001},
        {31, December, 2001},

        {1, January, 2002},
        {29, March, 2002},
        {1, April, 2002},
        {1, May, 2002},
        {25, December, 2002},
        {26, December, 2002},

        {1, January, 2003},
        {18, April, 2003},
        {21, April, 2003},
        {1, May, 2003},
        {25, December, 2003},
        {26, December, 2003},

        {1, January, 2004},
        {9, April, 2004},
        {12, April, 2004},

        {25, March, 2005},
        {28, March, 2005},
        {26, December, 2005},

        {14, April, 2006},
        {17, April, 2006},
        {1, May, 2006},
        {25, December, 2006},
        {26, December, 2006},
    };

    Calendar c = TARGET();
    checkHolidays(c.holidayList(Date(1, January, 1999), Date(31, December, 2006)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testGermanyFrankfurt) {
    BOOST_TEST_MESSAGE("Testing Frankfurt Stock Exchange holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2003},
        {18, April, 2003},
        {21, April, 2003},
        {1, May, 2003},
        {24, December, 2003},
        {25, December, 2003},
        {26, December, 2003},

        {1, January, 2004},
        {9, April, 2004},
        {12, April, 2004},
        {24, December, 2004},
    };

    Calendar c = Germany(Germany::FrankfurtStockExchange);
    checkHolidays(c.holidayList(Date(1, January, 2003), Date(31, December, 2004)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testGermanyEurex) {
    BOOST_TEST_MESSAGE("Testing Eurex holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2003},
        {18, April, 2003},
        {21, April, 2003},
        {1, May, 2003},
        {24, December, 2003},
        {25, December, 2003},
        {26, December, 2003},
        {31, December, 2003},

        {1, January, 2004},
        {9, April, 2004},
        {12, April, 2004},
        {24, December, 2004},
        {31, December, 2004},
    };

    Calendar c = Germany(Germany::Eurex);
    checkHolidays(c.holidayList(Date(1, January, 2003), Date(31, December, 2004)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testGermanyXetra) {
    BOOST_TEST_MESSAGE("Testing Xetra holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2003},
        {18, April, 2003},
        {21, April, 2003},
        {1, May, 2003},
        {24, December, 2003},
        {25, December, 2003},
        {26, December, 2003},

        {1, January, 2004},
        {9, April, 2004},
        {12, April, 2004},
        {24, December, 2004},
    };

    Calendar c = Germany(Germany::Xetra);
    checkHolidays(c.holidayList(Date(1, January, 2003), Date(31, December, 2004)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testUKSettlement) {
    BOOST_TEST_MESSAGE("Testing UK settlement holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2004},
        {9, April, 2004},
        {12, April, 2004},
        {3, May, 2004},
        {31, May, 2004},
        {30, August, 2004},
        {27, December, 2004},
        {28, December, 2004},

        {3, January, 2005},
        {25, March, 2005},
        {28, March, 2005},
        {2, May, 2005},
        {30, May, 2005},
        {29, August, 2005},
        {26, December, 2005},
        {27, December, 2005},

        {2, January, 2006},
        {14, April, 2006},
        {17, April, 2006},
        {1, May, 2006},
        {29, May, 2006},
        {28, August, 2006},
        {25, December, 2006},
        {26, December, 2006},

        {1, January, 2007},
        {6, April, 2007},
        {9, April, 2007},
        {7, May, 2007},
        {28, May, 2007},
        {27, August, 2007},
        {25, December, 2007},
        {26, December, 2007},
    };

    Calendar c = UnitedKingdom(UnitedKingdom::Settlement);
    checkHolidays(c.holidayList(Date(1, January, 2004), Date(31, December, 2007)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testUKExchange) {
    BOOST_TEST_MESSAGE("Testing London Stock Exchange holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2004},
        {9, April, 2004},
        {12, April, 2004},
        {3, May, 2004},
        {31, May, 2004},
        {30, August, 2004},
        {27, December, 2004},
        {28, December, 2004},

        {3, January, 2005},
        {25, March, 2005},
        {28, March, 2005},
        {2, May, 2005},
        {30, May, 2005},
        {29, August, 2005},
        {26, December, 2005},
        {27, December, 2005},

        {2, January, 2006},
        {14, April, 2006},
        {17, April, 2006},
        {1, May, 2006},
        {29, May, 2006},
        {28, August, 2006},
        {25, December, 2006},
        {26, December, 2006},

        {1, January, 2007},
        {6, April, 2007},
        {9, April, 2007},
        {7, May, 2007},
        {28, May, 2007},
        {27, August, 2007},
        {25, December, 2007},
        {26, December, 2007},
    };

    Calendar c = UnitedKingdom(UnitedKingdom::Exchange);
    checkHolidays(c.holidayList(Date(1, January, 2004), Date(31, December, 2007)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testUKMetals) {
    BOOST_TEST_MESSAGE("Testing London Metals Exchange holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2004},
        {9, April, 2004},
        {12, April, 2004},
        {3, May, 2004},
        {31, May, 2004},
        {30, August, 2004},
        {27, December, 2004},
        {28, December, 2004},

        {3, January, 2005},
        {25, March, 2005},
        {28, March, 2005},
        {2, May, 2005},
        {30, May, 2005},
        {29, August, 2005},
        {26, December, 2005},
        {27, December, 2005},

        {2, January, 2006},
        {14, April, 2006},
        {17, April, 2006},
        {1, May, 2006},
        {29, May, 2006},
        {28, August, 2006},
        {25, December, 2006},
        {26, December, 2006},

        {1, January, 2007},
        {6, April, 2007},
        {9, April, 2007},
        {7, May, 2007},
        {28, May, 2007},
        {27, August, 2007},
        {25, December, 2007},
        {26, December, 2007},
    };

    Calendar c = UnitedKingdom(UnitedKingdom::Metals);
    checkHolidays(c.holidayList(Date(1, January, 2004), Date(31, December, 2007)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testItalyExchange) {
    BOOST_TEST_MESSAGE("Testing Milan Stock Exchange holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2002},
        {29, March, 2002},
        {1, April, 2002},
        {1, May, 2002},
        {15, August, 2002},
        {24, December, 2002},
        {25, December, 2002},
        {26, December, 2002},
        {31, December, 2002},

        {1, January, 2003},
        {18, April, 2003},
        {21, April, 2003},
        {1, May, 2003},
        {15, August, 2003},
        {24, December, 2003},
        {25, December, 2003},
        {26, December, 2003},
        {31, December, 2003},

        {1, January, 2004},
        {9, April, 2004},
        {12, April, 2004},
        {24, December, 2004},
        {31, December, 2004},
    };

    Calendar c = Italy(Italy::Exchange);
    checkHolidays(c.holidayList(Date(1, January, 2002), Date(31, December, 2004)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testRussia) {
    BOOST_TEST_MESSAGE("Testing Russia holiday list...");

    std::vector<Date> expectedHol = {

    // exhaustive holiday list for the year 2012
        {1, January, 2012},
        {2, January, 2012},
        {7, January, 2012},
        {8, January, 2012},
        {14, January, 2012},
        {15, January, 2012},
        {21, January, 2012},
        {22, January, 2012},
        {28, January, 2012},
        {29, January, 2012},
        {4, February, 2012},
        {5, February, 2012},
        {11, February, 2012},
        {12, February, 2012},
        {18, February, 2012},
        {19, February, 2012},
        {23, February, 2012},
        {25, February, 2012},
        {26, February, 2012},
        {3, March, 2012},
        {4, March, 2012},
        {8, March, 2012},
        {9, March, 2012},
        {10, March, 2012},
        {17, March, 2012},
        {18, March, 2012},
        {24, March, 2012},
        {25, March, 2012},
        {31, March, 2012},
        {1, April, 2012},
        {7, April, 2012},
        {8, April, 2012},
        {14, April, 2012},
        {15, April, 2012},
        {21, April, 2012},
        {22, April, 2012},
        {29, April, 2012},
        {30, April, 2012},
        {1, May, 2012},
        {6, May, 2012},
        {9, May, 2012},
        {13, May, 2012},
        {19, May, 2012},
        {20, May, 2012},
        {26, May, 2012},
        {27, May, 2012},
        {2, June, 2012},
        {3, June, 2012},
        {10, June, 2012},
        {11, June, 2012},
        {12, June, 2012},
        {16, June, 2012},
        {17, June, 2012},
        {23, June, 2012},
        {24, June, 2012},
        {30, June, 2012},
        {1, July, 2012},
        {7, July, 2012},
        {8, July, 2012},
        {14, July, 2012},
        {15, July, 2012},
        {21, July, 2012},
        {22, July, 2012},
        {28, July, 2012},
        {29, July, 2012},
        {4, August, 2012},
        {5, August, 2012},
        {11, August, 2012},
        {12, August, 2012},
        {18, August, 2012},
        {19, August, 2012},
        {25, August, 2012},
        {26, August, 2012},
        {1, September, 2012},
        {2, September, 2012},
        {8, September, 2012},
        {9, September, 2012},
        {15, September, 2012},
        {16, September, 2012},
        {22, September, 2012},
        {23, September, 2012},
        {29, September, 2012},
        {30, September, 2012},
        {6, October, 2012},
        {7, October, 2012},
        {13, October, 2012},
        {14, October, 2012},
        {20, October, 2012},
        {21, October, 2012},
        {27, October, 2012},
        {28, October, 2012},
        {3, November, 2012},
        {4, November, 2012},
        {5, November, 2012},
        {10, November, 2012},
        {11, November, 2012},
        {17, November, 2012},
        {18, November, 2012},
        {24, November, 2012},
        {25, November, 2012},
        {1, December, 2012},
        {2, December, 2012},
        {8, December, 2012},
        {9, December, 2012},
        {15, December, 2012},
        {16, December, 2012},
        {22, December, 2012},
        {23, December, 2012},
        {29, December, 2012},
        {30, December, 2012},
        {31, December, 2012},

    // exhaustive holiday list for the year 2013
        {1, January, 2013},
        {2, January, 2013},
        {3, January, 2013},
        {4, January, 2013},
        {5, January, 2013},
        {6, January, 2013},
        {7, January, 2013},
        {12, January, 2013},
        {13, January, 2013},
        {19, January, 2013},
        {20, January, 2013},
        {26, January, 2013},
        {27, January, 2013},
        {2, February, 2013},
        {3, February, 2013},
        {9, February, 2013},
        {10, February, 2013},
        {16, February, 2013},
        {17, February, 2013},
        {23, February, 2013},
        {24, February, 2013},
        {2, March, 2013},
        {3, March, 2013},
        {8, March, 2013},
        {9, March, 2013},
        {10, March, 2013},
        {16, March, 2013},
        {17, March, 2013},
        {23, March, 2013},
        {24, March, 2013},
        {30, March, 2013},
        {31, March, 2013},
        {6, April, 2013},
        {7, April, 2013},
        {13, April, 2013},
        {14, April, 2013},
        {20, April, 2013},
        {21, April, 2013},
        {27, April, 2013},
        {28, April, 2013},
        {1, May, 2013},
        {4, May, 2013},
        {5, May, 2013},
        {9, May, 2013},
        {11, May, 2013},
        {12, May, 2013},
        {18, May, 2013},
        {19, May, 2013},
        {25, May, 2013},
        {26, May, 2013},
        {1, June, 2013},
        {2, June, 2013},
        {8, June, 2013},
        {9, June, 2013},
        {12, June, 2013},
        {15, June, 2013},
        {16, June, 2013},
        {22, June, 2013},
        {23, June, 2013},
        {29, June, 2013},
        {30, June, 2013},
        {6, July, 2013},
        {7, July, 2013},
        {13, July, 2013},
        {14, July, 2013},
        {20, July, 2013},
        {21, July, 2013},
        {27, July, 2013},
        {28, July, 2013},
        {3, August, 2013},
        {4, August, 2013},
        {10, August, 2013},
        {11, August, 2013},
        {17, August, 2013},
        {18, August, 2013},
        {24, August, 2013},
        {25, August, 2013},
        {31, August, 2013},
        {1, September, 2013},
        {7, September, 2013},
        {8, September, 2013},
        {14, September, 2013},
        {15, September, 2013},
        {21, September, 2013},
        {22, September, 2013},
        {28, September, 2013},
        {29, September, 2013},
        {5, October, 2013},
        {6, October, 2013},
        {12, October, 2013},
        {13, October, 2013},
        {19, October, 2013},
        {20, October, 2013},
        {26, October, 2013},
        {27, October, 2013},
        {2, November, 2013},
        {3, November, 2013},
        {4, November, 2013},
        {9, November, 2013},
        {10, November, 2013},
        {16, November, 2013},
        {17, November, 2013},
        {23, November, 2013},
        {24, November, 2013},
        {30, November, 2013},
        {1, December, 2013},
        {7, December, 2013},
        {8, December, 2013},
        {14, December, 2013},
        {15, December, 2013},
        {21, December, 2013},
        {22, December, 2013},
        {28, December, 2013},
        {29, December, 2013},
        {31, December, 2013},

    // exhaustive holiday list for the year 2014
        {1, January, 2014},
        {2, January, 2014},
        {3, January, 2014},
        {4, January, 2014},
        {5, January, 2014},
        {7, January, 2014},
        {11, January, 2014},
        {12, January, 2014},
        {18, January, 2014},
        {19, January, 2014},
        {25, January, 2014},
        {26, January, 2014},
        {1, February, 2014},
        {2, February, 2014},
        {8, February, 2014},
        {9, February, 2014},
        {15, February, 2014},
        {16, February, 2014},
        {22, February, 2014},
        {23, February, 2014},
        {1, March, 2014},
        {2, March, 2014},
        {8, March, 2014},
        {9, March, 2014},
        {10, March, 2014},
        {15, March, 2014},
        {16, March, 2014},
        {22, March, 2014},
        {23, March, 2014},
        {29, March, 2014},
        {30, March, 2014},
        {5, April, 2014},
        {6, April, 2014},
        {12, April, 2014},
        {13, April, 2014},
        {19, April, 2014},
        {20, April, 2014},
        {26, April, 2014},
        {27, April, 2014},
        {1, May, 2014},
        {3, May, 2014},
        {4, May, 2014},
        {9, May, 2014},
        {10, May, 2014},
        {11, May, 2014},
        {17, May, 2014},
        {18, May, 2014},
        {24, May, 2014},
        {25, May, 2014},
        {31, May, 2014},
        {1, June, 2014},
        {7, June, 2014},
        {8, June, 2014},
        {12, June, 2014},
        {14, June, 2014},
        {15, June, 2014},
        {21, June, 2014},
        {22, June, 2014},
        {28, June, 2014},
        {29, June, 2014},
        {5, July, 2014},
        {6, July, 2014},
        {12, July, 2014},
        {13, July, 2014},
        {19, July, 2014},
        {20, July, 2014},
        {26, July, 2014},
        {27, July, 2014},
        {2, August, 2014},
        {3, August, 2014},
        {9, August, 2014},
        {10, August, 2014},
        {16, August, 2014},
        {17, August, 2014},
        {23, August, 2014},
        {24, August, 2014},
        {30, August, 2014},
        {31, August, 2014},
        {6, September, 2014},
        {7, September, 2014},
        {13, September, 2014},
        {14, September, 2014},
        {20, September, 2014},
        {21, September, 2014},
        {27, September, 2014},
        {28, September, 2014},
        {4, October, 2014},
        {5, October, 2014},
        {11, October, 2014},
        {12, October, 2014},
        {18, October, 2014},
        {19, October, 2014},
        {25, October, 2014},
        {26, October, 2014},
        {1, November, 2014},
        {2, November, 2014},
        {4, November, 2014},
        {8, November, 2014},
        {9, November, 2014},
        {15, November, 2014},
        {16, November, 2014},
        {22, November, 2014},
        {23, November, 2014},
        {29, November, 2014},
        {30, November, 2014},
        {6, December, 2014},
        {7, December, 2014},
        {13, December, 2014},
        {14, December, 2014},
        {20, December, 2014},
        {21, December, 2014},
        {27, December, 2014},
        {28, December, 2014},
        {31, December, 2014},

    // exhaustive holiday list for the year 2015
        {1, January, 2015},
        {2, January, 2015},
        {3, January, 2015},
        {4, January, 2015},
        {7, January, 2015},
        {10, January, 2015},
        {11, January, 2015},
        {17, January, 2015},
        {18, January, 2015},
        {24, January, 2015},
        {25, January, 2015},
        {31, January, 2015},
        {1, February, 2015},
        {7, February, 2015},
        {8, February, 2015},
        {14, February, 2015},
        {15, February, 2015},
        {21, February, 2015},
        {22, February, 2015},
        {23, February, 2015},
        {28, February, 2015},
        {1, March, 2015},
        {7, March, 2015},
        {8, March, 2015},
        {9, March, 2015},
        {14, March, 2015},
        {15, March, 2015},
        {21, March, 2015},
        {22, March, 2015},
        {28, March, 2015},
        {29, March, 2015},
        {4, April, 2015},
        {5, April, 2015},
        {11, April, 2015},
        {12, April, 2015},
        {18, April, 2015},
        {19, April, 2015},
        {25, April, 2015},
        {26, April, 2015},
        {1, May, 2015},
        {2, May, 2015},
        {3, May, 2015},
        {9, May, 2015},
        {10, May, 2015},
        {11, May, 2015},
        {16, May, 2015},
        {17, May, 2015},
        {23, May, 2015},
        {24, May, 2015},
        {30, May, 2015},
        {31, May, 2015},
        {6, June, 2015},
        {7, June, 2015},
        {12, June, 2015},
        {13, June, 2015},
        {14, June, 2015},
        {20, June, 2015},
        {21, June, 2015},
        {27, June, 2015},
        {28, June, 2015},
        {4, July, 2015},
        {5, July, 2015},
        {11, July, 2015},
        {12, July, 2015},
        {18, July, 2015},
        {19, July, 2015},
        {25, July, 2015},
        {26, July, 2015},
        {1, August, 2015},
        {2, August, 2015},
        {8, August, 2015},
        {9, August, 2015},
        {15, August, 2015},
        {16, August, 2015},
        {22, August, 2015},
        {23, August, 2015},
        {29, August, 2015},
        {30, August, 2015},
        {5, September, 2015},
        {6, September, 2015},
        {12, September, 2015},
        {13, September, 2015},
        {19, September, 2015},
        {20, September, 2015},
        {26, September, 2015},
        {27, September, 2015},
        {3, October, 2015},
        {4, October, 2015},
        {10, October, 2015},
        {11, October, 2015},
        {17, October, 2015},
        {18, October, 2015},
        {24, October, 2015},
        {25, October, 2015},
        {31, October, 2015},
        {1, November, 2015},
        {4, November, 2015},
        {7, November, 2015},
        {8, November, 2015},
        {14, November, 2015},
        {15, November, 2015},
        {21, November, 2015},
        {22, November, 2015},
        {28, November, 2015},
        {29, November, 2015},
        {5, December, 2015},
        {6, December, 2015},
        {12, December, 2015},
        {13, December, 2015},
        {19, December, 2015},
        {20, December, 2015},
        {26, December, 2015},
        {27, December, 2015},
        {31, December, 2015},

    // exhaustive holiday list for the year 2016
        {1, January, 2016},
        {2, January, 2016},
        {3, January, 2016},
        {7, January, 2016},
        {8, January, 2016},
        {9, January, 2016},
        {10, January, 2016},
        {16, January, 2016},
        {17, January, 2016},
        {23, January, 2016},
        {24, January, 2016},
        {30, January, 2016},
        {31, January, 2016},
        {6, February, 2016},
        {7, February, 2016},
        {13, February, 2016},
        {14, February, 2016},
        {21, February, 2016},
        {23, February, 2016},
        {27, February, 2016},
        {28, February, 2016},
        {5, March, 2016},
        {6, March, 2016},
        {8, March, 2016},
        {12, March, 2016},
        {13, March, 2016},
        {19, March, 2016},
        {20, March, 2016},
        {26, March, 2016},
        {27, March, 2016},
        {2, April, 2016},
        {3, April, 2016},
        {9, April, 2016},
        {10, April, 2016},
        {16, April, 2016},
        {17, April, 2016},
        {23, April, 2016},
        {24, April, 2016},
        {30, April, 2016},
        {1, May, 2016},
        {2, May, 2016},
        {3, May, 2016},
        {7, May, 2016},
        {8, May, 2016},
        {9, May, 2016},
        {14, May, 2016},
        {15, May, 2016},
        {21, May, 2016},
        {22, May, 2016},
        {28, May, 2016},
        {29, May, 2016},
        {4, June, 2016},
        {5, June, 2016},
        {11, June, 2016},
        {12, June, 2016},
        {13, June, 2016},
        {18, June, 2016},
        {19, June, 2016},
        {25, June, 2016},
        {26, June, 2016},
        {2, July, 2016},
        {3, July, 2016},
        {9, July, 2016},
        {10, July, 2016},
        {16, July, 2016},
        {17, July, 2016},
        {23, July, 2016},
        {24, July, 2016},
        {30, July, 2016},
        {31, July, 2016},
        {6, August, 2016},
        {7, August, 2016},
        {13, August, 2016},
        {14, August, 2016},
        {20, August, 2016},
        {21, August, 2016},
        {27, August, 2016},
        {28, August, 2016},
        {3, September, 2016},
        {4, September, 2016},
        {10, September, 2016},
        {11, September, 2016},
        {17, September, 2016},
        {18, September, 2016},
        {24, September, 2016},
        {25, September, 2016},
        {1, October, 2016},
        {2, October, 2016},
        {8, October, 2016},
        {9, October, 2016},
        {15, October, 2016},
        {16, October, 2016},
        {22, October, 2016},
        {23, October, 2016},
        {29, October, 2016},
        {30, October, 2016},
        {4, November, 2016},
        {5, November, 2016},
        {6, November, 2016},
        {12, November, 2016},
        {13, November, 2016},
        {19, November, 2016},
        {20, November, 2016},
        {26, November, 2016},
        {27, November, 2016},
        {3, December, 2016},
        {4, December, 2016},
        {10, December, 2016},
        {11, December, 2016},
        {17, December, 2016},
        {18, December, 2016},
        {24, December, 2016},
        {25, December, 2016},
        {30, December, 2016},
        {31, December, 2016},
    };

    Calendar c = Russia(Russia::MOEX);
    checkHolidays(c.holidayList(Date(1, January, 2012), Date(31, December, 2016), true),
                  expectedHol);
}

BOOST_AUTO_TEST_CASE(testBrazil) {
    BOOST_TEST_MESSAGE("Testing Brazil holiday list...");

    std::vector<Date> expectedHol = {
        // {1,January,2005}, // Saturday
        {7, February, 2005},
        {8, February, 2005},
        {25, March, 2005},
        {21, April, 2005},
        // {1,May,2005}, // Sunday
        {26, May, 2005},
        {7, September, 2005},
        {12, October, 2005},
        {2, November, 2005},
        {15, November, 2005},
        // {25,December,2005}, // Sunday

        // {1,January,2006}, // Sunday
        {27, February, 2006},
        {28, February, 2006},
        {14, April, 2006},
        {21, April, 2006},
        {1, May, 2006},
        {15, June, 2006},
        {7, September, 2006},
        {12, October, 2006},
        {2, November, 2006},
        {15, November, 2006},
        {25, December, 2006},
    };

    Calendar c = Brazil();
    checkHolidays(c.holidayList(Date(1, January, 2005), Date(31, December, 2006)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testDenmark) {
    BOOST_TEST_MESSAGE("Testing Denmark holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2020},
        {9, April, 2020},
        {10, April, 2020},
        {13, April, 2020},
        {8, May, 2020},
        {21, May, 2020},
        {22, May, 2020},
        {1, June, 2020},
        {5, June, 2020},
        {24, December, 2020},
        {25, December, 2020},
        // Saturday: {26, December, 2020},
        {31, December, 2020},

        {1, January, 2021},
        {1, April, 2021},
        {2, April, 2021},
        {5, April, 2021},
        {30, April, 2021},
        {13, May, 2021},
        {14, May, 2021},
        {24, May, 2021},
        // Saturday: {5, June, 2021},
        {24, December, 2021},
        // Saturday: {25, December, 2021},
        // Sunday: {26, December, 2021},
        {31, December, 2021},

        // Saturday: {1, January, 2022},
        {14, April, 2022},
        {15, April, 2022},
        {18, April, 2022},
        {13, May, 2022},
        {26, May, 2022},
        {27, May, 2022},
        // Sunday: {5, June, 2022},
        {6, June, 2022},
        // Saturday: {24, December, 2022},
        // Sunday: {25, December, 2022},
        {26, December, 2022},
        // Saturday: {31, December, 2022},
    };

    Calendar c = Denmark();
    checkHolidays(c.holidayList(Date(1, January, 2020), Date(31, December, 2022)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testSouthKoreanSettlement) {
    BOOST_TEST_MESSAGE("Testing South-Korean settlement holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2004},
        {21, January, 2004},
        {22, January, 2004},
        {23, January, 2004},
        {1, March, 2004},
        {5, April, 2004},
        {15, April, 2004}, // election day
        {5, May, 2004},
        {26, May, 2004},
        {27, September, 2004},
        {28, September, 2004},
        {29, September, 2004},
        {8, February, 2005},
        {9, February, 2005},
        {10, February, 2005},
        {1, March, 2005},
        {5, April, 2005},
        {5, May, 2005},
        {6, June, 2005},
        {15, August, 2005},
        {19, September, 2005},
        {3, October, 2005},
        {30, January, 2006},
        {1, March, 2006},
        {1, May, 2006},
        {5, May, 2006},
        {31, May, 2006}, // election
        {6, June, 2006},
        {17, July, 2006},
        {15, August, 2006},
        {3, October, 2006},
        {5, October, 2006},
        {6, October, 2006},
        {25, December, 2006},
        {1, January, 2007},
        {19, February, 2007},
        {1, March, 2007},
        {1, May, 2007},
        {24, May, 2007},
        {6, June, 2007},
        {17, July, 2007},
        {15, August, 2007},
        {24, September, 2007},
        {25, September, 2007},
        {26, September, 2007},
        {3, October, 2007},
        {19, December, 2007}, // election
        {25, December, 2007},
        {1, January, 2008},
        {6, February, 2008},
        {7, February, 2008},
        {8, February, 2008},
        {9, April, 2008},
        {1, May, 2008},
        {5, May, 2008},
        {12, May, 2008},
        {6, June, 2008},
        {15, August, 2008},
        {15, September, 2008},
        {3, October, 2008},
        {25, December, 2008},
        {1, January, 2009},
        {26, January, 2009},
        {27, January, 2009},
        {1, May, 2009},
        {5, May, 2009},
        {2, October, 2009},
        {25, December, 2009},
        {1, January, 2010},
        {15, February, 2010},
        {1, March, 2010},
        {5, May, 2010},
        {21, May, 2010},
        {2, June, 2010},
        {21, September, 2010},
        {22, September, 2010},
        {23, September, 2010},
        {2, February, 2011},
        {3, February, 2011},
        {4, February, 2011},
        {1, March, 2011},
        {5, May, 2011},
        {10, May, 2011},
        {6, June, 2011},
        {15, August, 2011},
        {12, September, 2011},
        {13, September, 2011},
        {3, October, 2011},
        {23, January, 2012},
        {24, January, 2012},
        {1, March, 2012},
        {11, April, 2012},
        {1, May, 2012},
        {28, May, 2012},
        {6, June, 2012},
        {15, August, 2012},
        {1, October, 2012},
        {3, October, 2012},
        {19, December, 2012},
        {25, December, 2012},
        {1, January, 2013},
        {11, February, 2013},
        {1, March, 2013},
        {1, May, 2013},
        {17, May, 2013},
        {6, June, 2013},
        {15, August, 2013},
        {18, September, 2013},
        {19, September, 2013},
        {20, September, 2013},
        {3, October, 2013},
        {9, October, 2013},
        {25, December, 2013},
        {1, January, 2014},
        {30, January, 2014},
        {31, January, 2014},
        {1, May, 2014},
        {5, May, 2014},
        {6, May, 2014},
        {4, June, 2014},
        {6, June, 2014},
        {15, August, 2014},
        {8, September, 2014},
        {9, September, 2014},
        {10, September, 2014},
        {3, October, 2014},
        {9, October, 2014},
        {25, December, 2014},
        {1, January, 2015},
        {18, February, 2015},
        {19, February, 2015},
        {20, February, 2015},
        {1, May, 2015},
        {5, May, 2015},
        {25, May, 2015},
        {14, August, 2015},
        {28, September, 2015},
        {29, September, 2015},
        {9, October, 2015},
        {25, December, 2015},
        {1, January, 2016},
        {8, February, 2016},
        {9, February, 2016},
        {10, February, 2016},
        {1, March, 2016},
        {13, April, 2016},
        {5, May, 2016},
        {6, June, 2016},
        {15, August, 2016},
        {14, September, 2016},
        {15, September, 2016},
        {16, September, 2016},
        {3, October, 2016},
        {27, January, 2017},
        {30, January, 2017},
        {1, March, 2017},
        {1, May, 2017},
        {3, May, 2017},
        {5, May, 2017},
        {9, May, 2017},
        {6, June, 2017},
        {15, August, 2017},
        {3, October, 2017},
        {4, October, 2017},
        {5, October, 2017},
        {6, October, 2017},
        {9, October, 2017},
        {25, December, 2017},
        {1, January, 2018},
        {15, February, 2018},
        {16, February, 2018},
        {1, March, 2018},
        {1, May, 2018},
        {7, May, 2018},
        {22, May, 2018},
        {6, June, 2018},
        {13, June, 2018},
        {15, August, 2018},
        {24, September, 2018},
        {25, September, 2018},
        {26, September, 2018},
        {3, October, 2018},
        {9, October, 2018},
        {25, December, 2018},
        {1, January, 2019},
        {4, February, 2019},
        {5, February, 2019},
        {6, February, 2019},
        {1, March, 2019},
        {1, May, 2019},
        {6, May, 2019},
        {6, June, 2019},
        {15, August, 2019},
        {12, September, 2019},
        {13, September, 2019},
        {3, October, 2019},
        {9, October, 2019},
        {25, December, 2019},
        {1, January, 2020},
        {24, January, 2020},
        {27, January, 2020},
        {15, April, 2020},
        {30, April, 2020},
        {1, May, 2020},
        {5, May, 2020},
        {17, August, 2020},
        {30, September, 2020},
        {1, October, 2020},
        {2, October, 2020},
        {9, October, 2020},
        {25, December, 2020},
        {1, January, 2021},
        {11, February, 2021},
        {12, February, 2021},
        {1, March, 2021},
        {5, May, 2021},
        {19, May, 2021},
        {16, August, 2021},
        {20, September, 2021},
        {21, September, 2021},
        {22, September, 2021},
        {4, October, 2021},
        {11, October, 2021},
        {31, January, 2022},
        {1, February, 2022},
        {2, February, 2022},
        {1, March, 2022},
        {9, March, 2022},
        {5, May, 2022},
        {1, June, 2022},
        {6, June, 2022},
        {15, August, 2022},
        {9, September, 2022},
        {12, September, 2022},
        {3, October, 2022},
        {10, October, 2022},
        {23, January, 2023},
        {24, January, 2023},
        {1, March, 2023},
        {1, May, 2023},
        {5, May, 2023},
        {29, May, 2023},
        {6, June, 2023},
        {15, August, 2023},
        {28, September, 2023},
        {29, September, 2023},
        {2, October, 2023},
        {3, October, 2023},
        {9, October, 2023},
        {25, December, 2023},
        {1, January, 2024},
        {9, February, 2024},
        {12, February, 2024},
        {1, March, 2024},
        {10, April, 2024},
        {1, May, 2024},
        {6, May, 2024},
        {15, May, 2024},
        {6, June, 2024},
        {15, August, 2024},
        {16, September, 2024},
        {17, September, 2024},
        {18, September, 2024},
        {1, October, 2024},
        {3, October, 2024},
        {9, October, 2024},
        {25, December, 2024},
        {1, January, 2025},
        {27, January, 2025},
        {28, January, 2025},
        {29, January, 2025},
        {30, January, 2025},
        {3, March, 2025},
        {1, May, 2025},
        {5, May, 2025},
        {6, May, 2025},
        {6, June, 2025},
        {15, August, 2025},
        {3, October, 2025},
        {6, October, 2025},
        {7, October, 2025},
        {8, October, 2025},
        {9, October, 2025},
        {25, December, 2025},
        {1, January, 2026},
        {16, February, 2026},
        {17, February, 2026},
        {18, February, 2026},
        {2, March, 2026},
        {1, May, 2026},
        {5, May, 2026},
        {25, May, 2026},
        {17, August, 2026},
        {24, September, 2026},
        {25, September, 2026},
        {28, September, 2026},
        {5, October, 2026},
        {9, October, 2026},
        {25, December, 2026},
        {1, January, 2027},
        {8, February, 2027},
        {9, February, 2027},
        {1, March, 2027},
        {5, May, 2027},
        {13, May, 2027},
        {16, August, 2027},
        {14, September, 2027},
        {15, September, 2027},
        {16, September, 2027},
        {4, October, 2027},
        {11, October, 2027},
        {27, December, 2027},
        {26, January, 2028},
        {27, January, 2028},
        {28, January, 2028},
        {1, March, 2028},
        {1, May, 2028},
        {2, May, 2028},
        {5, May, 2028},
        {6, June, 2028},
        {15, August, 2028},
        {2, October, 2028},
        {3, October, 2028},
        {4, October, 2028},
        {5, October, 2028},
        {9, October, 2028},
        {25, December, 2028},
        {1, January, 2029},
        {12, February, 2029},
        {13, February, 2029},
        {14, February, 2029},
        {1, March, 2029},
        {1, May, 2029},
        {7, May, 2029},
        {21, May, 2029},
        {6, June, 2029},
        {15, August, 2029},
        {21, September, 2029},
        {24, September, 2029},
        {3, October, 2029},
        {9, October, 2029},
        {25, December, 2029},
        {1, January, 2030},
        {4, February, 2030},
        {5, February, 2030},
        {1, March, 2030},
        {1, May, 2030},
        {6, May, 2030},
        {9, May, 2030},
        {6, June, 2030},
        {15, August, 2030},
        {11, September, 2030},
        {12, September, 2030},
        {13, September, 2030},
        {3, October, 2030},
        {9, October, 2030},
        {25, December, 2030},
        {1, January, 2031},
        {22, January, 2031},
        {23, January, 2031},
        {24, January, 2031},
        {3, March, 2031},
        {1, May, 2031},
        {5, May, 2031},
        {28, May, 2031},
        {6, June, 2031},
        {15, August, 2031},
        {30, September, 2031},
        {1, October, 2031},
        {2, October, 2031},
        {3, October, 2031},
        {9, October, 2031},
        {25, December, 2031},
        {1, January, 2032},
        {10, February, 2032},
        {11, February, 2032},
        {12, February, 2032},
        {1, March, 2032},
        {5, May, 2032},
        {17, May, 2032},
        {16, August, 2032},
        {20, September, 2032},
        {21, September, 2032},
        {4, October, 2032},
        {11, October, 2032},
        {27, December, 2032},
        {31, January, 2033},
        {1, February, 2033},
        {2, February, 2033},
        {1, March, 2033},
        {5, May, 2033},
        {6, May, 2033},
        {6, June, 2033},
        {15, August, 2033},
        {7, September, 2033},
        {8, September, 2033},
        {9, September, 2033},
        {3, October, 2033},
        {10, October, 2033},
        {26, December, 2033},
        {20, February, 2034},
        {21, February, 2034},
        {1, March, 2034},
        {1, May, 2034},
        {5, May, 2034},
        {25, May, 2034},
        {6, June, 2034},
        {15, August, 2034},
        {26, September, 2034},
        {27, September, 2034},
        {28, September, 2034},
        {3, October, 2034},
        {9, October, 2034},
        {25, December, 2034},
        {1, January, 2035},
        {7, February, 2035},
        {8, February, 2035},
        {9, February, 2035},
        {1, March, 2035},
        {1, May, 2035},
        {7, May, 2035},
        {15, May, 2035},
        {6, June, 2035},
        {15, August, 2035},
        {17, September, 2035},
        {18, September, 2035},
        {3, October, 2035},
        {9, October, 2035},
        {25, December, 2035},
        {1, January, 2036},
        {28, January, 2036},
        {29, January, 2036},
        {30, January, 2036},
        {3, March, 2036},
        {1, May, 2036},
        {5, May, 2036},
        {6, May, 2036},
        {6, June, 2036},
        {15, August, 2036},
        {3, October, 2036},
        {6, October, 2036},
        {7, October, 2036},
        {9, October, 2036},
        {25, December, 2036},
        {1, January, 2037},
        {16, February, 2037},
        {17, February, 2037},
        {2, March, 2037},
        {1, May, 2037},
        {5, May, 2037},
        {22, May, 2037},
        {17, August, 2037},
        {23, September, 2037},
        {24, September, 2037},
        {25, September, 2037},
        {5, October, 2037},
        {9, October, 2037},
        {25, December, 2037},
        {1, January, 2038},
        {3, February, 2038},
        {4, February, 2038},
        {5, February, 2038},
        {1, March, 2038},
        {5, May, 2038},
        {11, May, 2038},
        {16, August, 2038},
        {13, September, 2038},
        {14, September, 2038},
        {15, September, 2038},
        {4, October, 2038},
        {11, October, 2038},
        {27, December, 2038},
        {24, January, 2039},
        {25, January, 2039},
        {26, January, 2039},
        {1, March, 2039},
        {2, May, 2039},
        {5, May, 2039},
        {6, June, 2039},
        {15, August, 2039},
        {3, October, 2039},
        {4, October, 2039},
        {5, October, 2039},
        {10, October, 2039},
        {26, December, 2039},
        {13, February, 2040},
        {14, February, 2040},
        {1, March, 2040},
        {1, May, 2040},
        {7, May, 2040},
        {18, May, 2040},
        {6, June, 2040},
        {15, August, 2040},
        {20, September, 2040},
        {21, September, 2040},
        {3, October, 2040},
        {9, October, 2040},
        {25, December, 2040},
        {1, January, 2041},
        {31, January, 2041},
        {1, February, 2041},
        {1, March, 2041},
        {1, May, 2041},
        {6, May, 2041},
        {7, May, 2041},
        {6, June, 2041},
        {15, August, 2041},
        {9, September, 2041},
        {10, September, 2041},
        {11, September, 2041},
        {3, October, 2041},
        {9, October, 2041},
        {25, December, 2041},
        {1, January, 2042},
        {21, January, 2042},
        {22, January, 2042},
        {23, January, 2042},
        {3, March, 2042},
        {1, May, 2042},
        {5, May, 2042},
        {26, May, 2042},
        {6, June, 2042},
        {15, August, 2042},
        {29, September, 2042},
        {30, September, 2042},
        {3, October, 2042},
        {9, October, 2042},
        {25, December, 2042},
        {1, January, 2043},
        {9, February, 2043},
        {10, February, 2043},
        {11, February, 2043},
        {2, March, 2043},
        {1, May, 2043},
        {5, May, 2043},
        {18, May, 2043},
        {17, August, 2043},
        {16, September, 2043},
        {17, September, 2043},
        {18, September, 2043},
        {5, October, 2043},
        {9, October, 2043},
        {25, December, 2043},
        {1, January, 2044},
        {29, January, 2044},
        {1, February, 2044},
        {1, March, 2044},
        {5, May, 2044},
        {6, May, 2044},
        {6, June, 2044},
        {15, August, 2044},
        {3, October, 2044},
        {4, October, 2044},
        {5, October, 2044},
        {6, October, 2044},
        {10, October, 2044},
        {26, December, 2044},
        {16, February, 2045},
        {17, February, 2045},
        {1, March, 2045},
        {1, May, 2045},
        {5, May, 2045},
        {24, May, 2045},
        {6, June, 2045},
        {15, August, 2045},
        {25, September, 2045},
        {26, September, 2045},
        {27, September, 2045},
        {3, October, 2045},
        {9, October, 2045},
        {25, December, 2045},
        {1, January, 2046},
        {5, February, 2046},
        {6, February, 2046},
        {7, February, 2046},
        {1, March, 2046},
        {1, May, 2046},
        {7, May, 2046},
        {14, May, 2046},
        {6, June, 2046},
        {15, August, 2046},
        {14, September, 2046},
        {17, September, 2046},
        {3, October, 2046},
        {9, October, 2046},
        {25, December, 2046},
        {1, January, 2047},
        {25, January, 2047},
        {28, January, 2047},
        {1, March, 2047},
        {1, May, 2047},
        {2, May, 2047},
        {6, May, 2047},
        {6, June, 2047},
        {15, August, 2047},
        {3, October, 2047},
        {4, October, 2047},
        {7, October, 2047},
        {9, October, 2047},
        {25, December, 2047},
        {1, January, 2048},
        {13, February, 2048},
        {14, February, 2048},
        {2, March, 2048},
        {1, May, 2048},
        {5, May, 2048},
        {20, May, 2048},
        {17, August, 2048},
        {21, September, 2048},
        {22, September, 2048},
        {23, September, 2048},
        {5, October, 2048},
        {9, October, 2048},
        {25, December, 2048},
        {1, January, 2049},
        {1, February, 2049},
        {2, February, 2049},
        {3, February, 2049},
        {1, March, 2049},
        {5, May, 2049},
        {10, May, 2049},
        {16, August, 2049},
        {10, September, 2049},
        {13, September, 2049},
        {4, October, 2049},
        {11, October, 2049},
        {27, December, 2049},
        {24, January, 2050},
        {25, January, 2050},
        {1, March, 2050},
        {5, May, 2050},
        {30, May, 2050},
        {6, June, 2050},
        {15, August, 2050},
        {29, September, 2050},
        {30, September, 2050},
        {3, October, 2050},
        {10, October, 2050},
        {26, December, 2050},
    };

    Calendar c = SouthKorea(SouthKorea::Settlement);
    checkHolidays(c.holidayList(Date(1, January, 2004), Date(31, December, 2050)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testKoreaStockExchange) {
    BOOST_TEST_MESSAGE("Testing Korea Stock Exchange holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2004},
        {21, January, 2004},
        {22, January, 2004},
        {23, January, 2004},
        {1, March, 2004},
        {5, April, 2004},
        {15, April, 2004}, // election day
        {5, May, 2004},
        {26, May, 2004},
        {27, September, 2004},
        {28, September, 2004},
        {29, September, 2004},
        {31, December, 2004},
        {8, February, 2005},
        {9, February, 2005},
        {10, February, 2005},
        {1, March, 2005},
        {5, April, 2005},
        {5, May, 2005},
        {6, June, 2005},
        {15, August, 2005},
        {19, September, 2005},
        {3, October, 2005},
        {30, December, 2005},
        {30, January, 2006},
        {1, March, 2006},
        {1, May, 2006},
        {5, May, 2006},
        {31, May, 2006}, // election
        {6, June, 2006},
        {17, July, 2006},
        {15, August, 2006},
        {3, October, 2006},
        {5, October, 2006},
        {6, October, 2006},
        {25, December, 2006},
        {29, December, 2006},
        {1, January, 2007},
        {19, February, 2007},
        {1, March, 2007},
        {1, May, 2007},
        {24, May, 2007},
        {6, June, 2007},
        {17, July, 2007},
        {15, August, 2007},
        {24, September, 2007},
        {25, September, 2007},
        {26, September, 2007},
        {3, October, 2007},
        {19, December, 2007}, // election
        {25, December, 2007},
        {31, December, 2007},
        {1, January, 2008},
        {6, February, 2008},
        {7, February, 2008},
        {8, February, 2008},
        {9, April, 2008},
        {1, May, 2008},
        {5, May, 2008},
        {12, May, 2008},
        {6, June, 2008},
        {15, August, 2008},
        {15, September, 2008},
        {3, October, 2008},
        {25, December, 2008},
        {31, December, 2008},
        {1, January, 2009},
        {26, January, 2009},
        {27, January, 2009},
        {1, May, 2009},
        {5, May, 2009},
        {2, October, 2009},
        {25, December, 2009},
        {31, December, 2009},
        {1, January, 2010},
        {15, February, 2010},
        {1, March, 2010},
        {5, May, 2010},
        {21, May, 2010},
        {2, June, 2010},
        {21, September, 2010},
        {22, September, 2010},
        {23, September, 2010},
        {31, December, 2010},
        {2, February, 2011},
        {3, February, 2011},
        {4, February, 2011},
        {1, March, 2011},
        {5, May, 2011},
        {10, May, 2011},
        {6, June, 2011},
        {15, August, 2011},
        {12, September, 2011},
        {13, September, 2011},
        {3, October, 2011},
        {30, December, 2011},
        {23, January, 2012},
        {24, January, 2012},
        {1, March, 2012},
        {11, April, 2012},
        {1, May, 2012},
        {28, May, 2012},
        {6, June, 2012},
        {15, August, 2012},
        {1, October, 2012},
        {3, October, 2012},
        {19, December, 2012},
        {25, December, 2012},
        {31, December, 2012},
        {1, January, 2013},
        {11, February, 2013},
        {1, March, 2013},
        {1, May, 2013},
        {17, May, 2013},
        {6, June, 2013},
        {15, August, 2013},
        {18, September, 2013},
        {19, September, 2013},
        {20, September, 2013},
        {3, October, 2013},
        {9, October, 2013},
        {25, December, 2013},
        {31, December, 2013},
        {1, January, 2014},
        {30, January, 2014},
        {31, January, 2014},
        {1, May, 2014},
        {5, May, 2014},
        {6, May, 2014},
        {4, June, 2014},
        {6, June, 2014},
        {15, August, 2014},
        {8, September, 2014},
        {9, September, 2014},
        {10, September, 2014},
        {3, October, 2014},
        {9, October, 2014},
        {25, December, 2014},
        {31, December, 2014},
        {1, January, 2015},
        {18, February, 2015},
        {19, February, 2015},
        {20, February, 2015},
        {1, May, 2015},
        {5, May, 2015},
        {25, May, 2015},
        {14, August, 2015},
        {28, September, 2015},
        {29, September, 2015},
        {9, October, 2015},
        {25, December, 2015},
        {31, December, 2015},
        {1, January, 2016},
        {8, February, 2016},
        {9, February, 2016},
        {10, February, 2016},
        {1, March, 2016},
        {13, April, 2016},
        {5, May, 2016},
        {6, May, 2016},
        {6, June, 2016},
        {15, August, 2016},
        {14, September, 2016},
        {15, September, 2016},
        {16, September, 2016},
        {3, October, 2016},
        {30, December, 2016},
        {27, January, 2017},
        {30, January, 2017},
        {1, March, 2017},
        {1, May, 2017},
        {3, May, 2017},
        {5, May, 2017},
        {9, May, 2017},
        {6, June, 2017},
        {15, August, 2017},
        {2, October, 2017},
        {3, October, 2017},
        {4, October, 2017},
        {5, October, 2017},
        {6, October, 2017},
        {9, October, 2017},
        {25, December, 2017},
        {29, December, 2017},
        {1, January, 2018},
        {15, February, 2018},
        {16, February, 2018},
        {1, March, 2018},
        {1, May, 2018},
        {7, May, 2018},
        {22, May, 2018},
        {6, June, 2018},
        {13, June, 2018},
        {15, August, 2018},
        {24, September, 2018},
        {25, September, 2018},
        {26, September, 2018},
        {3, October, 2018},
        {9, October, 2018},
        {25, December, 2018},
        {31, December, 2018},
        {1, January, 2019},
        {4, February, 2019},
        {5, February, 2019},
        {6, February, 2019},
        {1, March, 2019},
        {1, May, 2019},
        {6, May, 2019},
        {6, June, 2019},
        {15, August, 2019},
        {12, September, 2019},
        {13, September, 2019},
        {3, October, 2019},
        {9, October, 2019},
        {25, December, 2019},
        {31, December, 2019},
        {1, January, 2020},
        {24, January, 2020},
        {27, January, 2020},
        {15, April, 2020},
        {30, April, 2020},
        {1, May, 2020},
        {5, May, 2020},
        {17, August, 2020},
        {30, September, 2020},
        {1, October, 2020},
        {2, October, 2020},
        {9, October, 2020},
        {25, December, 2020},
        {31, December, 2020},
        {1, January, 2021},
        {11, February, 2021},
        {12, February, 2021},
        {1, March, 2021},
        {5, May, 2021},
        {19, May, 2021},
        {16, August, 2021},
        {20, September, 2021},
        {21, September, 2021},
        {22, September, 2021},
        {4, October, 2021},
        {11, October, 2021},
        {31, December, 2021},
        {31, January, 2022},
        {1, February, 2022},
        {2, February, 2022},
        {1, March, 2022},
        {9, March, 2022},
        {5, May, 2022},
        {1, June, 2022},
        {6, June, 2022},
        {15, August, 2022},
        {9, September, 2022},
        {12, September, 2022},
        {3, October, 2022},
        {10, October, 2022},
        {30, December, 2022},
        {23, January, 2023},
        {24, January, 2023},
        {1, March, 2023},
        {1, May, 2023},
        {5, May, 2023},
        {29, May, 2023},
        {6, June, 2023},
        {15, August, 2023},
        {28, September, 2023},
        {29, September, 2023},
        {2, October, 2023},
        {3, October, 2023},
        {9, October, 2023},
        {25, December, 2023},
        {29, December, 2023},
        {1, January, 2024},
        {9, February, 2024},
        {12, February, 2024},
        {1, March, 2024},
        {10, April, 2024},
        {1, May, 2024},
        {6, May, 2024},
        {15, May, 2024},
        {6, June, 2024},
        {15, August, 2024},
        {16, September, 2024},
        {17, September, 2024},
        {18, September, 2024},
        {1, October, 2024},
        {3, October, 2024},
        {9, October, 2024},
        {25, December, 2024},
        {31, December, 2024},
        {1, January, 2025},
        {27, January, 2025},
        {28, January, 2025},
        {29, January, 2025},
        {30, January, 2025},
        {3, March, 2025},
        {1, May, 2025},
        {5, May, 2025},
        {6, May, 2025},
        {6, June, 2025},
        {15, August, 2025},
        {3, October, 2025},
        {6, October, 2025},
        {7, October, 2025},
        {8, October, 2025},
        {9, October, 2025},
        {25, December, 2025},
        {31, December, 2025},
        {1, January, 2026},
        {16, February, 2026},
        {17, February, 2026},
        {18, February, 2026},
        {2, March, 2026},
        {1, May, 2026},
        {5, May, 2026},
        {25, May, 2026},
        {17, August, 2026},
        {24, September, 2026},
        {25, September, 2026},
        {28, September, 2026},
        {5, October, 2026},
        {9, October, 2026},
        {25, December, 2026},
        {31, December, 2026},
        {1, January, 2027},
        {8, February, 2027},
        {9, February, 2027},
        {1, March, 2027},
        {5, May, 2027},
        {13, May, 2027},
        {16, August, 2027},
        {14, September, 2027},
        {15, September, 2027},
        {16, September, 2027},
        {4, October, 2027},
        {11, October, 2027},
        {27, December, 2027},
        {31, December, 2027},
        {26, January, 2028},
        {27, January, 2028},
        {28, January, 2028},
        {1, March, 2028},
        {1, May, 2028},
        {2, May, 2028},
        {5, May, 2028},
        {6, June, 2028},
        {15, August, 2028},
        {2, October, 2028},
        {3, October, 2028},
        {4, October, 2028},
        {5, October, 2028},
        {9, October, 2028},
        {25, December, 2028},
        {29, December, 2028},
        {1, January, 2029},
        {12, February, 2029},
        {13, February, 2029},
        {14, February, 2029},
        {1, March, 2029},
        {1, May, 2029},
        {7, May, 2029},
        {21, May, 2029},
        {6, June, 2029},
        {15, August, 2029},
        {21, September, 2029},
        {24, September, 2029},
        {3, October, 2029},
        {9, October, 2029},
        {25, December, 2029},
        {31, December, 2029},
        {1, January, 2030},
        {4, February, 2030},
        {5, February, 2030},
        {1, March, 2030},
        {1, May, 2030},
        {6, May, 2030},
        {9, May, 2030},
        {6, June, 2030},
        {15, August, 2030},
        {11, September, 2030},
        {12, September, 2030},
        {13, September, 2030},
        {3, October, 2030},
        {9, October, 2030},
        {25, December, 2030},
        {31, December, 2030},
        {1, January, 2031},
        {22, January, 2031},
        {23, January, 2031},
        {24, January, 2031},
        {3, March, 2031},
        {1, May, 2031},
        {5, May, 2031},
        {28, May, 2031},
        {6, June, 2031},
        {15, August, 2031},
        {30, September, 2031},
        {1, October, 2031},
        {2, October, 2031},
        {3, October, 2031},
        {9, October, 2031},
        {25, December, 2031},
        {31, December, 2031},
        {1, January, 2032},
        {10, February, 2032},
        {11, February, 2032},
        {12, February, 2032},
        {1, March, 2032},
        {5, May, 2032},
        {17, May, 2032},
        {16, August, 2032},
        {20, September, 2032},
        {21, September, 2032},
        {4, October, 2032},
        {11, October, 2032},
        {27, December, 2032},
        {31, December, 2032},
        {31, January, 2033},
        {1, February, 2033},
        {2, February, 2033},
        {1, March, 2033},
        {5, May, 2033},
        {6, May, 2033},
        {6, June, 2033},
        {15, August, 2033},
        {7, September, 2033},
        {8, September, 2033},
        {9, September, 2033},
        {3, October, 2033},
        {10, October, 2033},
        {26, December, 2033},
        {30, December, 2033},
        {20, February, 2034},
        {21, February, 2034},
        {1, March, 2034},
        {1, May, 2034},
        {5, May, 2034},
        {25, May, 2034},
        {6, June, 2034},
        {15, August, 2034},
        {26, September, 2034},
        {27, September, 2034},
        {28, September, 2034},
        {3, October, 2034},
        {9, October, 2034},
        {25, December, 2034},
        {29, December, 2034},
        {1, January, 2035},
        {7, February, 2035},
        {8, February, 2035},
        {9, February, 2035},
        {1, March, 2035},
        {1, May, 2035},
        {7, May, 2035},
        {15, May, 2035},
        {6, June, 2035},
        {15, August, 2035},
        {17, September, 2035},
        {18, September, 2035},
        {3, October, 2035},
        {9, October, 2035},
        {25, December, 2035},
        {31, December, 2035},
        {1, January, 2036},
        {28, January, 2036},
        {29, January, 2036},
        {30, January, 2036},
        {3, March, 2036},
        {1, May, 2036},
        {5, May, 2036},
        {6, May, 2036},
        {6, June, 2036},
        {15, August, 2036},
        {3, October, 2036},
        {6, October, 2036},
        {7, October, 2036},
        {9, October, 2036},
        {25, December, 2036},
        {31, December, 2036},
        {1, January, 2037},
        {16, February, 2037},
        {17, February, 2037},
        {2, March, 2037},
        {1, May, 2037},
        {5, May, 2037},
        {22, May, 2037},
        {17, August, 2037},
        {23, September, 2037},
        {24, September, 2037},
        {25, September, 2037},
        {5, October, 2037},
        {9, October, 2037},
        {25, December, 2037},
        {31, December, 2037},
        {1, January, 2038},
        {3, February, 2038},
        {4, February, 2038},
        {5, February, 2038},
        {1, March, 2038},
        {5, May, 2038},
        {11, May, 2038},
        {16, August, 2038},
        {13, September, 2038},
        {14, September, 2038},
        {15, September, 2038},
        {4, October, 2038},
        {11, October, 2038},
        {27, December, 2038},
        {31, December, 2038},
        {24, January, 2039},
        {25, January, 2039},
        {26, January, 2039},
        {1, March, 2039},
        {2, May, 2039},
        {5, May, 2039},
        {6, June, 2039},
        {15, August, 2039},
        {3, October, 2039},
        {4, October, 2039},
        {5, October, 2039},
        {10, October, 2039},
        {26, December, 2039},
        {30, December, 2039},
        {13, February, 2040},
        {14, February, 2040},
        {1, March, 2040},
        {1, May, 2040},
        {7, May, 2040},
        {18, May, 2040},
        {6, June, 2040},
        {15, August, 2040},
        {20, September, 2040},
        {21, September, 2040},
        {3, October, 2040},
        {9, October, 2040},
        {25, December, 2040},
        {31, December, 2040},
        {1, January, 2041},
        {31, January, 2041},
        {1, February, 2041},
        {1, March, 2041},
        {1, May, 2041},
        {6, May, 2041},
        {7, May, 2041},
        {6, June, 2041},
        {15, August, 2041},
        {9, September, 2041},
        {10, September, 2041},
        {11, September, 2041},
        {3, October, 2041},
        {9, October, 2041},
        {25, December, 2041},
        {31, December, 2041},
        {1, January, 2042},
        {21, January, 2042},
        {22, January, 2042},
        {23, January, 2042},
        {3, March, 2042},
        {1, May, 2042},
        {5, May, 2042},
        {26, May, 2042},
        {6, June, 2042},
        {15, August, 2042},
        {29, September, 2042},
        {30, September, 2042},
        {3, October, 2042},
        {9, October, 2042},
        {25, December, 2042},
        {31, December, 2042},
        {1, January, 2043},
        {9, February, 2043},
        {10, February, 2043},
        {11, February, 2043},
        {2, March, 2043},
        {1, May, 2043},
        {5, May, 2043},
        {18, May, 2043},
        {17, August, 2043},
        {16, September, 2043},
        {17, September, 2043},
        {18, September, 2043},
        {5, October, 2043},
        {9, October, 2043},
        {25, December, 2043},
        {31, December, 2043},
        {1, January, 2044},
        {29, January, 2044},
        {1, February, 2044},
        {1, March, 2044},
        {5, May, 2044},
        {6, May, 2044},
        {6, June, 2044},
        {15, August, 2044},
        {3, October, 2044},
        {4, October, 2044},
        {5, October, 2044},
        {6, October, 2044},
        {10, October, 2044},
        {26, December, 2044},
        {30, December, 2044},
        {16, February, 2045},
        {17, February, 2045},
        {1, March, 2045},
        {1, May, 2045},
        {5, May, 2045},
        {24, May, 2045},
        {6, June, 2045},
        {15, August, 2045},
        {25, September, 2045},
        {26, September, 2045},
        {27, September, 2045},
        {3, October, 2045},
        {9, October, 2045},
        {25, December, 2045},
        {29, December, 2045},
        {1, January, 2046},
        {5, February, 2046},
        {6, February, 2046},
        {7, February, 2046},
        {1, March, 2046},
        {1, May, 2046},
        {7, May, 2046},
        {14, May, 2046},
        {6, June, 2046},
        {15, August, 2046},
        {14, September, 2046},
        {17, September, 2046},
        {3, October, 2046},
        {9, October, 2046},
        {25, December, 2046},
        {31, December, 2046},
        {1, January, 2047},
        {25, January, 2047},
        {28, January, 2047},
        {1, March, 2047},
        {1, May, 2047},
        {2, May, 2047},
        {6, May, 2047},
        {6, June, 2047},
        {15, August, 2047},
        {3, October, 2047},
        {4, October, 2047},
        {7, October, 2047},
        {9, October, 2047},
        {25, December, 2047},
        {31, December, 2047},
        {1, January, 2048},
        {13, February, 2048},
        {14, February, 2048},
        {2, March, 2048},
        {1, May, 2048},
        {5, May, 2048},
        {20, May, 2048},
        {17, August, 2048},
        {21, September, 2048},
        {22, September, 2048},
        {23, September, 2048},
        {5, October, 2048},
        {9, October, 2048},
        {25, December, 2048},
        {31, December, 2048},
        {1, January, 2049},
        {1, February, 2049},
        {2, February, 2049},
        {3, February, 2049},
        {1, March, 2049},
        {5, May, 2049},
        {10, May, 2049},
        {16, August, 2049},
        {10, September, 2049},
        {13, September, 2049},
        {4, October, 2049},
        {11, October, 2049},
        {27, December, 2049},
        {31, December, 2049},
        {24, January, 2050},
        {25, January, 2050},
        {1, March, 2050},
        {5, May, 2050},
        {30, May, 2050},
        {6, June, 2050},
        {15, August, 2050},
        {29, September, 2050},
        {30, September, 2050},
        {3, October, 2050},
        {10, October, 2050},
        {26, December, 2050},
        {30, December, 2050},
    };

    Calendar c = SouthKorea(SouthKorea::KRX);
    checkHolidays(c.holidayList(Date(1, January, 2004), Date(31, December, 2050)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testChinaSSE) {
    BOOST_TEST_MESSAGE("Testing China Shanghai Stock Exchange holiday list...");

    std::vector<Date> expectedHol = {

        // China Shanghai Securities Exchange holiday list in the year 2014
        {1, Jan, 2014},
        {31, Jan, 2014},
        {3, Feb, 2014},
        {4, Feb, 2014},
        {5, Feb, 2014},
        {6, Feb, 2014},
        {7, Apr, 2014},
        {1, May, 2014},
        {2, May, 2014},
        {2, Jun, 2014},
        {8, Sep, 2014},
        {1, Oct, 2014},
        {2, Oct, 2014},
        {3, Oct, 2014},
        {6, Oct, 2014},
        {7, Oct, 2014},

        // China Shanghai Securities Exchange holiday list in the year 2015
        {1, Jan, 2015},
        {2, Jan, 2015},
        {18, Feb, 2015},
        {19, Feb, 2015},
        {20, Feb, 2015},
        {23, Feb, 2015},
        {24, Feb, 2015},
        {6, Apr, 2015},
        {1, May, 2015},
        {22, Jun, 2015},
        {3, Sep, 2015},
        {4, Sep, 2015},
        {1, Oct, 2015},
        {2, Oct, 2015},
        {5, Oct, 2015},
        {6, Oct, 2015},
        {7, Oct, 2015},

        // China Shanghai Securities Exchange holiday list in the year 2016
        {1, Jan, 2016},
        {8, Feb, 2016},
        {9, Feb, 2016},
        {10, Feb, 2016},
        {11, Feb, 2016},
        {12, Feb, 2016},
        {4, Apr, 2016},
        {2, May, 2016},
        {9, Jun, 2016},
        {10, Jun, 2016},
        {15, Sep, 2016},
        {16, Sep, 2016},
        {3, Oct, 2016},
        {4, Oct, 2016},
        {5, Oct, 2016},
        {6, Oct, 2016},
        {7, Oct, 2016},

        // China Shanghai Securities Exchange holiday list in the year 2017
        {2, Jan, 2017},
        {27, Jan, 2017},
        {30, Jan, 2017},
        {31, Jan, 2017},
        {1, Feb, 2017},
        {2, Feb, 2017},
        {3, April, 2017},
        {4, April, 2017},
        {1, May, 2017},
        {29, May, 2017},
        {30, May, 2017},
        {2, Oct, 2017},
        {3, Oct, 2017},
        {4, Oct, 2017},
        {5, Oct, 2017},
        {6, Oct, 2017},

        // China Shanghai Securities Exchange holiday list in the year 2018
        {1, Jan, 2018},
        {15, Feb, 2018},
        {16, Feb, 2018},
        {19, Feb, 2018},
        {20, Feb, 2018},
        {21, Feb, 2018},
        {5, April, 2018},
        {6, April, 2018},
        {30, April, 2018},
        {1, May, 2018},
        {18, June, 2018},
        {24, September, 2018},
        {1, Oct, 2018},
        {2, Oct, 2018},
        {3, Oct, 2018},
        {4, Oct, 2018},
        {5, Oct, 2018},
        {31, December, 2018},

        // China Shanghai Securities Exchange holiday list in the year 2019
        {1, Jan, 2019},
        {4, Feb, 2019},
        {5, Feb, 2019},
        {6, Feb, 2019},
        {7, Feb, 2019},
        {8, Feb, 2019},
        {5, April, 2019},
        {1, May, 2019},
        {2, May, 2019},
        {3, May, 2019},
        {7, June, 2019},
        {13, September, 2019},
        {1, October, 2019},
        {2, October, 2019},
        {3, October, 2019},
        {4, October, 2019},
        {7, October, 2019},

        // China Shanghai Securities Exchange holiday list in the year 2020
        {1, Jan, 2020},
        {24, Jan, 2020},
        {27, Jan, 2020},
        {28, Jan, 2020},
        {29, Jan, 2020},
        {30, Jan, 2020},
        {31, Jan, 2020},
        {6, April, 2020},
        {1, May, 2020},
        {4, May, 2020},
        {5, May, 2020},
        {25, June, 2020},
        {26, June, 2020},
        {1, October, 2020},
        {2, October, 2020},
        {5, October, 2020},
        {6, October, 2020},
        {7, October, 2020},
        {8, October, 2020},

        // China Shanghai Securities Exchange holiday list in the year 2021
        {1, Jan, 2021},
        {11, Feb, 2021},
        {12, Feb, 2021},
        {15, Feb, 2021},
        {16, Feb, 2021},
        {17, Feb, 2021},
        {5, April, 2021},
        {3, May, 2021},
        {4, May, 2021},
        {5, May, 2021},
        {14, June, 2021},
        {20, September, 2021},
        {21, September, 2021},
        {1, October, 2021},
        {4, October, 2021},
        {5, October, 2021},
        {6, October, 2021},
        {7, October, 2021},

        // China Shanghai Securities Exchange holiday list in the year 2022
        {3, Jan, 2022},
        {31, Jan, 2022},
        {1, Feb, 2022},
        {2, Feb, 2022},
        {3, Feb, 2022},
        {4, Feb, 2022},
        {4, April, 2022},
        {5, April, 2022},
        {2, May, 2022},
        {3, May, 2022},
        {4, May, 2022},
        {3, June, 2022},
        {12, September, 2022},
        {3, October, 2022},
        {4, October, 2022},
        {5, October, 2022},
        {6, October, 2022},
        {7, October, 2022},

        // China Shanghai Securities Exchange holiday list in the year 2023
        {2, Jan, 2023},
        {23, Jan, 2023},
        {24, Jan, 2023},
        {25, Jan, 2023},
        {26, Jan, 2023},
        {27, Jan, 2023},
        {5, April, 2023},
        {1, May, 2023},
        {2, May, 2023},
        {3, May, 2023},
        {22, June, 2023},
        {23, June, 2023},
        {29, September, 2023},
        {2, October, 2023},
        {3, October, 2023},
        {4, October, 2023},
        {5, October, 2023},
        {6, October, 2023},

        // China Shanghai Securities Exchange holiday list in the year 2024
        {1, Jan, 2024},
        {9, Feb, 2024},
        {12, Feb, 2024},
        {13, Feb, 2024},
        {14, Feb, 2024},
        {15, Feb, 2024},
        {16, Feb, 2024},
        {4, April, 2024},
        {5, April, 2024},
        {1, May, 2024},
        {2, May, 2024},
        {3, May, 2024},
        {10, Jun, 2024},
        {16, Sep, 2024},
        {17, Sep, 2024},
        {1, Oct, 2024},
        {2, Oct, 2024},
        {3, Oct, 2024},
        {4, Oct, 2024},
        {7, Oct, 2024},

        // China Shanghai Securities Exchange holiday list in the year 2025
        {1, Jan, 2025},
        {28, Jan, 2025},
        {29, Jan, 2025},
        {30, Jan, 2025},
        {31, Jan, 2025},
        {3, Feb, 2025},
        {4, Feb, 2025},
        {4, April, 2025},
        {1, May, 2025},
        {2, May, 2025},
        {5, May, 2025},
        {2, Jun, 2025},
        {1, Oct, 2025},
        {2, Oct, 2025},
        {3, Oct, 2025},
        {6, Oct, 2025},
        {7, Oct, 2025},
        {8, Oct, 2025},

        // China Shanghai Securities Exchange holiday list in the year 2026
        {1, Jan, 2026},
        {2, Jan, 2026},
        {16, Feb, 2026},
        {17, Feb, 2026},
        {18, Feb, 2026},
        {19, Feb, 2026},
        {20, Feb, 2026},
        {23, Feb, 2026},
        {6, April, 2026},
        {1, May, 2026},
        {4, May, 2026},
        {5, May, 2026},
        {19, Jun, 2026},
        {25, Sep, 2026},
        {1, Oct, 2026},
        {2, Oct, 2026},
        {5, Oct, 2026},
        {6, Oct, 2026},
        {7, Oct, 2026},
    };

    Calendar c = China(China::SSE);
    checkHolidays(c.holidayList(Date(1, January, 2014), Date(31, December, 2026)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testChinaIB) {
    BOOST_TEST_MESSAGE("Testing China Inter Bank working weekends list...");

    std::vector<Date> expectedWorkingWeekEnds = {

        // China Inter Bank working weekends list in the year 2014
        {26, Jan, 2014},
        {8, Feb, 2014},
        {4, May, 2014},
        {28, Sep, 2014},
        {11, Oct, 2014},

        // China Inter Bank working weekends list in the year 2015
        {4, Jan, 2015},
        {15, Feb, 2015},
        {28, Feb, 2015},
        {6, Sep, 2015},
        {10, Oct, 2015},

        // China Inter Bank working weekends list in the year 2016
        {6, Feb, 2016},
        {14, Feb, 2016},
        {12, Jun, 2016},
        {18, Sep, 2016},
        {8, Oct, 2016},
        {9, Oct, 2016},

        // China Inter Bank working weekends list in the year 2017
        {22, Jan, 2017},
        {4, Feb, 2017},
        {1, April, 2017},
        {27, May, 2017},
        {30, Sep, 2017},

        // China Inter Bank working weekends list in the year 2018
        {11, Feb, 2018},
        {24, Feb, 2018},
        {8, April, 2018},
        {28, April, 2018},
        {29, Sep, 2018},
        {30, Sep, 2018},
        {29, December, 2018},

        // China Inter Bank working weekends list in the year 2019
        {2, Feb, 2019},
        {3, Feb, 2019},
        {28, April, 2019},
        {5, May, 2019},
        {29, September, 2019},
        {12, October, 2019},

        // China Inter Bank working weekends list in the year 2020
        {19, January, 2020},
        {26, April, 2020},
        {9, May, 2020},
        {28, June, 2020},
        {27, September, 2020},
        {10, October, 2020},

        // China Inter Bank working weekends list in the year 2021
        {7, Feb, 2021},
        {20, Feb, 2021},
        {25, April, 2021},
        {8, May, 2021},
        {18, September, 2021},
        {26, September, 2021},
        {9, October, 2021},

        // China Inter Bank working weekends list in the year 2022
        {29, Jan, 2022},
        {30, Jan, 2022},
        {2, April, 2022},
        {24, April, 2022},
        {7, May, 2022},
        {8, October, 2022},
        {9, October, 2022},

        // China Inter Bank working weekends list in the year 2023
        {28, Jan, 2023},
        {29, Jan, 2023},
        {23, April, 2023},
        {6, May, 2023},
        {25, June, 2023},
        {7, October, 2023},
        {8, October, 2023},

        // China Inter Bank working weekends list in the year 2024
        {4, Feb, 2024},
        {18, Feb, 2024},
        {7, April, 2024},
        {28, April, 2024},
        {11, May, 2024},
        {14, Sep, 2024},
        {29, Sep, 2024},
        {12, October, 2024},

        // China Inter Bank working weekends list in the year 2025
        {26, Jan, 2025},
        {8, Feb, 2025},
        {27, Apr, 2025},
        {28, Sep, 2025},
        {11, Oct, 2025},

        // China Inter Bank working weekends list in the year 2026
        {4, Jan, 2026},
        {14, Feb, 2026},
        {28, Feb, 2026},
        {9, May, 2026},
        {20, Sep, 2026},
        {10, Oct, 2026},
    };

    Calendar c = China(China::IB);
    Date start(1, Jan, 2014);
    Date end(31, Dec, 2026);

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
    std::vector<Date> expectedHol = {
        {1, Oct, 2024},
        {1, Oct, 2030},
        {1, Oct, 2036},
        {1, Oct, 2042},
        {1, Oct, 2048},
    };

    // Some years of non-Inaugurations
    std::vector<Date> expectedWorkingDays = {
        {1, Oct, 2018},
        {1, Oct, 2025},
        {1, Oct, 2026},
        {1, Oct, 2027},
        // 2028 falls on a weekend
        {1, Oct, 2029},
        {1, Oct, 2031},
        {1, Oct, 2032},
        // 2033 and 2034 fall on weekends
        {1, Oct, 2035},
    };

    Calendar mexico = Mexico();
    for (auto holiday : expectedHol) {
        if (!mexico.isHoliday(holiday)) {
            BOOST_FAIL(
                "Expected to have an Inauguration Day holiday in the Mexican calendar for date "
                << holiday);
        }
    }
    for (auto workingDay : expectedWorkingDays) {
        if (!mexico.isBusinessDay(workingDay)) {
            BOOST_FAIL("Did not expect to have a holiday in the Mexican calendar for date "
                       << workingDay);
        }
    }
}


BOOST_AUTO_TEST_CASE(testNewZealand) {
    BOOST_TEST_MESSAGE("Testing a few holiday rules for New Zealand...");

    auto auckland = NewZealand(NewZealand::Auckland);
    auto wellington = NewZealand(NewZealand::Wellington);

    for (const auto& calendar : {auckland, wellington}) {
        // mid-week New Year's day
        BOOST_TEST(calendar.isHoliday({1, January, 2025}));
        BOOST_TEST(calendar.isHoliday({2, January, 2025}));
        BOOST_TEST(calendar.isBusinessDay({3, January, 2025}));
        // New Year's day on Sunday
        BOOST_TEST(calendar.isHoliday({1, January, 2023}));
        BOOST_TEST(calendar.isHoliday({2, January, 2023}));
        BOOST_TEST(calendar.isHoliday({3, January, 2023}));
        BOOST_TEST(calendar.isBusinessDay({4, January, 2023}));
        // New Year's day on Saturday
        BOOST_TEST(calendar.isHoliday({1, January, 2022}));
        BOOST_TEST(calendar.isHoliday({2, January, 2022}));
        BOOST_TEST(calendar.isHoliday({3, January, 2022}));
        BOOST_TEST(calendar.isHoliday({4, January, 2022}));
        BOOST_TEST(calendar.isBusinessDay({5, January, 2022}));
        // New Year's day on Friday
        BOOST_TEST(calendar.isHoliday({1, January, 2027}));
        BOOST_TEST(calendar.isHoliday({2, January, 2027}));
        BOOST_TEST(calendar.isHoliday({3, January, 2027}));
        BOOST_TEST(calendar.isHoliday({4, January, 2027}));
        BOOST_TEST(calendar.isBusinessDay({5, January, 2027}));

        // mid-week Christmas day
        BOOST_TEST(calendar.isHoliday({25, December, 2024}));
        BOOST_TEST(calendar.isHoliday({26, December, 2024}));
        BOOST_TEST(calendar.isBusinessDay({27, December, 2024}));
        // Christmas day on Sunday
        BOOST_TEST(calendar.isHoliday({25, December, 2022}));
        BOOST_TEST(calendar.isHoliday({26, December, 2022}));
        BOOST_TEST(calendar.isHoliday({27, December, 2022}));
        BOOST_TEST(calendar.isBusinessDay({28, December, 2022}));
        // Christmas day on Saturday
        BOOST_TEST(calendar.isHoliday({25, December, 2021}));
        BOOST_TEST(calendar.isHoliday({26, December, 2021}));
        BOOST_TEST(calendar.isHoliday({27, December, 2021}));
        BOOST_TEST(calendar.isHoliday({28, December, 2021}));
        BOOST_TEST(calendar.isBusinessDay({29, December, 2021}));
        // Christmas day on Friday
        BOOST_TEST(calendar.isHoliday({25, December, 2026}));
        BOOST_TEST(calendar.isHoliday({26, December, 2026}));
        BOOST_TEST(calendar.isHoliday({27, December, 2026}));
        BOOST_TEST(calendar.isHoliday({28, December, 2026}));
        BOOST_TEST(calendar.isBusinessDay({29, December, 2026}));

        // Waitangi Day is moved to Monday but only since 2013
        BOOST_TEST(calendar.isHoliday({8, February, 2021}));
        BOOST_TEST(calendar.isHoliday({7, February, 2022}));
        BOOST_TEST(calendar.isBusinessDay({8, February, 2010}));
        BOOST_TEST(calendar.isBusinessDay({7, February, 2011}));

        // The same goes for ANZAC Day
        BOOST_TEST(calendar.isHoliday({27, April, 2020}));
        BOOST_TEST(calendar.isHoliday({26, April, 2021}));
        BOOST_TEST(calendar.isBusinessDay({27, April, 2009}));
        BOOST_TEST(calendar.isBusinessDay({26, April, 2010}));
    }

    // different Anniversary Day for the two calendars
    BOOST_TEST(auckland.isBusinessDay({22, January, 2024}));
    BOOST_TEST(wellington.isHoliday({22, January, 2024}));
    BOOST_TEST(auckland.isHoliday({29, January, 2024}));
    BOOST_TEST(wellington.isBusinessDay({29, January, 2024}));
    BOOST_TEST(auckland.isBusinessDay({19, January, 2026}));
    BOOST_TEST(wellington.isHoliday({19, January, 2026}));
    BOOST_TEST(auckland.isHoliday({26, January, 2026}));
    BOOST_TEST(wellington.isBusinessDay({26, January, 2026}));
    BOOST_TEST(auckland.isBusinessDay({25, January, 2027}));
    BOOST_TEST(wellington.isHoliday({25, January, 2027}));
    BOOST_TEST(auckland.isHoliday({1, February, 2027}));
    BOOST_TEST(wellington.isBusinessDay({1, February, 2027}));
}


BOOST_AUTO_TEST_CASE(testTASECalendar) {
    BOOST_TEST_MESSAGE("Testing TASE calendar...");

    auto c = Israel(Israel::TASE);

    auto is_weekend = [](Date d) {
        if (d >= Date{5, January, 2026})
            return d.weekday() == Saturday || d.weekday() == Sunday;
        else
            return d.weekday() == Friday || d.weekday() == Saturday;
    };

    // using checkHolidays with includeWeekEnds = false doesn't work in this case;
    // weekdays changed in 2026.

    std::vector<Date> expected2013 = {
        {24, February, 2013},  {25, March, 2013},     {26, March, 2013},     {31, March, 2013},
        {1, April, 2013},      {15, April, 2013},     {16, April, 2013},     {14, May, 2013},
        {15, May, 2013},       {16, July, 2013},      {4, September, 2013},  {5, September, 2013},
        {18, September, 2013}, {19, September, 2013}, {25, September, 2013}, {26, September, 2013},
    };

    auto holidays = c.holidayList(Date(1, January, 2013), Date(31, December, 2013), true);
    holidays.erase(std::remove_if(holidays.begin(), holidays.end(), is_weekend), holidays.end());
    checkHolidays(holidays, expected2013);

    std::vector<Date> expected2025 = {
        {13, April, 2025},   {30, April, 2025},     {1, May, 2025},        {2, June, 2025},
        {3, August, 2025},   {23, September, 2025}, {24, September, 2025}, {1, October, 2025},
        {2, October, 2025},  {6, October, 2025},    {7, October, 2025},    {13, October, 2025},
        {14, October, 2025},
    };

    holidays = c.holidayList(Date(1, January, 2025), Date(31, December, 2025), true);
    holidays.erase(std::remove_if(holidays.begin(), holidays.end(), is_weekend), holidays.end());
    checkHolidays(holidays, expected2025);
}

BOOST_AUTO_TEST_CASE(testSHIRCalendar) {
    BOOST_TEST_MESSAGE("Testing SHIR calendar...");

    std::vector<Date> expected = {
        Date(5, May, 2022),        Date(3, June, 2022),       Date(26, September, 2022),
        Date(27, September, 2022), Date(4, October, 2022),    Date(5, October, 2022),
        Date(10, October, 2022),   Date(17, October, 2022),   Date(1, November, 2022),
        Date(26, December, 2022),  Date(2, January, 2023),    Date(7, March, 2023),
        Date(8, March, 2023),      Date(5, April, 2023),      Date(6, April, 2023),
        Date(7, April, 2023),      Date(10, April, 2023),     Date(12, April, 2023),
        Date(26, April, 2023),     Date(26, May, 2023),       Date(29, May, 2023),
        Date(27, July, 2023),      Date(15, September, 2023), Date(25, September, 2023),
        Date(25, December, 2023),  Date(26, December, 2023),  Date(1, January, 2024),
        Date(27, February, 2024),  Date(25, March, 2024),     Date(29, March, 2024),
        Date(22, April, 2024),     Date(23, April, 2024),     Date(29, April, 2024),
        Date(14, May, 2024),       Date(27, May, 2024),       Date(12, June, 2024),
        Date(13, August, 2024),    Date(2, October, 2024),    Date(3, October, 2024),
        Date(4, October, 2024),    Date(11, October, 2024),   Date(17, October, 2024),
        Date(24, October, 2024),   Date(25, December, 2024),  Date(26, December, 2024),
        Date(1, January, 2025),    Date(14, March, 2025),     Date(18, April, 2025),
        Date(1, May, 2025),        Date(26, May, 2025),       Date(2, June, 2025),
        Date(22, September, 2025), Date(23, September, 2025), Date(24, September, 2025),
        Date(1, October, 2025),    Date(2, October, 2025),    Date(7, October, 2025),
        Date(14, October, 2025),   Date(25, December, 2025),  Date(26, December, 2025),
    };

    auto c = Israel(Israel::SHIR);
    checkHolidays(c.holidayList(Date(5, May, 2022), Date(31, December, 2025)), expected);
}

BOOST_AUTO_TEST_CASE(testTelborCalendar) {
    BOOST_TEST_MESSAGE("Testing Telbor calendar...");

    std::vector<Date> expected = {
        {5, May, 2022},        {30, May, 2022},       {26, September, 2022}, {27, September, 2022},
        {5, October, 2022},    {10, October, 2022},   {17, October, 2022},   {26, December, 2022},
        {7, March, 2023},      {8, March, 2023},      {5, April, 2023},      {6, April, 2023},
        {12, April, 2023},     {26, April, 2023},     {26, May, 2023},       {29, May, 2023},
        {27, July, 2023},      {25, September, 2023}, {25, December, 2023},  {26, December, 2023},
        {1, January, 2024},    {25, March, 2024},     {22, April, 2024},     {23, April, 2024},
        {29, April, 2024},     {14, May, 2024},       {27, May, 2024},       {12, June, 2024},
        {13, August, 2024},    {3, October, 2024},    {4, October, 2024},    {17, October, 2024},
        {24, October, 2024},   {25, December, 2024},  {26, December, 2024},  {1, January, 2025},
        {14, March, 2025},     {1, May, 2025},        {26, May, 2025},       {2, June, 2025},
        {23, September, 2025}, {24, September, 2025}, {2, October, 2025},    {7, October, 2025},
        {14, October, 2025},   {25, December, 2025},  {26, December, 2025},
    };

    auto c = Israel(Israel::Telbor);
    checkHolidays(c.holidayList(Date(5, May, 2022), Date(31, December, 2025)), expected);
}

BOOST_AUTO_TEST_CASE(testStartOfMonth) {
    BOOST_TEST_MESSAGE("Testing start-of-month calculation...");

    Calendar c = TARGET(); // any calendar would be OK

    Date som, counter = Date::minDate() + 2 * Months;
    Date last = Date::maxDate();

    while (counter < last) {
        som = c.startOfMonth(counter);
        // check that som is som
        if (!c.isStartOfMonth(som))
            BOOST_FAIL("\n  " << som.weekday() << " " << som << " is not the first business day in "
                              << som.month() << " " << som.year() << " according to " << c.name());
        // check that som is in the same month as counter
        if (som.month() != counter.month())
            BOOST_FAIL("\n  " << som << " is not in the same month as " << counter);
        // Check that previous business day is in a different month
        if (c.advance(som, -1, Days, Unadjusted).month() == som.month())
            BOOST_FAIL("\n  " << c.advance(som, -1, Days, Unadjusted) << " is in the same month as "
                              << som);
        counter = counter + 1;
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
        // Check that next business day is in a different month
        if (c.advance(eom, 1, Days, Unadjusted).month() == eom.month())
            BOOST_FAIL("\n  " << c.advance(eom, 1, Days, Unadjusted) << " is in the same month as "
                              << eom);
        counter = counter + 1;
    }
}

BOOST_AUTO_TEST_CASE(testBusinessDaysBetween) {

    BOOST_TEST_MESSAGE("Testing calculation of business days between dates...");

    std::vector<Date> testDates = {
        {1, February, 2002},  // isBusinessDay = true
        {4, February, 2002},  // isBusinessDay = true
        {16, May, 2003},      // isBusinessDay = true
        {17, December, 2003}, // isBusinessDay = true
        {17, December, 2004}, // isBusinessDay = true
        {19, December, 2005}, // isBusinessDay = true
        {2, January, 2006},   // isBusinessDay = true
        {13, March, 2006},    // isBusinessDay = true
        {15, May, 2006},      // isBusinessDay = true
        {17, March, 2006},    // isBusinessDay = true
        {15, May, 2006},      // isBusinessDay = true
        {26, July, 2006},     // isBusinessDay = true
        {26, July, 2006},     // isBusinessDay = true
        {27, July, 2006},     // isBusinessDay = true
        {29, July, 2006},     // isBusinessDay = false
        {29, July, 2006},     // isBusinessDay = false
    };

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

    // Test that a crossed range returns an empty vector
    BOOST_CHECK_EQUAL(germany.holidayList(endDate, firstDate, true).size(), 0);
    BOOST_CHECK_EQUAL(germany.businessDayList(endDate, firstDate).size(), 0);
    // Test that the range is inclusive on both sides
    BOOST_CHECK_EQUAL(germany.holidayList(firstDate, firstDate, true).size(),
                      static_cast<size_t>(germany.isHoliday(firstDate)));
    BOOST_CHECK_EQUAL(germany.businessDayList(firstDate, firstDate).size(),
                      static_cast<size_t>(germany.isBusinessDay(firstDate)));

    std::vector<Date> holidays = germany.holidayList(firstDate, endDate, true);
    std::vector<Date> businessDays = germany.businessDayList(firstDate, endDate);

    auto it_holidays = holidays.begin(), it_businessDays = businessDays.begin();
    for (Date d = firstDate; d <= endDate; d++) {
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

BOOST_AUTO_TEST_CASE(testMalta) {

    BOOST_TEST_MESSAGE("Testing Malta holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2026},
        {10, February, 2026},
        {19, March, 2026},
        {31, March, 2026},
        {6, April, 2026},
        {29, June, 2026},
        {8, September, 2026},
        {21, September, 2026},
        {8, December, 2026},
        {13, December, 2026},
        {24, December, 2026},
        {31, December, 2026},

        {10, February, 2027},
        {29, March, 2027},
        {31, March, 2027},
        {29, June, 2027},
        {15, August, 2027},
        {8, September, 2027},
        {21, September, 2027},
        {8, December, 2027},
        {13, December, 2027},
        {26, December, 2027},
    };

    Calendar c = Malta();
    checkHolidays(c.holidayList(Date(1, January, 2026), Date(31, December, 2027)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testMontenegro) {

    BOOST_TEST_MESSAGE("Testing Montenegro holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2026},
        {2, January, 2026},
        {1, May, 2026},
        {21, May, 2026},
        {22, May, 2026},
        {13, July, 2026},
        {14, July, 2026},
        {13, November, 2026},

        {1, January, 2027},
        {21, May, 2027},
        {13, July, 2027},
        {14, July, 2027},
    };

    Calendar c = Montenegro();
    checkHolidays(c.holidayList(Date(1, January, 2026), Date(31, December, 2027)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testNorthMacedonia) {

    BOOST_TEST_MESSAGE("Testing North Macedonia holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2026},
        {7, January, 2026},
        {20, March, 2026},
        {13, April, 2026},
        {1, May, 2026},
        {27, May, 2026},
        {8, September, 2026},
        {23, October, 2026},
        {8, December, 2026},

        {1, January, 2027},
        {7, January, 2027},
        {10, March, 2027},
        {3, May, 2027},
        {17, May, 2027},
        {24, May, 2027},
        {2, August, 2027},
        {8, September, 2027},
        {11, October, 2027},
        {8, December, 2027},
    };

    Calendar c = NorthMacedonia();
    checkHolidays(c.holidayList(Date(1, January, 2026), Date(31, December, 2027)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testNorthMacedoniaRamazanBajram) {

    BOOST_TEST_MESSAGE("Testing North Macedonia Ramazan Bajram (Eid al-Fitr) dates...");

    Calendar c = NorthMacedonia();

    BOOST_CHECK(c.isHoliday(Date(20, March, 2026)));
    BOOST_CHECK(c.isHoliday(Date(10, March, 2027)));
    BOOST_CHECK(c.isHoliday(Date(27, February, 2028)));
    BOOST_CHECK(c.isHoliday(Date(15, February, 2029)));
    BOOST_CHECK(c.isHoliday(Date(5, February, 2030)));
    BOOST_CHECK(c.isHoliday(Date(25, January, 2031)));
    BOOST_CHECK(c.isHoliday(Date(14, January, 2032)));
    BOOST_CHECK(c.isHoliday(Date(2, January, 2033)));
    BOOST_CHECK(c.isHoliday(Date(23, December, 2033)));
    BOOST_CHECK(c.isHoliday(Date(12, December, 2034)));
    BOOST_CHECK(c.isHoliday(Date(1, December, 2035)));
    BOOST_CHECK(c.isHoliday(Date(19, November, 2036)));
}

BOOST_AUTO_TEST_CASE(testNorthMacedoniaKurbanBajram) {

    BOOST_TEST_MESSAGE("Testing North Macedonia Kurban Bajram (Eid al-Adha) dates...");

    Calendar c = NorthMacedonia();

    BOOST_CHECK(c.isHoliday(Date(27, May, 2026)));
    BOOST_CHECK(c.isHoliday(Date(17, May, 2027)));
    BOOST_CHECK(c.isHoliday(Date(5, May, 2028)));
    BOOST_CHECK(c.isHoliday(Date(24, April, 2029)));
    BOOST_CHECK(c.isHoliday(Date(13, April, 2030)));
    BOOST_CHECK(c.isHoliday(Date(3, April, 2031)));
    BOOST_CHECK(c.isHoliday(Date(22, March, 2032)));
    BOOST_CHECK(c.isHoliday(Date(11, March, 2033)));
    BOOST_CHECK(c.isHoliday(Date(28, February, 2034)));
    BOOST_CHECK(c.isHoliday(Date(18, February, 2035)));
    BOOST_CHECK(c.isHoliday(Date(7, February, 2036)));
    BOOST_CHECK(c.isHoliday(Date(27, January, 2037)));
    BOOST_CHECK(c.isHoliday(Date(16, January, 2038)));
    BOOST_CHECK(c.isHoliday(Date(5, January, 2039)));
    BOOST_CHECK(c.isHoliday(Date(26, December, 2039)));
    BOOST_CHECK(c.isHoliday(Date(15, December, 2040)));
}

BOOST_AUTO_TEST_CASE(testSerbia) {

    BOOST_TEST_MESSAGE("Testing Serbia holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2026},
        {2, January, 2026},
        {7, January, 2026},
        {16, February, 2026},
        {3, April, 2026},
        {6, April, 2026},
        {1, May, 2026},
        {11, November, 2026},
        {31, December, 2026},

        {1, January, 2027},
        {7, January, 2027},
        {15, February, 2027},
        {16, February, 2027},
        {26, March, 2027},
        {29, March, 2027},
        {11, November, 2027},
        {31, December, 2027},
    };

    Calendar c = Serbia();
    checkHolidays(c.holidayList(Date(1, January, 2026), Date(31, December, 2027)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testSaudiArabiaFoundingDayAndEidAlAdha) {
    BOOST_TEST_MESSAGE("Testing Saudi Arabia Founding Day and Eid Al-Adha holidays...");

    Calendar saudi = SaudiArabia();

    // Founding Day observed on Sunday 23 Feb 2025 (22 Feb is Saturday)
    BOOST_CHECK(saudi.isHoliday(Date(23, February, 2025)));

    // Founding Day on Sunday 22 Feb 2026
    BOOST_CHECK(saudi.isHoliday(Date(22, February, 2026)));
    BOOST_CHECK(saudi.isBusinessDay(Date(19, February, 2026)));

    // Eid Al-Adha 2025 (Saudi Exchange: 5-10 Jun, resume 11 Jun)
    BOOST_CHECK(saudi.isHoliday(Date(5, June, 2025)));
    BOOST_CHECK(saudi.isHoliday(Date(10, June, 2025)));
    BOOST_CHECK(saudi.isBusinessDay(Date(4, June, 2025)));
    BOOST_CHECK(saudi.isBusinessDay(Date(11, June, 2025)));

    // Eid Al-Adha 2026 (Saudi Exchange: 22-30 May, resume 31 May)
    BOOST_CHECK(saudi.isHoliday(Date(25, May, 2026)));
    BOOST_CHECK(saudi.isBusinessDay(Date(21, May, 2026)));
    BOOST_CHECK(saudi.isBusinessDay(Date(31, May, 2026)));
}

BOOST_AUTO_TEST_CASE(testSlovenia) {

    BOOST_TEST_MESSAGE("Testing Slovenia holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2026},
        {2, January, 2026},
        {3, April, 2026},
        {6, April, 2026},
        {1, May, 2026},
        {25, June, 2026},
        {24, December, 2026},
        {25, December, 2026},
        {31, December, 2026},

        {1, January, 2027},
        {26, March, 2027},
        {29, March, 2027},
        {25, June, 2027},
        {24, December, 2027},
        {31, December, 2027},
    };

    Calendar c = Slovenia();
    checkHolidays(c.holidayList(Date(1, January, 2026), Date(31, December, 2027)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testCroatia) {

    BOOST_TEST_MESSAGE("Testing Croatia holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2026},
        {6, January, 2026},
        {3, April, 2026},
        {6, April, 2026},
        {1, May, 2026},
        {4, June, 2026},
        {22, June, 2026},
        {5, August, 2026},
        {18, November, 2026},
        {24, December, 2026},
        {25, December, 2026},
        {31, December, 2026},

        {1, January, 2027},
        {6, January, 2027},
        {26, March, 2027},
        {29, March, 2027},
        {27, May, 2027},
        {22, June, 2027},
        {5, August, 2027},
        {18, November, 2027},
        {24, December, 2027},
        {31, December, 2027},
    };

    Calendar c = Croatia();
    checkHolidays(c.holidayList(Date(1, January, 2026), Date(31, December, 2027)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testMalaysia) {

    BOOST_TEST_MESSAGE("Testing Malaysia holiday list...");

    std::vector<Date> expectedHol = {
        // 2024
        {1, January, 2024},    // New Year's Day
        {25, January, 2024},   // Thaipusam
        {1, February, 2024},   // Federal Territory Day
        {12, February, 2024},  // Chinese New Year (substitute)
        {28, March, 2024},     // Nuzul Al-Quran
        {10, April, 2024},     // Hari Raya Aidilfitri
        {11, April, 2024},     // Hari Raya Aidilfitri
        {1, May, 2024},        // Labour Day
        {22, May, 2024},       // Wesak Day
        {3, June, 2024},       // Agong's Birthday
        {17, June, 2024},      // Hari Raya Haji
        {8, July, 2024},       // Awal Muharram (substitute)
        {16, September, 2024}, // Malaysia Day / Maulidur Rasul
        {31, October, 2024},   // Deepavali
        {25, December, 2024},  // Christmas Day

        // 2025
        {1, January, 2025},    // New Year's Day
        {29, January, 2025},   // Chinese New Year
        {30, January, 2025},   // Chinese New Year
        {11, February, 2025},  // Thaipusam
        {18, March, 2025},     // Nuzul Al-Quran
        {31, March, 2025},     // Hari Raya Aidilfitri
        {1, April, 2025},      // Hari Raya Aidilfitri
        {1, May, 2025},        // Labour Day
        {12, May, 2025},       // Wesak Day
        {2, June, 2025},       // Agong's Birthday
        {27, June, 2025},      // Awal Muharram
        {1, September, 2025},  // National Day (substitute)
        {5, September, 2025},  // Maulidur Rasul
        {15, September, 2025}, // Malaysia Day (additional)
        {16, September, 2025}, // Malaysia Day
        {20, October, 2025},   // Deepavali
        {25, December, 2025},  // Christmas Day
    };

    Calendar c = Malaysia();
    checkHolidays(c.holidayList(Date(1, January, 2024), Date(31, December, 2025)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testMalaysiaVariableHolidays) {

    BOOST_TEST_MESSAGE("Testing Malaysia variable holidays across the tabulated range...");

    Calendar c = Malaysia();

    // One festival per tabulated year, so that a date added out of order in the
    // holiday table is caught: the table is looked up with a binary search, and
    // an unsorted entry makes its neighbours unfindable.
    BOOST_CHECK(c.isHoliday(Date(3, February, 2015))); // Thaipusam
    BOOST_CHECK(c.isHoliday(Date(24, January, 2016)));
    BOOST_CHECK(c.isHoliday(Date(9, February, 2017)));
    BOOST_CHECK(c.isHoliday(Date(31, January, 2018)));
    BOOST_CHECK(c.isHoliday(Date(21, January, 2019)));
    BOOST_CHECK(c.isHoliday(Date(9, February, 2020)));
    BOOST_CHECK(c.isHoliday(Date(28, January, 2021)));
    BOOST_CHECK(c.isHoliday(Date(18, January, 2022)));
    BOOST_CHECK(c.isHoliday(Date(5, February, 2023)));
    BOOST_CHECK(c.isHoliday(Date(25, January, 2024)));
    BOOST_CHECK(c.isHoliday(Date(11, February, 2025)));
    BOOST_CHECK(c.isHoliday(Date(22, January, 2027)));

    BOOST_CHECK(c.isHoliday(Date(19, February, 2015))); // Chinese New Year
    BOOST_CHECK(c.isHoliday(Date(8, February, 2016)));
    BOOST_CHECK(c.isHoliday(Date(28, January, 2017)));
    BOOST_CHECK(c.isHoliday(Date(16, February, 2018)));
    BOOST_CHECK(c.isHoliday(Date(5, February, 2019)));
    BOOST_CHECK(c.isHoliday(Date(25, January, 2020)));
    BOOST_CHECK(c.isHoliday(Date(12, February, 2021)));
    BOOST_CHECK(c.isHoliday(Date(1, February, 2022)));
    BOOST_CHECK(c.isHoliday(Date(22, January, 2023)));
    BOOST_CHECK(c.isHoliday(Date(10, February, 2024)));
    BOOST_CHECK(c.isHoliday(Date(29, January, 2025)));
    BOOST_CHECK(c.isHoliday(Date(17, February, 2026)));
    BOOST_CHECK(c.isHoliday(Date(6, February, 2027)));
}

BOOST_AUTO_TEST_CASE(testMalaysiaFixedHolidaysOutsideTabulatedRange) {

    BOOST_TEST_MESSAGE("Testing Malaysia fixed holidays outside the tabulated range...");

    Calendar c = Malaysia();

    // The variable festivals are tabulated only for 2015-2027, but the
    // fixed-date federal holidays are rule-based and must hold in any year.
    BOOST_CHECK(c.isHoliday(Date(1, January, 2035)));   // New Year's Day
    BOOST_CHECK(c.isHoliday(Date(1, February, 2035)));  // Federal Territory Day
    BOOST_CHECK(c.isHoliday(Date(1, May, 2035)));       // Labour Day
    BOOST_CHECK(c.isHoliday(Date(31, August, 2035)));   // National Day
    BOOST_CHECK(c.isHoliday(Date(25, December, 2035))); // Christmas Day

    // Malaysia Day only became a federal holiday in 2010.
    BOOST_CHECK(c.isHoliday(Date(16, September, 2035)));
    BOOST_CHECK(c.isBusinessDay(Date(16, September, 2009)));
}

BOOST_AUTO_TEST_CASE(testUzbekistan) {

    BOOST_TEST_MESSAGE("Testing Uzbekistan holiday list...");

    std::vector<Date> expectedHol = {
        {1, January, 2026},
        {20, March, 2026},
        {27, May, 2026},
        {1, September, 2026},
        {1, October, 2026},
        {8, December, 2026},
        {1, January, 2027},
        {8, March, 2027},
        {10, March, 2027},
        {17, May, 2027},
        {1, September, 2027},
        {1, October, 2027},
        {8, December, 2027},
    };

    Calendar c = Uzbekistan();
    checkHolidays(c.holidayList(Date(1, January, 2026), Date(31, December, 2027)), expectedHol);
}

BOOST_AUTO_TEST_CASE(testUzbekistanRamazonHayit) {

    BOOST_TEST_MESSAGE("Testing Uzbekistan Ramazon Hayit (Eid al-Fitr) dates...");

    Calendar c = Uzbekistan();

    BOOST_CHECK(c.isHoliday(Date(20, March, 2026)));
    BOOST_CHECK(c.isHoliday(Date(10, March, 2027)));
    BOOST_CHECK(c.isHoliday(Date(27, February, 2028)));
    BOOST_CHECK(c.isHoliday(Date(15, February, 2029)));
    BOOST_CHECK(c.isHoliday(Date(5, February, 2030)));
    BOOST_CHECK(c.isHoliday(Date(25, January, 2031)));
    BOOST_CHECK(c.isHoliday(Date(14, January, 2032)));
    BOOST_CHECK(c.isHoliday(Date(2, January, 2033)));
    BOOST_CHECK(c.isHoliday(Date(23, December, 2033)));
    BOOST_CHECK(c.isHoliday(Date(12, December, 2034)));
    BOOST_CHECK(c.isHoliday(Date(1, December, 2035)));
    BOOST_CHECK(c.isHoliday(Date(19, November, 2036)));
}

BOOST_AUTO_TEST_CASE(testUzbekistanQurbonHayit) {

    BOOST_TEST_MESSAGE("Testing Uzbekistan Qurbon Hayit (Eid al-Adha) dates...");

    Calendar c = Uzbekistan();

    BOOST_CHECK(c.isHoliday(Date(27, May, 2026)));
    BOOST_CHECK(c.isHoliday(Date(17, May, 2027)));
    BOOST_CHECK(c.isHoliday(Date(5, May, 2028)));
    BOOST_CHECK(c.isHoliday(Date(24, April, 2029)));
    BOOST_CHECK(c.isHoliday(Date(13, April, 2030)));
    BOOST_CHECK(c.isHoliday(Date(3, April, 2031)));
    BOOST_CHECK(c.isHoliday(Date(22, March, 2032)));
    BOOST_CHECK(c.isHoliday(Date(11, March, 2033)));
    BOOST_CHECK(c.isHoliday(Date(28, February, 2034)));
    BOOST_CHECK(c.isHoliday(Date(18, February, 2035)));
    BOOST_CHECK(c.isHoliday(Date(7, February, 2036)));
    BOOST_CHECK(c.isHoliday(Date(27, January, 2037)));
    BOOST_CHECK(c.isHoliday(Date(16, January, 2038)));
    BOOST_CHECK(c.isHoliday(Date(5, January, 2039)));
    BOOST_CHECK(c.isHoliday(Date(26, December, 2039)));
    BOOST_CHECK(c.isHoliday(Date(15, December, 2040)));
}

BOOST_AUTO_TEST_CASE(testIndia) {

    BOOST_TEST_MESSAGE("Testing India (NSE) clearing holiday list...");

    std::vector<Date> expectedHol = {
        // 2026
        // Municipal Corporation Election - Maharashtra
        Date(15, January, 2026),
        // Republic Day
        Date(26, January, 2026),
        // Chatrapati Shivaji Jayanti
        Date(19, February, 2026),
        // Holi
        Date(3, March, 2026),
        // Gudi Padwa
        Date(19, March, 2026),
        // Ram Navami
        Date(26, March, 2026),
        // Mahavir Jayanti
        Date(31, March, 2026),
        // Annual Bank Closing
        Date(1, April, 2026),
        // Good Friday
        Date(3, April, 2026),
        // Ambedkar Jayanti
        Date(14, April, 2026),
        // May Day
        Date(1, May, 2026),
        // Bakri Id
        Date(28, May, 2026),
        // Muharram
        Date(26, June, 2026),
        // Id-E-Milad
        Date(26, August, 2026),
        // Ganesh Chaturthi
        Date(14, September, 2026),
        // Gandhi Jayanti
        Date(2, October, 2026),
        // Dussehra
        Date(20, October, 2026),
        // Diwali - Balipratipada
        Date(10, November, 2026),
        // Gurunank Jayanti
        Date(24, November, 2026),
        // Christmas
        Date(25, December, 2026),
    };

    // The following 2026 holidays fall on weekends and are
    // therefore not included above:
    // Mahashivratri:      15-Feb-2026 (Sunday)
    // Id-Ul-Fitr:         21-Mar-2026 (Saturday)
    // Independence Day:   15-Aug-2026 (Saturday)
    // Diwali Laxmi Pujan: 08-Nov-2026 (Sunday)

    Calendar c = India();
    checkHolidays(c.holidayList(Date(1, January, 2026), Date(31, December, 2026)), expectedHol);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
