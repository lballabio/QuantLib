
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
/*! \file milan.cpp
      \brief Milan calendar

    \fullpath
    ql/Calendars/%milan.cpp
*/

// $Id$

#include <ql/Calendars/milan.hpp>

namespace QuantLib {

    namespace Calendars {

        bool Milan::MilCalendarImpl::isBusinessDay(const Date& date) const {
            Weekday w = date.weekday();
            Day d = date.dayOfMonth(), dd = date.dayOfYear();
            Month m = date.month();
            Year y = date.year();
            Day em = easterMonday(y);
            if ((w == Saturday || w == Sunday)
                // New Year's Day
                || (d == 1 && m == January)
                // Epiphany
                || (d == 6 && m == January)
                // Easter Monday
                || (dd == em)
                // Liberation Day
                || (d == 25 && m == April)
                // Labour Day
                || (d == 1 && m == May)
                // Republic Day
                || (d == 2 && m == June && y >= 2000)
                // Assumption
                || (d == 15 && m == August)
                // All Saints' Day
                || (d == 1 && m == November)
                // Immaculate Conception
                || (d == 8 && m == December)
                // Christmas
                || (d == 25 && m == December)
                // St. Stephen
                || (d == 26 && m == December)
                // December 31st, 1999 only
                || (d == 31 && m == December && y == 1999))
                    return false;
            return true;
        }

    }

}

