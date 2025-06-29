/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2015 Peter Caspers

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

#include <ql/indexes/iborindex.hpp>
#include <ql/termstructures/volatility/optionlet/optionletstripper.hpp>
#include <utility>

using std::vector;

namespace QuantLib {

    OptionletStripper::OptionletStripper(
        const ext::shared_ptr<CapFloorTermVolSurface>& termVolSurface,
        ext::shared_ptr<IborIndex> iborIndex,
        Handle<YieldTermStructure> discount,
        const VolatilityType type,
        const Real displacement,
        ext::optional<Period> optionletFrequency
    )
    : termVolSurface_(termVolSurface), iborIndex_(std::move(iborIndex)),
      discount_(std::move(discount)), nStrikes_(termVolSurface->strikes().size()),
      volatilityType_(type), displacement_(displacement), 
      optionletFrequency_(optionletFrequency) {

        if (volatilityType_ == Normal) {
            QL_REQUIRE(displacement_ == 0.0,
                       "non-null displacement is not allowed with Normal model");
        }

        registerWith(termVolSurface);
        registerWith(iborIndex_);
        registerWith(discount_);
        registerWith(Settings::instance().evaluationDate());

        Period indexTenor = optionletFrequency_ ? *optionletFrequency_ : iborIndex_->tenor();
        Period maxCapFloorTenor = termVolSurface->optionTenors().back();

        // optionlet tenors and capFloor lengths
        optionletTenors_.push_back(indexTenor);
        capFloorLengths_.push_back(optionletTenors_.back()+indexTenor);
        QL_REQUIRE(maxCapFloorTenor>=capFloorLengths_.back(),
                   "too short (" << maxCapFloorTenor <<
                   ") capfloor term vol termVolSurface");
        Period nextCapFloorLength = capFloorLengths_.back()+indexTenor;
        while (nextCapFloorLength<=maxCapFloorTenor) {
            optionletTenors_.push_back(capFloorLengths_.back());
            capFloorLengths_.push_back(nextCapFloorLength);
            nextCapFloorLength += indexTenor;
        }
        nOptionletTenors_ = optionletTenors_.size();
        
        optionletVolatilities_ =
            vector<vector<Volatility> >(nOptionletTenors_, 
                                        vector<Volatility>(nStrikes_));
        optionletStrikes_ = vector<vector<Rate> >(nOptionletTenors_,
                                                  termVolSurface->strikes());
        optionletDates_ = vector<Date>(nOptionletTenors_);
        optionletTimes_ = vector<Time>(nOptionletTenors_);
        atmOptionletRate_ = vector<Rate>(nOptionletTenors_);
        optionletPaymentDates_ = vector<Date>(nOptionletTenors_);
        optionletAccrualPeriods_ = vector<Time>(nOptionletTenors_);
    }

    const vector<Rate>& OptionletStripper::optionletStrikes(Size i) const {
        calculate();
        QL_REQUIRE(i<optionletStrikes_.size(),
                   "index (" << i <<
                   ") must be less than optionletStrikes size (" <<
                   optionletStrikes_.size() << ")");
        return optionletStrikes_[i];
    }   

    const vector<Volatility>&
    OptionletStripper::optionletVolatilities(Size i) const {
        calculate();
        QL_REQUIRE(i<optionletVolatilities_.size(),
                   "index (" << i <<
                   ") must be less than optionletVolatilities size (" <<
                   optionletVolatilities_.size() << ")");
        return optionletVolatilities_[i];
    }   

    const vector<Period>& OptionletStripper::optionletFixingTenors() const {
        return optionletTenors_;
    }

    const vector<Date>& OptionletStripper::optionletFixingDates() const {
        calculate();
        return optionletDates_;
    }
      
    const vector<Time>& OptionletStripper::optionletFixingTimes() const {
        calculate();
        return optionletTimes_;
    }
     
    Size OptionletStripper::optionletMaturities() const {
        return optionletTenors_.size();
    }

    const vector<Date>& OptionletStripper::optionletPaymentDates() const {
        calculate();
        return optionletPaymentDates_;
    }  

    const vector<Time>& OptionletStripper::optionletAccrualPeriods() const {
        calculate();
        return optionletAccrualPeriods_;
    }

    const vector<Rate>& OptionletStripper::atmOptionletRates() const {
        calculate();
        return atmOptionletRate_;
    }
    

    DayCounter OptionletStripper::dayCounter() const {
        return termVolSurface_->dayCounter();
    }

    Calendar OptionletStripper::calendar() const {
        return termVolSurface_->calendar();
    }

    Natural OptionletStripper::settlementDays() const {
        return termVolSurface_->settlementDays();
    }

    BusinessDayConvention OptionletStripper::businessDayConvention() const {
        return termVolSurface_->businessDayConvention();
    }

    ext::shared_ptr<CapFloorTermVolSurface>
    OptionletStripper::termVolSurface() const {
        return termVolSurface_;
    }

    ext::shared_ptr<IborIndex> OptionletStripper::iborIndex() const {
        return iborIndex_;
    }

    Real OptionletStripper::displacement() const {
        return displacement_;
    }

    VolatilityType OptionletStripper::volatilityType() const {
        return volatilityType_;
    }

    ext::optional<Period> OptionletStripper::optionletFrequency() const {
        return optionletFrequency_;
    }

}
