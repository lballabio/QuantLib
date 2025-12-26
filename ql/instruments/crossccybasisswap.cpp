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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/instruments/crossccybasisswap.hpp>

namespace QuantLib  {

CrossCcyBasisSwap::CrossCcyBasisSwap(Real payNominal, const Currency& payCurrency, const Schedule& paySchedule,
                                     const ext::shared_ptr<IborIndex>& payIndex, Spread paySpread, Real payGearing,
                                     Real recNominal, const Currency& recCurrency, const Schedule& recSchedule,
                                     const ext::shared_ptr<IborIndex>& recIndex, Spread recSpread, Real recGearing,
                                     Size payPaymentLag, Size recPaymentLag, ext::optional<bool> payIncludeSpread,
                                     ext::optional<Natural> payLookback, ext::optional<Size> payLockoutDays,
                                     ext::optional<bool> payIsAveraged, ext::optional<bool> recIncludeSpread,
                                     ext::optional<Natural> recLookback, ext::optional<Size> recLockoutDays,
                                     ext::optional<bool> recIsAveraged, const bool telescopicValueDates)
    : CrossCcySwap(2), payNominal_(payNominal), payCurrency_(payCurrency), paySchedule_(paySchedule),
      payIndex_(payIndex), paySpread_(paySpread), payGearing_(payGearing), recNominal_(recNominal),
      recCurrency_(recCurrency), recSchedule_(recSchedule), recIndex_(recIndex), recSpread_(recSpread),
      recGearing_(recGearing), payPaymentLag_(payPaymentLag), recPaymentLag_(recPaymentLag),
      payIncludeSpread_(payIncludeSpread), payLookback_(payLookback), payLockoutDays_(payLockoutDays), 
      payIsAveraged_(payIsAveraged), recIncludeSpread_(recIncludeSpread), recLookback_(recLookback), 
      recLockoutDays_(recLockoutDays), recIsAveraged_(recIsAveraged), telescopicValueDates_(telescopicValueDates) {
    registerWith(payIndex_);
    registerWith(recIndex_);
    initialize();
}

void CrossCcyBasisSwap::initialize() {
    // Pay leg
    if (auto on = ext::dynamic_pointer_cast<OvernightIndex>(payIndex_)) {
        // ON leg
        legs_[0] = OvernightLeg(paySchedule_, on)
                        .withNotionals(payNominal_)
                        .withSpreads(paySpread_)
                        .withGearings(payGearing_)
                        .withPaymentLag(payPaymentLag_)
                        .withSpreads(payIncludeSpread_ ? *payIncludeSpread_ : false)
                        .withLookbackDays(payLookback_ ? *payLookback_ : 0)
                        .withLockoutDays(payLockoutDays_ ? *payLockoutDays_ : 0)
                        .withAveragingMethod(payIsAveraged_ ? 
                            (*payIsAveraged_ ? RateAveraging::Simple : RateAveraging::Compound) : RateAveraging::Compound)
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
    // Pay leg notional exchange at start.
    Date initialPayDate = paySchedule_.dates().front();
    ext::shared_ptr<CashFlow> initialPayCF(new SimpleCashFlow(-payNominal_, initialPayDate));
    legs_[0].insert(legs_[0].begin(), initialPayCF);
    // Pay leg notional exchange at end.
    Date finalPayDate = paySchedule_.dates().back();
    ext::shared_ptr<CashFlow> finalPayCF(new SimpleCashFlow(payNominal_, finalPayDate));
    legs_[0].push_back(finalPayCF);

    // Receive leg
    if (auto on = ext::dynamic_pointer_cast<OvernightIndex>(recIndex_)) {
        // ON leg
        legs_[1] = OvernightLeg(recSchedule_, on)
                        .withNotionals(recNominal_)
                        .withSpreads(recSpread_)
                        .withGearings(recGearing_)
                        .withPaymentLag(recPaymentLag_)
                        .withSpreads(recIncludeSpread_ ? *recIncludeSpread_ : false)
                        .withLookbackDays(recLookback_ ? *recLookback_ : 0)
                        .withLockoutDays(recLockoutDays_ ? *recLockoutDays_ : 0)
                        .withAveragingMethod(recIsAveraged_ ? 
                            (*recIsAveraged_ ? RateAveraging::Simple : RateAveraging::Compound) : RateAveraging::Compound)
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
    // Receive leg notional exchange at start.
    Date initialRecDate = recSchedule_.dates().front();
    ext::shared_ptr<CashFlow> initialRecCF(new SimpleCashFlow(-recNominal_, initialRecDate));
    legs_[1].insert(legs_[1].begin(), initialRecCF);
    // Receive leg notional exchange at end.
    Date finalRecDate = recSchedule_.dates().back();
    ext::shared_ptr<CashFlow> finalRecCF(new SimpleCashFlow(recNominal_, finalRecDate));
    legs_[1].push_back(finalRecCF);

    // Register the instrument with all cashflows on each leg.
    for (Size legNo = 0; legNo < 2; legNo++) {
        Leg::iterator it;
        for (it = legs_[legNo].begin(); it != legs_[legNo].end(); ++it) {
            registerWith(*it);
        }
    }
}

void CrossCcyBasisSwap::setupArguments(PricingEngine::arguments* args) const {

    CrossCcySwap::setupArguments(args);

    CrossCcyBasisSwap::arguments* arguments = dynamic_cast<CrossCcyBasisSwap::arguments*>(args);

    /* Returns here if e.g. args is CrossCcySwap::arguments which
       is the case if PricingEngine is a CrossCcySwap::engine. */
    if (!arguments)
        return;

    arguments->paySpread = paySpread_;
    arguments->recSpread = recSpread_;
}

void CrossCcyBasisSwap::fetchResults(const PricingEngine::results* r) const {

    CrossCcySwap::fetchResults(r);

    const CrossCcyBasisSwap::results* results = dynamic_cast<const CrossCcyBasisSwap::results*>(r);
    if (results) {
        /* If PricingEngine::results are of type
           CrossCcyBasisSwap::results */
        fairPaySpread_ = results->fairPaySpread;
        fairRecSpread_ = results->fairRecSpread;
    } else {
        /* If not, e.g. if the engine is a CrossCcySwap::engine */
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

void CrossCcyBasisSwap::setupExpired() const {
    CrossCcySwap::setupExpired();
    fairPaySpread_ = Null<Spread>();
    fairRecSpread_ = Null<Spread>();
}

void CrossCcyBasisSwap::arguments::validate() const {
    CrossCcySwap::arguments::validate();
    QL_REQUIRE(paySpread != Null<Spread>(), "Pay spread cannot be null");
    QL_REQUIRE(recSpread != Null<Spread>(), "Rec spread cannot be null");
}

void CrossCcyBasisSwap::results::reset() {
    CrossCcySwap::results::reset();
    fairPaySpread = Null<Spread>();
    fairRecSpread = Null<Spread>();
}
} // namespace QuantLib
