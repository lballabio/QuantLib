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

/*! \file newzealand.hpp
    \brief New Zealand calendar
*/

#ifndef quantlib_new_zealand_calendar_hpp
#define quantlib_new_zealand_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! New Zealand calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday or
            Tuesday)</li>
        <li>Day after New Year's Day, January 2st (possibly moved to
            Monday or Tuesday)</li>
        <li>Anniversary Day, Monday nearest January 22nd</li>
        <li>Waitangi Day. February 6th</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>ANZAC Day. April 25th</li>
        <li>Queen's Birthday, first Monday in June</li>
        <li>Labour Day, fourth Monday in October</li>
        <li>Christmas, December 25th (possibly moved to Monday or Tuesday)</li>
        <li>Boxing Day, December 26th (possibly moved to Monday or
            Tuesday)</li>
        </ul>
        \note The holiday rules for New Zealand were documented by
              David Gilbert for IDB (http://www.jrefinery.com/ibd/)

        \ingroup calendars
    */
    class NewZealand : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "New Zealand"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        NewZealand();
    };

}


#endif
