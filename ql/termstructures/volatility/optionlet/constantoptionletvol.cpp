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

#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <ql/termstructures/volatility/flatsmilesection.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    // floating reference date, floating market data
    ConstantOptionletVol::ConstantOptionletVol(Natural settlementDays,
                                               const Handle<Quote>& volatility,
                                               const Calendar& cal,
                                               BusinessDayConvention bdc,
                                               const DayCounter& dc)
    : OptionletVolatilityStructure(settlementDays, cal, bdc, dc),
      volatility_(volatility) {
        registerWith(volatility_);
    }

    // fixed reference date, floating market data
    ConstantOptionletVol::ConstantOptionletVol(const Date& referenceDate,
                                               const Handle<Quote>& volatility,
                                               const Calendar& cal,
                                               BusinessDayConvention bdc,
                                               const DayCounter& dc)
    : OptionletVolatilityStructure(referenceDate, cal, bdc, dc),
      volatility_(volatility) {
        registerWith(volatility_);
    }

    // floating reference date, fixed market data
    ConstantOptionletVol::ConstantOptionletVol(Natural settlementDays,
                                               Volatility volatility,
                                               const Calendar& cal,
                                               BusinessDayConvention bdc,
                                               const DayCounter& dc)
    : OptionletVolatilityStructure(settlementDays, cal, bdc, dc),
      volatility_(boost::shared_ptr<Quote>(new SimpleQuote(volatility))) {}

    // fixed reference date, fixed market data
    ConstantOptionletVol::ConstantOptionletVol(const Date& referenceDate,
                                               Volatility volatility,
                                               const Calendar& cal,
                                               BusinessDayConvention bdc,
                                               const DayCounter& dc)
    : OptionletVolatilityStructure(referenceDate, cal, bdc, dc),
      volatility_(boost::shared_ptr<Quote>(new SimpleQuote(volatility))) {}

    boost::shared_ptr<SmileSection>
    ConstantOptionletVol::smileSectionImpl(const Date& optionDate) const {
        Volatility atmVol = volatility_->value();
        return boost::shared_ptr<SmileSection>(new
            FlatSmileSection(optionDate,
                             atmVol,
                             dayCounter(),
                             referenceDate()));
    }

    boost::shared_ptr<SmileSection>
    ConstantOptionletVol::smileSectionImpl(Time optionTime) const {
        Volatility atmVol = volatility_->value();
        return boost::shared_ptr<SmileSection>(new
            FlatSmileSection(optionTime,
                             atmVol,
                             dayCounter()));
    }

    Volatility ConstantOptionletVol::volatilityImpl(const Date&,
                                                    Rate) const {
        return volatility_->value();
    }

    Volatility ConstantOptionletVol::volatilityImpl(Time,
                                                    Rate) const {
        return volatility_->value();
    }

}
