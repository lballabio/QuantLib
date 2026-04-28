/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
 Copyright (C) 2025 Paolo D'Elia
 All rights reserved.

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

#include <ql/instruments/constnotionalcrossccyswap.hpp>
#include <ql/cashflows/simplecashflow.hpp>

namespace QuantLib {

ConstNotionalCrossCcySwap::ConstNotionalCrossCcySwap(const Leg& firstLeg, const Currency& firstLegCcy, const Leg& secondLeg,
                           const Currency& secondLegCcy)
    : Swap(firstLeg, secondLeg), inCcyLegNPV_(2, 0.0), inCcyLegBPS_(2, 0.0), npvDateDiscounts_(2, 0.0) {
    currencies_.resize(2);
    currencies_[0] = firstLegCcy;
    currencies_[1] = secondLegCcy;
}

ConstNotionalCrossCcySwap::ConstNotionalCrossCcySwap(const std::vector<Leg>& legs, const std::vector<bool>& payer,
                           const std::vector<Currency>& currencies)
    : Swap(legs, payer), currencies_(currencies), inCcyLegNPV_(legs.size(), 0.0), inCcyLegBPS_(legs.size(), 0.0), npvDateDiscounts_(legs.size(), 0.0)  {
    QL_REQUIRE(payer.size() == currencies_.size(), "Size mismatch "
                                                   "between payer ("
                                                       << payer.size() << ") and currencies (" << currencies_.size()
                                                       << ")");
}

ConstNotionalCrossCcySwap::ConstNotionalCrossCcySwap(Size legs)
    : Swap(legs), currencies_(legs), inCcyLegNPV_(legs, 0.0), inCcyLegBPS_(legs, 0.0), npvDateDiscounts_(legs, 0.0) {}

void ConstNotionalCrossCcySwap::setupArguments(PricingEngine::arguments* args) const {

    Swap::setupArguments(args);

    ConstNotionalCrossCcySwap::arguments* arguments = dynamic_cast<ConstNotionalCrossCcySwap::arguments*>(args);
    QL_REQUIRE(arguments, "The arguments are not of type "
                          "cross currency swap");

    arguments->currencies = currencies_;
}

void ConstNotionalCrossCcySwap::fetchResults(const PricingEngine::results* r) const {

    Swap::fetchResults(r);

    const ConstNotionalCrossCcySwap::results* results = dynamic_cast<const ConstNotionalCrossCcySwap::results*>(r);
    QL_REQUIRE(results, "The results are not of type "
                        "cross currency swap");

    if (!results->inCcyLegNPV.empty()) {
        QL_REQUIRE(results->inCcyLegNPV.size() == inCcyLegNPV_.size(),
                   "Wrong number of in currency leg NPVs returned by engine");
        inCcyLegNPV_ = results->inCcyLegNPV;
    } else {
        std::fill(inCcyLegNPV_.begin(), inCcyLegNPV_.end(), Null<Real>());
    }

    if (!results->inCcyLegBPS.empty()) {
        QL_REQUIRE(results->inCcyLegBPS.size() == inCcyLegBPS_.size(),
                   "Wrong number of in currency leg BPSs returned by engine");
        inCcyLegBPS_ = results->inCcyLegBPS;
    } else {
        std::fill(inCcyLegBPS_.begin(), inCcyLegBPS_.end(), Null<Real>());
    }

    if (!results->npvDateDiscounts.empty()) {
        QL_REQUIRE(results->npvDateDiscounts.size() == npvDateDiscounts_.size(),
                   "Wrong number of npv date discounts returned by engine");
        npvDateDiscounts_ = results->npvDateDiscounts;
    } else {
        std::fill(npvDateDiscounts_.begin(), npvDateDiscounts_.end(), Null<DiscountFactor>());
    }
}

void ConstNotionalCrossCcySwap::setupExpired() const {
    Swap::setupExpired();
    std::fill(inCcyLegBPS_.begin(), inCcyLegBPS_.end(), 0.0);
    std::fill(inCcyLegNPV_.begin(), inCcyLegNPV_.end(), 0.0);
    std::fill(npvDateDiscounts_.begin(), npvDateDiscounts_.end(), 0.0);
}

void ConstNotionalCrossCcySwap::arguments::validate() const {
    Swap::arguments::validate();
    QL_REQUIRE(legs.size() == currencies.size(), "Number of legs is not equal to number of currencies");
}

void ConstNotionalCrossCcySwap::results::reset() {
    Swap::results::reset();
    inCcyLegNPV.clear();
    inCcyLegBPS.clear();
    npvDateDiscounts.clear();
}

void ConstNotionalCrossCcySwap::addNotionalExchangesToLeg(Leg& leg, const Calendar& calendar, 
    const Date earliestDate, const Date maturityDate, const Natural paymentLag, 
    const BusinessDayConvention legBdc, Real nominal) {
    // Initial notional exchange
    Date aDate = calendar.advance(earliestDate, paymentLag, Days, legBdc);
    ext::shared_ptr<CashFlow> aCashflow = ext::make_shared<SimpleCashFlow>(-nominal, aDate);
    leg.insert(leg.begin(), aCashflow);

    // Final notional exchange
    aDate = calendar.advance(maturityDate, paymentLag, Days, legBdc);
    aCashflow = ext::make_shared<SimpleCashFlow>(nominal, aDate);
    leg.push_back(aCashflow);
}

} // namespace QuantLib
