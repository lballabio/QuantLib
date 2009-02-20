/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2009 StatPro Italia srl

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

#include <ql/instruments/zerocouponinflationswap.hpp>

namespace QuantLib {

    ZeroCouponInflationSwap::ZeroCouponInflationSwap(
                                             const Date& start,
                                             const Date& maturity,
                                             const Period &lag,
                                             Rate fixedRate,
                                             const Calendar& calendar,
                                             BusinessDayConvention convention,
                                             const DayCounter& dayCounter)
    : InflationSwap(start, maturity, lag, calendar, convention, dayCounter),
      fixedRate_(fixedRate) {}


    Rate ZeroCouponInflationSwap::fixedRate() const {
        return fixedRate_;
    }



    void ZeroCouponInflationSwap::setupArguments(
                                       PricingEngine::arguments* args) const {
        InflationSwap::setupArguments(args);

        ZeroCouponInflationSwap::arguments* arguments =
            dynamic_cast<ZeroCouponInflationSwap::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->fixedRate = fixedRate_;
    }


    void ZeroCouponInflationSwap::arguments::validate() const {
        InflationSwap::arguments::validate();

        QL_REQUIRE(fixedRate != Null<Rate>(), "fixed rate not provided");
    }

}

