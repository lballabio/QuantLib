/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Allen Kuo

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

#include <ql/experimental/callablebonds/callablebondvolstructure.hpp>
#include <ql/time/period.hpp>

namespace QuantLib {

    CallableBondVolatilityStructure::CallableBondVolatilityStructure(
                                                    const DayCounter& dc,
                                                    BusinessDayConvention bdc)
    : TermStructure(dc), bdc_(bdc) {}

    CallableBondVolatilityStructure::CallableBondVolatilityStructure(
                                                    const Date& referenceDate,
                                                    const Calendar& calendar,
                                                    const DayCounter& dc,
                                                    BusinessDayConvention bdc)
    : TermStructure(referenceDate, calendar, dc), bdc_(bdc) {}

    CallableBondVolatilityStructure::CallableBondVolatilityStructure(
                                                    Natural settlementDays,
                                                    const Calendar& calendar,
                                                    const DayCounter& dc,
                                                    BusinessDayConvention bdc)
    : TermStructure(settlementDays, calendar, dc), bdc_(bdc) {}

    Time CallableBondVolatilityStructure::maxBondLength() const {
        return timeFromReference(referenceDate()+maxBondTenor());
    }

    std::pair<Time,Time>
    CallableBondVolatilityStructure::convertDates(
                                              const Date& optionDate,
                                              const Period& bondTenor) const {
        Date end = optionDate + bondTenor;
        QL_REQUIRE(end>optionDate,
                   "negative bond tenor (" << bondTenor << ") given");
        Time optionTime = timeFromReference(optionDate);
        Time timeLength = dayCounter().yearFraction(optionDate, end);
        return std::make_pair(optionTime, timeLength);
    }

    void CallableBondVolatilityStructure::checkRange(const Date& optionDate,
                                                     const Period& bondTenor,
                                                     Rate k,
                                                     bool extrapolate) const {
        TermStructure::checkRange(timeFromReference(optionDate),
                                  extrapolate);
        QL_REQUIRE(bondTenor.length() > 0,
                   "negative bond tenor (" << bondTenor << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   bondTenor <= maxBondTenor(),
                   "bond tenor (" << bondTenor << ") is past max tenor ("
                   << maxBondTenor() << ")");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }

}

