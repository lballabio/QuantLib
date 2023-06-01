/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Skandinaviska Enskilda Banken AB (publ)

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

/*! \file bfix.hpp
    \brief Bloomberg fixing calendar
*/

#ifndef quantlib_bfix_calendar_hpp
#define quantlib_bfix_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Bloomberg fixing calendar
    /*! Holidays for the Bloomberg fixing calendar
        (data from follows methodology in:
        <https://data.bloomberglp.com/notices/sites/3/2016/04/bfix_methodology.pdf>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (moved to Monday if occurring on a Sunday)</li>
        <li>Good Friday</li>
        <li>Christmas Day, December 25th (moved to Monday if occurring on a Sunday)</li>
        </ul>

        \test the correctness of the returned results is tested
              against a list of known holidays.

        \ingroup calendars
    */
    class BFix : public Calendar {
      private:
        class Impl : public QuantLib::Calendar::WesternImpl {
          public:
            std::string name() const override { return "Bloomberg fixing"; }
            bool isBusinessDay(const QuantLib::Date&) const override;
        };

      public:
        BFix();
    };

}


#endif
