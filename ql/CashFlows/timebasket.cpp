
/*
 Copyright (C) 2003 Decillion Pty(Ltd)
 Copyright (C) 2003 StatPro Italia srl

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

#include <ql/CashFlows/timebasket.hpp>

namespace QuantLib {

    TimeBasket::TimeBasket(const std::vector<Date>& dates,
                           const std::vector<double>& values) {
        QL_REQUIRE(dates.size() == values.size(),
                   "number of dates differs from number of values");
        for (Size i = 0; i < dates.size(); i++)
            (*this)[dates[i]] = values[i];
    }

    TimeBasket TimeBasket::rebin(const std::vector<Date>& buckets) const {
        QL_REQUIRE(buckets.size() > 0, "empty bucket structure");

        std::vector<Date> sbuckets = buckets;
        std::sort(sbuckets.begin(), sbuckets.end());

        TimeBasket result;

        for (Size i = 0; i < sbuckets.size(); i++)
            result[sbuckets[i]] = 0.0;

        for (const_iterator j = begin(); j != end(); j++) {
            Date date = j->first;
            double value = j->second;
            Date pDate = Null<Date>(), nDate = Null<Date>();

            std::vector<Date>::const_iterator bi =
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
                double pDays = (double)(pDate-date);
                double nDays = (double)(date-nDate);
                double tDays = (double)(pDate-nDate);
                result[pDate] += value*(nDays/tDays);
                result[nDate] += value*(pDays/tDays);
            }
        }
        return result;
    }

}

