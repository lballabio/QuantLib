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

#include <ql/instruments/inflationswap.hpp>

namespace QuantLib {

    InflationSwap::InflationSwap(const Date& start, const Date& maturity,
                                 const Period& lag, const Calendar& calendar,
                                 BusinessDayConvention convention,
                                 const DayCounter& dayCounter,
                                 const Handle<YieldTermStructure>& yieldTS)
    : start_(start), maturity_(maturity), lag_(lag), calendar_(calendar),
      bdc_(convention), dayCounter_(dayCounter), yieldTS_(yieldTS) {

        baseDate_ = calendar_.adjust(start_ - lag_, bdc_);
        maturity_ = calendar_.adjust(maturity_, bdc_);

        registerWith(yieldTS_);
    }

    Date InflationSwap::baseDate() const {
        return baseDate_;
    }

    Period InflationSwap::lag() const {
        return lag_;
    }

    Date InflationSwap::startDate() const {
        return start_;
    }

    Date InflationSwap::maturityDate() const {
        return maturity_;
    }

    Calendar InflationSwap::calendar() const {
        return calendar_;
    }

    BusinessDayConvention InflationSwap::businessDayConvention() const {
        return bdc_;
    }

    DayCounter InflationSwap::dayCounter() const {
        return dayCounter_;
    }

}

