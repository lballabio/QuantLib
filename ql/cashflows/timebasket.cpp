/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Decillion Pty(Ltd)
 Copyright (C) 2003 StatPro Italia srl

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

#include <ql/cashflows/timebasket.hpp>
#include <ql/errors.hpp>
#include <algorithm>

namespace QuantLib {

    TimeBasket::TimeBasket(const std::vector<Date>& dates,
                           const std::vector<Real>& values) {
        QL_REQUIRE(dates.size() == values.size(),
                   "number of dates differs from number of values");
        super& self = *this;
        for (Size i = 0; i < dates.size(); i++)
            self[dates[i]] = values[i];
    }

    TimeBasket TimeBasket::rebin(const std::vector<Date>& buckets) const {
        QL_REQUIRE(!buckets.empty(), "empty bucket structure");

        std::vector<Date> sbuckets = buckets;
        std::sort(sbuckets.begin(), sbuckets.end());

        TimeBasket result;

        for (auto& sbucket : sbuckets)
            result[sbucket] = 0.0;

        for (auto j : *this) {
            Date date = j.first;
            Real value = j.second;
            Date pDate = Null<Date>(), nDate = Null<Date>();

            auto bi =
                std::lower_bound(sbuckets.begin(), sbuckets.end(), date);

            if (bi == sbuckets.end())
                pDate = sbuckets.back();
            else
                pDate = *bi;

            if (bi != sbuckets.begin() && bi != sbuckets.end())
                nDate = *(bi-1);

            if (pDate == date || nDate == Null<Date>()) {
                result[pDate] += value;
            } else {
                Real pDays = Real(pDate-date);
                Real nDays = Real(date-nDate);
                Real tDays = Real(pDate-nDate);
                result[pDate] += value*(nDays/tDays);
                result[nDate] += value*(pDays/tDays);
            }
        }
        return result;
    }

}

