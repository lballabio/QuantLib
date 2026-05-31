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

#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/instruments/constnotionalcrosscurrencybasisswap.hpp>
#include <utility>

namespace QuantLib  {

ConstNotionalCrossCurrencyBasisSwap::ConstNotionalCrossCurrencyBasisSwap(
                                     Real payNominal, Currency  payCurrency, Schedule  paySchedule,
                                     const ext::shared_ptr<IborIndex>& payIndex, Spread paySpread, Real payGearing,
                                     Real recNominal, Currency  recCurrency, Schedule  recSchedule,
                                     const ext::shared_ptr<IborIndex>& recIndex, Spread recSpread, Real recGearing,
                                     Integer payPaymentLag, Integer recPaymentLag,
                                     bool payCompoundSpread, Natural payLookbackDays, bool payObservationShift,
                                     Natural payLockoutDays, RateAveraging::Type payAveragingMethod,
                                     bool recCompoundSpread, Natural recLookbackDays, bool recObservationShift,
                                     Natural recLockoutDays, RateAveraging::Type recAveragingMethod,
                                     const bool telescopicValueDates)
    : ConstNotionalCrossCurrencySwap(2), payNominal_(payNominal), payCurrency_(std::move(payCurrency)), paySchedule_(std::move(paySchedule)),
      payIndex_(payIndex), paySpread_(paySpread), payGearing_(payGearing), recNominal_(recNominal),
      recCurrency_(std::move(recCurrency)), recSchedule_(std::move(recSchedule)), recIndex_(recIndex), recSpread_(recSpread),
      recGearing_(recGearing), payPaymentLag_(payPaymentLag), recPaymentLag_(recPaymentLag),
      payCompoundSpread_(payCompoundSpread), payLookbackDays_(payLookbackDays),
      payObservationShift_(payObservationShift), payLockoutDays_(payLockoutDays),
      payAveragingMethod_(payAveragingMethod), recCompoundSpread_(recCompoundSpread),
      recLookbackDays_(recLookbackDays), recObservationShift_(recObservationShift),
      recLockoutDays_(recLockoutDays), recAveragingMethod_(recAveragingMethod), telescopicValueDates_(telescopicValueDates) {
    registerWith(payIndex_);
    registerWith(recIndex_);
    initialize();
}

void ConstNotionalCrossCurrencyBasisSwap::initialize() {
    // Pay leg
    if (auto on = ext::dynamic_pointer_cast<OvernightIndex>(payIndex_)) {
        // ON leg
        legs_[0] = OvernightLeg(paySchedule_, on)
                        .withNotionals(payNominal_)
                        .withSpreads(paySpread_)
                        .withGearings(payGearing_)
                        .withPaymentLag(payPaymentLag_)
                        .compoundingSpreadDaily(payCompoundSpread_)
                        .withLookbackDays(payLookbackDays_)
                        .withObservationShift(payObservationShift_)
                        .withLockoutDays(payLockoutDays_)
                        .withAveragingMethod(payAveragingMethod_)
                        .withTelescopicValueDates(telescopicValueDates_);
    } else {
        // Ibor leg
        legs_[0] = IborLeg(paySchedule_, payIndex_)
                       .withNotionals(payNominal_)
                       .withSpreads(paySpread_)
                       .withGearings(payGearing_)
                       .withPaymentLag(payPaymentLag_);
    }
    payer_[0] = -1.0;
    currencies_[0] = payCurrency_;

    // Receive leg
    if (auto on = ext::dynamic_pointer_cast<OvernightIndex>(recIndex_)) {
        // ON leg
        legs_[1] = OvernightLeg(recSchedule_, on)
                        .withNotionals(recNominal_)
                        .withSpreads(recSpread_)
                        .withGearings(recGearing_)
                        .withPaymentLag(recPaymentLag_)
                        .compoundingSpreadDaily(recCompoundSpread_)
                        .withLookbackDays(recLookbackDays_)
                        .withObservationShift(recObservationShift_)
                        .withLockoutDays(recLockoutDays_)
                        .withAveragingMethod(recAveragingMethod_)
                        .withTelescopicValueDates(telescopicValueDates_);
    } else {
        // Ibor leg
        legs_[1] = IborLeg(recSchedule_, recIndex_)
                       .withNotionals(recNominal_)
                       .withSpreads(recSpread_)
                       .withGearings(recGearing_)
                       .withPaymentLag(recPaymentLag_);
    }
    payer_[1] = +1.0;
    currencies_[1] = recCurrency_;

    auto earliestDate = std::min(CashFlows::startDate(legs_[0]),
                                 CashFlows::startDate(legs_[1]));
    auto maturityDate = std::max(CashFlows::maturityDate(legs_[0]),
                                 CashFlows::maturityDate(legs_[1]));

    // Add notional exchanges on payLeg
    ConstNotionalCrossCurrencySwap::addNotionalExchangesToLeg(legs_[0], paySchedule_.calendar(), earliestDate, maturityDate,
                                            payPaymentLag_, paySchedule_.businessDayConvention(), payNominal_);

    // Add notional exchanges on recLeg
    ConstNotionalCrossCurrencySwap::addNotionalExchangesToLeg(legs_[1], recSchedule_.calendar(), earliestDate, maturityDate,
                                            recPaymentLag_, recSchedule_.businessDayConvention(), recNominal_);

    // Register the instrument with all cashflows on each leg.
    for (Size legNo = 0; legNo < 2; legNo++) {
        Leg::iterator it;
        for (it = legs_[legNo].begin(); it != legs_[legNo].end(); ++it) {
            registerWith(*it);
        }
    }
}

void ConstNotionalCrossCurrencyBasisSwap::setupArguments(PricingEngine::arguments* args) const {

    ConstNotionalCrossCurrencySwap::setupArguments(args);

    auto* arguments = dynamic_cast<ConstNotionalCrossCurrencyBasisSwap::arguments*>(args);

    /* Returns here if e.g. args is ConstNotionalCrossCurrencySwap::arguments which
       is the case if PricingEngine is a ConstNotionalCrossCurrencySwap::engine. */
    if (arguments == nullptr)
        return;

    arguments->paySpread = paySpread_;
    arguments->recSpread = recSpread_;
}

void ConstNotionalCrossCurrencyBasisSwap::fetchResults(const PricingEngine::results* r) const {

    ConstNotionalCrossCurrencySwap::fetchResults(r);

    const auto* results = dynamic_cast<const ConstNotionalCrossCurrencyBasisSwap::results*>(r);
    if (results != nullptr) {
        /* If PricingEngine::results are of type
           ConstNotionalCrossCurrencyBasisSwap::results */
        fairPaySpread_ = results->fairPaySpread;
        fairRecSpread_ = results->fairRecSpread;
    } else {
        /* If not, e.g. if the engine is a ConstNotionalCrossCurrencySwap::engine */
        fairPaySpread_ = Null<Spread>();
        fairRecSpread_ = Null<Spread>();
    }

    /* Calculate the fair pay and receive spreads if they are null */
    static Spread basisPoint = 1.0e-4;
    if (fairPaySpread_ == Null<Spread>()) {
        if (legBPS_[0] != Null<Real>())
            fairPaySpread_ = paySpread_ - NPV_ / (legBPS_[0] / basisPoint);
    }
    if (fairRecSpread_ == Null<Spread>()) {
        if (legBPS_[1] != Null<Real>())
            fairRecSpread_ = recSpread_ - NPV_ / (legBPS_[1] / basisPoint);
    }
}

void ConstNotionalCrossCurrencyBasisSwap::setupExpired() const {
    ConstNotionalCrossCurrencySwap::setupExpired();
    fairPaySpread_ = Null<Spread>();
    fairRecSpread_ = Null<Spread>();
}

void ConstNotionalCrossCurrencyBasisSwap::arguments::validate() const {
    ConstNotionalCrossCurrencySwap::arguments::validate();
    QL_REQUIRE(paySpread != Null<Spread>(), "Pay spread cannot be null");
    QL_REQUIRE(recSpread != Null<Spread>(), "Rec spread cannot be null");
}

void ConstNotionalCrossCurrencyBasisSwap::results::reset() {
    ConstNotionalCrossCurrencySwap::results::reset();
    fairPaySpread = Null<Spread>();
    fairRecSpread = Null<Spread>();
}

} // namespace QuantLib
