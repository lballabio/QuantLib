
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file target.cpp
    \brief TARGET calendar

    \fullpath
    ql/Calendars/%target.cpp
*/

// $Id$

#include <ql/Calendars/target.hpp>

namespace QuantLib {

    namespace Calendars {

        bool TARGET::Impl::isBusinessDay(const Date& date) const {
            Weekday w = date.weekday();
            Day d = date.dayOfMonth(), dd = date.dayOfYear();
            Month m = date.month();
            Year y = date.year();
            Day em = easterMonday(y);
            if ((w == Saturday || w == Sunday)
                // New Year's Day
                || (d == 1  && m == January)
                // Good Friday
                || (dd == em-3)
                // Easter Monday
                || (dd == em)
                // Labour Day
                || (d == 1  && m == May)
                // Christmas
                || (d == 25 && m == December)
                // Day of Goodwill
                || (d == 26 && m == December)
                // December 31st, 1998 and 1999 only
                || (d == 31 && m == December && (y == 1998 || y == 1999)))
                    return false;
            return true;
        }

    }

}

