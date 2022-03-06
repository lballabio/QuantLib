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

/*! \file japan.hpp
    \brief Japanese calendar
*/

#ifndef quantlib_japanese_calendar_hpp
#define quantlib_japanese_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Japanese calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Bank Holiday, January 2nd</li>
        <li>Bank Holiday, January 3rd</li>
        <li>Coming of Age Day, 2nd Monday in January</li>
        <li>National Foundation Day, February 11th</li>
        <li>Emperor's Birthday, February 23rd since 2020 and December 23rd before</li>
        <li>Vernal Equinox</li>
        <li>Greenery Day, April 29th</li>
        <li>Constitution Memorial Day, May 3rd</li>
        <li>Holiday for a Nation, May 4th</li>
        <li>Children's Day, May 5th</li>
        <li>Marine Day, 3rd Monday in July</li>
        <li>Mountain Day, August 11th (from 2016 onwards)</li>
        <li>Respect for the Aged Day, 3rd Monday in September</li>
        <li>Autumnal Equinox</li>
        <li>Health and Sports Day, 2nd Monday in October</li>
        <li>National Culture Day, November 3rd</li>
        <li>Labor Thanksgiving Day, November 23rd</li>
        <li>Bank Holiday, December 31st</li>
        <li>a few one-shot holidays</li>
        </ul>
        Holidays falling on a Sunday are observed on the Monday following
        except for the bank holidays associated with the new year.

        \ingroup calendars
    */
    class Japan : public Calendar {
      private:
        class Impl : public Calendar::Impl {
          public:
            std::string name() const override { return "Japan"; }
            bool isWeekend(Weekday) const override;
            bool isBusinessDay(const Date&) const override;
        };
      public:
        Japan();
    };

}


#endif


#ifndef id_d31eb060af1f82d795863e6d2ded1b3b
#define id_d31eb060af1f82d795863e6d2ded1b3b
inline bool test_d31eb060af1f82d795863e6d2ded1b3b(const int* i) {
    return i != nullptr;
}
#endif
