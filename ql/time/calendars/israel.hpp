/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Paolo Mazzocchi
 Copyright (C) 2015 Riccardo Barone

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

/*! \file israel.hpp
    \brief Israelian calendar
*/

#ifndef quantlib_israel_calendar_hpp
#define quantlib_israel_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Israel calendar
    /*! Due to the lack of reliable sources, the settlement calendar
        has the same holidays as the Tel Aviv stock-exchange.

        Holidays for the Tel-Aviv Stock Exchange (TASE)
        (data from <http://www.tase.co.il>):
        <ul>
        <li>Weekdays: Friday and Saturday until 2025, Saturday and Sunday since 2026</li>
        </ul>
        Other holidays for wich no rule is given
        (data available for 2013-2044 only:)
        <ul>
        <li>Purim, Adar 14th (between Feb 24th & Mar 26th)</li>
        <li>Passover I, Nisan 15th (between Mar 26th & Apr 25th)</li>
        <li>Passover VII, Nisan 21st (between Apr 1st & May 1st)</li>
        <li>Memorial Day, Nisan 27th (between Apr 7th & May 7th)</li>
        <li>Indipendence Day, Iyar 5th (between Apr 15th & May 15th)</li>
        <li>Pentecost (Shavuot), Sivan 6th (between May 15th & June 14th)</li>
        <li>Fast Day</li>
        <li>Jewish New Year, Tishrei 1st & 2nd (between Sep 5th & Oct 5th)</li>
        <li>Yom Kippur, Tishrei 10th (between Sep 14th & Oct 14th)</li>
        <li>Sukkoth, Tishrei 15th (between Sep 19th & Oct 19th)</li>
        <li>Simchat Tora, Tishrei 22nd (between Sep 26th & Oct 26th)</li>
        </ul>

        Holidays for the Telbor fixing calendar:
        <ul>
        <li>Weekdays: Saturday and Sunday</li>
        <li>Purim I and II</li>
        <li>Passover Eve, I and VII</li>
        <li>Indipendence Day</li>
        <li>Pentecost (Shavuot)</li>
        <li>Fast Day</li>
        <li>Jewish New Year I and II</li>
        <li>Yom Kippur</li>
        <li>Sukkoth</li>
        <li>Simchat Torah</li>
        <li>Western New Year, January 1st</li>
        <li>Spring Bank Holiday, last Monday of May</li>
        <li>Christmas, December 25th</li>
        <li>Boxing Day, December 26th</li>
        <li>One-off days for elections</li>
        </ul>

        Holidays for the SHIR fixing calendar:
        <ul>
        <li>Weekdays: Saturday and Sunday</li>
        <li>Purim I and II</li>
        <li>Passover Eve, I and VII</li>
        <li>Indipendence Day</li>
        <li>Pentecost (Shavuot)</li>
        <li>Fast Day</li>
        <li>Jewish New Year Eve, I and II</li>
        <li>Yom Kippur and its Eve</li>
        <li>Sukkoth</li>
        <li>Simchat Torah</li>
        <li>Western New Year, January 1st</li>
        <li>Good Friday</li>
        <li>Spring Bank Holiday, last Monday of May</li>
        <li>Christmas, December 25th</li>
        <li>Boxing Day, December 26th</li>
        <li>One-off days for elections</li>
        </ul>

        \ingroup calendars
    */
    class Israel : public Calendar {
      private:
        class TelAvivImpl;
        class TelborImpl;
        class ShirImpl;
      public:
          enum Market {
              Settlement,     //!< generic settlement calendar
              TASE,           //!< Tel-Aviv stock exchange calendar (Fri/Sat weekends, pre-2026; Sat/Sun afterwards)
              Telbor,         //!< Telbor fixing calendar (Sat/Sun weekends)
              SHIR            //!< SHIR fixing calendar (Sat/Sun weekends)
          };
          Israel(Market market = Settlement);
    };

}


#endif
