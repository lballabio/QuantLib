
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file sydney.hpp
    \brief Sydney calendar
*/

#ifndef quantlib_sydney_calendar_h
#define quantlib_sydney_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    //! %Sydney calendar (New South Wales, Australia)
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Australia Day, January 26th (possibly moved to Monday)</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>ANZAC Day. April 25th (possibly moved to Monday)</li>
        <li>Queen's Birthday, second Monday in June</li>
        <li>Bank Holiday, first Monday in August</li>
        <li>Labour Day, first Monday in October</li>
        <li>Christmas, December 25th (possibly moved to Monday or Tuesday)</li>
        <li>Boxing Day, December 26th (possibly moved to Monday or 
            Tuesday)</li>
        </ul>

        \ingroup calendars
    */
    class Sydney : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Sydney"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        Sydney();
    };

}


#endif
