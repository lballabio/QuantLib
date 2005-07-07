/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2003 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file copenhagen.hpp
    \brief Copenhagen calendar
*/

#ifndef quantlib_copenhagen_calendar_h
#define quantlib_copenhagen_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    //! %Copenhagen calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>Maunday Thursday</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>General Prayer Day, 25 days after Easter Monday</li>
        <li>Ascension</li>
        <li>Whit (Pentecost) Monday </li>
        <li>New Year's Day, January 1st</li>
        <li>Constitution Day, June 5th</li>
        <li>Christmas, December 25th</li>
        <li>Boxing Day, December 26th</li>
        </ul>

        \ingroup calendars
    */
    class Copenhagen : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Copenhagen"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        Copenhagen();
    };

}


#endif
