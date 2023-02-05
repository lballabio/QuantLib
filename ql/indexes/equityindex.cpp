/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Marcin Rybacki

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

#include <ql/indexes/equityindex.hpp>
#include <ql/settings.hpp>
#include <sstream>
#include <utility>

namespace QuantLib {

    EquityIndex::EquityIndex(std::string name,
                             Currency currency,
                             Calendar fixingCalendar,
                             Handle<YieldTermStructure> rate,
                             Handle<YieldTermStructure> dividend)
    : name_(std::move(name)), currency_(std::move(currency)),
      rate_(std::move(rate)), dividend_(std::move(dividend)),
      settlementCalendar_(std::move(fixingCalendar)) {

        registerWith(rate_);
        registerWith(dividend_);
        registerWith(Settings::instance().evaluationDate());
        registerWith(IndexManager::instance().notifier(EquityIndex::name()));
    }

    Rate EquityIndex::fixing(const Date& fixingDate, bool forecastTodaysFixing) const {

        QL_REQUIRE(isValidFixingDate(fixingDate), "Fixing date " << fixingDate << " is not valid");

        Date today = Settings::instance().evaluationDate();

        if (fixingDate > today || (fixingDate == today && forecastTodaysFixing))
            return forecastFixing(fixingDate);

        if (fixingDate <= today) {
            // today's fixing is required
            // even without enforcing it
            return pastFixing(fixingDate);
        }

        return forecastFixing(fixingDate);
    }

    Real EquityIndex::pastFixing(const Date& fixingDate) const {
        QL_REQUIRE(isValidFixingDate(fixingDate), fixingDate << " is not a valid fixing date");
        Real result = timeSeries()[fixingDate];

        QL_REQUIRE(result != Null<Real>(), "Missing " << name() << " fixing for " << fixingDate);
        return result;
    }

    Real EquityIndex::forecastFixing(const Date& fixingDate) const {
        QL_REQUIRE(!rate_.empty(),
                   "null interest rate term structure set to this instance of " << name());

        Date today = Settings::instance().evaluationDate();
        
        Real spot = pastFixing(today);

        Real forward;
        if (!dividend_.empty()) {
            forward = spot * dividend_->discount(fixingDate) / rate_->discount(fixingDate);
        } else {
            forward = spot / rate_->discount(fixingDate);
        }
        return forward;
    }

}