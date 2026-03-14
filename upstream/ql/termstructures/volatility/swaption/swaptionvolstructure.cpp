/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/math/rounding.hpp>

namespace QuantLib {

    SwaptionVolatilityStructure::SwaptionVolatilityStructure(
                                                    BusinessDayConvention bdc,
                                                    const DayCounter& dc)
    : VolatilityTermStructure(bdc, dc) {}

    SwaptionVolatilityStructure::SwaptionVolatilityStructure(
                                                const Date& referenceDate,
                                                const Calendar& calendar,
                                                BusinessDayConvention bdc,
                                                const DayCounter& dc)
    : VolatilityTermStructure(referenceDate, calendar, bdc, dc) {}

    SwaptionVolatilityStructure::SwaptionVolatilityStructure(
                                                Natural settlementDays,
                                                const Calendar& calendar,
                                                BusinessDayConvention bdc,
                                                const DayCounter& dc)
    : VolatilityTermStructure(settlementDays, calendar, bdc, dc) {}


    Time SwaptionVolatilityStructure::swapLength(const Period& p) const {
        QL_REQUIRE(p.length()>0,
                   "non-positive swap tenor (" << p << ") given");
        switch (p.units()) {
          case Months:
            return p.length()/12.0;
          case Years:
            return static_cast<Time>(p.length());
          default:
            QL_FAIL("invalid Time Unit (" << p.units() << ") for swap length");
        }
    }

    Time SwaptionVolatilityStructure::swapLength(const Date& start,
                                                 const Date& end) const {
        QL_REQUIRE(end>start, "swap end date (" << end <<
                   ") must be greater than start (" << start << ")");
        Time result = (end-start)/365.25*12.0; // month unit
        result = ClosestRounding(0)(result);
        result /= 12.0; // year unit
        return result;
    }

    void SwaptionVolatilityStructure::checkSwapTenor(const Period& swapTenor,
                                                     bool extrapolate) const {
        QL_REQUIRE(swapTenor.length() > 0,
                   "non-positive swap tenor (" << swapTenor << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   swapTenor <= maxSwapTenor(),
                   "swap tenor (" << swapTenor << ") is past max tenor ("
                   << maxSwapTenor() << ")");
    }

    void SwaptionVolatilityStructure::checkSwapTenor(Time swapLength,
                                                     bool extrapolate) const {
        QL_REQUIRE(swapLength > 0.0,
                   "non-positive swap length (" << swapLength << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   swapLength <= maxSwapLength(),
                   "swap tenor (" << swapLength << ") is past max tenor ("
                   << maxSwapLength() << ")");
    }

}
