/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano
 Copyright (C) 2006, 2007 StatPro Italia srl

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

#include <ql/termstructures/volatility/swaption/swaptionconstantvol.hpp>
#include <ql/termstructures/volatility/flatsmilesection.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    // floating reference date, floating market data
    ConstantSwaptionVolatility::ConstantSwaptionVolatility(
                                                    Natural settlementDays,
                                                    const Handle<Quote>& vol,
                                                    const DayCounter& dc,
                                                    const Calendar& cal,
                                                    BusinessDayConvention bdc)
    : SwaptionVolatilityStructure(settlementDays, cal, dc, bdc),
      volatility_(vol), maxSwapTenor_(100*Years) {
        registerWith(volatility_);
    }

    // fixed reference date, floating market data
    ConstantSwaptionVolatility::ConstantSwaptionVolatility(
                                                    const Date& referenceDate,
                                                    const Handle<Quote>& vol,
                                                    const DayCounter& dc,
                                                    const Calendar& cal,
                                                    BusinessDayConvention bdc)
    : SwaptionVolatilityStructure(referenceDate, cal, dc, bdc),
      volatility_(vol), maxSwapTenor_(100*Years) {
        registerWith(volatility_);
    }

    // floating reference date, fixed market data
    ConstantSwaptionVolatility::ConstantSwaptionVolatility(
                                                    Natural settlementDays,
                                                    Volatility vol,
                                                    const DayCounter& dc,
                                                    const Calendar& cal,
                                                    BusinessDayConvention bdc)
    : SwaptionVolatilityStructure(settlementDays, cal, dc, bdc),
      volatility_(boost::shared_ptr<Quote>(new SimpleQuote(vol))),
      maxSwapTenor_(100*Years) {}

    // fixed reference date, fixed market data
    ConstantSwaptionVolatility::ConstantSwaptionVolatility(
                                                    const Date& referenceDate,
                                                    Volatility vol,
                                                    const DayCounter& dc,
                                                    const Calendar& cal,
                                                    BusinessDayConvention bdc)
    : SwaptionVolatilityStructure(referenceDate, cal, dc, bdc),
      volatility_(boost::shared_ptr<Quote>(new SimpleQuote(vol))),
      maxSwapTenor_(100*Years) {}

    boost::shared_ptr<SmileSection>
    ConstantSwaptionVolatility::smileSectionImpl(const Date& d,
                                                 const Period&) const {
        Volatility atmVol = volatility_->value();
        return boost::shared_ptr<SmileSection>(new
            FlatSmileSection(d, atmVol, dayCounter(), referenceDate()));
    }

    boost::shared_ptr<SmileSection>
    ConstantSwaptionVolatility::smileSectionImpl(Time optionTime,
                                                 Time) const {
        Volatility atmVol = volatility_->value();
        return boost::shared_ptr<SmileSection>(new
            FlatSmileSection(optionTime, atmVol, dayCounter()));
    }

    Volatility ConstantSwaptionVolatility::volatilityImpl(const Date&,
                                                          const Period&,
                                                          Rate) const {
        return volatility_->value();
    }

    Volatility ConstantSwaptionVolatility::volatilityImpl(Time,
                                                          Time,
                                                          Rate) const {
        return volatility_->value();
    }

}
