/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Ferdinando Ametrano

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

#include <ql/termstructures/yield/compositediscountcurve.hpp>

namespace QuantLib {

     CompositeDiscountCurve::CompositeDiscountCurve(
                                      const Handle<YieldTermStructure>& first,
                                      const Handle<YieldTermStructure>& second,
                                      const Date& joinDate,
                                      bool allowExtrapolatedJunction,
                                      bool allowExtrapolation)
    : YieldTermStructure(Date()),
      first_(first), second_(second), joinDate_(joinDate),
      allowExtrapolatedJunction_(allowExtrapolatedJunction),
      allowExtrapolation_(allowExtrapolation) {
        registerWith(first_);
        registerWith(second_);

        if (!first_.empty() && !second_.empty()) {
            checkFirst();
            checkSecond();
        } else {
            if (!first_.empty())
                checkFirst();
            if (!second_.empty())
                checkSecond();
        }
    }

    void CompositeDiscountCurve::checkFirst() const {
        QL_REQUIRE(first_->referenceDate()<=joinDate_,
                   "first discount curve reference date (" <<
                   first_->referenceDate() <<
                   ") must be not greater than join date (" <<
                   joinDate_ << ")");
        QL_REQUIRE(first_->maxDate()>=joinDate_ || allowExtrapolatedJunction_,
                   "first discount curve max date (" << first_->maxDate() <<
                   ") is earlier than join date (" << joinDate_ <<
                   ") and extrapolated junction is not allowed");
    }

    void CompositeDiscountCurve::checkSecond() const {
        QL_REQUIRE(second_->referenceDate()<=joinDate_,
                   "second discount curve reference date (" <<
                   second_->referenceDate() <<
                   ") must be not greater than join date (" <<
                   joinDate_ << ")");
        QL_REQUIRE(second_->maxDate()>=joinDate_ || allowExtrapolation_,
                   "second discount curve max date (" << second_->maxDate() <<
                   ") is earlier than join date (" << joinDate_ << ")");
    }

    void CompositeDiscountCurve::checkDayCount() const {
            QL_REQUIRE(second_->dayCounter()==first_->dayCounter(),
                       "DayCounter mismatch between composed curves: " <<
                       "first curve has " << first_->dayCounter() << ", " <<
                       "second curve has " << second_->dayCounter());
    }

    DiscountFactor CompositeDiscountCurve::discountImpl(Time t) const {

        #ifdef QL_EXTRA_SAFETY_CHECKS
            checkFirst();
        #endif
        Time joinTime = first_->timeFromReference(joinDate_);
        if (t<=joinTime)
            return first_->discount(t, allowExtrapolatedJunction_);

        #ifdef QL_EXTRA_SAFETY_CHECKS
            checkSecond();
            checkDayCount();
        #endif
        DiscountFactor firstDisc =
                    first_->discount(joinDate_, allowExtrapolatedJunction_);
        DiscountFactor secondDisc =
                    second_->discount(joinDate_, allowExtrapolation_);
        return second_->discount(t,allowExtrapolation_) / secondDisc*firstDisc;
    }

}
