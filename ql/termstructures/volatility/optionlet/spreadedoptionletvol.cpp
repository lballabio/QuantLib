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

#include <ql/termstructures/volatility/optionlet/spreadedoptionletvol.hpp>
#include <ql/termstructures/volatility/spreadedsmilesection.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    SpreadedOptionletVol::SpreadedOptionletVol(
            const Handle<OptionletVolatilityStructure>& underlyingVol,
            const Handle<Quote>& spread)
    : OptionletVolatilityStructure(underlyingVol->settlementDays(), 
                                   underlyingVol->calendar(),
                                   underlyingVol->businessDayConvention(),
                                   underlyingVol->dayCounter()),
      underlyingVolStructure_(underlyingVol),
      spread_(spread) {
          registerWith(underlyingVolStructure_);
          registerWith(spread_);
          //enableExtrapolation(underlyingVol->allowsExtrapolation());
    }

    Volatility SpreadedOptionletVol::volatilityImpl(const Date& optionDate,
                                                    Rate strike) const {
        return underlyingVolStructure_->volatility(optionDate,
                                                   strike)+spread_->value();
    }

    Volatility SpreadedOptionletVol::volatilityImpl(Time optionTime,
                                                    Rate strike) const {
        return underlyingVolStructure_->volatility(optionTime,
                                                   strike)+spread_->value();
    }

    boost::shared_ptr<SmileSection>
    SpreadedOptionletVol::smileSectionImpl(const Date& optionDate) const {
        boost::shared_ptr<SmileSection> underlyingSmile =
            underlyingVolStructure_->smileSection(optionDate);
        return boost::shared_ptr<SmileSection>(new
            SpreadedSmileSection(underlyingSmile, spread_));
    }

    boost::shared_ptr<SmileSection>
    SpreadedOptionletVol::smileSectionImpl(Time optionTime) const {
        boost::shared_ptr<SmileSection> underlyingSmile =
            underlyingVolStructure_->smileSection(optionTime);
        return boost::shared_ptr<SmileSection>(new
            SpreadedSmileSection(underlyingSmile, spread_));
    }

    Rate SpreadedOptionletVol::minStrike() const {
        return underlyingVolStructure_->minStrike();
    }
    
    Rate SpreadedOptionletVol::maxStrike() const {
        return underlyingVolStructure_->maxStrike();
    }

    BusinessDayConvention
    SpreadedOptionletVol::businessDayConvention() const {
        return underlyingVolStructure_->businessDayConvention();
    }

    DayCounter SpreadedOptionletVol::dayCounter() const {
        return underlyingVolStructure_->dayCounter();
    }
    
    Date SpreadedOptionletVol::maxDate() const {
        return underlyingVolStructure_->maxDate();
    }

    Time SpreadedOptionletVol::maxTime() const {
        return underlyingVolStructure_->maxTime();
    }
        
    const Date& SpreadedOptionletVol::referenceDate() const {
        return underlyingVolStructure_->referenceDate();
    }
        
    Calendar SpreadedOptionletVol::calendar() const {
        return underlyingVolStructure_->calendar();
    }
        
    Natural SpreadedOptionletVol::settlementDays() const {
        return underlyingVolStructure_->settlementDays();
    }
}
