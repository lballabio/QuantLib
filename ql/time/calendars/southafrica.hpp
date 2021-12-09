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

/*! \file southafrica.hpp
    \brief South-African calendar
*/

#ifndef quantlib_south_african_calendar_hpp
#define quantlib_south_african_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! South-African calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday)</li>
        <li>Good Friday</li>
        <li>Family Day, Easter Monday</li>
        <li>Human Rights Day, March 21st (possibly moved to Monday)</li>
        <li>Freedom Day, April 27th (possibly moved to Monday)</li>
        <li>Workers Day, May 1st (possibly moved to Monday)</li>
        <li>Youth Day, June 16th (possibly moved to Monday)</li>
        <li>National Women's Day, August 9th
        (possibly moved to Monday)</li>
        <li>Heritage Day, September 24th (possibly moved to Monday)</li>
        <li>Day of Reconciliation, December 16th
        (possibly moved to Monday)</li>
        <li>Christmas, December 25th </li>
        <li>Day of Goodwill, December 26th (possibly moved to Monday)</li>
        <li>Election Days</li>
        </ul>

        \ingroup calendars
    */
    class SouthAfrica : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "South Africa"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        SouthAfrica();
    };

}


#endif
