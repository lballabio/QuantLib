/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>

namespace QuantLib {

    LocalVolTermStructure::LocalVolTermStructure(BusinessDayConvention bdc,
                                                 const DayCounter& dc)
    : VolatilityTermStructure(bdc, dc) {}

    LocalVolTermStructure::LocalVolTermStructure(const Date& referenceDate,
                                                 const Calendar& cal,
                                                 BusinessDayConvention bdc,
                                                 const DayCounter& dc)
    : VolatilityTermStructure(referenceDate, cal, bdc, dc) {}

    LocalVolTermStructure::LocalVolTermStructure(Natural settlementDays,
                                                 const Calendar& cal,
                                                 BusinessDayConvention bdc,
                                                 const DayCounter& dc)
    : VolatilityTermStructure(settlementDays, cal, bdc, dc) {}

    Volatility LocalVolTermStructure::localVol(const Date& d,
                                               Real underlyingLevel,
                                               bool extrapolate) const {
        checkRange(d, extrapolate);
        checkStrike(underlyingLevel, extrapolate);
        Time t = timeFromReference(d);
        return localVolImpl(t, underlyingLevel);
    }

    Volatility LocalVolTermStructure::localVol(Time t,
                                               Real underlyingLevel,
                                               bool extrapolate) const {
        checkRange(t, extrapolate);
        checkStrike(underlyingLevel, extrapolate);
        return localVolImpl(t, underlyingLevel);
    }

    void LocalVolTermStructure::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<LocalVolTermStructure>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            QL_FAIL("not a local-volatility term structure visitor");
    }

}
