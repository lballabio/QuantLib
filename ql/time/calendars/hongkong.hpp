/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group
 Copyright (C) 2007, 2009, 2010, 2011 StatPro Italia srl

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

/*! \file hongkong.hpp
    \brief Hong Kong calendars
*/

#ifndef quantlib_hongkong_calendar_hpp
#define quantlib_hongkong_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Hong Kong calendars
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday)</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Labor Day, May 1st (possibly moved to Monday)</li>
        <li>SAR Establishment Day, July 1st (possibly moved to Monday)</li>
        <li>National Day, October 1st (possibly moved to Monday)</li>
        <li>Christmas, December 25th</li>
        <li>Boxing Day, December 26th</li>
        </ul>

        Other holidays for which no rule is given
        (data available for 2004-2015 only:)
        <ul>
        <li>Lunar New Year</li>
        <li>Chinese New Year</li>
        <li>Ching Ming Festival</li>
        <li>Buddha's birthday</li>
        <li>Tuen NG Festival</li>
        <li>Mid-autumn Festival</li>
        <li>Chung Yeung Festival</li>
        </ul>

        Data from <http://www.hkex.com.hk>

        \ingroup calendars
    */
    class HongKong : public Calendar {
      private:
        class HkexImpl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Hong Kong stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        enum Market { HKEx    //!< Hong Kong stock exchange
        };
        HongKong(Market m = HKEx);
    };

}


#endif
