
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

/*! \file milan.hpp
    \brief Milan calendar
*/

#ifndef quantlib_milan_calendar_h
#define quantlib_milan_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    //! %Milan calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Epiphany, January 6th</li>
        <li>Easter Monday</li>
        <li>Liberation Day, April 25th</li>
        <li>Labour Day, May 1st</li>
        <li>Republic Day, June 2nd (since 2000)</li>
        <li>Assumption, August 15th</li>
        <li>All Saint's Day, November 1st</li>
        <li>Immaculate Conception, December 8th</li>
        <li>Christmas, December 25th</li>
        <li>St. Stephen, December 26th</li>
        </ul>

        \ingroup calendars
    */
    class Milan : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Milan"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        Milan()
        : Calendar(boost::shared_ptr<Calendar::Impl>(new Milan::Impl)) {}
    };

}


#endif
