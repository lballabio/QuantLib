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

/*! \file mtmcrosscurrencybasisswap.hpp
    \brief Mark-to-market (resetting-notional) cross-currency basis swap
*/

#ifndef quantlib_mtm_cross_currency_basis_swap_hpp
#define quantlib_mtm_cross_currency_basis_swap_hpp

#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/rateaveraging.hpp>
#include <ql/experimental/fx/fxresetcashflows.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/crosscurrencyswap.hpp>
#include <ql/optional.hpp>
#include <ql/pricingengine.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

//! Mark-to-market cross-currency basis swap
/*! A float-vs-float cross-currency basis swap in which the notional of one leg
    (the \e resettable leg) is reset for each coupon period to track the
    prevailing FX rate, while the other leg keeps a constant notional that is
    exchanged at inception and maturity.  Each reset has a fixing date, on
    which the FX rate is observed, and a value date at the start of the accrual
    period.

    Following the convention of MtMCrossCurrencyBasisSwapRateHelper, the two legs
    are identified by their role in the FX pair: the \e base-currency leg and the
    \e quote-currency leg, and \c isFxBaseCurrencyLegResettable selects which of
    them carries the resetting notional.  The trade \c Type controls whether the
    base-currency leg is paid or received.

    Resetting the notional removes most of the FX exposure of the principal and
    is the market-standard convention for many currency pairs.  The resettable
    leg is built from FX-linked cash flows: FxResetCoupon instances, whose
    notional is the constant-leg notional converted at the period's FX-reset
    observation, and FxResetNotionalExchange instances paying the netted
    notional differences (the first notional at inception, the reset difference
    at each period boundary, and the last notional at maturity).

    During instrument pricing, the FX rates are supplied to local cash-flow
    wrappers by an engine that understands FX-resetting legs (see
    DiscountingMtMCrossCurrencyBasisSwapEngine).  Independently querying the
    live reset cash flows requires assigning an FxResetPricer with
    setFxResetPricer.  A future reset is projected as the forward FX implied by
    its pricer's discount curves.  If a reset fixing date precedes the
    evaluation date, the realized exchange rate must be available from
    \c ExchangeRateManager on that fixing date.  Direct, inverse, and derived
    exchange rates are supported.

    \ingroup instruments
*/
class MtMCrossCurrencyBasisSwap : public CrossCurrencySwap {
  public:
    class arguments;
    class results;
    class engine;

    enum class Type { PayFxBaseCurrency, ReceiveFxBaseCurrency };

    //! \name Constructors
    //@{
    /*! Payment lags apply only to coupon payments.  Initial and final notional
        exchanges remain on the effective and maturity dates, adjusted by their
        leg's payment convention.  Interim reset exchanges occur on the current
        reset value date.

        \param type  Pay or receive the FX base-currency leg.
        \param isFxBaseCurrencyLegResettable  If true the base-currency (first)
                                   leg is the resettable leg; otherwise the
                                   quote-currency (second) leg resets.  The other
                                   leg keeps a constant notional and exchanges it
                                   at inception and maturity.
        \param fxResetFixingDays   Fixing days used to derive each FX fixing date
                                   from its accrual-start value date.  The default
                                   preserves the legacy behavior of fixing on the
                                   accrual start date.
        \param fxResetFixingCalendar   Calendar used to derive each FX fixing date.
                                       For a non-zero fixing lag,
                                       an empty fixing calendar is replaced by the
                                       resettable leg's schedule calendar.
        \param fxBasePaymentLag  Coupon payment lag for the base-currency leg.
        \param fxQuotePaymentLag Coupon payment lag for the quote-currency leg.
        \param fxBasePaymentConvention  Payment convention for the base-currency
                                        leg and its notional exchanges.
        \param fxQuotePaymentConvention Payment convention for the quote-currency
                                        leg and its notional exchanges.
        \param useIndexedCoupons If provided, overrides the global IborCoupon
                                  setting for both legs.
        \param fxBaseStubIndexConfig  Index selection applied to irregular
                                  coupons of the base-currency leg when it is an
                                  Ibor leg (see StubIndexConfig).  The default
                                  prices broken periods off the leg's own index.
        \param fxQuoteStubIndexConfig  As fxBaseStubIndexConfig, for the
                                  quote-currency leg.
    */
    MtMCrossCurrencyBasisSwap(
        Type type,
        Real fxBaseNominal, Currency fxBaseCurrency, Schedule fxBaseSchedule,
        const ext::shared_ptr<IborIndex>& fxBaseIndex,
        Spread fxBaseSpread,
        Real fxBaseGearing,
        Real fxQuoteNominal, Currency fxQuoteCurrency, Schedule fxQuoteSchedule,
        const ext::shared_ptr<IborIndex>& fxQuoteIndex,
        Spread fxQuoteSpread,
        Real fxQuoteGearing,
        bool isFxBaseCurrencyLegResettable,
        Natural fxResetFixingDays = 0,
        const Calendar& fxResetFixingCalendar = Calendar(),
        Integer fxBasePaymentLag = 0, Integer fxQuotePaymentLag = 0,
        BusinessDayConvention fxBasePaymentConvention = Following,
        BusinessDayConvention fxQuotePaymentConvention = Following,
        bool fxBaseCompoundSpread = false,
        Natural fxBaseLookbackDays = Null<Natural>(),
        bool fxBaseObservationShift = false,
        Natural fxBaseLockoutDays = 0,
        RateAveraging::Type fxBaseAveragingMethod = RateAveraging::Compound,
        bool fxQuoteCompoundSpread = false,
        Natural fxQuoteLookbackDays = Null<Natural>(),
        bool fxQuoteObservationShift = false,
        Natural fxQuoteLockoutDays = 0,
        RateAveraging::Type fxQuoteAveragingMethod = RateAveraging::Compound,
        bool telescopicValueDates = false,
        std::optional<bool> useIndexedCoupons = std::nullopt,
        StubIndexConfig fxBaseStubIndexConfig = {},
        StubIndexConfig fxQuoteStubIndexConfig = {});
    //@}

    //! \name Instrument interface
    //@{
    void setupArguments(PricingEngine::arguments* args) const override;
    void fetchResults(const PricingEngine::results*) const override;
    //@}

    //! \name Inspectors
    //@{
    Type type() const { return type_; }

    bool paysFxBaseCurrency() const { return type_ == Type::PayFxBaseCurrency; }

    Real fxBaseNominal() const { return fxBaseNominal_; }
    const Currency& fxBaseCurrency() const { return fxBaseCurrency_; }
    const Schedule& fxBaseSchedule() const { return fxBaseSchedule_; }
    const ext::shared_ptr<IborIndex>& fxBaseIndex() const { return fxBaseIndex_; }
    Spread fxBaseSpread() const { return fxBaseSpread_; }
    Real fxBaseGearing() const { return fxBaseGearing_; }
    const StubIndexConfig& fxBaseStubIndexConfig() const {
        return fxBaseStubIndexConfig_;
    }
    const StubIndexConfig& fxQuoteStubIndexConfig() const {
        return fxQuoteStubIndexConfig_;
    }

    Real fxQuoteNominal() const { return fxQuoteNominal_; }
    const Currency& fxQuoteCurrency() const { return fxQuoteCurrency_; }
    const Schedule& fxQuoteSchedule() const { return fxQuoteSchedule_; }
    const ext::shared_ptr<IborIndex>& fxQuoteIndex() const { return fxQuoteIndex_; }
    Spread fxQuoteSpread() const { return fxQuoteSpread_; }
    Real fxQuoteGearing() const { return fxQuoteGearing_; }

    Real payNominal() const { return paysFxBaseCurrency() ? fxBaseNominal_ : fxQuoteNominal_; }
    const Currency& payCurrency() const {
        return paysFxBaseCurrency() ? fxBaseCurrency_ : fxQuoteCurrency_;
    }
    const Schedule& paySchedule() const {
        return paysFxBaseCurrency() ? fxBaseSchedule_ : fxQuoteSchedule_;
    }
    const ext::shared_ptr<IborIndex>& payIndex() const {
        return paysFxBaseCurrency() ? fxBaseIndex_ : fxQuoteIndex_;
    }
    Spread paySpread() const { return paysFxBaseCurrency() ? fxBaseSpread_ : fxQuoteSpread_; }
    Real payGearing() const { return paysFxBaseCurrency() ? fxBaseGearing_ : fxQuoteGearing_; }

    Real recNominal() const { return paysFxBaseCurrency() ? fxQuoteNominal_ : fxBaseNominal_; }
    const Currency& recCurrency() const {
        return paysFxBaseCurrency() ? fxQuoteCurrency_ : fxBaseCurrency_;
    }
    const Schedule& recSchedule() const {
        return paysFxBaseCurrency() ? fxQuoteSchedule_ : fxBaseSchedule_;
    }
    const ext::shared_ptr<IborIndex>& recIndex() const {
        return paysFxBaseCurrency() ? fxQuoteIndex_ : fxBaseIndex_;
    }
    Spread recSpread() const { return paysFxBaseCurrency() ? fxQuoteSpread_ : fxBaseSpread_; }
    Real recGearing() const { return paysFxBaseCurrency() ? fxQuoteGearing_ : fxBaseGearing_; }

    //! true if the base-currency (first) leg is the resettable leg
    bool isFxBaseCurrencyLegResettable() const { return isFxBaseCurrencyLegResettable_; }
    Size resettingLegIndex() const { return isFxBaseCurrencyLegResettable_ ? 0 : 1; }
    Size constantLegIndex() const { return isFxBaseCurrencyLegResettable_ ? 1 : 0; }
    const Leg& resettingLeg() const { return legs_[resettingLegIndex()]; }
    const Leg& constantLeg() const { return legs_[constantLegIndex()]; }
    //! notional of the constant leg, in its own currency
    Real constantLegNotional() const {
        return isFxBaseCurrencyLegResettable_ ? fxQuoteNominal_ : fxBaseNominal_;
    }
    Natural fxResetFixingDays() const { return fxResetConvention_.fixingDays(); }
    Calendar fxResetFixingCalendar() const { return fxResetConvention_.fixingCalendar(); }
    BusinessDayConvention fxBasePaymentConvention() const {
        return fxBasePaymentConvention_;
    }
    BusinessDayConvention fxQuotePaymentConvention() const {
        return fxQuotePaymentConvention_;
    }
    /*! FX rates used to determine the resettable coupon notionals.

        The values follow the order of the non-occurred FxResetCoupon
        instances in resettingLeg().
    */
    const std::vector<Real>& fxResetRates() const {
        calculate();
        return fxResetRates_;
    }
    /*! Notionals of the resettable coupons, in resetting-leg currency.

        The values follow the order of the non-occurred FxResetCoupon
        instances in resettingLeg().
    */
    const std::vector<Real>& fxResetNotionals() const {
        calculate();
        return fxResetNotionals_;
    }
    //@}

    //! \name Additional interface
    //@{
    Spread fairFxBaseSpread() const {
        calculate();
        QL_REQUIRE(fairFxBaseSpread_ != Null<Real>(), "Fair FX-base spread is not available");
        return fairFxBaseSpread_;
    }
    Spread fairFxQuoteSpread() const {
        calculate();
        QL_REQUIRE(fairFxQuoteSpread_ != Null<Real>(), "Fair FX-quote spread is not available");
        return fairFxQuoteSpread_;
    }
    Spread fairPaySpread() const {
        return paysFxBaseCurrency() ? fairFxBaseSpread() : fairFxQuoteSpread();
    }
    Spread fairRecSpread() const {
        return paysFxBaseCurrency() ? fairFxQuoteSpread() : fairFxBaseSpread();
    }
    //@}

  protected:
    //! \name Instrument interface
    //@{
    void setupExpired() const override;
    //@}

  private:
    void initialize();

    Type type_;
    Real fxBaseNominal_;
    Currency fxBaseCurrency_;
    Schedule fxBaseSchedule_;
    ext::shared_ptr<IborIndex> fxBaseIndex_;
    Spread fxBaseSpread_;
    Real fxBaseGearing_;
    StubIndexConfig fxBaseStubIndexConfig_;

    Real fxQuoteNominal_;
    Currency fxQuoteCurrency_;
    Schedule fxQuoteSchedule_;
    ext::shared_ptr<IborIndex> fxQuoteIndex_;
    Spread fxQuoteSpread_;
    Real fxQuoteGearing_;
    StubIndexConfig fxQuoteStubIndexConfig_;

    bool isFxBaseCurrencyLegResettable_;
    FxResetConvention fxResetConvention_;

    Integer fxBasePaymentLag_;
    Integer fxQuotePaymentLag_;
    BusinessDayConvention fxBasePaymentConvention_;
    BusinessDayConvention fxQuotePaymentConvention_;
    std::optional<bool> useIndexedCoupons_;

    // OIS only
    bool fxBaseCompoundSpread_;
    Natural fxBaseLookbackDays_;
    bool fxBaseObservationShift_;
    Natural fxBaseLockoutDays_;
    RateAveraging::Type fxBaseAveragingMethod_;
    bool fxQuoteCompoundSpread_;
    Natural fxQuoteLookbackDays_;
    bool fxQuoteObservationShift_;
    Natural fxQuoteLockoutDays_;
    RateAveraging::Type fxQuoteAveragingMethod_;
    bool telescopicValueDates_;

    mutable Spread fairFxBaseSpread_;
    mutable Spread fairFxQuoteSpread_;
    mutable std::vector<Real> fxResetRates_;
    mutable std::vector<Real> fxResetNotionals_;
};


class MtMCrossCurrencyBasisSwap::arguments : public CrossCurrencySwap::arguments {
  public:
    //! index of the resettable leg
    Size resettingLegIndex = Null<Size>();
    //! index of the constant-notional leg whose notional is converted at reset
    Size constantLegIndex = Null<Size>();
    //! convention for the FX fixing and its associated spot value date
    Natural fxResetFixingDays = 0;
    Calendar fxResetFixingCalendar;
    Spread fxBaseSpread = Null<Spread>();
    Spread fxQuoteSpread = Null<Spread>();
    void validate() const override;
};


class MtMCrossCurrencyBasisSwap::results : public CrossCurrencySwap::results {
  public:
    Spread fairFxBaseSpread = Null<Spread>();
    Spread fairFxQuoteSpread = Null<Spread>();
    std::vector<Real> fxResetRates;
    std::vector<Real> fxResetNotionals;
    void reset() override;
};


class MtMCrossCurrencyBasisSwap::engine
    : public GenericEngine<MtMCrossCurrencyBasisSwap::arguments,
                           MtMCrossCurrencyBasisSwap::results> {};

}

#endif
