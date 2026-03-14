/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano
 Copyright (C) 2006, 2007 StatPro Italia srl
 Copyright (C) 2015 Peter Caspers

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

#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/flatsmilesection.hpp>
#include <ql/termstructures/volatility/swaption/swaptionconstantvol.hpp>
#include <utility>

namespace QuantLib {

    // floating reference date, floating market data
    ConstantSwaptionVolatility::ConstantSwaptionVolatility(Natural settlementDays,
                                                           const Calendar& cal,
                                                           BusinessDayConvention bdc,
                                                           Handle<Quote> vol,
                                                           const DayCounter& dc,
                                                           const VolatilityType type,
                                                           const Real shift)
    : SwaptionVolatilityStructure(settlementDays, cal, bdc, dc), volatility_(std::move(vol)),
      maxSwapTenor_(100 * Years), volatilityType_(type), shift_(shift) {
        registerWith(volatility_);
    }

    // fixed reference date, floating market data
    ConstantSwaptionVolatility::ConstantSwaptionVolatility(const Date& referenceDate,
                                                           const Calendar& cal,
                                                           BusinessDayConvention bdc,
                                                           Handle<Quote> vol,
                                                           const DayCounter& dc,
                                                           const VolatilityType type,
                                                           const Real shift)
    : SwaptionVolatilityStructure(referenceDate, cal, bdc, dc), volatility_(std::move(vol)),
      maxSwapTenor_(100 * Years), volatilityType_(type), shift_(shift) {
        registerWith(volatility_);
    }

    // floating reference date, fixed market data
    ConstantSwaptionVolatility::ConstantSwaptionVolatility(
                                                    Natural settlementDays,
                                                    const Calendar& cal,
                                                    BusinessDayConvention bdc,
                                                    Volatility vol,
                                                    const DayCounter& dc,
                                                    const VolatilityType type,
                                                    const Real shift)
    : SwaptionVolatilityStructure(settlementDays, cal, bdc, dc),
      volatility_(ext::shared_ptr<Quote>(new SimpleQuote(vol))),
      maxSwapTenor_(100*Years), volatilityType_(type), shift_(shift) {}

    // fixed reference date, fixed market data
    ConstantSwaptionVolatility::ConstantSwaptionVolatility(
                                                    const Date& referenceDate,
                                                    const Calendar& cal,
                                                    BusinessDayConvention bdc,
                                                    Volatility vol,
                                                    const DayCounter& dc,
                                                    const VolatilityType type,
                                                    const Real shift)
    : SwaptionVolatilityStructure(referenceDate, cal, bdc, dc),
      volatility_(ext::shared_ptr<Quote>(new SimpleQuote(vol))),
      maxSwapTenor_(100*Years), volatilityType_(type), shift_(shift) {}

    ext::shared_ptr<SmileSection>
    ConstantSwaptionVolatility::smileSectionImpl(const Date& d,
                                                 const Period&) const {
        Volatility atmVol = volatility_->value();
        return ext::shared_ptr<SmileSection>(
            new FlatSmileSection(d, atmVol, dayCounter(), referenceDate(),
                                 Null<Rate>(), volatilityType_, shift_));
    }

    ext::shared_ptr<SmileSection>
    ConstantSwaptionVolatility::smileSectionImpl(Time optionTime,
                                                 Time) const {
        Volatility atmVol = volatility_->value();
        return ext::shared_ptr<SmileSection>(
            new FlatSmileSection(optionTime, atmVol, dayCounter(), Null<Rate>(),
                                 volatilityType_, shift_));
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
