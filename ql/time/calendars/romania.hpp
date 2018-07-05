/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 StatPro Italia srl
 Copyright (C) 2015 Riccardo Barone
 Copyright (C) 2018 Matthias Lungwitz

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

/*! \file romania.hpp
    \brief Romanian calendar
*/

#ifndef quantlib_romania_calendar_hpp
#define quantlib_romania_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Romanian calendars
    /*! Public holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li> Day after New Year's Day, January 2nd</li>
        <li>Unification Day, January 24th</li>
        <li>Orthodox Easter (only Sunday and Monday)</li>
        <li>Labour Day, May 1st</li>
        <li>Pentecost with Monday (50th and 51st days after the 
            Othodox Easter)</li>
        <li>Children's Day, June 1st (since 2017)</li>
        <li>St Marys Day, August 15th</li>
        <li>Feast of St Andrew, November 30th</li>
        <li>National Day, December 1st</li>
        <li>Christmas, December 25th</li>
        <li>2nd Day of Christmas, December 26th</li>
        </ul>
        
        \ingroup calendars
    */
    class Romania : public Calendar {
      private:
        class Impl : public Calendar::OrthodoxImpl {
          public:
            std::string name() const { return "Romania"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        Romania();
    };


}


#endif
