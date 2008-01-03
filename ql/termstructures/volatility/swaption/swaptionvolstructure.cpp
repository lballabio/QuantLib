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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/time/period.hpp>

namespace QuantLib {

    SwaptionVolatilityStructure::SwaptionVolatilityStructure(
                                                    const Calendar& cal,
                                                    const DayCounter& dc,
                                                    BusinessDayConvention bdc)
    : VolatilityTermStructure(cal, bdc, dc) {}

    SwaptionVolatilityStructure::SwaptionVolatilityStructure(
                                                const Date& referenceDate,
                                                const Calendar& calendar,
                                                const DayCounter& dc,
                                                BusinessDayConvention bdc)
    : VolatilityTermStructure(referenceDate, calendar, bdc, dc) {}

    SwaptionVolatilityStructure::SwaptionVolatilityStructure(
                                                Natural settlementDays,
                                                const Calendar& calendar,
                                                const DayCounter& dc,
                                                BusinessDayConvention bdc)
    : VolatilityTermStructure(settlementDays, calendar, bdc, dc) {}


    Time SwaptionVolatilityStructure::maxSwapLength() const {
        Date d = optionDateFromTenor(maxSwapTenor());
        return timeFromReference(d);
    }

    Time SwaptionVolatilityStructure::convertSwapTenor(const Date& optionDate,
                                                       const Period& p) const {
        Date end = optionDate + p;
        QL_REQUIRE(end>optionDate,
                   "negative swap tenor (" << p << ") given");
        return dayCounter().yearFraction(optionDate, end);
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
        QL_REQUIRE(swapLength > 0,
                   "non-positive swap length (" << swapLength << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   swapLength <= maxSwapLength(),
                   "swap tenor (" << swapLength << ") is past max tenor ("
                   << maxSwapLength() << ")");
    }

}
