/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano

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

#include <ql/termstructures/voltermstructure.hpp>

namespace QuantLib {

    VolatilityTermStructure::VolatilityTermStructure(BusinessDayConvention bdc,
                                                     const DayCounter& dc,
                                                     bool extrapolate)
    : TermStructure(dc, extrapolate), bdc_(bdc) {}

    VolatilityTermStructure::VolatilityTermStructure(const Date& referenceDate,
                                                     const Calendar& cal,
                                                     BusinessDayConvention bdc,
                                                     const DayCounter& dc,
                                                     bool extrapolate)
    : TermStructure(referenceDate, cal, dc, extrapolate), bdc_(bdc) {}

    VolatilityTermStructure::VolatilityTermStructure(Natural settlementDays,
                                                     const Calendar& cal,
                                                     BusinessDayConvention bdc,
                                                     const DayCounter& dc,
                                                     bool extrapolate)
    : TermStructure(settlementDays, cal, dc, extrapolate), bdc_(bdc) {}

    void VolatilityTermStructure::checkStrike(Rate k,
                                              bool extrapolate) const {
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }

}
