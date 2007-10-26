/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2007 StatPro Italia srl

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

#include <ql/termstructures/volatility/interestrate/optionlet/constantoptionletvol.hpp>

namespace QuantLib {

    ConstantOptionletVol::ConstantOptionletVol(const Handle<Quote>& volatility,
                                               const Calendar& cal,
                                               const DayCounter& dc,
                                               BusinessDayConvention bdc)
    : OptionletVolatilityStructure(0, NullCalendar(), bdc, dc),
      volatility_(volatility) {
        registerWith(volatility_);
    }

    ConstantOptionletVol::ConstantOptionletVol(const Date& referenceDate,
                                               const Handle<Quote>& volatility,
                                               const Calendar& cal,
                                               const DayCounter& dc,
                                               BusinessDayConvention bdc)
    : OptionletVolatilityStructure(referenceDate, Calendar(), bdc, dc),
      volatility_(volatility) {
        registerWith(volatility_);
    }

    ConstantOptionletVol::ConstantOptionletVol(Volatility volatility,
                                               const Calendar& cal,
                                               const DayCounter& dc,
                                               BusinessDayConvention bdc)
    : OptionletVolatilityStructure(0, NullCalendar(), bdc, dc),
      volatility_(boost::shared_ptr<Quote>(new SimpleQuote(volatility))) {}

    ConstantOptionletVol::ConstantOptionletVol(const Date& referenceDate,
                                               Volatility volatility,
                                               const Calendar& cal,
                                               const DayCounter& dc,
                                               BusinessDayConvention bdc)
    : OptionletVolatilityStructure(referenceDate, Calendar(), bdc, dc),
      volatility_(boost::shared_ptr<Quote>(new SimpleQuote(volatility))) {}

}
