/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Thiago Rodrigues

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

#include <ql/instruments/di1future.hpp>
#include <ql/event.hpp>
#include <ql/settings.hpp>
#include <cmath>
#include <utility>

namespace QuantLib {

    Di1Future::Di1Future(ext::shared_ptr<OvernightIndex> overnightIndex,
                         const Date& maturityDate,
                         Handle<Quote> convexityAdjustment)
    : overnightIndex_(std::move(overnightIndex)), maturityDate_(maturityDate),
      convexityAdjustment_(std::move(convexityAdjustment)) {
        QL_REQUIRE(overnightIndex_, "null overnight index");
        QL_REQUIRE(maturityDate_ != Date(), "null maturity date");
        registerWith(overnightIndex_);
        registerWith(convexityAdjustment_);
        registerWith(Settings::instance().evaluationDate());
    }

    bool Di1Future::isExpired() const {
        // on its maturity date the contract settles at 100,000 points
        return detail::simple_event(maturityDate_).hasOccurred(Date(), false);
    }

    Real Di1Future::convexityAdjustment() const {
        return convexityAdjustment_.empty() ? 0.0 : convexityAdjustment_->value();
    }

    Date Di1Future::referenceDate() const {
        return Settings::instance().evaluationDate();
    }

    Date::serial_type Di1Future::businessDays() const {
        return overnightIndex_->dayCounter().dayCount(referenceDate(), maturityDate_);
    }

    Rate Di1Future::rate() const {
        calculate();
        QL_REQUIRE(rate_ != Null<Rate>(), "rate not available for expired contract");
        return rate_;
    }

    Real Di1Future::unitPrice() const {
        return NPV();
    }

    void Di1Future::setupExpired() const {
        Instrument::setupExpired();
        rate_ = Null<Rate>();
    }

    void Di1Future::performCalculations() const {
        Handle<YieldTermStructure> curve = overnightIndex_->forwardingTermStructure();
        QL_REQUIRE(!curve.empty(),
                   "null term structure set to this instance of " << overnightIndex_->name());

        Date today = referenceDate();
        QL_REQUIRE(maturityDate_ > today,
                   "maturity date (" << maturityDate_ << ") must be later than the reference date ("
                                     << today << ")");
        Date::serial_type n = businessDays();
        QL_REQUIRE(n > 0, "no business days between the reference date ("
                              << today << ") and the maturity date (" << maturityDate_ << ")");

        Time tau = n / 252.0;
        Real growth = curve->discount(today) / curve->discount(maturityDate_);
        rate_ = std::pow(growth, 1.0 / tau) - 1.0 + convexityAdjustment();
        NPV_ = 100000.0 / std::pow(1.0 + rate_, tau);
    }

}
