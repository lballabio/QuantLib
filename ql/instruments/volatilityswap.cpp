/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Yang Ye

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

#include <ql/event.hpp>
#include <ql/instruments/volatilityswap.hpp>

namespace QuantLib {

    VolatilitySwap::VolatilitySwap(Position::Type position,
                                   Real strike,
                                   Real notional,
                                   const Date& startDate,
                                   const Date& maturityDate)
    : position_(position), strike_(strike), notional_(notional),
      startDate_(startDate), maturityDate_(maturityDate) {}

    Real VolatilitySwap::fairVolatility() const {
        calculate();
        QL_REQUIRE(fairVolatility_ != Null<Real>(), "result not available");
        return fairVolatility_;
    }

    void VolatilitySwap::setupExpired() const {
        Instrument::setupExpired();
        fairVolatility_ = Null<Real>();
    }

    void VolatilitySwap::setupArguments(PricingEngine::arguments* args) const {
        auto* arguments = dynamic_cast<VolatilitySwap::arguments*>(args);
        QL_REQUIRE(arguments != nullptr, "wrong argument type");

        arguments->position = position_;
        arguments->strike = strike_;
        arguments->notional = notional_;
        arguments->startDate = startDate_;
        arguments->maturityDate = maturityDate_;
    }

    void VolatilitySwap::fetchResults(const PricingEngine::results* r) const {
        Instrument::fetchResults(r);
        const auto* results = dynamic_cast<const VolatilitySwap::results*>(r);
        QL_REQUIRE(results != nullptr, "wrong result type");
        fairVolatility_ = results->fairVolatility;
    }

    void VolatilitySwap::arguments::validate() const {
        QL_REQUIRE(strike != Null<Real>(), "no strike given");
        QL_REQUIRE(strike >= 0.0, "negative strike given");
        QL_REQUIRE(notional != Null<Real>(), "no notional given");
        QL_REQUIRE(notional > 0.0, "negative or null notional given");
        QL_REQUIRE(startDate != Date(), "null start date given");
        QL_REQUIRE(maturityDate != Date(), "null maturity date given");
        QL_REQUIRE(maturityDate > startDate,
                   "maturity date (" << maturityDate << ") must be after "
                   "start date (" << startDate << ")");
    }

    bool VolatilitySwap::isExpired() const {
        return detail::simple_event(maturityDate_).hasOccurred();
    }

}
