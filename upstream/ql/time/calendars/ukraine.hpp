/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

/*! \file ukraine.hpp
    \brief Ukrainian calendars
*/

#ifndef quantlib_ukrainian_calendar_hpp
#define quantlib_ukrainian_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Ukrainian calendars
    /*! Holidays for the Ukrainian stock exchange
        (data from <http://www.ukrse.kiev.ua/eng/>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Orthodox Christmas, January 7th</li>
        <li>International Women's Day, March 8th</li>
        <li>Easter Monday</li>
        <li>Holy Trinity Day, 50 days after Easter</li>
        <li>International Workers' Solidarity Days, May 1st and 2nd</li>
        <li>Victory Day, May 9th</li>
        <li>Constitution Day, June 28th</li>
        <li>Independence Day, August 24th</li>
        <li>Defender's Day, October 14th (since 2015)</li>
        </ul>
        Holidays falling on a Saturday or Sunday might be moved to the
        following Monday.

        \ingroup calendars
    */
    class Ukraine : public Calendar {
      private:
        class UseImpl final : public Calendar::OrthodoxImpl {
          public:
            std::string name() const override { return "Ukrainian stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        enum Market { USE    //!< Ukrainian stock exchange
        };
        Ukraine(Market m = USE);
    };

}


#endif
