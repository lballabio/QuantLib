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

#include <ql/termstructures/volatilities/interestrate/caplet/spreadedcapletvolstructure.hpp>

namespace QuantLib {

    SpreadedCapletVolatilityStructure::SpreadedCapletVolatilityStructure(
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
          enableExtrapolation(underlyingVol->allowsExtrapolation());
    }

    Volatility SpreadedCapletVolatilityStructure::volatilityImpl(
                                                    Time length,
                                                    Rate strike) const {
        return underlyingVolStructure_->volatility(length, strike)
                                                        +spread_->value();
    }

    Rate SpreadedCapletVolatilityStructure::minStrike() const {
        return underlyingVolStructure_->minStrike();
    }
    
    Rate SpreadedCapletVolatilityStructure::maxStrike() const {
        return underlyingVolStructure_->maxStrike();
    }

    DayCounter SpreadedCapletVolatilityStructure::dayCounter() const {
        return underlyingVolStructure_->dayCounter();
    }
    
    Date SpreadedCapletVolatilityStructure::maxDate() const {
        return underlyingVolStructure_->maxDate();
    }

    Time SpreadedCapletVolatilityStructure::maxTime() const {
        return underlyingVolStructure_->maxTime();
    }
        
    const Date& SpreadedCapletVolatilityStructure::referenceDate() const {
        return underlyingVolStructure_->referenceDate();
    }
        
    Calendar SpreadedCapletVolatilityStructure::calendar() const {
        return underlyingVolStructure_->calendar();
    }
        
    Natural SpreadedCapletVolatilityStructure::settlementDays() const {
        return underlyingVolStructure_->settlementDays();
    }
}
