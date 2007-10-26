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
        //Date d = referenceDate()+maxSwapTenor();
        Date d = optionDateFromTenor(maxSwapTenor());
        Time t = timeFromReference(d);
        return t;
    }

    std::pair<Time,Time>
    SwaptionVolatilityStructure::convertDates(const Date& optionDate,
                                              const Period& swapTenor) const {
        Date end = optionDate + swapTenor;
        QL_REQUIRE(end>optionDate,
                   "negative swap tenor (" << swapTenor << ") given");
        Time optionTime = timeFromReference(optionDate);
        Time timeLength = dayCounter().yearFraction(optionDate, end);
        return std::make_pair(optionTime, timeLength);
    }

    void SwaptionVolatilityStructure::checkRange(
             const Date& optionDate, const Period& swapTenor,
             Rate k, bool extrapolate) const {
        TermStructure::checkRange(timeFromReference(optionDate),
                                  extrapolate);
        QL_REQUIRE(swapTenor.length() > 0,
                   "negative swap tenor (" << swapTenor << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   swapTenor <= maxSwapTenor(),
                   "swap tenor (" << swapTenor << ") is past max tenor ("
                   << maxSwapTenor() << ")");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }

}
