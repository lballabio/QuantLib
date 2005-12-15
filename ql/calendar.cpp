/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2005 StatPro Italia srl
 Copyright (C) 2004 Jeff Yu

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

#include <ql/calendar.hpp>

namespace QuantLib {

    void Calendar::addHoliday(const Date& d) {
        // if d was a genuine holiday previously removed, revert the change
        impl_->removedHolidays.erase(d);
        // if it's already a holiday, leave the calendar alone.
        // Otherwise, add it.
        if (impl_->isBusinessDay(d))
            impl_->addedHolidays.insert(d);
    }

    void Calendar::removeHoliday(const Date& d) {
        // if d was an artificially-added holiday, revert the change
        impl_->addedHolidays.erase(d);
        // if it's already a business day, leave the calendar alone.
        // Otherwise, add it.
        if (!impl_->isBusinessDay(d))
            impl_->removedHolidays.insert(d);
    }

    Date Calendar::adjust(const Date& d,
                          BusinessDayConvention c,
                          const Date& origin) const {
        QL_REQUIRE(d != Date(), "null date");

        if (c == Unadjusted)
            return d;

        Date d1 = d;
        if (c == Following || c == ModifiedFollowing ||
            c == MonthEndReference) {
            while (isHoliday(d1))
                d1++;
            if (c == ModifiedFollowing || c == MonthEndReference) {
                if (d1.month() != d.month()) {
                    return adjust(d,Preceding);
                }
                if (c == MonthEndReference && origin != Date()) {
                    if (isEndOfMonth(origin) && !isEndOfMonth(d1)) {
                        d1 = Date::endOfMonth(d1);
                        return adjust(d1,Preceding);
                    }
                }
            }
        } else if (c == Preceding || c == ModifiedPreceding) {
            while (isHoliday(d1))
                d1--;
            if (c == ModifiedPreceding && d1.month() != d.month()) {
                return adjust(d,Following);
            }
        } else {
            QL_FAIL("unknown business-day convention");
        }
        return d1;
    }

    Date Calendar::advance(const Date& d, Integer n, TimeUnit unit,
                           BusinessDayConvention c) const {
        QL_REQUIRE(d!=Date(), "null date");
        if (n == 0) {
            return adjust(d,c);
        } else if (unit == Days) {
            Date d1 = d;
            if (n > 0) {
                while (n > 0) {
                    d1++;
                    while (isHoliday(d1))
                        d1++;
                    n--;
                }
            } else {
                while (n < 0) {
                    d1--;
                    while(isHoliday(d1))
                        d1--;
                    n++;
                }
            }
            return d1;
        } else {
            Date d1 = d + n*unit;
            return adjust(d1,c,d);
        }
        QL_DUMMY_RETURN(Date());
    }

    Date Calendar::advance(const Date & d,
                           const Period & p,
                           BusinessDayConvention c)const {
        return advance(d, p.length(), p.units(), c);
    }

   // Western calendars

    Day Calendar::WesternImpl::easterMonday(Year y) {
        static const Day EasterMonday[] = {
            107,  98,  90, 103,  95, 114, 106,  91, 111, 102,   // 1900-1909
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
             90, 110, 102,  86, 106,  98, 111, 102,  94, 107,   // 2070-2079
             99,  90, 110,  95,  87, 106,  91, 111, 103,  94,   // 2080-2089
            107,  99,  91, 103,  95, 115, 107,  91, 111, 103    // 2090-2099
        };
        return EasterMonday[y-1900];
    }

    // Orthodox calendars

    Day Calendar::OrthodoxImpl::easterMonday(Year y) {
        static const Day EasterMonday[] = {
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
            114,  99, 119, 110, 102, 115, 107, 126, 118, 103    // 2090-2099
        };
        return EasterMonday[y-1901];
    }

}
