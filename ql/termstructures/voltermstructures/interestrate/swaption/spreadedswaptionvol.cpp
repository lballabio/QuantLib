/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti

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

#include <ql/termstructures/voltermstructures/interestrate/swaption/spreadedswaptionvol.hpp>
#include <ql/termstructures/voltermstructures/smilesection.hpp>

namespace QuantLib {

    SpreadedSwaptionVolatilityStructure::SpreadedSwaptionVolatilityStructure(
            const Handle<SwaptionVolatilityStructure>& underlyingVolStructure,
            const Handle<Quote>& spread)
    : SwaptionVolatilityStructure(underlyingVolStructure->settlementDays(),
                                  underlyingVolStructure->calendar(),
                                  underlyingVolStructure->dayCounter(),
                                  underlyingVolStructure->businessDayConvention()),
      underlyingVolStructure_(underlyingVolStructure),
      spread_(spread) {
          registerWith(underlyingVolStructure_);
          registerWith(spread_);
          enableExtrapolation(underlyingVolStructure->allowsExtrapolation());
    }

    Volatility SpreadedSwaptionVolatilityStructure::volatilityImpl(
                                                    Time optionTime,
                                                    Time swapLength,
                                                    Rate strike) const {
        return underlyingVolStructure_
            ->volatility(optionTime, swapLength, strike)+spread_->value();
    }

    boost::shared_ptr<SmileSection>
        SpreadedSwaptionVolatilityStructure::smileSectionImpl(
                                            Time optionTime,
                                            Time swapLength) const {
        boost::shared_ptr<SmileSection> underlyingSmile =
            underlyingVolStructure_->smileSection(optionTime, swapLength);
        return boost::shared_ptr<SmileSection>(new
            SpreadedSmileSection(underlyingSmile, spread_));
    }

    boost::shared_ptr<SmileSection>
        SpreadedSwaptionVolatilityStructure::smileSectionImpl(
                                        const Date& optionDate,
                                        const Period& swapTenor) const {
        boost::shared_ptr<SmileSection> underlyingSmile =
            underlyingVolStructure_->smileSection(optionDate,swapTenor);
        return boost::shared_ptr<SmileSection>(new
            SpreadedSmileSection(underlyingSmile, spread_));
    }

    const Period& SpreadedSwaptionVolatilityStructure::maxSwapTenor() const {
        return underlyingVolStructure_->maxSwapTenor();
    }

    Rate SpreadedSwaptionVolatilityStructure::minStrike() const {
        return underlyingVolStructure_->minStrike();
    }

    Rate SpreadedSwaptionVolatilityStructure::maxStrike() const {
        return underlyingVolStructure_->maxStrike();
    }

    Time SpreadedSwaptionVolatilityStructure::maxSwapLength() const {
        return underlyingVolStructure_->maxSwapLength();
    }

    BusinessDayConvention
    SpreadedSwaptionVolatilityStructure::businessDayConvention() const {
        return underlyingVolStructure_->businessDayConvention();
    }

    std::pair<Time,Time>
    SpreadedSwaptionVolatilityStructure::convertDates(
                                            const Date& optionDate,
                                            const Period& swapTenor) const {
        return underlyingVolStructure_->convertDates(optionDate, swapTenor);
    }

    DayCounter SpreadedSwaptionVolatilityStructure::dayCounter() const {
        return underlyingVolStructure_->dayCounter();
    }

    Date SpreadedSwaptionVolatilityStructure::maxDate() const {
        return underlyingVolStructure_->maxDate();
    }

    Time SpreadedSwaptionVolatilityStructure::maxTime() const {
        return underlyingVolStructure_->maxTime();
    }

    const Date& SpreadedSwaptionVolatilityStructure::referenceDate() const {
        return underlyingVolStructure_->referenceDate();
    }

    Calendar SpreadedSwaptionVolatilityStructure::calendar() const {
        return underlyingVolStructure_->calendar();
    }

    Natural SpreadedSwaptionVolatilityStructure::settlementDays() const {
        return underlyingVolStructure_->settlementDays();
    }

}
