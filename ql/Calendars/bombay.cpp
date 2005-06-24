/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

#include <ql/Calendars/bombay.hpp>

namespace QuantLib {

    Bombay::Bombay() {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new Bombay::Impl);
        impl_ = impl;
    }

    bool Bombay::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        Day dd = date.dayOfYear();
        Day em = easterMonday(y);

        if ((w == Saturday || w == Sunday)
            // Republic Day
            || (d == 26 && m == January)
            // Good Friday
            || (dd == em-3)
            // Ambedkar Jayanti
            || (d == 14 && m == April)
            // Independence Day
            || (d == 15 && m == August)
            // Gandhi Jayanti
            || (d == 2 && m == October)
            // Christmas
            || (d == 25 && m == December)
            )
            return false;
        if (y == 2005) {
            // Moharram, Holi, Maharashtra Day, and Ramzan Id fall
            // on Saturday or Sunday in 2005
            if (// Bakri Id
                (d == 21 && m == January)
                // Ganesh Chaturthi
                || (d == 7 && m == September)
                // Dasara
                || (d == 12 && m == October)
                // Laxmi Puja
                || (d == 1 && m == November)
                // Bhaubeej
                || (d == 3 && m == November)
                // Guru Nanak Jayanti
                || (d == 15 && m == November)
                )
                return false;
        }
        return true;
    }

}

