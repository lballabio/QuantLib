
/*
 Copyright (C) 2002 Andre Louw.

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

    \fullpath
    ql/TermStructures/%discountcurve.cpp
*/

// $Id$

#include <ql/TermStructures/discountcurve.hpp>

namespace QuantLib {

    namespace TermStructures {

        DiscountCurve::DiscountCurve(const Date & todaysDate,
            const Calendar & calendar, int settlementDays,
            const DayCounter & dayCounter, Currency currency,
            const std::vector < Date > &dates,
            const std::vector < DiscountFactor > &discounts)
        : todaysDate_(todaysDate), calendar_(calendar),
          settlementDays_(settlementDays), dayCounter_(dayCounter),
          currency_(currency), dates_(dates), discounts_(discounts) {

             times_.resize(dates.size());
             QL_REQUIRE(dates_[0]>=settlementDate(),
                "DiscountCurveDiscountCurve : invalid first date greater than "
                "settlement date");
             QL_REQUIRE(discounts_[0]<=1.0, "DiscountCurveDiscountCurve :"
                " invalid first discount greater that one");

             for(Size i = 1; i < dates.size(); i++) {
                 QL_REQUIRE(dates_[i]>dates_[i-1],
                    "DiscountCurveDiscountCurve : invalid date");
                 QL_REQUIRE(discounts_[i]<=discounts_[i-1],
                    "DiscountCurveDiscountCurve : invalid discount");
                 times_[i] = dayCounter_.yearFraction(settlementDate(),
                    dates_[i]);
             }

            interpolation_ = Handle < DfInterpolation >
                (new DfInterpolation(times_.begin(), times_.end(),
                discounts.begin(), true));
      }

      DiscountFactor DiscountCurve::discountImpl(Time t,
        bool extrapolate) const
      {
         QL_REQUIRE(t >= 0.0 && (t <= times_.back() || extrapolate),
                    "DiscountCurve: time (" +
                    DoubleFormatter::toString(t) +
                    ") outside curve definition [" +
                    DoubleFormatter::toString(0.0) + ", " +
                    DoubleFormatter::toString(times_.back()) + "]");
         return (*interpolation_) (t);
      }

   }
}
