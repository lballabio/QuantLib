
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file newyork.hpp
    \brief New York calendar
*/

#ifndef quantlib_newyork_calendar_h
#define quantlib_newyork_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    //! New York calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday if
            actually on Sunday, or to Friday if on Saturday)</li>
        <li>Martin Luther King's birthday, third Monday in January</li>
        <li>Washington's birthday, third Monday in February</li>
        <li>Good Friday</li>
        <li>Memorial Day, last Monday in May</li>
        <li>Independence Day, July 4th (moved to Monday if Sunday or
            Friday if Saturday)</li>
        <li>Labor Day, first Monday in September</li>
        <li>Thanksgiving Day, fourth Thursday in November</li>
        <li>Christmas, December 25th (moved to Monday if Sunday or Friday
            if Saturday)</li>
        </ul>
        Data from http://www.nyse.com
    */
    class NewYork : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "NewYork"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        NewYork()
        : Calendar(Handle<Calendar::Impl>(new NewYork::Impl)) {}
    };

}


#endif
