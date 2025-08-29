/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2004 Jeff Yu
 Copyright (C) 2014 Paolo Mazzocchi
 Copyright (C) 2020 Leonardo Arcari
 Copyright (C) 2020 Kline s.r.l.

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

#include <ql/time/calendar.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    namespace {

        // Requires: from < to.
        Date::serial_type daysBetweenImpl(const Calendar& cal,
                                          const Date& from, const Date& to,
                                          bool includeFirst, bool includeLast) {
            auto res = static_cast<Date::serial_type>(includeLast && cal.isBusinessDay(to));
            for (Date d = includeFirst ? from : from + 1; d < to; ++d) {
                res += static_cast<Date::serial_type>(cal.isBusinessDay(d));
            }
            return res;
        }

    }

    void Calendar::addHoliday(const Date& d) {
        QL_REQUIRE(impl_, "no calendar implementation provided");

#ifdef QL_HIGH_RESOLUTION_DATE
        const Date _d(d.dayOfMonth(), d.month(), d.year());
#else
        const Date& _d = d;
#endif

        // if d was a genuine holiday previously removed, revert the change
        impl_->removedHolidays.erase(_d);
        // if it's already a holiday, leave the calendar alone.
        // Otherwise, add it.
        if (impl_->isBusinessDay(_d))
            impl_->addedHolidays.insert(_d);
    }

    void Calendar::removeHoliday(const Date& d) {
        QL_REQUIRE(impl_, "no calendar implementation provided");

#ifdef QL_HIGH_RESOLUTION_DATE
        const Date _d(d.dayOfMonth(), d.month(), d.year());
#else
        const Date& _d = d;
#endif

        // if d was an artificially-added holiday, revert the change
        impl_->addedHolidays.erase(_d);
        // if it's already a business day, leave the calendar alone.
        // Otherwise, add it.
        if (!impl_->isBusinessDay(_d))
            impl_->removedHolidays.insert(_d);
    }

    void Calendar::resetAddedAndRemovedHolidays() {
        impl_->addedHolidays.clear();
        impl_->removedHolidays.clear();
    }

    Date Calendar::adjust(const Date& d,
                          BusinessDayConvention c) const {
        QL_REQUIRE(d != Date(), "null date");

        if (c == Unadjusted)
            return d;

        Date d1 = d;
        if (c == Following || c == ModifiedFollowing 
            || c == HalfMonthModifiedFollowing) {
            while (isHoliday(d1))
                ++d1;
            if (c == ModifiedFollowing 
                || c == HalfMonthModifiedFollowing) {
                if (d1.month() != d.month()) {
                    return adjust(d, Preceding);
                }
                if (c == HalfMonthModifiedFollowing) {
                    if (d.dayOfMonth() <= 15 && d1.dayOfMonth() > 15) {
                        return adjust(d, Preceding);
                    }
                }
            }
        } else if (c == Preceding || c == ModifiedPreceding) {
            while (isHoliday(d1))
                --d1;
            if (c == ModifiedPreceding && d1.month() != d.month()) {
                return adjust(d,Following);
            }
        } else if (c == Nearest) {
            Date d2 = d;
            while (isHoliday(d1) && isHoliday(d2))
            {
                ++d1;
                --d2;
            }
            if (isHoliday(d1))
                return d2;
            else
                return d1;
        } else {
            QL_FAIL("unknown business-day convention");
        }
        return d1;
    }

    Date Calendar::advance(const Date& d,
                           Integer n, TimeUnit unit,
                           BusinessDayConvention c,
                           bool endOfMonth) const {
        QL_REQUIRE(d!=Date(), "null date");
        if (n == 0) {
            return adjust(d,c);
        } else if (unit == Days) {
            Date d1 = d;
            if (n > 0) {
                while (n > 0) {
                    ++d1;
                    while (isHoliday(d1))
                        ++d1;
                    --n;
                }
            } else {
                while (n < 0) {
                    --d1;
                    while(isHoliday(d1))
                        --d1;
                    ++n;
                }
            }
            return d1;
        } else if (unit == Weeks) {
            Date d1 = d + n*unit;
            return adjust(d1,c);
        } else {
            Date d1 = d + n*unit;

            // we are sure the unit is Months or Years
            if (endOfMonth) {
                if (c == Unadjusted) {
                    // move to the last calendar day if d is the last calendar day
                    if (Date::isEndOfMonth(d)) return Date::endOfMonth(d1);
                } else {
                    // move to the last business day if d is the last business day
                    if (isEndOfMonth(d)) return Calendar::endOfMonth(d1);
                }
            }
            return adjust(d1, c);
        }
    }

    Date Calendar::advance(const Date & d,
                           const Period & p,
                           BusinessDayConvention c,
                           bool endOfMonth) const {
        return advance(d, p.length(), p.units(), c, endOfMonth);
    }

    Date::serial_type Calendar::businessDaysBetween(const Date& from,
                                                    const Date& to,
                                                    bool includeFirst,
                                                    bool includeLast) const {
        return (from < to) ? daysBetweenImpl(*this, from, to, includeFirst, includeLast) :
               (from > to) ? -daysBetweenImpl(*this, to, from, includeLast, includeFirst) :
               Date::serial_type(includeFirst && includeLast && isBusinessDay(from));
    }



   // Western calendars

    bool Calendar::WesternImpl::isWeekend(Weekday w) const {
        return w == Saturday || w == Sunday;
    }

    Day Calendar::WesternImpl::easterMonday(Year y) {
        static const unsigned char EasterMonday[] = {
                  98,  90, 103,  95, 114, 106,  91, 111, 102,   // 1901-1909
             87, 107,  99,  83, 103,  95, 115,  99,  91, 111,   // 1910-1919
             96,  87, 107,  92, 112, 103,  95, 108, 100,  91,   // 1920-1929
            111,  96,  88, 107,  92, 112, 104,  88, 108, 100,   // 1930-1939
             85, 104,  96, 116, 101,  92, 112,  97,  89, 108,   // 1940-1949
            100,  85, 105,  96, 109, 101,  93, 112,  97,  89,   // 1950-1959
            109,  93, 113, 105,  90, 109, 101,  86, 106,  97,   // 1960-1969
             89, 102,  94, 113, 105,  90, 110, 101,  86, 106,   // 1970-1979
             98, 110, 102,  94, 114,  98,  90, 110,  95,  86,   // 1980-1989
            106,  91, 111, 102,  94, 107,  99,  90, 103,  95,   // 1990-1999
            115, 106,  91, 111, 103,  87, 107,  99,  84, 103,   // 2000-2009
             95, 115, 100,  91, 111,  96,  88, 107,  92, 112,   // 2010-2019
            104,  95, 108, 100,  92, 111,  96,  88, 108,  92,   // 2020-2029
            112, 104,  89, 108, 100,  85, 105,  96, 116, 101,   // 2030-2039
             93, 112,  97,  89, 109, 100,  85, 105,  97, 109,   // 2040-2049
            101,  93, 113,  97,  89, 109,  94, 113, 105,  90,   // 2050-2059
            110, 101,  86, 106,  98,  89, 102,  94, 114, 105,   // 2060-2069
             90, 110, 102,  86, 106,  98, 111, 102,  94, 114,   // 2070-2079
             99,  90, 110,  95,  87, 106,  91, 111, 103,  94,   // 2080-2089
            107,  99,  91, 103,  95, 115, 107,  91, 111, 103,   // 2090-2099
             88, 108, 100,  85, 105,  96, 109, 101,  93, 112,   // 2100-2109
             97,  89, 109,  93, 113, 105,  90, 109, 101,  86,   // 2110-2119
            106,  97,  89, 102,  94, 113, 105,  90, 110, 101,   // 2120-2129
             86, 106,  98, 110, 102,  94, 114,  98,  90, 110,   // 2130-2139
             95,  86, 106,  91, 111, 102,  94, 107,  99,  90,   // 2140-2149
            103,  95, 115, 106,  91, 111, 103,  87, 107,  99,   // 2150-2159
             84, 103,  95, 115, 100,  91, 111,  96,  88, 107,   // 2160-2169
             92, 112, 104,  95, 108, 100,  92, 111,  96,  88,   // 2170-2179
            108,  92, 112, 104,  89, 108, 100,  85, 105,  96,   // 2180-2189
            116, 101,  93, 112,  97,  89, 109, 100,  85, 105    // 2190-2199
        };
        return EasterMonday[y-1901];
    }

    // Orthodox calendars

    bool Calendar::OrthodoxImpl::isWeekend(Weekday w) const {
        return w == Saturday || w == Sunday;
    }

    Day Calendar::OrthodoxImpl::easterMonday(Year y) {
        static const unsigned char EasterMonday[] = {
                 105, 118, 110, 102, 121, 106, 126, 118, 102,   // 1901-1909
            122, 114,  99, 118, 110,  95, 115, 106, 126, 111,   // 1910-1919
            103, 122, 107,  99, 119, 110, 123, 115, 107, 126,   // 1920-1929
            111, 103, 123, 107,  99, 119, 104, 123, 115, 100,   // 1930-1939
            120, 111,  96, 116, 108, 127, 112, 104, 124, 115,   // 1940-1949
            100, 120, 112,  96, 116, 108, 128, 112, 104, 124,   // 1950-1959
            109, 100, 120, 105, 125, 116, 101, 121, 113, 104,   // 1960-1969
            117, 109, 101, 120, 105, 125, 117, 101, 121, 113,   // 1970-1979
             98, 117, 109, 129, 114, 105, 125, 110, 102, 121,   // 1980-1989
            106,  98, 118, 109, 122, 114, 106, 118, 110, 102,   // 1990-1999
            122, 106, 126, 118, 103, 122, 114,  99, 119, 110,   // 2000-2009
             95, 115, 107, 126, 111, 103, 123, 107,  99, 119,   // 2010-2019
            111, 123, 115, 107, 127, 111, 103, 123, 108,  99,   // 2020-2029
            119, 104, 124, 115, 100, 120, 112,  96, 116, 108,   // 2030-2039
            128, 112, 104, 124, 116, 100, 120, 112,  97, 116,   // 2040-2049
            108, 128, 113, 104, 124, 109, 101, 120, 105, 125,   // 2050-2059
            117, 101, 121, 113, 105, 117, 109, 101, 121, 105,   // 2060-2069
            125, 110, 102, 121, 113,  98, 118, 109, 129, 114,   // 2070-2079
            106, 125, 110, 102, 122, 106,  98, 118, 110, 122,   // 2080-2089
            114,  99, 119, 110, 102, 115, 107, 126, 118, 103,   // 2090-2099
            123, 115, 100, 120, 112,  96, 116, 108, 128, 112,   // 2100-2109
            104, 124, 109, 100, 120, 105, 125, 116, 108, 121,   // 2110-2119
            113, 104, 124, 109, 101, 120, 105, 125, 117, 101,   // 2120-2129
            121, 113,  98, 117, 109, 129, 114, 105, 125, 110,   // 2130-2139
            102, 121, 113,  98, 118, 109, 129, 114, 106, 125,   // 2140-2149
            110, 102, 122, 106, 126, 118, 103, 122, 114,  99,   // 2150-2159
            119, 110, 102, 115, 107, 126, 111, 103, 123, 114,   // 2160-2169
             99, 119, 111, 130, 115, 107, 127, 111, 103, 123,   // 2170-2179
            108,  99, 119, 104, 124, 115, 100, 120, 112, 103,   // 2180-2189
            116, 108, 128, 119, 104, 124, 116, 100, 120, 112    // 2190-2199
        };
        return EasterMonday[y-1901];
    }

    std::vector<Date> Calendar::holidayList(
        const Date& from, const Date& to, bool includeWeekEnds) const {

        QL_REQUIRE(to>=from, "'from' date ("
            << from << ") must be equal to or earlier than 'to' date ("
            << to << ")");
        std::vector<Date> result;
        for (Date d = from; d <= to; ++d) {
            if (isHoliday(d) && (includeWeekEnds || !isWeekend(d.weekday())))
                result.push_back(d);
       }
       return result;
    }

    std::vector<Date> Calendar::businessDayList(
        const Date& from, const Date& to) const {

        QL_REQUIRE(to>=from, "'from' date ("
            << from << ") must be equal to or earlier than 'to' date ("
            << to << ")");
        std::vector<Date> result;
        for (Date d = from; d <= to; ++d) {
            if (isBusinessDay(d))
                result.push_back(d);
       }
       return result;
    }
}
