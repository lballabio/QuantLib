/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl
 Copyright (C) 2024 Skandinaviska Enskilda Banken AB (publ)

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file mexico.hpp
    \brief Mexican calendars
*/

#ifndef quantlib_mexico_calendar_hpp
#define quantlib_mexico_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! %Mexican calendars
    /*! Holidays for the Mexican stock exchange
        (data from <http://www.bmv.com.mx/>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Constitution Day, first Monday in February
            (February 5th before 2006)</li>
        <li>Birthday of Benito Juarez, third Monday in February
            (March 21st before 2006)</li>
        <li>Holy Thursday</li>
        <li>Good Friday</li>
        <li>Labour Day, May 1st</li>
        <li>National Day, September 16th</li>
        <li>Inauguration Day, October 1st, every sixth year starting 2024</li>
        <li>All Souls Day, November 2nd (bank holiday, not a public one)</li>
        <li>Revolution Day, third Monday in November
            (November 20th before 2006)</li>
        <li>Our Lady of Guadalupe, December 12th</li>
        <li>Christmas, December 25th</li>
        </ul>

        \ingroup calendars
    */
    class Mexico : public Calendar {
      private:
        class BmvImpl final : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Mexican stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        enum Market { BMV    //!< Mexican stock exchange
        };
        Mexico(Market m = BMV);
    };

}


#endif
