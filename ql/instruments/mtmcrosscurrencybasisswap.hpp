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

/*! \file mtmcrosscurrencybasisswap.hpp
    \brief Mark-to-market (resetting-notional) cross-currency basis swap
*/

#ifndef quantlib_mtm_cross_currency_basis_swap_hpp
#define quantlib_mtm_cross_currency_basis_swap_hpp

#include <ql/cashflows/rateaveraging.hpp>
#include <ql/indexes/fxindex.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/crosscurrencyswap.hpp>
#include <ql/pricingengine.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

//! Mark-to-market cross-currency basis swap
/*! A float-vs-float cross-currency basis swap in which the notional of one leg
    (the \e resettable leg) is reset at the start of each coupon period to track
    the prevailing FX rate, while the other leg keeps a constant notional that is
    exchanged at inception and maturity.

    Following the convention of MtMCrossCurrencyBasisSwapRateHelper, the two legs
    are identified by their role in the FX pair: the \e base-currency leg and the
    \e quote-currency leg, and \c isFxBaseCurrencyLegResettable selects which of
    them carries the resetting notional.  The trade \c Type controls whether the
    base-currency leg is paid or received.

    Resetting the notional removes most of the FX exposure of the principal and
    is the market-standard convention for many currency pairs.  The resettable
    leg therefore carries no explicit initial/final notional-exchange cash flows:
    the per-period re-exchanges that replace them are accounted for analytically
    by the pricing engine.  This instrument must be priced with an engine that
    understands the resettable leg (see DiscountingMtMCrossCurrencyBasisSwapEngine);
    pricing it with a plain constant-notional engine would ignore the resets.

    \ingroup instruments
*/
class MtMCrossCurrencyBasisSwap : public CrossCurrencySwap {
  public:
    struct ResettingLegData {
        ResettingLegData() = default;
        ResettingLegData(Size resettingLegIndex,
                         Size constantLegIndex,
                         Real constantLegNotional,
                         Integer paymentLag,
                         Calendar paymentCalendar,
                         BusinessDayConvention paymentConvention,
                         ext::shared_ptr<FxIndex> fxIndex = {});

        //! index of the resettable leg
        Size resettingLegIndex = 0;
        //! index of the constant-notional leg whose notional is converted at reset
        Size constantLegIndex = 1;
        //! notional of the constant leg, in its own currency
        Real constantLegNotional = Null<Real>();
        //! payment convention for the implicit reset exchanges
        Integer paymentLag = 0;
        Calendar paymentCalendar;
        BusinessDayConvention paymentConvention = Following;
        //! historical FX reset fixings, quoted resettable currency per constant-leg currency
        ext::shared_ptr<FxIndex> fxIndex;
    };

    class arguments;
    class results;
    class engine;

    enum class Type { PayFxBaseCurrency, ReceiveFxBaseCurrency };

    //! \name Constructors
    //@{
    /*! \param type  Pay or receive the FX base-currency leg.
        \param isFxBaseCurrencyLegResettable  If true the base-currency (first)
                                   leg is the resettable leg; otherwise the
                                   quote-currency (second) leg resets.  The other
                                   leg keeps a constant notional and exchanges it
                                   at inception and maturity.
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
        const ext::shared_ptr<FxIndex>& fxIndex = {},
        Integer fxBasePaymentLag = 0, Integer fxQuotePaymentLag = 0,
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
        bool telescopicValueDates = false);
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
    const ResettingLegData& resettingLegData() const { return resettingLegData_; }
    Size resettingLegIndex() const { return resettingLegData_.resettingLegIndex; }
    Size constantLegIndex() const { return resettingLegData_.constantLegIndex; }
    Real constantLegNotional() const { return resettingLegData_.constantLegNotional; }
    const ext::shared_ptr<FxIndex>& fxIndex() const { return resettingLegData_.fxIndex; }
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
    void validateResettingLegData() const;

    Type type_;
    ResettingLegData resettingLegData_;
    Real fxBaseNominal_;
    Currency fxBaseCurrency_;
    Schedule fxBaseSchedule_;
    ext::shared_ptr<IborIndex> fxBaseIndex_;
    Spread fxBaseSpread_;
    Real fxBaseGearing_;

    Real fxQuoteNominal_;
    Currency fxQuoteCurrency_;
    Schedule fxQuoteSchedule_;
    ext::shared_ptr<IborIndex> fxQuoteIndex_;
    Spread fxQuoteSpread_;
    Real fxQuoteGearing_;

    bool isFxBaseCurrencyLegResettable_;

    Integer fxBasePaymentLag_;
    Integer fxQuotePaymentLag_;

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
};


class MtMCrossCurrencyBasisSwap::arguments : public CrossCurrencySwap::arguments {
  public:
    ResettingLegData resettingLegData;
    Spread fxBaseSpread = Null<Spread>();
    Spread fxQuoteSpread = Null<Spread>();
    void validate() const override;
};


class MtMCrossCurrencyBasisSwap::results : public CrossCurrencySwap::results {
  public:
    Spread fairFxBaseSpread = Null<Spread>();
    Spread fairFxQuoteSpread = Null<Spread>();
    void reset() override;
};


class MtMCrossCurrencyBasisSwap::engine
    : public GenericEngine<MtMCrossCurrencyBasisSwap::arguments,
                           MtMCrossCurrencyBasisSwap::results> {};

}

#endif
