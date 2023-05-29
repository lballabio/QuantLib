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

#ifndef quantlib_test_schedule_hpp
#define quantlib_test_schedule_hpp

#include <boost/test/unit_test.hpp>

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

class ScheduleTest {
  public:
    static void testDailySchedule();
    static void testEndDateWithEomAdjustment();
    static void testDatesPastEndDateWithEomAdjustment();
    static void testDatesSameAsEndDateWithEomAdjustment();
    static void testForwardDatesWithEomAdjustment();
    static void testBackwardDatesWithEomAdjustment();
    static void testDoubleFirstDateWithEomAdjustment();
    static void testFirstDateWithEomAdjustment();
    static void testNextToLastWithEomAdjustment();
    static void testEffectiveDateWithEomAdjustment();
    static void testCDS2015Convention();
    static void testCDS2015ConventionGrid();
    static void testCDSConventionGrid();
    static void testOldCDSConventionGrid();
    static void testCDS2015ConventionSampleDates();
    static void testCDSConventionSampleDates();
    static void testOldCDSConventionSampleDates();
    static void testCDS2015ZeroMonthsMatured();
    static void testDateConstructor();
    static void testFourWeeksTenor();
    static void testScheduleAlwaysHasAStartDate();
    static void testShortEomSchedule();
    static void testFirstDateOnMaturity();
    static void testNextToLastDateOnStart();
    static void testTruncation();
    static boost::unit_test_framework::test_suite* suite();
};


#endif
