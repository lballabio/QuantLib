/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file switzerland.hpp
    \brief Swiss calendar
*/

#ifndef quantlib_swiss_calendar_hpp
#define quantlib_swiss_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Swiss calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Berchtoldstag, January 2nd</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Ascension Day</li>
        <li>Whit Monday</li>
        <li>Labour Day, May 1st</li>
        <li>National Day, August 1st</li>
        <li>Christmas, December 25th</li>
        <li>St. Stephen's Day, December 26th</li>
        </ul>

        \ingroup calendars
    */
    class Switzerland : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Switzerland"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        Switzerland();
    };

}


#endif


#ifndef id_c51f45ab1a47ad94e021a49ebd9c8d65
#define id_c51f45ab1a47ad94e021a49ebd9c8d65
inline bool test_c51f45ab1a47ad94e021a49ebd9c8d65(int* i) { return i != 0; }
#endif
