
/*
 Copyright (C) 2004 Ferdinando Ametrano

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

#include <ql/Functions/calendars.hpp>

namespace QuantLib {

    std::vector<Date> holidayList(const Calendar& calendar, const Date& from,
        const Date& to, bool includeWeekEnds) {

        QL_REQUIRE(to>from, "'from' date must be lower than 'to' date");
        Date d = from;
        std::vector<Date> result;
        while (d<=to) {
            if (!calendar.isBusinessDay(d)) {
                if (includeWeekEnds) {
                    result.push_back(d);
                } else if (d.weekday()==Saturday) {
                    d = d.plusDays(1);
                } else if (d.weekday()!=Sunday) {
                    result.push_back(d);
                }
            }
            d = d.plusDays(1);
       }

       return result;

    }

}
