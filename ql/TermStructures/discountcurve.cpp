
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
    \brief pre-bootstrapped discount curve

    \fullpath
    ql/TermStructures/%discountcurve.cpp
*/

// $Id$

#include <ql/TermStructures/discountcurve.hpp>

namespace QuantLib
{

   namespace TermStructures
   {

      DiscountCurve::DiscountCurve(const Date & todaysDate,
                                   const Calendar & calendar,
                                   int settlementDays,
                                   const DayCounter & dayCounter,
                                   Currency currency,
                                   const std::vector < Date > &dates,
                                   const std::vector < DiscountFactor >
                                   &discounts):todaysDate_(todaysDate),
         calendar_(calendar),
         settlementDays_(settlementDays),
         dayCounter_(dayCounter),
         currency_(currency), dates_(dates), discounts_(discounts)
      {
         times_.resize(dates.size());
         for(unsigned int i = 0; i < dates.size(); i++)
            times_[i] = dayCounter_.yearFraction(settlementDate(), dates_[i]);
         interpolation_ =
            Handle < DfInterpolation >
            (new DfInterpolation(times_.begin(),
                                 times_.end(), discounts.begin(), true));
      }
      DiscountFactor DiscountCurve::discountImpl(Time t, bool extrapolate) const
      {
         QL_REQUIRE(t >= 0.0 && (t <= times_.back() || extrapolate),
                    "DiscountCurve: time (" +
                    DoubleFormatter::toString(t) +
                    ") outside curve definition [" +
                    DoubleFormatter::toString(0.0) + ", " +
                    DoubleFormatter::toString(times_.back()) + "]");
         return (*interpolation_) (t);
      }
      Rate DiscountCurve::forwardImpl(Time t, bool extrapolate) const
      {
         QL_REQUIRE(t >= 0.0 && (t <= times_.back() || extrapolate),
                    "DiscountCurve: time (" +
                    DoubleFormatter::toString(t) +
                    ") outside curve definition [" +
                    DoubleFormatter::toString(0.0) + ", " +
                    DoubleFormatter::toString(times_.back()) + "]");
         return *discounts_.begin() / discountImpl(t, extrapolate) * t;
      }
      Rate DiscountCurve::zeroYieldImpl(Time t, bool extrapolate) const
      {
         return forwardImpl(t, extrapolate);
      }
   }
}
