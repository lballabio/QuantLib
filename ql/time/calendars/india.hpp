/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2007, 2008, 2009, 2010, 2011 StatPro Italia srl
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

/*! \file india.hpp
    \brief Indian calendars
*/

#ifndef quantlib_indian_calendar_hpp
#define quantlib_indian_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Indian calendars
    /*! Holidays for the National Stock Exchange
        (data from <http://www.nse-india.com/>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>Republic Day, January 26th</li>
        <li>Good Friday</li>
        <li>Ambedkar Jayanti, April 14th</li>
        <li>May Day, May 1st</li>
        <li>Independence Day, August 15th</li>
        <li>Gandhi Jayanti, October 2nd</li>
        <li>Christmas, December 25th</li>
        </ul>

        Other holidays for which no rule is given
        (data available for 2005-2014, 2019-2025 only:)
        <ul>
        <li>Bakri Id</li>
        <li>Moharram</li>
        <li>Mahashivratri</li>
        <li>Holi</li>
        <li>Ram Navami</li>
        <li>Mahavir Jayanti</li>
        <li>Id-E-Milad</li>
        <li>Maharashtra Day</li>
        <li>Buddha Pournima</li>
        <li>Ganesh Chaturthi</li>
        <li>Dasara</li>
        <li>Laxmi Puja</li>
        <li>Bhaubeej</li>
        <li>Ramzan Id</li>
        <li>Guru Nanak Jayanti</li>
        </ul>

        Note: The holidays Ramzan Id, Bakri Id and Id-E-Milad rely on estimates for 2023-2025.
        \ingroup calendars
    */
    class India : public Calendar {
      private:
        class NseImpl final : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "National Stock Exchange of India"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        enum Market { NSE    //!< National Stock Exchange
        };
        India(Market m = NSE);
    };

}


#endif
