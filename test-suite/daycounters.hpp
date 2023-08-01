/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl

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

#ifndef quantlib_test_day_counters_hpp
#define quantlib_test_day_counters_hpp

#include <boost/test/unit_test.hpp>

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

class DayCounterTest {
   public:
    static void testActualActual();
    static void testActualActualIsma();
    static void testActualActualWithSchedule();
    static void testActualActualWithAnnualSchedule();
    static void testActualActualWithSemiannualSchedule();
    static void testSimple();
    static void testOne();
    static void testBusiness252();
    static void testThirty365();
    static void testThirty360_BondBasis();
    static void testThirty360_EurobondBasis();
    static void testThirty360_ISDA();
    static void testActual365_Canadian();
    static void testIntraday();
    static void testActualActualOutOfScheduleRange();
    static void testAct366();
    static void testAct36525();
    static void testActualConsistency();
    static void testYearFraction2DateBulk();
    static void testYearFraction2DateRounding();
    static boost::unit_test_framework::test_suite* suite();
};


#endif
