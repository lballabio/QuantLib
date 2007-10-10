/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
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

#include <ql/voltermstructures/interestrate/optionlet/optionletstripper.hpp>
#include <ql/indexes/iborindex.hpp>


namespace QuantLib {

    OptionletStripper::OptionletStripper(
            const boost::shared_ptr<CapFloorTermVolSurface>& termVolSurface,
            const boost::shared_ptr<IborIndex>& index)
    : termVolSurface_(termVolSurface),
      index_(index),
      nStrikes_(termVolSurface->strikes().size()){
        
        registerWith(termVolSurface);
        registerWith(index);
        registerWith(Settings::instance().evaluationDate());

        Period indexTenor = index->tenor();
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
        
        optionletVolatilities_ = std::vector<std::vector<Volatility> >(nOptionletTenors_, 
                                                                       std::vector<Volatility>(nStrikes_));
        optionletStrikes_ = std::vector<std::vector<Rate> >(nOptionletTenors_, termVolSurface->strikes());
        optionletDates_ = std::vector<Date>(nOptionletTenors_);
        optionletTimes_ = std::vector<Time>(nOptionletTenors_);
        atmOptionletRate_ = std::vector<Rate>(nOptionletTenors_);
        optionletPaymentDates_ = std::vector<Date>(nOptionletTenors_);
        optionletAccrualPeriods_ = std::vector<Time>(nOptionletTenors_);
    }

    const std::vector<Rate>& OptionletStripper::optionletStrikes(Size i) const{
        calculate();
        QL_REQUIRE(i<optionletStrikes_.size(), "i >= optionletStrikes_.size()");
        return optionletStrikes_[i];
    }   

    const std::vector<Volatility>& OptionletStripper::optionletVolatilities(Size i) const{
        calculate();
        QL_REQUIRE(i<optionletVolatilities_.size(), "i >= optionletVolatilities_.size()");
        return optionletVolatilities_[i];
    }   

    const std::vector<Period>& OptionletStripper::optionletTenors() const {
        return optionletTenors_;
    }

    const std::vector<Date>& OptionletStripper::optionletDates() const {
        calculate();
        return optionletDates_;
    }
      
    const std::vector<Date>& OptionletStripper::optionletPaymentDates() const {
        calculate();
        return optionletPaymentDates_;
    }  

    const std::vector<Time>& OptionletStripper::optionletTimes() const {
        calculate();
        return optionletTimes_;
    }
     
    const std::vector<Time>& OptionletStripper::optionletAccrualPeriods() const {
        calculate();
        return optionletAccrualPeriods_;
    }

    const std::vector<Rate>& OptionletStripper::atmOptionletRate() const {
        calculate();
        return atmOptionletRate_;
    }
    
    boost::shared_ptr<CapFloorTermVolSurface>
    OptionletStripper::termVolSurface() const {
        return termVolSurface_;
    }

    boost::shared_ptr<IborIndex> OptionletStripper::index() const {
        return index_;
    }
}
