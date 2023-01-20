/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2011 Chris Kenyon

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

#include <ql/termstructures/volatility/inflation/constantcpivolatility.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    ConstantCPIVolatility:: ConstantCPIVolatility(const Handle<Quote>& vol,
                                                  Natural settlementDays,
                                                  const Calendar& cal,
                                                  BusinessDayConvention bdc,
                                                  const DayCounter& dc,
                                                  const Period& observationLag,
                                                  Frequency frequency,
                                                  bool indexIsInterpolated)
    : CPIVolatilitySurface(settlementDays, cal, bdc, dc,
                           observationLag, frequency, indexIsInterpolated),
      volatility_(vol) {}

    ConstantCPIVolatility:: ConstantCPIVolatility(Volatility vol,
                                                  Natural settlementDays,
                                                  const Calendar& cal,
                                                  BusinessDayConvention bdc,
                                                  const DayCounter& dc,
                                                  const Period& observationLag,
                                                  Frequency frequency,
                                                  bool indexIsInterpolated)
    : CPIVolatilitySurface(settlementDays, cal, bdc, dc,
                           observationLag, frequency, indexIsInterpolated),
      volatility_(ext::make_shared<SimpleQuote>(vol)) {}


    Volatility ConstantCPIVolatility::volatilityImpl(Time, Rate) const {
        return volatility_->value();
    }

}

