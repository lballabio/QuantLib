/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Paolo Mazzocchi
 Copyright (C) 2015 Riccardo Barone

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

#include <ql/time/calendars/israel.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    namespace {

        bool isPurim(const Date& d) {
            static std::set<Date> knownDates = {
                {21, March,    2000},
                {9,  March,    2001},
                {26, February, 2002},
                {18, March,    2003},
                {7,  March,    2004},
                {25, March,    2005},
                {14, March,    2006},
                {4,  March,    2007},
                {21, March,    2008},
                {10, March,    2009},
                {28, February, 2010},
                {20, March,    2011},
                {8,  March,    2012},
                {24, February, 2013},
                {16, March,    2014},
                {05, March,    2015},
                {24, March,    2016},
                {12, March,    2017},
                {1,  March,    2018},
                {21, March,    2019},
                {10, March,    2020},
                {26, February, 2021},
                {17, March,    2022},
                {7,  March,    2023},
                {24, March,    2024},
                {14, March,    2025},
                {3,  March,    2026},
                {23, March,    2027},
                {12, March,    2028},
                {1,  March,    2029},
                {19, March,    2030},
                {9,  March,    2031},
                {26, February, 2032},
                {15, March,    2033},
                {5,  March,    2034},
                {25, March,    2035},
                {13, March,    2036},
                {1,  March,    2037},
                {21, March,    2038},
                {10, March,    2039},
                {28, February, 2040},
                {17, March,    2041},
                {6,  March,    2042},
                {26, March,    2043},
                {13, March,    2044},
                {3,  March,    2045},
                {22, March,    2046},
                {12, March,    2047},
                {28, February, 2048},
                {18, March,    2049},
                {8,  March,    2050},
            };
            return knownDates.find(d) != knownDates.end();
        }

        bool isPassover1st(const Date& d) {
            static std::set<Date> knownDates = {
                {20, April, 2000},
                {8,  April, 2001},
                {28, March, 2002},
                {17, April, 2003},
                {6,  April, 2004},
                {24, April, 2005},
                {13, April, 2006},
                {3,  April, 2007},
                {20, April, 2008},
                {9,  April, 2009},
                {30, March, 2010},
                {19, April, 2011},
                {7,  April, 2012},
                {26, March, 2013},
                {15, April, 2014},
                {4,  April, 2015},
                {23, April, 2016},
                {11, April, 2017},
                {31, March, 2018},
                {20, April, 2019},
                {9,  April, 2020},
                {28, March, 2021},
                {16, April, 2022},
                {6 , April, 2023},
                {23, April, 2024},
                {13, April, 2025},
                {2 , April, 2026},
                {22, April, 2027},
                {11, April, 2028},
                {31, March, 2029},
                {18, April, 2030},
                {8,  April, 2031},
                {27, March, 2032},
                {14, April, 2033},
                {4,  April, 2034},
                {24, April, 2035},
                {12, April, 2036},
                {31, March, 2037},
                {20, April, 2038},
                {9,  April, 2039},
                {29, March, 2040},
                {16, April, 2041},
                {5,  April, 2042},
                {25, April, 2043},
                {12, April, 2044},
                {2,  April, 2045},
                {21, April, 2046},
                {11, April, 2047},
                {29, March, 2048},
                {17, April, 2049},
                {7,  April, 2050},
            };
            return knownDates.find(d) != knownDates.end();
        }

        bool isIndependenceDay(const Date& d) {
            static std::set<Date> knownDates = {
                {10, May,   2000},
                {26, April, 2001},
                {17, April, 2002},
                {7,  May,   2003},
                {27, April, 2004},
                {12, May,   2005},
                {3,  May,   2006},
                {24, April, 2007},
                {8,  May,   2008},
                {29, April, 2009},
                {20, April, 2010},
                {10, May,   2011},
                {26, April, 2012},
                {16, April, 2013},
                {6,  May,   2014},
                {23, April, 2015},
                {12, May,   2016},
                {2,  May,   2017},
                {19, April, 2018},
                {9,  May,   2019},
                {29, April, 2020},
                {15, April, 2021},
                {5,  May,   2022},
                {26, April, 2023},
                {14, May,   2024},
                {1,  May,   2025},
                {22, April, 2026},
                {12, May,   2027},
                {2,  May,   2028},
                {19, April, 2029},
                {8,  May,   2030},
                {29, April, 2031},
                {15, April, 2032},
                {4,  May,   2033},
                {25, April, 2034},
                {15, May,   2035},
                {1,  May,   2036},
                {21, April, 2037},
                {10, May,   2038},
                {28, April, 2039},
                {18, April, 2040},
                {7,  May,   2041},
                {24, April, 2042},
                {14, May,   2043},
                {3,  May,   2044},
                {20, April, 2045},
                {10, May,   2046},
                {1,  May,   2047},
                {16, April, 2048},
                {6,  May,   2049},
                {27, April, 2050},
            };
            return knownDates.find(d) != knownDates.end();
        }

        bool isMemorialDay(const Date& d) {
            return isIndependenceDay(d+1);
        }

        bool isShavuot(const Date& d) {
            static std::set<Date> knownDates = {
                {9,  June, 2000},
                {28, May,  2001},
                {17, May,  2002},
                {6,  June, 2003},
                {26, May,  2004},
                {13, June, 2005},
                {2,  June, 2006},
                {23, May,  2007},
                {9,  June, 2008},
                {29, May,  2009},
                {19, May,  2010},
                {8,  June, 2011},
                {27, May,  2012},
                {15, May,  2013},
                {4,  June, 2014},
                {24, May,  2015},
                {12, June, 2016},
                {31, May,  2017},
                {20, May,  2018},
                {9,  June, 2019},
                {29, May,  2020},
                {17, May,  2021},
                {5,  June, 2022},
                {26, May,  2023},
                {12, June, 2024},
                {2,  June, 2025},
                {22, May,  2026},
                {11, June, 2027},
                {31, May,  2028},
                {20, May,  2029},
                {7,  June, 2030},
                {28, May,  2031},
                {16, May,  2032},
                {3,  June, 2033},
                {24, May,  2034},
                {13, June, 2035},
                {1,  June, 2036},
                {20, May,  2037},
                {9,  June, 2038},
                {29, May,  2039},
                {18, May,  2040},
                {5,  June, 2041},
                {25, May,  2042},
                {14, June, 2043},
                {1,  June, 2044},
                {22, May,  2045},
                {10, June, 2046},
                {31, May,  2047},
                {18, May,  2048},
                {6,  June, 2049},
                {27, May,  2050},
            };
            return knownDates.find(d) != knownDates.end();
        }

        bool isFastDay(const Date& d) {
            static std::set<Date> knownDates = {
                {10, August, 2000},
                {29, July,   2001},
                {18, July,   2002},
                {7,  August, 2003},
                {27, July,   2004},
                {14, August, 2005},
                {3,  August, 2006},
                {24, July,   2007},
                {10, August, 2008},
                {30, July,   2009},
                {20, July,   2010},
                {9,  August, 2011},
                {29, July,   2012},
                {16, July,   2013},
                {5,  August, 2014},
                {26, July,   2015},
                {14, August, 2016},
                {1,  August, 2017},
                {22, July,   2018},
                {11, August, 2019},
                {30, July,   2020},
                {18, July,   2021},
                {7,  August, 2022},
                {27, July,   2023},
                {13, August, 2024},
                {3,  August, 2025},
                {23, July,   2026},
                {12, August, 2027},
                {1,  August, 2028},
                {22, July,   2029},
                {8,  August, 2030},
                {29, July,   2031},
                {18, July,   2032},
                {4,  August, 2033},
                {25, July,   2034},
                {14, August, 2035},
                {3,  August, 2036},
                {21, July,   2037},
                {10, August, 2038},
                {31, July,   2039},
                {19, July,   2040},
                {6,  August, 2041},
                {27, July,   2042},
                {16, August, 2043},
                {2,  August, 2044},
                {23, July,   2045},
                {12, August, 2046},
                {1,  August, 2047},
                {19, July,   2048},
                {8,  August, 2049},
                {28, July,   2050},
            };
            return knownDates.find(d) != knownDates.end();
        }

        bool isNewYearsDay(const Date& d) {
            static std::set<Date> knownDates = {
                {30, September, 2000},
                {17, September, 2001},
                {7,  September, 2002},
                {27, September, 2003},
                {16, September, 2004},
                {4,  October,   2005},
                {23, September, 2006},
                {13, September, 2007},
                {30, September, 2008},
                {19, September, 2009},
                {9,  September, 2010},
                {29, September, 2011},
                {17, September, 2012},
                {5,  September, 2013},
                {25, September, 2014},
                {14, September, 2015},
                {3,  October,   2016},
                {21, September, 2017},
                {10, September, 2018},
                {30, September, 2019},
                {19, September, 2020},
                {7,  September, 2021},
                {26, September, 2022},
                {16, September, 2023},
                {3,  October,   2024},
                {23, September, 2025},
                {12, September, 2026},
                {2,  October,   2027},
                {21, September, 2028},
                {10, September, 2029},
                {28, September, 2030},
                {18, September, 2031},
                {6,  September, 2032},
                {24, September, 2033},
                {14, September, 2034},
                {4,  October,   2035},
                {22, September, 2036},
                {10, September, 2037},
                {30, September, 2038},
                {19, September, 2039},
                {8,  September, 2040},
                {26, September, 2041},
                {15, September, 2042},
                {5,  October,   2043},
                {22, September, 2044},
                {12, September, 2045},
                {1,  October,   2046},
                {21, September, 2047},
                {8,  September, 2048},
                {27, September, 2049},
                {17, September, 2050},
            };
            return knownDates.find(d) != knownDates.end();
        }

        bool isYomKippur(const Date& d) {
            return isNewYearsDay(d-9);
        }

        bool isSukkot(const Date& d) {
            return isYomKippur(d-5);
        }

        bool isSimchatTorah(const Date& d) {
            return isSukkot(d-7);
        }

    }


    class Israel::TelAvivImpl final : public Calendar::Impl {
      public:
        std::string name() const override { return "Tel Aviv stock exchange"; }
        bool isWeekend(Weekday) const override;
        bool isBusinessDay(const Date&) const override;
    };

    class Israel::ShirImpl final : public Calendar::WesternImpl {
      public:
        std::string name() const override { return "SHIR fixing calendar"; }
        bool isBusinessDay(const Date&) const override;
    };

    Israel::Israel(Israel::Market market) {
        // all calendar instances share the same implementation instance
        static auto telAvivImpl = ext::make_shared<Israel::TelAvivImpl>();
        static auto shirImpl = ext::make_shared<Israel::ShirImpl>();
        switch (market) {
        case Settlement:
        case TASE:
            impl_ = telAvivImpl;
            break;
        case SHIR:
            impl_ = shirImpl;
            break;
        default:
            QL_FAIL("unknown market");
        }
    }

    bool Israel::TelAvivImpl::isWeekend(Weekday w) const {
        return w == Friday || w == Saturday;
    }

    bool Israel::TelAvivImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Year y = date.year();

        if (isWeekend(w)
            || isPurim(date)
            || (y <= 2020 && isPassover1st(date+1)) // Eve of Passover, until 2020
            || isPassover1st(date)
            || isPassover1st(date-5) // Eve of Passover VII, until 2020
            || isPassover1st(date-6) // Passover VII
            || isMemorialDay(date)
            || isIndependenceDay(date)
            || (y <= 2020 && isShavuot(date+1)) // Eve of Shavuot, until 2020
            || isShavuot(date)
            || isFastDay(date)
            || (y <= 2019 && isNewYearsDay(date+1))  // Eve of new year, until 2019
            || isNewYearsDay(date)
            || isNewYearsDay(date-1)  // 2nd day of new year
            || isYomKippur(date+1) // Eve of Yom Kippur
            || isYomKippur(date)
            || isSukkot(date+1)  // Eve of Sukkot
            || isSukkot(date)
            || isSimchatTorah(date+1)  // Eve of Simchat Torah
            || isSimchatTorah(date))
            return false; // NOLINT(readability-simplify-boolean-expr)

        return true;
    }

    bool Israel::ShirImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Day dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();

        if (isWeekend(w)
            || isPurim(date)
            || isPurim(date-1) // Purim (Jerusalem)
            || isPassover1st(date+1) // Eve of Passover
            || isPassover1st(date)
            || isPassover1st(date-6) // Last day of Passover
            || isIndependenceDay(date)
            || isShavuot(date)
            || isFastDay(date)
            || isNewYearsDay(date+1)  // Eve of new year, until 2019
            || isNewYearsDay(date)
            || isNewYearsDay(date-1)  // 2nd day of new year
            || isYomKippur(date+1) // Eve of Yom Kippur
            || isYomKippur(date)
            || isSukkot(date)
            || isSimchatTorah(date)
            // one-off closings
            || (d == 27 && m == February && y == 2024) // Municipal elections
            // holidays abroad
            || (d == 1 && m == January)  // Western New Year's day
            || dd == easterMonday(y) - 3  // Good Friday
            || (d >= 25 && w == Monday && m == May && y != 2022) // Spring Bank Holiday
            || (d == 3 && m == June && y == 2022)
            || (d == 25 && m == December)  // Christmas
            || (d == 26 && m == December)  // Boxing day
            // other days when fixings were not published
            || (d == 1 && m == November && y == 2022) // no idea why
            || (d == 2 && m == January && y == 2023) // Maybe New Year's Day is adjusted to Monday?
            || (d == 10 && m == April && y == 2023) // Easter Monday, not a holiday in 2024 and 2025
            )
            return false; // NOLINT(readability-simplify-boolean-expr)

        return true;
    }

}
