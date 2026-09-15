/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Lawrenz Law

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

#include <ql/time/calendars/philippines.hpp>
#include <algorithm>
#include <vector>

namespace QuantLib {

    namespace {

        // Holidays that cannot be derived from a rule: the Islamic and
        // Chinese festivals, the additional special (non-working) days
        // proclaimed in a given year only, election days, and the days on
        // which the exchange itself was closed.
        bool isTabulatedPhilippineHoliday(const Date& date) {
            static const std::vector<Date> holidays = {
                // 2020
                Date(25, January, 2020),  // Chinese New Year
                Date(25, February, 2020), // EDSA People Power Revolution Anniversary
                Date(17, March, 2020),    // COVID-19 trading suspension
                Date(18, March, 2020),    // Market Holiday
                Date(25, May, 2020),      // Eid'l Fitr
                Date(31, July, 2020),     // Eid'l Adha
                Date(2, November, 2020),  // All Souls' Day
                Date(12, November, 2020), // No Trading
                Date(24, December, 2020), // Christmas Eve
                // 2021
                Date(12, February, 2021), // Chinese New Year
                Date(25, February, 2021), // EDSA People Power Revolution Anniversary
                Date(13, May, 2021),      // Eid'l Fitr
                Date(20, July, 2021),     // Eid'l Adha
                // 2022
                Date(1, February, 2022),   // Chinese New Year
                Date(25, February, 2022),  // EDSA People Power Revolution Anniversary
                Date(3, May, 2022),        // Eid'l Fitr
                Date(9, May, 2022),        // National and local elections
                Date(10, July, 2022),      // Eid'l Adha
                Date(26, September, 2022), // No Trading
                Date(31, October, 2022),   // Additional special (non-working) day
                Date(26, December, 2022),  // Additional special (non-working) day
                // 2023
                Date(2, January, 2023),   // Additional special (non-working) day
                Date(24, February, 2023), // EDSA anniversary, in lieu of February 25th
                Date(10, April, 2023),    // Araw ng Kagitingan, in lieu of April 9th
                Date(21, April, 2023),    // Eid'l Fitr
                Date(28, June, 2023),     // Eid'l Adha
                Date(30, October, 2023),  // Barangay and Sangguniang Kabataan elections
                Date(2, November, 2023),  // All Souls' Day
                Date(27, November, 2023), // Bonifacio Day, in lieu of November 30th
                Date(26, December, 2023), // Additional special (non-working) day
                // 2024
                Date(9, February, 2024),  // Additional special (non-working) day
                Date(10, February, 2024), // Chinese New Year
                Date(10, April, 2024),    // Eid'l Fitr
                Date(17, June, 2024),     // Eid'l Adha
                Date(24, July, 2024),     // Closed - Typhoon
                Date(23, August, 2024),   // Ninoy Aquino Day, in lieu of August 21st
                Date(2, November, 2024),  // All Souls' Day
                Date(24, December, 2024), // Christmas Eve
                // 2025
                Date(29, January, 2025),  // Chinese New Year
                Date(1, April, 2025),     // Eid'l Fitr
                Date(12, May, 2025),      // National and local elections
                Date(6, June, 2025),      // Eid'l Adha
                Date(27, July, 2025),     // Founding anniversary of the Iglesia ni Cristo
                Date(31, October, 2025),  // Additional special (non-working) day
                Date(24, December, 2025), // Christmas Eve
                // 2026
                Date(17, February, 2026), // Chinese New Year
                Date(20, March, 2026),    // Eid'l Fitr
                Date(27, May, 2026),      // Eid'l Adha
                Date(2, November, 2026),  // All Souls' Day
                Date(24, December, 2026), // Christmas Eve
                // 2027
                Date(6, February, 2027), // Chinese New Year
                Date(10, March, 2027),   // Eid'l Fitr
                Date(17, May, 2027),     // Eid'l Adha
            };
            return std::binary_search(holidays.begin(), holidays.end(), date);
        }

        // Rule-based holidays that a proclamation moved to another date or
        // turned into a special working day for one year only.
        bool isWithdrawnPhilippineHoliday(const Date& date) {
            static const std::vector<Date> withdrawn = {
                Date(31, December, 2021), // declared a special working day
                Date(30, November, 2023), // Bonifacio Day moved to November 27th
                Date(21, August, 2024),   // Ninoy Aquino Day moved to August 23rd
            };
            return std::binary_search(withdrawn.begin(), withdrawn.end(), date);
        }

    }

    Philippines::Philippines(Market) {
        // all calendar instances share the same implementation instance
        static ext::shared_ptr<Calendar::Impl> impl(new Philippines::PseImpl);
        impl_ = impl;
    }

    bool Philippines::PseImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);

        if (isWeekend(w))
            return false;

        if (isWithdrawnPhilippineHoliday(date))
            return true;

        if ( // New Year's Day
            (d == 1 && m == January)
            // Maundy Thursday
            || (dd == em - 4)
            // Good Friday
            || (dd == em - 3)
            // Araw ng Kagitingan (Day of Valor)
            || (d == 9 && m == April)
            // Labor Day
            || (d == 1 && m == May)
            // Independence Day
            || (d == 12 && m == June)
            // Ninoy Aquino Day (a special non-working day since RA 9256)
            || (d == 21 && m == August && y >= 2004)
            // National Heroes Day (the last Monday of August)
            || (d >= 25 && m == August && w == Monday)
            // All Saints' Day
            || (d == 1 && m == November)
            // Bonifacio Day
            || (d == 30 && m == November)
            // Feast of the Immaculate Conception (since RA 10966)
            || (d == 8 && m == December && y >= 2018)
            // Christmas Day
            || (d == 25 && m == December)
            // Rizal Day
            || (d == 30 && m == December)
            // Last Day of the Year
            || (d == 31 && m == December))
            return false;

        return !isTabulatedPhilippineHoliday(date);
    }

}
