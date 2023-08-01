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

/*! \file hungary.hpp
    \brief Hungarian calendar
*/

#ifndef quantlib_hungarian_calendar_hpp
#define quantlib_hungarian_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Hungarian calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>Good Friday (since 2017)</li>
        <li>Easter Monday</li>
        <li>Whit(Pentecost) Monday </li>
        <li>New Year's Day, January 1st</li>
        <li>National Day, March 15th</li>
        <li>Labour Day, May 1st</li>
        <li>Constitution Day, August 20th</li>
        <li>Republic Day, October 23rd</li>
        <li>All Saints Day, November 1st</li>
        <li>Christmas, December 25th</li>
        <li>2nd Day of Christmas, December 26th</li>
        </ul>

        \ingroup calendars
    */
    class Hungary : public Calendar {
      private:
        class Impl final : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Hungary"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        Hungary();
    };

}


#endif
