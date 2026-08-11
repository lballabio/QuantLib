/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 SoftSolution srl

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

/*! \file montenegro.hpp
    \brief Montenegro calendars
*/

#ifndef quantlib_montenegro_calendar_hpp
#define quantlib_montenegro_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Montenegro calendar
    /*! Holidays for the Montenegrin Stock Exchange (https://mnse.me):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Yearâ€™s Day, January 1st<\li>
        <li>New Year Holiday, January 2nd<\li>
        <li>Labour Day, May 1st<\li>
        <li>Labour Day Holiday, May 2nd<\li>
        <li>Independence Day, May 21st<\li>
        <li>Independence Day Holiday, May 22nd<\li>
        <li>Statehood Day, July 13th<\li>
        <li>Statehood Day Holiday, July 14th<\li>
        <li>Njegos Day, November 13th<\li>
        <li>Njegos Day Holiday, November 14th<\li>
        </ul>
    */
    class Montenegro : public Calendar {
    private:
        class MnseImpl final : public Calendar::WesternImpl {
        public:
            std::string name() const override { return "Montenegro Stock Exchange"; }
            bool isBusinessDay(const Date&) const override;
        };

    public:
        enum Market {
            MNSE //!< Montenegro Stock Exchange
        };
        explicit Montenegro(Market m = MNSE);
    };

}

#endif
