/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 StatPro Italia srl

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

/*! \file russia.hpp
    \brief Russian calendar
*/

#ifndef quantlib_russia_calendar_hpp
#define quantlib_russia_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Russian calendar
    /*! Public holidays (see <http://www.cbr.ru/eng/>:):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year holidays and Christmas, January 1st to 8th</li>
        <li>Defender of the Fatherland Day, February 23rd (possibly
            moved to Monday)</li>
        <li>International Women's Day, March 8th (possibly moved to
            Monday)</li>
        <li>Labour Day, May 1st (possibly moved to Monday)</li>
        <li>Victory Day, May 9th (possibly moved to Monday)</li>
        <li>Russia Day, June 12th (possibly moved to Monday)</li>
        <li>Unity Day, November 4th (possibly moved to Monday)</li>
        </ul>

        \ingroup calendars
    */
    class Russia : public Calendar {
      private:
        class SettlementImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Russian settlement"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        Russia();
    };

}


#endif
