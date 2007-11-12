/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon

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

#include <ql/instruments/zciis.hpp>

namespace QuantLib {

    ZeroCouponInflationSwap::ZeroCouponInflationSwap(
                        const Date& start,
                        const Date& maturity,
                        const Period &lag,
                        Rate fixedRate,
                        const Calendar& calendar,
                        BusinessDayConvention convention,
                        const DayCounter& dayCounter,
                        const Handle<YieldTermStructure>& yieldTS,
                        const Handle<ZeroInflationTermStructure>& inflationTS)
    : InflationSwap(start, maturity, lag, calendar, convention,
                    dayCounter, yieldTS),
      fixedRate_(fixedRate), inflationTS_(inflationTS) {
        registerWith(inflationTS_);
    }


    bool ZeroCouponInflationSwap::isExpired() const {
        return yieldTS_->referenceDate() > maturity_;
    }


    Rate ZeroCouponInflationSwap::fairRate() const {
        return inflationTS_->zeroRate(maturity_ - lag_);
    }


    Rate ZeroCouponInflationSwap::fixedRate() const {
        return fixedRate_;
    }


    void ZeroCouponInflationSwap::performCalculations() const {

        // the observation lag is also taken into account in fairRate();
        // discount is relative to the payment date, not the observation date.
        Real T = dayCounter_.yearFraction(inflationTS_->baseDate(),
                                          maturity_ - lag_);
        NPV_ = yieldTS_->discount(maturity_) *
            (std::pow(1.0 + fixedRate_, T) - std::pow(1.0 + fairRate(), T));
        errorEstimate_ = 0.0;
    }

}

