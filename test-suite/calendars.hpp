/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2008 StatPro Italia srl
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

#ifndef quantlib_test_calendars_hpp
#define quantlib_test_calendars_hpp

#include <boost/test/unit_test.hpp>

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

class CalendarTest {
  public:
    static void testRussia();
    static void testBrazil();
//    static void testItalySettlement();
    static void testItalyExchange();

    static void testUKSettlement();
    static void testUKExchange();
    static void testUKMetals();

//    static void testGermanySettlement();
    static void testGermanyFrankfurt();
    static void testGermanyXetra();
    static void testGermanyEurex();

    static void testTARGET();

    static void testUSSettlement();
    static void testUSGovernmentBondMarket();
    static void testUSNewYorkStockExchange();

    static void testSouthKoreanSettlement();
    static void testKoreaStockExchange();

    static void testChinaSSE();
    static void testChinaIB();

    static void testModifiedCalendars();
    static void testJointCalendars();
    static void testBespokeCalendars();

    static void testEndOfMonth();
    static void testBusinessDaysBetween();

    static void testIntradayAddHolidays();
    static void testDayLists();

    static boost::unit_test_framework::test_suite* suite();
};


#endif
