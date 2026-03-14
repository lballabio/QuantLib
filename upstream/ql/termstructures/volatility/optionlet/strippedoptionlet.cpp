/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti
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

#include <ql/instruments/makecapfloor.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/termstructures/volatility/optionlet/strippedoptionlet.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

using std::vector;

namespace QuantLib {

    StrippedOptionlet::StrippedOptionlet(Natural settlementDays,
                                         const Calendar& calendar,
                                         BusinessDayConvention bdc,
                                         ext::shared_ptr<IborIndex> iborIndex,
                                         const vector<Date>& optionletDates,
                                         const vector<Rate>& strikes,
                                         vector<vector<Handle<Quote>>> v,
                                         DayCounter dc,
                                         VolatilityType type,
                                         Real displacement)
    : StrippedOptionlet(settlementDays,
                        calendar,
                        bdc,
                        std::move(iborIndex),
                        optionletDates,
                        vector<vector<Rate>>(optionletDates.size(), strikes),
                        std::move(v),
                        std::move(dc),
                        type,
                        displacement) {}

    StrippedOptionlet::StrippedOptionlet(Natural settlementDays,
                                         const Calendar& calendar,
                                         BusinessDayConvention bdc,
                                         ext::shared_ptr<IborIndex> iborIndex,
                                         const vector<Date>& optionletDates,
                                         const vector<vector<Rate>>& strikes,
                                         vector<vector<Handle<Quote>>> v,
                                         DayCounter dc,
                                         VolatilityType type,
                                         Real displacement)
    : calendar_(calendar), settlementDays_(settlementDays), businessDayConvention_(bdc),
      dc_(std::move(dc)), iborIndex_(std::move(iborIndex)), type_(type),
      displacement_(displacement), nOptionletDates_(optionletDates.size()),
      optionletDates_(optionletDates), optionletTimes_(nOptionletDates_),
      optionletAtmRates_(nOptionletDates_), optionletStrikes_(strikes),
      optionletVolQuotes_(std::move(v)) {
        checkInputs();

        for (Size i = 0; i < nOptionletDates_; ++i)
            optionletVolatilities_.emplace_back(strikes[i].size());

        registerWith(Settings::instance().evaluationDate());
        registerWithMarketData();

        Date refDate = calendar.advance(Settings::instance().evaluationDate(),
                                        settlementDays, Days);

        for (Size i=0; i<nOptionletDates_; ++i)
            optionletTimes_[i] = dc_.yearFraction(refDate, optionletDates_[i]);
    }

    void StrippedOptionlet::checkInputs() const {

        QL_REQUIRE(!optionletDates_.empty(), "empty optionlet tenor vector");
        QL_REQUIRE(nOptionletDates_==optionletVolQuotes_.size(),
                   "mismatch between number of option tenors (" <<
                   nOptionletDates_ << ") and number of volatility rows (" <<
                   optionletVolQuotes_.size() << ")");
        QL_REQUIRE(optionletDates_[0]>Settings::instance().evaluationDate(),
                   "first option date (" << optionletDates_[0] << ") is in the past");
        for (Size i=1; i<nOptionletDates_; ++i)
            QL_REQUIRE(optionletDates_[i]>optionletDates_[i-1],
                       "non increasing option dates: " << io::ordinal(i) <<
                       " is " << optionletDates_[i-1] << ", " <<
                       io::ordinal(i+1) << " is " << optionletDates_[i]);
        QL_REQUIRE(nOptionletDates_ == optionletStrikes_.size(),
                   "mismatch between number of option tenors (" << nOptionletDates_
                                                                << ") and number of strikes ("
                                                                << optionletStrikes_.size() << ")");
        for (Size i = 0; i < nOptionletDates_; ++i) {
            QL_REQUIRE(optionletStrikes_[i].size() == optionletVolQuotes_[i].size(),
                       "mismatch between number of option tenors ("
                           << nOptionletDates_ << ") and number of vol columns at date " << i
                           << " (" << optionletVolQuotes_[i].size());
            for (Size j = 1; j < optionletStrikes_[i].size(); ++j)
                QL_REQUIRE(optionletStrikes_[i][j - 1] < optionletStrikes_[i][j],
                           "non increasing strikes at date "
                               << i << ": " << io::ordinal(j) << " is "
                               << io::rate(optionletStrikes_[0][j - 1]) << ", "
                               << io::ordinal(j + 1) << " is "
                               << io::rate(optionletStrikes_[0][j]));
        }
    }

    void StrippedOptionlet::registerWithMarketData() {
        for (Size i = 0; i < nOptionletDates_; ++i)
            for (auto& j : optionletVolQuotes_[i])
                registerWith(j);
    }

    void StrippedOptionlet::performCalculations() const {
        for (Size i = 0; i < nOptionletDates_; ++i)
            for (Size j = 0; j < optionletVolQuotes_[i].size(); ++j)
                optionletVolatilities_[i][j] = optionletVolQuotes_[i][j]->value();
    }

    const vector<Rate>& StrippedOptionlet::optionletStrikes(Size i) const{
        QL_REQUIRE(i<optionletStrikes_.size(),
                   "index (" << i <<
                   ") must be less than optionletStrikes size (" <<
                   optionletStrikes_.size() << ")");
        return optionletStrikes_[i];
    }

    const vector<Volatility>&
    StrippedOptionlet::optionletVolatilities(Size i) const{
        calculate();
        QL_REQUIRE(i<optionletVolatilities_.size(),
                   "index (" << i <<
                   ") must be less than optionletVolatilities size (" <<
                   optionletVolatilities_.size() << ")");
        return optionletVolatilities_[i];
    }

    const vector<Date>& StrippedOptionlet::optionletFixingDates() const {
        calculate();
        return optionletDates_;
    }

    const vector<Time>& StrippedOptionlet::optionletFixingTimes() const {
        calculate();
        return optionletTimes_;
    }

    Size StrippedOptionlet::optionletMaturities() const {
        return nOptionletDates_;
    }

    const vector<Time>& StrippedOptionlet::atmOptionletRates() const {
        calculate();
        for (Size i=0; i<nOptionletDates_; ++i)
            optionletAtmRates_[i] = iborIndex_->fixing(optionletDates_[i], true);
        return optionletAtmRates_;
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

    VolatilityType StrippedOptionlet::volatilityType() const {
        return type_;
    }

    Real StrippedOptionlet::displacement() const {
        return displacement_;
    }

}
