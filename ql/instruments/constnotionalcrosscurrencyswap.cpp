/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
 Copyright (C) 2025 Paolo D'Elia

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

#include <ql/instruments/constnotionalcrosscurrencyswap.hpp>

namespace QuantLib {

ConstNotionalCrossCurrencySwap::ConstNotionalCrossCurrencySwap(const Leg& firstLeg, const Currency& firstLegCcy,
                                                                       const Leg& secondLeg, const Currency& secondLegCcy)
: CrossCurrencySwap(firstLeg, firstLegCcy, secondLeg, secondLegCcy) {}

ConstNotionalCrossCurrencySwap::ConstNotionalCrossCurrencySwap(const std::vector<Leg>& legs, const std::vector<bool>& payer,
                                                                       const std::vector<Currency>& currencies)
: CrossCurrencySwap(legs, payer, currencies) {}

ConstNotionalCrossCurrencySwap::ConstNotionalCrossCurrencySwap(Size legs)
: CrossCurrencySwap(legs) {}

void ConstNotionalCrossCurrencySwap::setupArguments(PricingEngine::arguments* args) const {

    CrossCurrencySwap::setupArguments(args);

    auto* arguments = dynamic_cast<ConstNotionalCrossCurrencySwap::arguments*>(args);
    QL_REQUIRE(arguments, "The arguments are not of type "
                          "cross currency swap");
}

void ConstNotionalCrossCurrencySwap::fetchResults(const PricingEngine::results* r) const {

    CrossCurrencySwap::fetchResults(r);

    const auto* results = dynamic_cast<const ConstNotionalCrossCurrencySwap::results*>(r);
    QL_REQUIRE(results, "The results are not of type cross currency swap");
}

void ConstNotionalCrossCurrencySwap::setupExpired() const {
    CrossCurrencySwap::setupExpired();
}

void ConstNotionalCrossCurrencySwap::arguments::validate() const {
    CrossCurrencySwap::arguments::validate();
}

void ConstNotionalCrossCurrencySwap::results::reset() {
    CrossCurrencySwap::results::reset();
}

} // namespace QuantLib
