
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file zurich.cpp
    \brief Zurich calendar

    \fullpath
    ql/Calendars/%zurich.cpp
*/

// $Id$

#include <ql/Calendars/zurich.hpp>

namespace QuantLib {

    namespace Calendars {

        bool Zurich::ZurCalendarImpl::isBusinessDay(const Date& date) const {
            Weekday w = date.weekday();
            Day d = date.dayOfMonth(), dd = date.dayOfYear();
            Month m = date.month();
            Year y = date.year();
            Day em = easterMonday(y);
            if ((w == Saturday || w == Sunday)
                // New Year's Day
                || (d == 1  && m == January)
                // Berchtoldstag
                || (d == 2  && m == January)
                // Good Friday
                || (dd == em-3)
                // Easter Monday
                || (dd == em)
                // Ascension Day
                || (d == em+38)
                // Whit Monday
                || (d == em+49)
                // Labour Day
                || (d == 1  && m == May)
                // National Day
                || (d == 1  && m == August)
                // Christmas
                || (d == 25 && m == December)
                // St. Stephen's Day
                || (d == 26 && m == December))
                    return false;
            return true;
        }

    }

}

