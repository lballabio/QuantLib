
/*
 Copyright (C) 2000-2004 StatPro Italia srl
 Copyright (C) 2004 Jeff Yu

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/calendar.hpp>
#if !(defined(QL_PATCH_MICROSOFT) || defined(QL_PATCH_BORLAND))
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#endif

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

    #if !(defined(QL_PATCH_MICROSOFT) || defined(QL_PATCH_BORLAND))

    void Calendar::load(const std::string& filename) {
        std::ifstream file(filename.c_str());
        QL_REQUIRE(file, "failed to open " + filename);

        static std::string comment = "(#.*)$";
        boost::regex record(
                   "^\\s*(\\w+)\\s*:\\s*([+-]?)\\s*(\\d{4})-(\\d{2})-(\\d{2})"
                   "\\s*(#.*)?$");
        boost::regex empty("^\\s*(#.*)?$");
        boost::smatch m;

        std::string line;
        while (std::getline(file, line)) {
            if (boost::regex_search(line, m, record)) {
                std::string key = m[1];
                std::string flag = m[2];
                if (key == name()) {
                    Day day = boost::lexical_cast<Day>(std::string(m[5]));
                    Month month = 
                        Month(boost::lexical_cast<Integer>(std::string(m[4])));
                    Year year = boost::lexical_cast<Year>(std::string(m[3]));
                    if (flag == "-")
                        removeHoliday(Date(day,month,year));
                    else
                        addHoliday(Date(day,month,year));
                }
            } else if (boost::regex_search(line, m, empty)) {
                continue;
            } else {
                QL_FAIL("badly formatted line: " + line);
            }
        }
    }

    #endif

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
                    if (isEndOfMonth(origin) &&
                        !isEndOfMonth(d1)) {
                        d1 = Date(d1.lastDayOfMonth(),d1.month(),d1.year());
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
            Date d1 = d.plus(n,unit);
            return adjust(d1,c,d);
        }
        QL_DUMMY_RETURN(Date());
    }

    Date Calendar::advance(const Date & d,
                           const Period & p,
                           BusinessDayConvention c)const {
        return advance(d,
                       p.length(),
                       p.units(),
                       c);
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

}
