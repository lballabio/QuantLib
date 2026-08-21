/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Kyrylo Protsenko

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
#include <ql/experimental/fx/fxresetcashflows.hpp>
#include <ql/experimental/fx/mtmcrosscurrencybasisswap.hpp>
#include <utility>

namespace QuantLib {

MtMCrossCurrencyBasisSwap::MtMCrossCurrencyBasisSwap(
    Type type,
    Real fxBaseNominal, Currency fxBaseCurrency, Schedule fxBaseSchedule,
    const ext::shared_ptr<IborIndex>& fxBaseIndex, Spread fxBaseSpread, Real fxBaseGearing,
    Real fxQuoteNominal, Currency fxQuoteCurrency, Schedule fxQuoteSchedule,
    const ext::shared_ptr<IborIndex>& fxQuoteIndex, Spread fxQuoteSpread, Real fxQuoteGearing,
    bool isFxBaseCurrencyLegResettable,
    Natural fxResetFixingDays, const Calendar& fxResetFixingCalendar,
    Integer fxBasePaymentLag, Integer fxQuotePaymentLag,
    BusinessDayConvention fxBasePaymentConvention,
    BusinessDayConvention fxQuotePaymentConvention,
    bool fxBaseCompoundSpread, Natural fxBaseLookbackDays, bool fxBaseObservationShift,
    Natural fxBaseLockoutDays, RateAveraging::Type fxBaseAveragingMethod,
    bool fxQuoteCompoundSpread, Natural fxQuoteLookbackDays, bool fxQuoteObservationShift,
    Natural fxQuoteLockoutDays, RateAveraging::Type fxQuoteAveragingMethod,
    const bool telescopicValueDates,
    std::optional<bool> useIndexedCoupons,
    StubIndexConfig fxBaseStubIndexConfig, StubIndexConfig fxQuoteStubIndexConfig)
: CrossCurrencySwap(2),
  type_(type),
  fxBaseNominal_(fxBaseNominal),
  fxBaseCurrency_(std::move(fxBaseCurrency)), fxBaseSchedule_(std::move(fxBaseSchedule)),
  fxBaseIndex_(fxBaseIndex), fxBaseSpread_(fxBaseSpread), fxBaseGearing_(fxBaseGearing),
  fxBaseStubIndexConfig_(std::move(fxBaseStubIndexConfig)),
  fxQuoteNominal_(fxQuoteNominal), fxQuoteCurrency_(std::move(fxQuoteCurrency)),
  fxQuoteSchedule_(std::move(fxQuoteSchedule)), fxQuoteIndex_(fxQuoteIndex),
  fxQuoteSpread_(fxQuoteSpread), fxQuoteGearing_(fxQuoteGearing),
  fxQuoteStubIndexConfig_(std::move(fxQuoteStubIndexConfig)),
  isFxBaseCurrencyLegResettable_(isFxBaseCurrencyLegResettable),
  fxResetConvention_(
      fxResetFixingDays,
      fxResetFixingDays != 0 && fxResetFixingCalendar.empty() ?
          (isFxBaseCurrencyLegResettable_ ? fxBaseSchedule_.calendar() :
                                           fxQuoteSchedule_.calendar()) :
          fxResetFixingCalendar),
  fxBasePaymentLag_(fxBasePaymentLag), fxQuotePaymentLag_(fxQuotePaymentLag),
  fxBasePaymentConvention_(fxBasePaymentConvention),
  fxQuotePaymentConvention_(fxQuotePaymentConvention),
  useIndexedCoupons_(useIndexedCoupons),
  fxBaseCompoundSpread_(fxBaseCompoundSpread), fxBaseLookbackDays_(fxBaseLookbackDays),
  fxBaseObservationShift_(fxBaseObservationShift), fxBaseLockoutDays_(fxBaseLockoutDays),
  fxBaseAveragingMethod_(fxBaseAveragingMethod),
  fxQuoteCompoundSpread_(fxQuoteCompoundSpread), fxQuoteLookbackDays_(fxQuoteLookbackDays),
  fxQuoteObservationShift_(fxQuoteObservationShift), fxQuoteLockoutDays_(fxQuoteLockoutDays),
  fxQuoteAveragingMethod_(fxQuoteAveragingMethod), telescopicValueDates_(telescopicValueDates) {
    registerWith(fxBaseIndex_);
    registerWith(fxQuoteIndex_);
    initialize();
}

void MtMCrossCurrencyBasisSwap::initialize() {
    // Base-currency leg
    if (auto on = ext::dynamic_pointer_cast<OvernightIndex>(fxBaseIndex_)) {
        legs_[0] = OvernightLeg(fxBaseSchedule_, on)
                       .withNotionals(fxBaseNominal_)
                       .withSpreads(fxBaseSpread_)
                       .withGearings(fxBaseGearing_)
                       .withPaymentAdjustment(fxBasePaymentConvention_)
                       .withPaymentCalendar(fxBaseSchedule_.calendar())
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
                       .withPaymentAdjustment(fxBasePaymentConvention_)
                       .withPaymentCalendar(fxBaseSchedule_.calendar())
                       .withPaymentLag(fxBasePaymentLag_)
                       .withIndexedCoupons(useIndexedCoupons_)
                       .withStubIndexConfig(fxBaseStubIndexConfig_);
    }
    payer_[0] = paysFxBaseCurrency() ? -1.0 : +1.0;
    currencies_[0] = fxBaseCurrency_;

    // Quote-currency leg
    if (auto on = ext::dynamic_pointer_cast<OvernightIndex>(fxQuoteIndex_)) {
        legs_[1] = OvernightLeg(fxQuoteSchedule_, on)
                       .withNotionals(fxQuoteNominal_)
                       .withSpreads(fxQuoteSpread_)
                       .withGearings(fxQuoteGearing_)
                       .withPaymentAdjustment(fxQuotePaymentConvention_)
                       .withPaymentCalendar(fxQuoteSchedule_.calendar())
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
                       .withPaymentAdjustment(fxQuotePaymentConvention_)
                       .withPaymentCalendar(fxQuoteSchedule_.calendar())
                       .withPaymentLag(fxQuotePaymentLag_)
                       .withIndexedCoupons(useIndexedCoupons_)
                       .withStubIndexConfig(fxQuoteStubIndexConfig_);
    }
    payer_[1] = -payer_[0];
    currencies_[1] = fxQuoteCurrency_;

    auto earliestDate =
        std::min(CashFlows::startDate(legs_[0]), CashFlows::startDate(legs_[1]));
    auto maturityDate =
        std::max(CashFlows::maturityDate(legs_[0]), CashFlows::maturityDate(legs_[1]));

    // The resettable leg's coupons are replaced by FX-resetting equivalents,
    // whose notional is the constant-leg notional converted at each period's
    // reset, and the netted notional exchanges of a mark-to-market leg are
    // added: the first notional at inception, the reset difference at each
    // period boundary, and the last notional at maturity.  The FX rates the
    // flows convert at are attached later by the pricing engine.
    QL_REQUIRE(constantLegNotional() != Null<Real>(), "Constant leg notional cannot be null");
    QL_REQUIRE(constantLegNotional() != 0.0, "Constant leg notional cannot be zero");

    Size resettingLegNo = resettingLegIndex();
    const Schedule& resettingSchedule = resettingLegNo == 0 ? fxBaseSchedule_ : fxQuoteSchedule_;
    BusinessDayConvention paymentConvention =
        resettingLegNo == 0 ? fxBasePaymentConvention_ : fxQuotePaymentConvention_;
    Calendar paymentCalendar = resettingSchedule.calendar();

    Leg resettingLeg;
    resettingLeg.reserve(2 * legs_[resettingLegNo].size() + 1);
    std::optional<FxReset> previousReset;  // empty: first exchange has no maturing period
    for (const auto& cf : legs_[resettingLegNo]) {
        auto coupon = ext::dynamic_pointer_cast<FloatingRateCoupon>(cf);
        QL_REQUIRE(coupon, "unexpected non-coupon cash flow on the resettable leg");
        FxReset reset = fxResetConvention_.reset(coupon->accrualStartDate());
        // An interim exchange establishes the new notional at the current
        // accrual period's FX value date.  Coupon payment lag does not apply
        // to notional exchanges.  The initial exchange is made on the
        // effective date, adjusted with the leg's payment convention.
        Date exchangeDate =
            previousReset ?
                reset.valueDate() :
                paymentCalendar.adjust(earliestDate, paymentConvention);
        resettingLeg.push_back(ext::make_shared<FxResetNotionalExchange>(
            exchangeDate, constantLegNotional(), previousReset, reset));
        resettingLeg.push_back(
            ext::make_shared<FxResetCoupon>(coupon, constantLegNotional(), reset));
        previousReset = reset;
    }
    resettingLeg.push_back(ext::make_shared<FxResetNotionalExchange>(
        paymentCalendar.adjust(maturityDate, paymentConvention),
        constantLegNotional(), previousReset, std::nullopt));
    sortLegByDate(resettingLeg);
    legs_[resettingLegNo] = resettingLeg;

    // Only the constant-notional leg gets the inception/maturity exchange flows.
    if (resettingLegNo != 0)
        CrossCurrencySwap::addNotionalExchangesToLeg(
            legs_[0], fxBaseSchedule_.calendar(), earliestDate, maturityDate,
            fxBasePaymentConvention_, fxBaseNominal_);

    if (resettingLegNo != 1)
        CrossCurrencySwap::addNotionalExchangesToLeg(
            legs_[1], fxQuoteSchedule_.calendar(), earliestDate, maturityDate,
            fxQuotePaymentConvention_, fxQuoteNominal_);

    for (Size legNo = 0; legNo < 2; ++legNo) {
        for (auto& cf : legs_[legNo])
            registerWith(cf);
    }
}

void MtMCrossCurrencyBasisSwap::setupArguments(PricingEngine::arguments* args) const {

    CrossCurrencySwap::setupArguments(args);

    auto* arguments = dynamic_cast<MtMCrossCurrencyBasisSwap::arguments*>(args);

    QL_REQUIRE(arguments != nullptr, "wrong argument type");

    arguments->resettingLegIndex = resettingLegIndex();
    arguments->constantLegIndex = constantLegIndex();
    arguments->fxResetFixingDays = fxResetConvention_.fixingDays();
    arguments->fxResetFixingCalendar = fxResetConvention_.fixingCalendar();
    arguments->fxBaseSpread = fxBaseSpread_;
    arguments->fxQuoteSpread = fxQuoteSpread_;
}

void MtMCrossCurrencyBasisSwap::fetchResults(const PricingEngine::results* r) const {

    CrossCurrencySwap::fetchResults(r);

    const auto* results = dynamic_cast<const MtMCrossCurrencyBasisSwap::results*>(r);
    if (results != nullptr) {
        fairFxBaseSpread_ = results->fairFxBaseSpread;
        fairFxQuoteSpread_ = results->fairFxQuoteSpread;
        fxResetRates_ = results->fxResetRates;
        fxResetNotionals_ = results->fxResetNotionals;
    } else {
        fairFxBaseSpread_ = Null<Spread>();
        fairFxQuoteSpread_ = Null<Spread>();
        fxResetRates_.clear();
        fxResetNotionals_.clear();
    }

    static Spread basisPoint = 1.0e-4;
    if (fairFxBaseSpread_ == Null<Spread>()) {
        if (legBPS_[0] != Null<Real>()) {
            QL_REQUIRE(legBPS_[0] != 0.0,
                       "cannot calculate fair FX-base spread: null leg BPS");
            fairFxBaseSpread_ = fxBaseSpread_ - NPV_ / (legBPS_[0] / basisPoint);
        }
    }
    if (fairFxQuoteSpread_ == Null<Spread>()) {
        if (legBPS_[1] != Null<Real>()) {
            QL_REQUIRE(legBPS_[1] != 0.0,
                       "cannot calculate fair FX-quote spread: null leg BPS");
            fairFxQuoteSpread_ = fxQuoteSpread_ - NPV_ / (legBPS_[1] / basisPoint);
        }
    }
}

void MtMCrossCurrencyBasisSwap::setupExpired() const {
    CrossCurrencySwap::setupExpired();
    fairFxBaseSpread_ = Null<Spread>();
    fairFxQuoteSpread_ = Null<Spread>();
    fxResetRates_.clear();
    fxResetNotionals_.clear();
}

void MtMCrossCurrencyBasisSwap::arguments::validate() const {
    CrossCurrencySwap::arguments::validate();
    QL_REQUIRE(resettingLegIndex != Null<Size>(), "Resetting leg index cannot be null");
    QL_REQUIRE(constantLegIndex != Null<Size>(), "Constant leg index cannot be null");
    QL_REQUIRE(resettingLegIndex < legs.size(),
               "Resetting leg index (" << resettingLegIndex << ") out of range");
    QL_REQUIRE(constantLegIndex < legs.size(),
               "Constant leg index (" << constantLegIndex << ") out of range");
    QL_REQUIRE(resettingLegIndex != constantLegIndex,
               "Resetting leg and constant leg must be different");
    QL_REQUIRE(fxBaseSpread != Null<Spread>(), "FX-base spread cannot be null");
    QL_REQUIRE(fxQuoteSpread != Null<Spread>(), "FX-quote spread cannot be null");
}

void MtMCrossCurrencyBasisSwap::results::reset() {
    CrossCurrencySwap::results::reset();
    fairFxBaseSpread = Null<Spread>();
    fairFxQuoteSpread = Null<Spread>();
    fxResetRates.clear();
    fxResetNotionals.clear();
}

} // namespace QuantLib
