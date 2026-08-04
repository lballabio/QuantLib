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

#include <ql/time/calendars/malaysia.hpp>
#include <algorithm>
#include <vector>

namespace QuantLib {

    namespace {

        // Holidays that cannot be derived from a rule: the lunar (Chinese and
        // Hindu) and Islamic festivals, the substitute days granted when a
        // gazetted holiday falls on a Sunday, and one-off federal closures.
        bool isTabulatedHoliday(const Date& date) {
            static const std::vector<Date> holidays = {
                // 2015
                Date(3, January, 2015),    // Maulidur Rasul
                Date(2, February, 2015),   // Federal Territory Day
                Date(3, February, 2015),   // Thaipusam
                Date(19, February, 2015),  // Chinese New Year
                Date(20, February, 2015),  // Chinese New Year
                Date(4, May, 2015),        // Wesak Day
                Date(6, June, 2015),       // Agong's Birthday
                Date(4, July, 2015),       // Nuzul Al-Quran
                Date(17, July, 2015),      // Hari Raya Aidilfitri
                Date(18, July, 2015),      // Hari Raya Aidilfitri
                Date(24, September, 2015), // Hari Raya Haji
                Date(14, October, 2015),   // Awal Muharram
                Date(10, November, 2015),  // Deepavali
                Date(24, December, 2015),  // Maulidur Rasul
                // 2016
                Date(24, January, 2016),   // Thaipusam
                Date(25, January, 2016),   // Thaipusam
                Date(8, February, 2016),   // Chinese New Year
                Date(9, February, 2016),   // Chinese New Year
                Date(2, May, 2016),        // Labour Day
                Date(21, May, 2016),       // Wesak Day
                Date(4, June, 2016),       // Agong's Birthday
                Date(22, June, 2016),      // Nuzul Al-Quran
                Date(6, July, 2016),       // Hari Raya Aidilfitri
                Date(7, July, 2016),       // Hari Raya Aidilfitri
                Date(12, September, 2016), // Hari Raya Haji
                Date(2, October, 2016),    // Awal Muharram
                Date(3, October, 2016),    // Awal Muharram
                Date(29, October, 2016),   // Deepavali
                Date(12, December, 2016),  // Maulidur Rasul
                Date(26, December, 2016),  // Christmas Day (in lieu)
                // 2017
                Date(2, January, 2017),    // New Year's Day (in lieu)
                Date(28, January, 2017),   // Chinese New Year
                Date(29, January, 2017),   // Chinese New Year
                Date(30, January, 2017),   // Chinese New Year
                Date(9, February, 2017),   // Thaipusam
                Date(11, February, 2017),  // Thaipusam
                Date(24, April, 2017),     // Installation of the 15th Yang di-Pertuan Agong
                Date(10, May, 2017),       // Wesak Day
                Date(3, June, 2017),       // Agong's Birthday
                Date(11, June, 2017),      // Nuzul Al-Quran
                Date(12, June, 2017),      // Nuzul Al-Quran
                Date(25, June, 2017),      // Hari Raya Aidilfitri
                Date(26, June, 2017),      // Hari Raya Aidilfitri
                Date(27, June, 2017),      // Hari Raya Aidilfitri
                Date(1, September, 2017),  // Hari Raya Haji
                Date(4, September, 2017),  // Special holiday, 29th SEA Games
                Date(22, September, 2017), // Awal Muharram
                Date(18, October, 2017),   // Deepavali
                Date(1, December, 2017),   // Maulidur Rasul
                // 2018
                Date(31, January, 2018),   // Thaipusam
                Date(16, February, 2018),  // Chinese New Year
                Date(17, February, 2018),  // Chinese New Year
                Date(9, May, 2018),        // GE14 polling day
                Date(10, May, 2018),       // Special Holiday
                Date(11, May, 2018),       // Special Holiday
                Date(29, May, 2018),       // Wesak Day
                Date(2, June, 2018),       // Nuzul Al-Quran
                Date(15, June, 2018),      // Hari Raya Aidilfitri
                Date(16, June, 2018),      // Hari Raya Aidilfitri
                Date(22, August, 2018),    // Hari Raya Haji
                Date(10, September, 2018), // Agong's Birthday
                Date(11, September, 2018), // Awal Muharram
                Date(17, September, 2018), // Malaysia Day (in lieu)
                Date(6, November, 2018),   // Deepavali
                Date(20, November, 2018),  // Maulidur Rasul
                // 2019
                Date(21, January, 2019),  // Thaipusam
                Date(5, February, 2019),  // Chinese New Year
                Date(6, February, 2019),  // Chinese New Year
                Date(20, May, 2019),      // Wesak Day
                Date(22, May, 2019),      // Nuzul Al-Quran
                Date(5, June, 2019),      // Hari Raya Aidilfitri
                Date(6, June, 2019),      // Hari Raya Aidilfitri (in lieu)
                Date(30, July, 2019),     // Installation of the 16th Yang di-Pertuan Agong
                Date(11, August, 2019),   // Hari Raya Haji
                Date(12, August, 2019),   // Hari Raya Haji (in lieu)
                Date(1, September, 2019), // Awal Muharram
                Date(2, September, 2019), // Awal Muharram (in lieu)
                Date(9, September, 2019), // Agong's Birthday
                Date(27, October, 2019),  // Deepavali
                Date(28, October, 2019),  // Deepavali (in lieu)
                Date(9, November, 2019),  // Maulidur Rasul
                // 2020
                Date(25, January, 2020),  // Chinese New Year
                Date(26, January, 2020),  // Chinese New Year
                Date(27, January, 2020),  // Chinese New Year
                Date(9, February, 2020),  // Thaipusam
                Date(7, May, 2020),       // Wesak Day
                Date(10, May, 2020),      // Nuzul Al-Quran
                Date(11, May, 2020),      // Nuzul Al-Quran (in lieu)
                Date(24, May, 2020),      // Hari Raya Aidilfitri
                Date(25, May, 2020),      // Hari Raya Aidilfitri (in lieu)
                Date(26, May, 2020),      // Hari Raya Aidilfitri (in lieu)
                Date(8, June, 2020),      // Agong's Birthday
                Date(31, July, 2020),     // Hari Raya Haji
                Date(20, August, 2020),   // Awal Muharram
                Date(29, October, 2020),  // Maulidur Rasul
                Date(14, November, 2020), // Deepavali
                // 2021
                Date(28, January, 2021),  // Thaipusam
                Date(12, February, 2021), // Chinese New Year
                Date(13, February, 2021), // Chinese New Year
                Date(29, April, 2021),    // Nuzul Al-Quran
                Date(13, May, 2021),      // Hari Raya Aidilfitri
                Date(14, May, 2021),      // Hari Raya Aidilfitri (in lieu)
                Date(26, May, 2021),      // Wesak Day
                Date(7, June, 2021),      // Agong's Birthday
                Date(20, July, 2021),     // Hari Raya Haji
                Date(10, August, 2021),   // Awal Muharram
                Date(19, October, 2021),  // Maulidur Rasul
                Date(4, November, 2021),  // Deepavali
                Date(3, December, 2021),  // Special Holiday (Federal Territories)
                // 2022
                Date(18, January, 2022),  // Thaipusam
                Date(1, February, 2022),  // Chinese New Year
                Date(2, February, 2022),  // Chinese New Year
                Date(3, February, 2022),  // Federal Territory Day (in lieu)
                Date(19, April, 2022),    // Nuzul Al-Quran
                Date(2, May, 2022),       // Hari Raya Aidilfitri
                Date(3, May, 2022),       // Hari Raya Aidilfitri (in lieu)
                Date(4, May, 2022),       // Labour Day (in lieu)
                Date(15, May, 2022),      // Wesak Day
                Date(16, May, 2022),      // Wesak Day (in lieu)
                Date(6, June, 2022),      // Agong's Birthday
                Date(10, July, 2022),     // Hari Raya Haji
                Date(11, July, 2022),     // Hari Raya Haji (in lieu)
                Date(30, July, 2022),     // Awal Muharram
                Date(9, October, 2022),   // Maulidur Rasul
                Date(10, October, 2022),  // Maulidur Rasul (in lieu)
                Date(24, October, 2022),  // Deepavali
                Date(18, November, 2022), // Special days
                Date(28, November, 2022), // Special Bank Holiday
                Date(26, December, 2022), // Christmas Day (in lieu)
                // 2023
                Date(2, January, 2023),    // New Year's Day (in lieu)
                Date(22, January, 2023),   // Chinese New Year
                Date(23, January, 2023),   // Chinese New Year
                Date(24, January, 2023),   // Chinese New Year
                Date(5, February, 2023),   // Thaipusam
                Date(6, February, 2023),   // Thaipusam (in lieu)
                Date(8, April, 2023),      // Nuzul Al-Quran
                Date(21, April, 2023),     // Hari Raya Aidilfitri (in lieu)
                Date(22, April, 2023),     // Hari Raya Aidilfitri
                Date(23, April, 2023),     // Hari Raya Aidilfitri (in lieu)
                Date(24, April, 2023),     // Hari Raya Aidilfitri (in lieu)
                Date(4, May, 2023),        // Wesak Day
                Date(5, June, 2023),       // Agong's Birthday
                Date(29, June, 2023),      // Hari Raya Haji
                Date(19, July, 2023),      // Awal Muharram
                Date(28, September, 2023), // Maulidur Rasul
                Date(12, November, 2023),  // Deepavali
                Date(13, November, 2023),  // Deepavali (in lieu)
                // 2024
                Date(25, January, 2024),   // Thaipusam
                Date(10, February, 2024),  // Chinese New Year
                Date(11, February, 2024),  // Chinese New Year
                Date(12, February, 2024),  // Chinese New Year
                Date(28, March, 2024),     // Nuzul Al-Quran
                Date(10, April, 2024),     // Hari Raya Aidilfitri
                Date(11, April, 2024),     // Hari Raya Aidilfitri
                Date(22, May, 2024),       // Wesak Day
                Date(3, June, 2024),       // Agong's Birthday
                Date(17, June, 2024),      // Hari Raya Haji
                Date(7, July, 2024),       // Awal Muharram
                Date(8, July, 2024),       // Awal Muharram
                Date(16, September, 2024), // Maulidur Rasul
                Date(31, October, 2024),   // Deepavali
                // 2025
                Date(29, January, 2025),   // Chinese New Year
                Date(30, January, 2025),   // Chinese New Year
                Date(11, February, 2025),  // Thaipusam
                Date(18, March, 2025),     // Nuzul Al-Quran
                Date(30, March, 2025),     // Hari Raya Aidilfitri (in lieu)
                Date(31, March, 2025),     // Hari Raya Aidilfitri
                Date(1, April, 2025),      // Hari Raya Aidilfitri (in lieu)
                Date(12, May, 2025),       // Wesak Day
                Date(2, June, 2025),       // Agong's Birthday
                Date(7, June, 2025),       // Hari Raya Haji
                Date(27, June, 2025),      // Awal Muharram
                Date(1, September, 2025),  // National Day (in lieu)
                Date(5, September, 2025),  // Maulidur Rasul
                Date(15, September, 2025), // Malaysia Day (in lieu)
                Date(20, October, 2025),   // Deepavali
                // 2026
                Date(1, February, 2026),  // Thaipusam
                Date(2, February, 2026),  // Thaipusam (in lieu)
                Date(17, February, 2026), // Chinese New Year
                Date(18, February, 2026), // Chinese New Year
                Date(7, March, 2026),     // Nuzul Al-Quran
                Date(20, March, 2026),    // Hari Raya Aidilfitri (in lieu)
                Date(21, March, 2026),    // Hari Raya Aidilfitri
                Date(22, March, 2026),    // Hari Raya Aidilfitri (in lieu)
                Date(23, March, 2026),    // Hari Raya Aidilfitri (in lieu)
                Date(27, May, 2026),      // Hari Raya Haji
                Date(31, May, 2026),      // Wesak Day
                Date(1, June, 2026),      // Agong's Birthday
                Date(17, June, 2026),     // Awal Muharram
                Date(25, August, 2026),   // Maulidur Rasul
                Date(8, November, 2026),  // Deepavali
                Date(9, November, 2026),  // Deepavali (in lieu)
                // 2027
                Date(22, January, 2027),  // Thaipusam
                Date(6, February, 2027),  // Chinese New Year
                Date(7, February, 2027),  // Chinese New Year
                Date(24, February, 2027), // Nuzul Al-Quran
                Date(10, March, 2027),    // Hari Raya Aidilfitri
                Date(11, March, 2027),    // Hari Raya Aidilfitri (in lieu)
                Date(17, May, 2027),      // Hari Raya Haji (in lieu)
                Date(20, May, 2027),      // Wesak Day
                Date(5, June, 2027),      // Agong's Birthday
                Date(6, June, 2027),      // Awal Muharram
                Date(15, August, 2027),   // Maulidur Rasul
                Date(30, October, 2027),  // Deepavali
            };
            return std::binary_search(holidays.begin(), holidays.end(), date);
        }

    }

    Malaysia::Malaysia(Market) {
        // all calendar instances share the same implementation instance
        static ext::shared_ptr<Calendar::Impl> impl(new Malaysia::KlseImpl);
        impl_ = impl;
    }

    bool Malaysia::KlseImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();

        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Federal Territory Day
            || (d == 1 && m == February)
            // Labour Day
            || (d == 1 && m == May)
            // National Day
            || (d == 31 && m == August)
            // Malaysia Day (a federal holiday since 2010)
            || (d == 16 && m == September && y >= 2010)
            // Christmas Day
            || (d == 25 && m == December))
            return false;

        return !isTabulatedHoliday(date);
    }

}
