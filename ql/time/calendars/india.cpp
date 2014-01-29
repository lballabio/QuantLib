/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2007, 2008, 2009, 2010, 2011 StatPro Italia srl

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

#include <ql/time/calendars/india.hpp>

namespace QuantLib {

    India::India(Market) {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new India::NseImpl);
        impl_ = impl;
    }

    bool India::NseImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        Day dd = date.dayOfYear();
        Day em = easterMonday(y);

        if (isWeekend(w)
            // Republic Day
            || (d == 26 && m == January)
            // Good Friday
            || (dd == em-3)
            // Ambedkar Jayanti
            || (d == 14 && m == April)
            // May Day
            || (d == 1 && m == May)
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

        if (y == 2006) {
            if (// Bakri Id
                (d == 11 && m == January)
                // Moharram
                || (d == 9 && m == February)
                // Holi
                || (d == 15 && m == March)
                // Ram Navami
                || (d == 6 && m == April)
                // Mahavir Jayanti
                || (d == 11 && m == April)
                // Maharashtra Day
                || (d == 1 && m == May)
                // Bhaubeej
                || (d == 24 && m == October)
                // Ramzan Id
                || (d == 25 && m == October)
                )
                return false;
        }

        if (y == 2007) {
            if (// Bakri Id
                (d == 1 && m == January)
                // Moharram
                || (d == 30 && m == January)
                // Mahashivratri
                || (d == 16 && m == February)
                // Ram Navami
                || (d == 27 && m == March)
                // Maharashtra Day
                || (d == 1 && m == May)
                // Buddha Pournima
                || (d == 2 && m == May)
                // Laxmi Puja
                || (d == 9 && m == November)
                // Bakri Id (again)
                || (d == 21 && m == December)
                )
                return false;
        }

        if (y == 2008) {
            if (// Mahashivratri
                (d == 6 && m == March)
                // Id-E-Milad
                || (d == 20 && m == March)
                // Mahavir Jayanti
                || (d == 18 && m == April)
                // Maharashtra Day
                || (d == 1 && m == May)
                // Buddha Pournima
                || (d == 19 && m == May)
                // Ganesh Chaturthi
                || (d == 3 && m == September)
                // Ramzan Id
                || (d == 2 && m == October)
                // Dasara
                || (d == 9 && m == October)
                // Laxmi Puja
                || (d == 28 && m == October)
                // Bhau bhij
                || (d == 30 && m == October)
                // Gurunanak Jayanti
                || (d == 13 && m == November)
                // Bakri Id
                || (d == 9 && m == December)
                )
                return false;
        }

        if (y == 2009) {
            if (// Moharram
                (d == 8 && m == January)
                // Mahashivratri
                || (d == 23 && m == February)
                // Id-E-Milad
                || (d == 10 && m == March)
                // Holi
                || (d == 11 && m == March)
                // Ram Navmi
                || (d == 3 && m == April)
                // Mahavir Jayanti
                || (d == 7 && m == April)
                // Maharashtra Day
                || (d == 1 && m == May)
                // Ramzan Id
                || (d == 21 && m == September)
                // Dasara
                || (d == 28 && m == September)
                // Bhau Bhij
                || (d == 19 && m == October)
                // Gurunanak Jayanti
                || (d == 2 && m == November)
                // Moharram (again)
                || (d == 28 && m == December)
                )
                return false;
        }

        if (y == 2010) {
            if (// New Year's Day
                (d == 1 && m == January)
                // Mahashivratri
                || (d == 12 && m == February)
                // Holi
                || (d == 1 && m == March)
                // Ram Navmi
                || (d == 24 && m == March)
                // Ramzan Id
                || (d == 10 && m == September)
                // Laxmi Puja
                || (d == 5 && m == November)
                // Bakri Id
                || (d == 17 && m == November)
                // Moharram
                || (d == 17 && m == December)
                )
                return false;
        }

        if (y == 2011) {
            if (// Mahashivratri
                (d == 2 && m == March)
                // Ram Navmi
                || (d == 12 && m == April)
                // Ramzan Id
                || (d == 31 && m == August)
                // Ganesh Chaturthi
                || (d == 1 && m == September)
                // Dasara
                || (d == 6 && m == October)
                // Laxmi Puja
                || (d == 26 && m == October)
                // Diwali - Balipratipada
                || (d == 27 && m == October)
                // Bakri Id
                || (d == 7 && m == November)
                // Gurunanak Jayanti
                || (d == 10 && m == November)
                // Moharram
                || (d == 6 && m == December)
                )
                return false;
        }

        if (y == 2012) {
            if (// Mahashivratri
                (d == 20 && m == February)
                // Holi
                || (d == 8 && m == March)
                // Mahavir Jayanti
                || (d == 5 && m == April)
                // Ramzan Id
                || (d == 20 && m == August)
                // Ganesh Chaturthi
                || (d == 19 && m == September)
                // Dasara
                || (d == 24 && m == October)
                // Diwali - Balipratipada
                || (d == 14 && m == November)
                // Gurunanak Jayanti
                || (d == 28 && m == November)
                )
                return false;
        }

        if (y == 2013) {
            if (// Holi
                (d == 27 && m == March)
                // Ram Navmi
                || (d == 19 && m == April)
                // Mahavir Jayanti
                || (d == 24 && m == April)
                // Ramzan Id
                || (d == 9 && m == August)
                // Ganesh Chaturthi
                || (d == 9 && m == September)
                // Bakri Id
                || (d == 16 && m == October)
                // Diwali - Balipratipada
                || (d == 4 && m == November)
                // Moharram
                || (d == 14 && m == November)
                )
                return false;
        }

        if (y == 2014) {
            if (// Mahashivratri
                (d == 27 && m == February)
                // Holi
                || (d == 17 && m == March)
                // Ram Navmi
                || (d == 8 && m == April)
                // Ramzan Id
                || (d == 29 && m == July)
                // Ganesh Chaturthi
                || (d == 29 && m == August)
                // Dasera
                || (d == 3 && m == October)
                // Bakri Id
                || (d == 6 && m == October)
                // Diwali - Balipratipada
                || (d == 24 && m == October)
                // Moharram
                || (d == 4 && m == November)
                // Gurunank Jayanti
                || (d == 6 && m == November)
                )
                return false;
        }

        return true;
    }

}

