/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group
 Copyright (C) 2007, 2011 StatPro Italia srl

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

/*! \file saudiarabia.hpp
    \brief Saudi Arabian calendar
*/

#ifndef quantlib_saudi_arabian_calendar_hpp
#define quantlib_saudi_arabian_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Saudi Arabian calendar
    /*! Holidays for the Tadawul financial market
        (data from <https://www.saudiexchange.sa>):
        <ul>
        <li>Thursdays and Fridays (until 28 June 2013)</li>
        <li>Fridays and Saturdays (from 29 June 2013)</li>
        <li>National Day of Saudi Arabia, September 23rd</li>
        <li>Founding Day of Saudi Arabia, February 22nd (from 2022)</li>
        </ul>

        Other holidays for which no rule is given
        (Eid Al-Adha and Eid Al-Fitr windows through 2022 use published
        Gregorian dates, with exchange-documented ranges for 2012-2015;
        windows from 2023 through 2029 follow the Saudi Exchange holiday
        calendar):
        <ul>
        <li>Eid Al-Adha</li>
        <li>Eid Al-Fitr</li>
        </ul>

        \ingroup calendars
    */
    class SaudiArabia : public Calendar {
      private:
        class TadawulImpl final : public Calendar::Impl {
          public:
            std::string name() const override { return "Tadawul"; }
            bool isWeekend(Weekday) const override;
            bool isBusinessDay(const Date&) const override;
        };
      public:
        enum Market { Tadawul    //!< Tadawul financial market
        };
        SaudiArabia(Market m = Tadawul);
    };

}


#endif
