/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group

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

/*! \file riyadh.hpp
    \brief Saudi Arabian calendar
*/

#ifndef quantlib_saudi_arabian_calendar_hpp
#define quantlib_saudi_arabian_calendar_hpp

#include <ql/calendar.hpp>

namespace QuantLib {

    //! Saudi Arabian calendar
    /*! Holidays:
        <ul>
        <li>Fridays</li>
        </ul>

        Other holidays for which no rule is given
        (data available for 2004-2005 only:)
        <ul>
        <li>EID AL-ADHA</li>
        <li>EID AL-FITR</li>
        </ul>

        \ingroup calendars
    */
    class SaudiArabia : public Calendar {
      private:
        class Impl : public Calendar::Impl {
          public:
            std::string name() const { return "Saudi Arabia"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        SaudiArabia();
    };

    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to SaudiArabia */
    typedef SaudiArabia Riyadh;
    #endif

}


#endif
