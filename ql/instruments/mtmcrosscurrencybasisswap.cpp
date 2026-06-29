/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/instruments/mtmcrosscurrencybasisswap.hpp>
#include <utility>

namespace QuantLib {

namespace {

void checkResettingLegData(
    const MtMCrossCurrencyBasisSwap::ResettingLegData& resettingLegData,
    Size numberOfLegs,
    const std::vector<Currency>& currencies) {

    QL_REQUIRE(resettingLegData.resettingLegIndex < numberOfLegs,
               "Resetting leg index (" << resettingLegData.resettingLegIndex << ") out of range");
    QL_REQUIRE(resettingLegData.constantLegIndex < numberOfLegs,
               "Constant leg index (" << resettingLegData.constantLegIndex << ") out of range");
    QL_REQUIRE(resettingLegData.resettingLegIndex != resettingLegData.constantLegIndex,
               "Resetting leg and constant leg must be different");
    QL_REQUIRE(resettingLegData.constantLegNotional != Null<Real>(),
               "Constant leg notional cannot be null");
    QL_REQUIRE(resettingLegData.constantLegNotional != 0.0,
               "Constant leg notional cannot be zero");

    if (resettingLegData.fxIndex) {
        // The FxIndex may quote the pair in either market orientation; require only
        // that its two currencies are the swap's two leg currencies.  The pricing
        // engine orients the fixing to resettable-per-constant as needed.
        const Currency& src = resettingLegData.fxIndex->sourceCurrency();
        const Currency& tgt = resettingLegData.fxIndex->targetCurrency();
        const Currency& constantCcy = currencies[resettingLegData.constantLegIndex];
        const Currency& resettableCcy = currencies[resettingLegData.resettingLegIndex];
        QL_REQUIRE((src == constantCcy && tgt == resettableCcy) ||
                       (src == resettableCcy && tgt == constantCcy),
                   "FxIndex " << resettingLegData.fxIndex->name() << " currencies (" << src << ", "
                              << tgt << ") must be the swap's leg currencies (" << constantCcy
                              << ", " << resettableCcy << ")");
    }
}

}

MtMCrossCurrencyBasisSwap::ResettingLegData::ResettingLegData(
    Size resettingLegIndex,
    Size constantLegIndex,
    Real constantLegNotional,
    Integer paymentLag,
    Calendar paymentCalendar,
    BusinessDayConvention paymentConvention,
    ext::shared_ptr<FxIndex> fxIndex)
: resettingLegIndex(resettingLegIndex), constantLegIndex(constantLegIndex),
  constantLegNotional(constantLegNotional), paymentLag(paymentLag),
  paymentCalendar(std::move(paymentCalendar)), paymentConvention(paymentConvention),
  fxIndex(std::move(fxIndex)) {}

MtMCrossCurrencyBasisSwap::MtMCrossCurrencyBasisSwap(
    Type type,
    Real fxBaseNominal, Currency fxBaseCurrency, Schedule fxBaseSchedule,
    const ext::shared_ptr<IborIndex>& fxBaseIndex, Spread fxBaseSpread, Real fxBaseGearing,
    Real fxQuoteNominal, Currency fxQuoteCurrency, Schedule fxQuoteSchedule,
    const ext::shared_ptr<IborIndex>& fxQuoteIndex, Spread fxQuoteSpread, Real fxQuoteGearing,
    bool isFxBaseCurrencyLegResettable,
    const ext::shared_ptr<FxIndex>& fxIndex,
    Integer fxBasePaymentLag, Integer fxQuotePaymentLag,
    bool fxBaseCompoundSpread, Natural fxBaseLookbackDays, bool fxBaseObservationShift,
    Natural fxBaseLockoutDays, RateAveraging::Type fxBaseAveragingMethod,
    bool fxQuoteCompoundSpread, Natural fxQuoteLookbackDays, bool fxQuoteObservationShift,
    Natural fxQuoteLockoutDays, RateAveraging::Type fxQuoteAveragingMethod,
    const bool telescopicValueDates)
: CrossCurrencySwap(2),
  type_(type),
  resettingLegData_(isFxBaseCurrencyLegResettable ? 0 : 1,
                    isFxBaseCurrencyLegResettable ? 1 : 0,
                    isFxBaseCurrencyLegResettable ? fxQuoteNominal : fxBaseNominal,
                    isFxBaseCurrencyLegResettable ? fxBasePaymentLag : fxQuotePaymentLag,
                    isFxBaseCurrencyLegResettable ? fxBaseSchedule.calendar()
                                                  : fxQuoteSchedule.calendar(),
                    isFxBaseCurrencyLegResettable ? fxBaseSchedule.businessDayConvention()
                                                  : fxQuoteSchedule.businessDayConvention(),
                    fxIndex),
  fxBaseNominal_(fxBaseNominal),
  fxBaseCurrency_(std::move(fxBaseCurrency)), fxBaseSchedule_(std::move(fxBaseSchedule)),
  fxBaseIndex_(fxBaseIndex), fxBaseSpread_(fxBaseSpread), fxBaseGearing_(fxBaseGearing),
  fxQuoteNominal_(fxQuoteNominal), fxQuoteCurrency_(std::move(fxQuoteCurrency)),
  fxQuoteSchedule_(std::move(fxQuoteSchedule)), fxQuoteIndex_(fxQuoteIndex),
  fxQuoteSpread_(fxQuoteSpread), fxQuoteGearing_(fxQuoteGearing),
  isFxBaseCurrencyLegResettable_(isFxBaseCurrencyLegResettable),
  fxBasePaymentLag_(fxBasePaymentLag), fxQuotePaymentLag_(fxQuotePaymentLag),
  fxBaseCompoundSpread_(fxBaseCompoundSpread), fxBaseLookbackDays_(fxBaseLookbackDays),
  fxBaseObservationShift_(fxBaseObservationShift), fxBaseLockoutDays_(fxBaseLockoutDays),
  fxBaseAveragingMethod_(fxBaseAveragingMethod),
  fxQuoteCompoundSpread_(fxQuoteCompoundSpread), fxQuoteLookbackDays_(fxQuoteLookbackDays),
  fxQuoteObservationShift_(fxQuoteObservationShift), fxQuoteLockoutDays_(fxQuoteLockoutDays),
  fxQuoteAveragingMethod_(fxQuoteAveragingMethod), telescopicValueDates_(telescopicValueDates) {
    registerWith(fxBaseIndex_);
    registerWith(fxQuoteIndex_);
    registerWith(resettingLegData_.fxIndex);
    initialize();
}

void MtMCrossCurrencyBasisSwap::initialize() {
    // Base-currency leg
    if (auto on = ext::dynamic_pointer_cast<OvernightIndex>(fxBaseIndex_)) {
        legs_[0] = OvernightLeg(fxBaseSchedule_, on)
                       .withNotionals(fxBaseNominal_)
                       .withSpreads(fxBaseSpread_)
                       .withGearings(fxBaseGearing_)
                       .withPaymentLag(fxBasePaymentLag_)
                       .compoundingSpreadDaily(fxBaseCompoundSpread_)
                       .withLookbackDays(fxBaseLookbackDays_)
                       .withObservationShift(fxBaseObservationShift_)
                       .withLockoutDays(fxBaseLockoutDays_)
                       .withAveragingMethod(fxBaseAveragingMethod_)
                       .withTelescopicValueDates(telescopicValueDates_);
    } else {
        legs_[0] = IborLeg(fxBaseSchedule_, fxBaseIndex_)
                       .withNotionals(fxBaseNominal_)
                       .withSpreads(fxBaseSpread_)
                       .withGearings(fxBaseGearing_)
                       .withPaymentLag(fxBasePaymentLag_);
    }
    payer_[0] = paysFxBaseCurrency() ? -1.0 : +1.0;
    currencies_[0] = fxBaseCurrency_;

    // Quote-currency leg
    if (auto on = ext::dynamic_pointer_cast<OvernightIndex>(fxQuoteIndex_)) {
        legs_[1] = OvernightLeg(fxQuoteSchedule_, on)
                       .withNotionals(fxQuoteNominal_)
                       .withSpreads(fxQuoteSpread_)
                       .withGearings(fxQuoteGearing_)
                       .withPaymentLag(fxQuotePaymentLag_)
                       .compoundingSpreadDaily(fxQuoteCompoundSpread_)
                       .withLookbackDays(fxQuoteLookbackDays_)
                       .withObservationShift(fxQuoteObservationShift_)
                       .withLockoutDays(fxQuoteLockoutDays_)
                       .withAveragingMethod(fxQuoteAveragingMethod_)
                       .withTelescopicValueDates(telescopicValueDates_);
    } else {
        legs_[1] = IborLeg(fxQuoteSchedule_, fxQuoteIndex_)
                       .withNotionals(fxQuoteNominal_)
                       .withSpreads(fxQuoteSpread_)
                       .withGearings(fxQuoteGearing_)
                       .withPaymentLag(fxQuotePaymentLag_);
    }
    payer_[1] = -payer_[0];
    currencies_[1] = fxQuoteCurrency_;

    auto earliestDate =
        std::min(CashFlows::startDate(legs_[0]), CashFlows::startDate(legs_[1]));
    auto maturityDate =
        std::max(CashFlows::maturityDate(legs_[0]), CashFlows::maturityDate(legs_[1]));

    // The resettable leg carries no explicit notional exchanges: the per-period
    // FX re-exchanges that replace them are handled analytically by the engine.
    // Only the constant-notional leg gets the inception/maturity exchange flows.
    if (resettingLegIndex() != 0)
        CrossCurrencySwap::addNotionalExchangesToLeg(
            legs_[0], fxBaseSchedule_.calendar(), earliestDate, maturityDate, fxBasePaymentLag_,
            fxBaseSchedule_.businessDayConvention(), fxBaseNominal_);

    if (resettingLegIndex() != 1)
        CrossCurrencySwap::addNotionalExchangesToLeg(
            legs_[1], fxQuoteSchedule_.calendar(), earliestDate, maturityDate, fxQuotePaymentLag_,
            fxQuoteSchedule_.businessDayConvention(), fxQuoteNominal_);

    for (Size legNo = 0; legNo < 2; ++legNo) {
        for (auto& cf : legs_[legNo])
            registerWith(cf);
    }

    validateResettingLegData();
}

void MtMCrossCurrencyBasisSwap::validateResettingLegData() const {
    checkResettingLegData(resettingLegData_, legs_.size(), currencies_);
}

void MtMCrossCurrencyBasisSwap::setupArguments(PricingEngine::arguments* args) const {

    CrossCurrencySwap::setupArguments(args);

    auto* arguments = dynamic_cast<MtMCrossCurrencyBasisSwap::arguments*>(args);

    QL_REQUIRE(arguments != nullptr, "wrong argument type");

    arguments->resettingLegData = resettingLegData_;
    arguments->fxBaseSpread = fxBaseSpread_;
    arguments->fxQuoteSpread = fxQuoteSpread_;
}

void MtMCrossCurrencyBasisSwap::fetchResults(const PricingEngine::results* r) const {

    CrossCurrencySwap::fetchResults(r);

    const auto* results = dynamic_cast<const MtMCrossCurrencyBasisSwap::results*>(r);
    if (results != nullptr) {
        fairFxBaseSpread_ = results->fairFxBaseSpread;
        fairFxQuoteSpread_ = results->fairFxQuoteSpread;
    } else {
        fairFxBaseSpread_ = Null<Spread>();
        fairFxQuoteSpread_ = Null<Spread>();
    }

    static Spread basisPoint = 1.0e-4;
    if (fairFxBaseSpread_ == Null<Spread>()) {
        if (legBPS_[0] != Null<Real>())
            fairFxBaseSpread_ = fxBaseSpread_ - NPV_ / (legBPS_[0] / basisPoint);
    }
    if (fairFxQuoteSpread_ == Null<Spread>()) {
        if (legBPS_[1] != Null<Real>())
            fairFxQuoteSpread_ = fxQuoteSpread_ - NPV_ / (legBPS_[1] / basisPoint);
    }
}

void MtMCrossCurrencyBasisSwap::setupExpired() const {
    CrossCurrencySwap::setupExpired();
    fairFxBaseSpread_ = Null<Spread>();
    fairFxQuoteSpread_ = Null<Spread>();
}

void MtMCrossCurrencyBasisSwap::arguments::validate() const {
    CrossCurrencySwap::arguments::validate();
    checkResettingLegData(resettingLegData, legs.size(), currencies);
    QL_REQUIRE(fxBaseSpread != Null<Spread>(), "FX-base spread cannot be null");
    QL_REQUIRE(fxQuoteSpread != Null<Spread>(), "FX-quote spread cannot be null");
}

void MtMCrossCurrencyBasisSwap::results::reset() {
    CrossCurrencySwap::results::reset();
    fairFxBaseSpread = Null<Spread>();
    fairFxQuoteSpread = Null<Spread>();
}

} // namespace QuantLib
