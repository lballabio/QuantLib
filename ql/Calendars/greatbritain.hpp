
/*
 Copyright (C) 2004 Ferdinando Ametrano
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

/*! \file london.hpp
    \brief Great Britain calendar
*/

#ifndef quantlib_greatbritain_calendar_h
#define quantlib_greatbritain_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    //! Great Britain settlement calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday)</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Early May Bank Holiday, first Monday of May</li>
        <li>Spring Bank Holiday, last Monday of May</li>
        <li>Summer Bank Holiday, last Monday of August</li>
        <li>Christmas Day, December 25th (possibly moved to Monday or Tuesday)</li>
        <li>Boxing Day, December 26th (possibly moved to Monday or
            Tuesday)</li>
        </ul>

        Data from http://www.dti.gov.uk/er/bankhol.htm

        \ingroup calendars
    */
    class GreatBritain : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "GreatBritain"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        GreatBritain();
    };

    //! London Stock Exchange settlement calendar
    typedef GreatBritain LondonStockExchange;

    //! London Metals Exchange settlement calendar
    typedef GreatBritain LondonMetalsExchange;

}


#endif
