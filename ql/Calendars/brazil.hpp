/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Piter Dias

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file brazil.hpp
    \brief Brazil calendar
*/

#ifndef quantlib_brazil_calendar_hpp
#define quantlib_brazil_calendar_hpp

#include <ql/calendar.hpp>

namespace QuantLib {

    //! %Brazil calendar
    /*! Banking holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Tiradentes's Day, April 21th</li>
        <li>Labour Day, May 1st</li>
        <li>Independence Day, September 21th</li>
        <li>Nossa Sra. Aparecida Day, October 12th</li>
        <li>Dead Day, October 2nd</li>
        <li>Republic Day, November 15th</li>
        <li>Christmas, December 25th</li>
        <li>Passion of Christ</li>
        <li>Carnival</li>
        <li>Corpus Christi</li>

        \ingroup calendars

        \test the correctness of the returned results is tested
              against a list of known holidays.
    */
    class Brazil : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Brazil"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        Brazil();
    };

}


#endif
