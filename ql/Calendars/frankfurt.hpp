
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

/*! \file frankfurt.hpp
    \brief Frankfurt calendar
*/

#ifndef quantlib_frankfurt_calendar_h
#define quantlib_frankfurt_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    //! %Frankfurt calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Ascension Thursday</li>
        <li>Whit Monday</li>
        <li>Corpus Christi</li>
        <li>Labour Day, May 1st</li>
        <li>National Day, October 3rd</li>
        <li>Christmas Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>Boxing Day, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>
    */
    class Frankfurt : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Frankfurt"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        Frankfurt()
        : Calendar(boost::shared_ptr<Calendar::Impl>(new Frankfurt::Impl)) {}
    };

}


#endif
