/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 QuantLib contributors

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

#include <ql/time/calendars/islamicholidays.hpp>
#include <algorithm>

namespace QuantLib {

    bool isEidAlFitr(const Date& d) {
        static std::vector<Date> dates = {
            Date(20, March,    2026),
            Date(10, March,    2027),
            Date(27, February, 2028),
            Date(15, February, 2029),
            Date(5,  February, 2030),
            Date(25, January,  2031),
            Date(14, January,  2032),
            Date(2,  January,  2033),
            Date(23, December, 2033),
            Date(12, December, 2034),
            Date(1,  December, 2035),
            Date(19, November, 2036),
            Date(8,  November, 2037),
            Date(29, October,  2038),
            Date(19, October,  2039),
            Date(7,  October,  2040),
        };
        return std::any_of(dates.begin(), dates.end(),
            [&d](const Date& p) { return d == p; });
    }

    bool isEidAlAdha(const Date& d) {
        static std::vector<Date> dates = {
            Date(27, May,      2026),
            Date(17, May,      2027),
            Date(5,  May,      2028),
            Date(24, April,    2029),
            Date(13, April,    2030),
            Date(3,  April,    2031),
            Date(22, March,    2032),
            Date(11, March,    2033),
            Date(28, February, 2034),
            Date(18, February, 2035),
            Date(7,  February, 2036),
            Date(27, January,  2037),
            Date(16, January,  2038),
            Date(5,  January,  2039),
            Date(26, December, 2039),
            Date(15, December, 2040),
        };
        return std::any_of(dates.begin(), dates.end(),
            [&d](const Date& p) { return d == p; });
    }

}
