
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

#ifndef quantlib_test_calendars_hpp
#define quantlib_test_calendars_hpp

#include <boost/test/unit_test.hpp>

/*! \class QuantLib::Calendar <ql/calendar.hpp>

    \test the methods for adding and removing holidays are tested by
          inspecting the calendar before and after their invocation.
*/

/*! \class QuantLib::JointCalendar <ql/Calendars/jointcalendar.hpp>

    \test the correctness of the returned results is tested by
          reproducing the calculations.
*/

class CalendarTest {
  public:
    static void testModifiedCalendars();
    static void testJointCalendars();
    static boost::unit_test_framework::test_suite* suite();
};


#endif
