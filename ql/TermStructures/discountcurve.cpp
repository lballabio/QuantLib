
/*
 Copyright (C) 2002, 2003 Andre Louw.

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file discountcurve.cpp
    \brief Term structure based on loglinear interpolation of discount factors
*/

#include <ql/TermStructures/discountcurve.hpp>

namespace QuantLib {

    namespace TermStructures {

        DiscountCurve::DiscountCurve(
                             const Date &todaysDate,
                             const std::vector < Date > &dates,
                             const std::vector < DiscountFactor > &discounts,
                             const DayCounter & dayCounter)
        : todaysDate_(todaysDate), dates_(dates), discounts_(discounts),
          dayCounter_(dayCounter) {

            QL_REQUIRE(dates_.size()>1, "DiscountCurve::DiscountCurve : "
                "too few dates");
            QL_REQUIRE(discounts_.size()==dates_.size(),
                "DiscountCurve::DiscountCurve : "
                "dates/discounts mismatch");
            QL_REQUIRE(discounts_[0]==1.0,
                "DiscountCurve::DiscountCurve : "
                "the first discount must be == 1.0 "
                "to flag the corrsponding date as settlement date");

            times_.resize(dates_.size());
            times_[0]=0.0;
            for(Size i = 1; i < dates_.size(); i++) {
                QL_REQUIRE(dates_[i]>dates_[i-1],
                   "DiscountCurve::DiscountCurve : invalid date");
                QL_REQUIRE(discounts_[i] > 0.0,
                   "DiscountCurve::DiscountCurve : invalid discount");
                times_[i] = dayCounter_.yearFraction(dates_[0],
                   dates_[i]);
             }

            interpolation_ = Handle < DfInterpolation >
                (new DfInterpolation(times_.begin(), times_.end(),
                discounts_.begin()));
      }


      DiscountFactor DiscountCurve::discountImpl(Time t,
        bool extrapolate) const
      {
         QL_REQUIRE(t >= 0.0,
             "DiscountCurve::discountImpl "
             "negative time (" + DoubleFormatter::toString(t) +
             ") not allowed");
         return (*interpolation_) (t, extrapolate);
      }

   }
}
