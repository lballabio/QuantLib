/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Katiuscia Manzoni

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

#include <ql/voltermstructures/interestrate/optionlet/optionletstripperadapter.hpp>
#include <ql/voltermstructures/interestrate/optionlet/optionletstripper.hpp>
#include <ql/voltermstructures/interestrate/capfloor/capfloortermvolsurface.hpp>

namespace QuantLib {

    OptionletStripperAdapter::OptionletStripperAdapter(const Handle<OptionletStripper>& optionletStripper)
    : OptionletVolatilityStructure(optionletStripper->termVolSurface()->settlementDays(),
                                   optionletStripper->termVolSurface()->calendar(),
                                   optionletStripper->termVolSurface()->businessDayConvention(),
                                   optionletStripper->termVolSurface()->dayCounter()),
      optionletStripper_(optionletStripper){

        registerWith(optionletStripper_);
    }

    Volatility OptionletStripperAdapter::volatilityImpl(Time length,
                                                        Rate strike) const {
        calculate();
        return interpolation_(strike, length, true);
    }
        
    void OptionletStripperAdapter::performCalculations() const {

        const std::vector<Rate>& strikes = optionletStripper_->strikes();
        const std::vector<Time>& optionletTimes = optionletStripper_->optionletTimes();
        interpolation_ = BilinearInterpolation(
                                       strikes.begin(),  
                                       strikes.end(),
                                       optionletTimes.begin(),
                                       optionletTimes.end(),
                                       optionletStripper_->optionletVolatilities());    
    }

    Rate OptionletStripperAdapter::minStrike() const {
        return optionletStripper_->strikes().front();
    }
    
    Rate OptionletStripperAdapter::maxStrike() const {
        return optionletStripper_->strikes().back();
    }
    
    Date OptionletStripperAdapter::maxDate() const {
        return optionletStripper_->optionletDates().back();
    }
    
    const Date& OptionletStripperAdapter::referenceDate() const {
        return optionletStripper_->termVolSurface()->referenceDate();
    }
}
