

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
/*! \file tokyo.cpp
    \brief Tokyo calendar

    \fullpath
    ql/Calendars/%tokyo.cpp
*/

// $Id$

#include <ql/Calendars/tokyo.hpp>

namespace QuantLib {

    namespace Calendars {

        bool Tokyo::TokCalendarImpl::isBusinessDay(const Date& date) const {
            Weekday w = date.weekday();
            Day d = date.dayOfMonth();
            Month m = date.month();
//            Year y = date.year();
            if ((w == Saturday || w == Sunday)
                // New Year's Day
                || (d == 1  && m == January)
                // Bank Holiday
                || (d == 2  && m == January)
                // Bank Holiday
                || (d == 3  && m == January)
                // Coming of Age Day (2nd Monday in January)
                || (w == Monday && (d >= 8 && d <= 14) && m == January)
                // National Foundation Day
                || ((d == 11 || (d == 12 && w == Monday)) && m == February)
                // Vernal Equinox 
                // it is not really fixed - this should be corrected!
                || ((d == 21 || (d == 22 && w == Monday)) && m == March)
                // Greenery Day
                || ((d == 29 || (d == 30 && w == Monday)) && m == April)
                // Constitution Memorial Day
                || (d == 3  && m == May)
                // Holiday for a Nation
                || (d == 4  && m == May)
                // Children's Day
                || ((d == 5  || (d == 6 && w == Monday)) && m == May)
                // Marine Day
                || ((d == 20 || (d == 21 && w == Monday)) && m == July)
                // Respect for the Aged Day
                || ((d == 15 || (d == 16 && w == Monday)) && m == September)
                // Autumnal Equinox 
                // it is not really fixed - this should be corrected!
                || ((d == 23 || (d == 24 && w == Monday)) && m == September)
                // Health and Sports Day (2nd Monday in October)
                || (w == Monday && (d >= 8 && d <= 14) && m == October)
                // National Culture Day
                || ((d == 3  || (d == 4 && w == Monday)) && m == November)
                // Labor Thanksgiving Day
                || ((d == 23 || (d == 24 && w == Monday)) && m == November)
                // Emperor's Birthday
                || ((d == 23 || (d == 24 && w == Monday)) && m == December)
                // Bank Holiday
                || (d == 31 && m == December))
                    return false;
            return true;
        }

    }

}

