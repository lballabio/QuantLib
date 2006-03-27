
/*
 Copyright (C) 2005 Sercan Atalik

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

/*! \file istanbul.hpp
    \brief Turkish calendar
*/

#ifndef quantlib_turkish_calendar_hpp
#define quantlib_turkish_calendar_hpp

#include <ql/calendar.hpp>

namespace QuantLib {

    //! Turkish calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>National Holidays (April 23rd, May 19th, August 30th,
            October 29th</li>
        <li>Local Holidays (Kurban, Ramadan; 2004 to 2009 only) </li>
        </ul>

        \ingroup calendars
    */
    class Turkey : public Calendar {
      private:
        class Impl : public Calendar::Impl {
          public:
            std::string name() const { return "Turkey"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        Turkey();
    };

}


#endif
