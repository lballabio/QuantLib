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

#include <ql/termstructures/volatility/optionlet/strippedoptionlet.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/makecapfloor.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    StrippedOptionlet::StrippedOptionlet(const Date& referenceDate,
                          const Calendar& calendar,
                          Natural settlementDays,
                          BusinessDayConvention businessDayConvention,
                          const DayCounter& dc,
                          const boost::shared_ptr<IborIndex>& index,
                          const std::vector<Period>& optionletTenors,
                          const std::vector<Rate>& strikes,
                          const std::vector<std::vector<Handle<Quote> > >& optionletVolQuotes)
    : referenceDate_(referenceDate),
      calendar_(calendar),
      settlementDays_(settlementDays),
      businessDayConvention_(businessDayConvention),
      dc_(dc),
      index_(index),
      nOptionletTenors_(optionletTenors.size()),
      optionletTenors_(optionletTenors),
      nStrikes_(strikes.size()),
      optionletVolQuotes_(optionletVolQuotes){
        
        optionletStrikes_ = std::vector<std::vector<Rate> >(nOptionletTenors_, strikes);
        
        checkInputs();
        registerWith(Settings::instance().evaluationDate());
        registerWithMarketData();

        optionletVolatilities_ = 
            std::vector<std::vector<Volatility> >(nOptionletTenors_, 
                                                  std::vector<Volatility>(nStrikes_));
        
        
        optionletDates_ = std::vector<Date>(nOptionletTenors_);
        optionletTimes_ = std::vector<Time>(nOptionletTenors_);

        CapFloorTermVolSurface tmp(referenceDate_, calendar_, optionletTenors_, 
                                   optionletStrikes_[0], optionletVolQuotes_, 
                                   businessDayConvention_, dc_);
        
        for (Size i=0; i<nOptionletTenors_; ++i) {
            optionletDates_[i] = tmp.optionDateFromTenor(optionletTenors_[i]);
            optionletTimes_[i] = dc_.yearFraction(referenceDate_, optionletDates_[i]);
        }

    }

    void StrippedOptionlet::performCalculations() const {

        for (Size i=0; i<optionletVolatilities_.size(); ++i)
            for (Size j=0; j<optionletVolatilities_[0].size(); ++j)
                optionletVolatilities_[i][j] = optionletVolQuotes_[i][j]->value();

    }

    void StrippedOptionlet::checkInputs() const {

        QL_REQUIRE(!optionletTenors_.empty(), "empty optionlet tenor vector");
        QL_REQUIRE(nOptionletTenors_==optionletVolQuotes_.size(),
                   "mismatch between number of option tenors (" <<
                   nOptionletTenors_ << ") and number of volatility rows (" <<
                   optionletVolQuotes_.size() << ")");
        QL_REQUIRE(optionletTenors_[0]>0*Days,
                   "negative first option tenor: " << optionletTenors_[0]);
        for (Size i=1; i<nOptionletTenors_; ++i)
            QL_REQUIRE(optionletTenors_[i]>optionletTenors_[i-1],
                       "non increasing option tenor: " << io::ordinal(i-1) <<
                       " is " << optionletTenors_[i-1] << ", " <<
                       io::ordinal(i) << " is " << optionletTenors_[i]);

        QL_REQUIRE(nStrikes_==optionletVolQuotes_[0].size(),
                   "mismatch between strikes(" << optionletStrikes_[0].size() <<
                   ") and vol columns (" << optionletVolQuotes_[0].size() << ")");
        for (Size j=1; j<nStrikes_; ++j)
            QL_REQUIRE(optionletStrikes_[0][j-1]<optionletStrikes_[0][j],
                       "non increasing strikes: " << io::ordinal(j-1) <<
                       " is " << io::rate(optionletStrikes_[0][j-1]) << ", " <<
                       io::ordinal(j) << " is " << io::rate(optionletStrikes_[0][j]));
    }

    void StrippedOptionlet::registerWithMarketData()
    {
        for (Size i=0; i<nOptionletTenors_; ++i)
            for (Size j=0; j<nStrikes_; ++j)
                registerWith(optionletVolQuotes_[i][j]);
    }

    const std::vector<Rate>& StrippedOptionlet::optionletStrikes(Size i) const{
        QL_REQUIRE(i<optionletStrikes_.size(), "i >= optionletStrikes_.size()");
        return optionletStrikes_[i];
    }   

    const std::vector<Volatility>& StrippedOptionlet::optionletVolatilities(Size i) const{
        calculate();
        QL_REQUIRE(i<optionletVolatilities_.size(), "i >= optionletVolatilities_.size()");
        return optionletVolatilities_[i];
    }   


    const std::vector<Date>& StrippedOptionlet::optionletDates() const {
        calculate();
        return optionletDates_;
    }
      
    const std::vector<Time>& StrippedOptionlet::optionletTimes() const {
        calculate();
        return optionletTimes_;
    }

    DayCounter StrippedOptionlet::dayCounter() const {
        return dc_;
    }

    Calendar StrippedOptionlet::calendar() const {
        return calendar_;
    }

    Natural StrippedOptionlet::settlementDays() const {
        return settlementDays_;
    }

    BusinessDayConvention StrippedOptionlet::businessDayConvention() const {
        return businessDayConvention_;
    }

    const Date& StrippedOptionlet::referenceDate() const {
        return referenceDate_;
    }

}
