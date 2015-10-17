/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/experimental/fx/fxtarfengine.hpp>

namespace QuantLib {

FxTarfEngine::FxTarfEngine(const Handle<YieldTermStructure> discount)
    : discount_(discount) {
    QL_REQUIRE(!discount_.empty(), "no discount curve given");
    registerWith(discount_);
    registerWith(Settings::instance().evaluationDate());
}

void FxTarfEngine::calculate() const {

    Date today = Settings::instance().evaluationDate();

    // for the case where we have a non settled amount we need the discount
    // factor for the associated payment date

    int nextPaymentIndex = 1;
    while (detail::simple_event(arguments_.schedule.date(nextPaymentIndex))
               .hasOccurred())
        ++nextPaymentIndex;

    // npv of a already fixed, but unsettled amount

    unsettledAmountNpv_ = 0.0;
    if (!arguments_.isLastAmountSettled) {
        unsettledAmountNpv_ =
            arguments_.lastAmount * arguments_.sourceNominal *
            discount_->discount(arguments_.schedule.date(nextPaymentIndex));
    }

    // case where only one fixing is left which is today or everything is
    // fixed already

    if (arguments_.openFixingDates.back() <= today) {
        Real lastFixingNpv = 0.0;
        if (arguments_.openFixingDates.back() == today) {
            lastFixingNpv =
                arguments_.instrument->payout(arguments_.index->fixing(
                    arguments_.openFixingDates.back())) *
                arguments_.sourceNominal * discount_->discount(today);
        }
        results_.value = unsettledAmountNpv_ + lastFixingNpv;
        results_.errorEstimate = 0.0;
        return;
    }

    // we have at least one fixing left which is tommorow or later
    // this is handled by specialized engines ...
}
} // namespace QuantLib
