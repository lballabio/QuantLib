
/*
 Copyright (C) 2002, 2003 Decillion Pty(Ltd)

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

#include <ql/TermStructures/discountcurve.hpp>
#include <ql/TermStructures/compoundforward.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED
    DiscountCurve::DiscountCurve(const Date& todaysDate,
                                 const std::vector<Date>& dates,
                                 const std::vector<DiscountFactor>& discounts,
                                 const DayCounter& dayCounter)
    : DiscountStructure(todaysDate, dates[0]), dayCounter_(dayCounter),
      dates_(dates), discounts_(discounts) {
        QL_REQUIRE(dates_.size() > 0,
                   "no input Dates given");
        QL_REQUIRE(discounts_.size() > 0,
                   "no input Discount factors given");
        QL_REQUIRE(discounts_.size() == dates_.size(),
                   "dates/discount factors count mismatch");
        QL_REQUIRE(discounts_[0] == 1.0,
                   "the first discount must be == 1.0 "
                   "to flag the corrsponding date as settlement date");

        times_.resize(dates_.size());
        times_[0] = 0.0;
        for(Size i = 1; i < dates_.size(); i++) {
            QL_REQUIRE(dates_[i] > dates_[i-1],
                       "invalid date ("+
                       DateFormatter::toString(dates_[i])+", vs "+
                       DateFormatter::toString(dates_[i-1])+")");
            QL_REQUIRE(discounts_[i] > 0.0, "negative discount");
            times_[i] = dayCounter.yearFraction(dates_[0], dates_[i]);
        }
        interpolation_ = LogLinearInterpolation(times_.begin(), times_.end(),
                                                discounts_.begin());
    }
    #endif

    DiscountCurve::DiscountCurve(const std::vector<Date>& dates,
                                 const std::vector<DiscountFactor>& discounts,
                                 const DayCounter& dayCounter)
    #ifdef QL_DISABLE_DEPRECATED
    : YieldTermStructure(dates[0]),
    #else
    : DiscountStructure(dates[0]),
    #endif
      dayCounter_(dayCounter), dates_(dates), discounts_(discounts) {
        QL_REQUIRE(dates_.size() > 0,
                   "no input Dates given");
        QL_REQUIRE(discounts_.size() > 0,
                   "no input Discount factors given");
        QL_REQUIRE(discounts_.size() == dates_.size(),
                   "dates/discount factors count mismatch");
        QL_REQUIRE(discounts_[0] == 1.0,
                   "the first discount must be == 1.0 "
                   "to flag the corrsponding date as settlement date");

        times_.resize(dates_.size());
        times_[0] = 0.0;
        for(Size i = 1; i < dates_.size(); i++) {
            QL_REQUIRE(dates_[i] > dates_[i-1],
                       "invalid date ("+
                       DateFormatter::toString(dates_[i])+", vs "+
                       DateFormatter::toString(dates_[i-1])+")");
            QL_REQUIRE(discounts_[i] > 0.0, "negative discount");
            times_[i] = dayCounter.yearFraction(dates_[0], dates_[i]);
        }
        interpolation_ = LogLinearInterpolation(times_.begin(), times_.end(),
                                                discounts_.begin());
    }

    DiscountFactor DiscountCurve::discountImpl(Time t) const {
        if (t == 0.0) {
            return discounts_[0];
        } else {
            Size n = referenceNode(t);
            if (t == times_[n]) {
                return discounts_[n];
            } else {
                return interpolation_(t, true);
            }
        }
        QL_DUMMY_RETURN(DiscountFactor());
    }

    Size DiscountCurve::referenceNode(Time t) const {
        if (t >= times_.back())
            return times_.size()-1;
        std::vector<Time>::const_iterator i=times_.begin(),
            j=times_.end(), k;
        while (j-i > 1) {
            k = i+(j-i)/2;
            if (t <= *k)
                j = k;
            else
                i = k;
        }
        return (j-times_.begin());
    }

}
