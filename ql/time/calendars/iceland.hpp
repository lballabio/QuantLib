/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

/*! \file iceland.hpp
    \brief Icelandic calendars
*/

#ifndef quantlib_iceland_calendar_hpp
#define quantlib_iceland_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Icelandic calendars
    /*! Holidays for the Iceland stock exchange
        (data from <http://www.icex.is/is/calendar?languageID=1>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Holy Thursday</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>First day of Summer (third or fourth Thursday in April)</li>
        <li>Labour Day, May 1st</li>
        <li>Ascension Thursday</li>
        <li>Pentecost Monday</li>
        <li>Independence Day, June 17th</li>
        <li>Commerce Day, first Monday in August</li>
        <li>Christmas, December 25th</li>
        <li>Boxing Day, December 26th</li>
        </ul>

        \ingroup calendars
    */
    class Iceland : public Calendar {
      private:
        class IcexImpl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Iceland stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        enum Market { ICEX    //!< Iceland stock exchange
        };
        Iceland(Market m = ICEX);
    };

}


#endif
