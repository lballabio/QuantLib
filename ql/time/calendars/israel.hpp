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
 <http://quantlib.org/license.shtml>.

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

        Holidays for the Tel-Aviv Stock Exchange
        (data from <http://www.tase.co.il>):
        <ul>
        <li>Friday</li>
        <li>Saturday</li>
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


        \ingroup calendars
    */
    class Israel : public Calendar {
      private:
        class TelAvivImpl : public Calendar::Impl {
          public:
            std::string name() const override { return "Tel Aviv stock exchange"; }
            bool isWeekend(Weekday) const override;
            bool isBusinessDay(const Date&) const override;
        };
      public:
          enum Market { Settlement,     //!< generic settlement calendar
                        TASE            //!< Tel-Aviv stock exchange calendar
          };
          Israel(Market market = Settlement);
    };

}


#endif


#ifndef id_2c3d01070824d3fcd11ff5a4ff25d074
#define id_2c3d01070824d3fcd11ff5a4ff25d074
inline bool test_2c3d01070824d3fcd11ff5a4ff25d074(int* i) { return i != 0; }
#endif
