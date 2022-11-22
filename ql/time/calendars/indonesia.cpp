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

#include <ql/time/calendars/indonesia.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    Indonesia::Indonesia(Market market) {
        // all calendar instances share the same implementation instance
        static ext::shared_ptr<Calendar::Impl> bejImpl(
                                                      new Indonesia::BejImpl);
        switch (market) {
          case BEJ:
          case JSX:
          case IDX:
            impl_ = bejImpl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }

    bool Indonesia::BejImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        Day dd = date.dayOfYear();
        Day em = easterMonday(y);

        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Good Friday
            || (dd == em-3)
            // Ascension Thursday
            || (dd == em+38)
            // Independence Day
            || (d == 17 && m == August)
            // Christmas
            || (d == 25 && m == December)
            )
            return false;

        if (y == 2005) {
            if (// Idul Adha
                (d == 21 && m == January)
                // Imlek
                || (d == 9 && m == February)
                // Moslem's New Year Day
                || (d == 10 && m == February)
                // Nyepi
                || (d == 11 && m == March)
                // Birthday of Prophet Muhammad SAW
                || (d == 22 && m == April)
                // Waisak
                || (d == 24 && m == May)
                // Ascension of Prophet Muhammad SAW
                || (d == 2 && m == September)
                // Idul Fitri
                || ((d == 3 || d == 4) && m == November)
                // National leaves
                || ((d == 2 || d == 7 || d == 8) && m == November)
                || (d == 26 && m == December)
                )
                return false;
        }

        if (y == 2006) {
            if (// Idul Adha
                (d == 10 && m == January)
                // Moslem's New Year Day
                || (d == 31 && m == January)
                // Nyepi
                || (d == 30 && m == March)
                // Birthday of Prophet Muhammad SAW
                || (d == 10 && m == April)
                // Ascension of Prophet Muhammad SAW
                || (d == 21 && m == August)
                // Idul Fitri
                || ((d == 24 || d == 25) && m == October)
                // National leaves
                || ((d == 23 || d == 26 || d == 27) && m == October)
                )
                return false;
        }

        if (y == 2007) {
            if (// Nyepi
                (d == 19 && m == March)
                // Waisak
                || (d == 1 && m == June)
                // Ied Adha
                || (d == 20 && m == December)
                // National leaves
                || (d == 18 && m == May)
                || ((d == 12 || d == 15 || d == 16) && m == October)
                || ((d == 21 || d == 24) && m == October)
                )
                return false;
        }

        if (y == 2008) {
            if (// Islamic New Year
                ((d == 10 || d == 11) && m == January)
                // Chinese New Year
                || ((d == 7 || d == 8) && m == February)
                // Saka's New Year
                || (d == 7 && m == March)
                // Birthday of the prophet Muhammad SAW
                || (d == 20 && m == March)
                // Vesak Day
                || (d == 20 && m == May)
                // Isra' Mi'raj of the prophet Muhammad SAW
                || (d == 30 && m == July)
                // National leave
                || (d == 18 && m == August)
                // Ied Fitr
                || (d == 30 && m == September)
                || ((d == 1 || d == 2 || d == 3) && m == October)
                // Ied Adha
                || (d == 8 && m == December)
                // Islamic New Year
                || (d == 29 && m == December)
                // New Year's Eve
                || (d == 31 && m == December)
                )
                return false;
        }

        if (y == 2009) {
            if (// Public holiday
                (d == 2 && m == January)
                // Chinese New Year
                || (d == 26 && m == January)
                // Birthday of the prophet Muhammad SAW
                || (d == 9 && m == March)
                // Saka's New Year
                || (d == 26 && m == March)
                // National leave
                || (d == 9 && m == April)
                // Isra' Mi'raj of the prophet Muhammad SAW
                || (d == 20 && m == July)
                // Ied Fitr
                || (d >= 18 && d <= 23 && m == September)
                // Ied Adha
                || (d == 27 && m == November)
                // Islamic New Year
                || (d == 18 && m == December)
                // Public Holiday
                || (d == 24 && m == December)
                // Trading holiday
                || (d == 31 && m == December)
                )
                return false;
        }

        if (y == 2010) {
            if (// Birthday of the prophet Muhammad SAW
                   (d == 26 && m == February)
                // Saka's New Year
                || (d == 16 && m == March)
                // Birth of Buddha
                || (d == 28 && m == May)
                // Ied Fitr
                || (d >= 8 && d <= 14 && m == September)
                // Ied Adha
                || (d == 17 && m == November)
                // Islamic New Year
                || (d == 7 && m == December)
                // Public Holiday
                || (d == 24 && m == December)
                // Trading holiday
                || (d == 31 && m == December)
                )
                return false;
        }

        if (y == 2011) {
            if (// Chinese New Year
                   (d == 3 && m == February)
                // Birthday of the prophet Muhammad SAW
                || (d == 15 && m == February)
                // Birth of Buddha
                || (d == 17 && m == May)
                // Isra' Mi'raj of the prophet Muhammad SAW
                || (d == 29 && m == June)
                // Ied Fitr
                || (d >= 29 && m == August)
                || (d <= 2 && m == September)
                // Public Holiday
                || (d == 26 && m == December)
                )
                return false;
        }

        if (y == 2012) {
            if (// Chinese New Year
                (d == 23 && m == January)
                // Saka New Year
                || (d == 23 && m == March)
                // Ied ul-Fitr
                || (d >= 20 && d <= 22 && m == August)
                // Eid ul-Adha
                || (d == 26 && m == October)
                // Islamic New Year
                || (d >= 15 && d <= 16 && m == November)
                // Public Holiday
                || (d == 24 && m == December)
                // Trading Holiday
                || (d == 31 && m == December)
                )
                return false;
        }

        if (y == 2013) {
            if (// Birthday of the prophet Muhammad SAW
                (d == 24 && m == January)
                // Saka New Year
                || (d == 12 && m == March)
                // Isra' Mi'raj of the prophet Muhammad SAW
                || (d == 6 && m == June)
                // Ied ul-Fitr
                || (d >= 5 && d <= 9 && m == August)
                // Eid ul-Adha
                || (d >= 14 && d <= 15 && m == October)
                // Islamic New Year
                || (d == 5 && m == November)
                // Public Holiday
                || (d == 26 && m == December)
                // Trading Holiday
                || (d == 31 && m == December)
                )
                return false;
        }

        if (y == 2014) {
            if (// Birthday of the prophet Muhammad SAW
                (d == 14 && m == January)
                // Chinese New Year
                || (d == 31 && m == January)
                // Saka New Year
                || (d == 31 && m == March)
                // Labour Day
                || (d == 1 && m == May)
                // Birth of Buddha
                || (d == 15 && m == May)
                // Isra' Mi'raj of the prophet Muhammad SAW
                || (d == 27 && m == May)
                // Ascension Day of Jesus Christ
                || (d == 29 && m == May)
                // Ied ul-Fitr
                || ((d >= 28 && m == July) || (d == 1 && m == August))
                // Public Holiday
                || (d == 26 && m == December)
                // Trading Holiday
                || (d == 31 && m == December)
                )
                return false;
        }

        return true;
    }

}

