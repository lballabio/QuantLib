
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file frankfurt.cpp
    \brief Frankfurt calendar

    \fullpath
    Calendars/%frankfurt.cpp
*/

// $Id$

#include "ql/Calendars/frankfurt.hpp"

namespace QuantLib {

    namespace Calendars {

        bool Frankfurt::FFTCalendarImpl::isBusinessDay(
          const Date& date) const {
            Weekday w = date.weekday();
            Day d = date.dayOfMonth(), dd = date.dayOfYear();
            Month m = date.month();
            Year y = date.year();
            Day em = easterMonday(y);
            if ((w == Saturday || w == Sunday)
                // New Year's Day
                || (d == 1 && m == January)
                // Good Friday
                || (dd == em-3)
                // Easter Monday
                || (dd == em)
                // Ascension Thursday
                || (dd == em+38)
                // Whit Monday
                || (dd == em+49)
                // Corpus Christi
                || (dd == em+59)
                // Labour Day
                || (d == 1 && m == May)
                // National Day
                || (d == 3 && m == October)
                // Christmas Eve
                || (d == 24 && m == December)
                // Christmas
                || (d == 25 && m == December)
                // Boxing Day
                || (d == 26 && m == December)
                // New Year's Eve
                || (d == 31 && m == December))
                    return false;
            return true;
        }

    }

}

