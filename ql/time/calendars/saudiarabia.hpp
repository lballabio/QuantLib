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
 <http://quantlib.org/license.shtml>.

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
        (data from <http://www.tadawul.com.sa>):
        <ul>
        <li>Thursdays</li>
        <li>Fridays</li>
        <li>National Day of Saudi Arabia, September 23rd</li>
        </ul>

        Other holidays for which no rule is given
        (data available sparsely for 2004-2011 only:)
        <ul>
        <li>Eid Al-Adha</li>
        <li>Eid Al-Fitr</li>
        </ul>

        \ingroup calendars
    */
    class SaudiArabia : public Calendar {
      private:
        class TadawulImpl : public Calendar::Impl {
          public:
            std::string name() const { return "Tadawul"; }
            bool isWeekend(Weekday) const;
            bool isBusinessDay(const Date&) const;
        };
      public:
        enum Market { Tadawul    //!< Tadawul financial market
        };
        SaudiArabia(Market m = Tadawul);
    };

}


#endif
