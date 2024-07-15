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
#include <utility>

namespace QuantLib {

    namespace {
        Real resolveSpot(const Handle<Quote>& spot, Real lastFixing) {
            QL_REQUIRE(!spot.empty() || lastFixing != Null<Real>(),
                       "Cannot forecast equity index, missing both spot and historical index");
            return spot.empty() ? lastFixing : spot->value();
        }
    }

    EquityIndex::EquityIndex(std::string name,
                             Calendar fixingCalendar,
                             Handle<YieldTermStructure> interest,
                             Handle<YieldTermStructure> dividend,
                             Handle<Quote> spot)
    : EquityIndex(std::move(name),
                  std::move(fixingCalendar),
                  Currency(),
                  std::move(interest),
                  std::move(dividend),
                  std::move(spot)) {}

    EquityIndex::EquityIndex(std::string name,
                             Calendar fixingCalendar,
                             Currency currency,
                             Handle<YieldTermStructure> interest,
                             Handle<YieldTermStructure> dividend,
                             Handle<Quote> spot)
    : name_(std::move(name)), fixingCalendar_(std::move(fixingCalendar)),
      currency_(std::move(currency)), interest_(std::move(interest)),
      dividend_(std::move(dividend)), spot_(std::move(spot)) {

        registerWith(interest_);
        registerWith(dividend_);
        registerWith(spot_);
        registerWith(Settings::instance().evaluationDate());
        registerWith(IndexManager::instance().notifier(EquityIndex::name()));
    }

    Real EquityIndex::fixing(const Date& fixingDate, bool forecastTodaysFixing) const {

        QL_REQUIRE(isValidFixingDate(fixingDate), "Fixing date " << fixingDate << " is not valid");

        Date today = Settings::instance().evaluationDate();

        if (fixingDate > today || (fixingDate == today && forecastTodaysFixing))
            return forecastFixing(fixingDate);

        Real result = pastFixing(fixingDate);

        if (result != Null<Real>())
            // if historical fixing is present use it
            return result;
        
        if (fixingDate == today && !spot_.empty())
            // Today's fixing is missing, but spot is
            // provided, so use it as proxy
            return spot_->value();
        
        QL_FAIL("Missing " << name() << " fixing for " << fixingDate);
    }

    Real EquityIndex::pastFixing(const Date& fixingDate) const {
        QL_REQUIRE(isValidFixingDate(fixingDate), fixingDate << " is not a valid fixing date");
        return timeSeries()[fixingDate];
    }

    Real EquityIndex::forecastFixing(const Date& fixingDate) const {
        QL_REQUIRE(!interest_.empty(),
                   "null interest rate term structure set to this instance of " << name());

        Date today = Settings::instance().evaluationDate();
        Date lastFixingDate = fixingCalendar_.adjust(today, BusinessDayConvention::Preceding);

        Real spot = resolveSpot(spot_, pastFixing(lastFixingDate));

        Real forward;
        if (!dividend_.empty()) {
            forward = spot * dividend_->discount(fixingDate) / interest_->discount(fixingDate);
        } else {
            forward = spot / interest_->discount(fixingDate);
        }
        return forward;
    }

    ext::shared_ptr<EquityIndex> EquityIndex::clone(const Handle<YieldTermStructure>& interest,
                                                    const Handle<YieldTermStructure>& dividend,
                                                    const Handle<Quote>& spot) const {
        return ext::make_shared<EquityIndex>(name(), fixingCalendar(), currency(), interest,
                                             dividend, spot);
    }
}