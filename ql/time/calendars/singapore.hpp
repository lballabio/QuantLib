/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group
 Copyright (C) 2007, 2008, 2009, 2010 StatPro Italia srl
 Copyright (C) 2024 Skandinaviska Enskilda Banken AB (publ)

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

/*! \file singapore.hpp
    \brief Singapore calendars
*/

#ifndef quantlib_singapore_calendar_hpp
#define quantlib_singapore_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! %Singapore calendars
    /*! Holidays for the Singapore exchange
        (data from
         <http://www.sgx.com/wps/portal/sgxweb/home/trading/securities/trading_hours_calendar>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's day, January 1st</li>
        <li>Good Friday</li>
        <li>Labour Day, May 1st</li>
        <li>National Day, August 9th</li>
        <li>Christmas, December 25th </li>
        </ul>

        Other holidays for which no rule is given
        (data available for 2004-2010, 2012-2014, 2019-2024 only:)
        <ul>
        <li>Chinese New Year</li>
        <li>Hari Raya Haji</li>
        <li>Vesak Poya Day</li>
        <li>Deepavali</li>
        <li>Diwali</li>
        <li>Hari Raya Puasa</li>
        </ul>

        \ingroup calendars
    */
    class Singapore : public Calendar {
      private:
        class SgxImpl final : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Singapore exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        enum Market { SGX    //!< Singapore exchange
        };
        Singapore(Market m = SGX);
    };

}


#endif
