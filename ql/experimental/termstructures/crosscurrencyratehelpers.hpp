/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Marcin Rybacki
 Copyright (C) 2025 Uzair Beg
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

/*! \file crosscurrencyratehelpers.hpp
    \brief FX and cross currency basis swaps rate helpers
*/

#ifndef quantlib_crosscurrencyratehelpers_hpp
#define quantlib_crosscurrencyratehelpers_hpp

#include <ql/cashflows/stubiborcoupon.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/instruments/constnotionalcrosscurrencybasisswap.hpp>
#include <ql/instruments/constnotionalcrosscurrencyfixedvsfloatingswap.hpp>
#include <ql/experimental/fx/mtmcrosscurrencybasisswap.hpp>
#include <ql/optional.hpp>

namespace QuantLib {

    class CrossCurrencySwapRateHelperBase : public RelativeDateRateHelper {
      public:
        void setTermStructure(YieldTermStructure* t) override;

      protected:
        CrossCurrencySwapRateHelperBase(const Handle<Quote>& quote,
                                        const Period& tenor,
                                        Natural fixingDays,
                                        Calendar calendar,
                                        BusinessDayConvention convention,
                                        bool endOfMonth,
                                        Handle<YieldTermStructure> collateralCurve,
                                        Integer paymentLag,
                                        bool paymentLagOnNotionalExchanges = false);

        void initializeDatesFromLegs(const Leg& firstLeg, const Leg& secondLeg);

        Period tenor_;
        Natural fixingDays_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        bool endOfMonth_;
        Integer paymentLag_;
        bool paymentLagOnNotionalExchanges_;

        Handle<YieldTermStructure> collateralHandle_;

        RelinkableHandle<YieldTermStructure> termStructureHandle_;

        Date initialNotionalExchangeDate_;
        Date finalNotionalExchangeDate_;
    };


    //! Base class for cross-currency basis swap rate helpers
    class CrossCurrencyBasisSwapRateHelperBase : public CrossCurrencySwapRateHelperBase {
      protected:
        CrossCurrencyBasisSwapRateHelperBase(const Handle<Quote>& basis,
                                             const Period& tenor,
                                             Natural fixingDays,
                                             Calendar calendar,
                                             BusinessDayConvention convention,
                                             bool endOfMonth,
                                             ext::shared_ptr<IborIndex> baseCurrencyIndex,
                                             ext::shared_ptr<IborIndex> quoteCurrencyIndex,
                                             Handle<YieldTermStructure> collateralCurve,
                                             bool isFxBaseCurrencyCollateralCurrency,
                                             bool isBasisOnFxBaseCurrencyLeg,
                                             std::optional<Frequency> paymentFrequency = std::nullopt,
                                             Integer paymentLag = 0,
                                             std::optional<Frequency> quoteCurrencyPaymentFrequency = std::nullopt,
                                             std::optional<bool> useIndexedCoupons = std::nullopt,
                                             bool paymentLagOnNotionalExchanges = false,
                                             StubIndexSelection baseStubIndexSelection = {},
                                             StubIndexSelection quoteStubIndexSelection = {});

        void initializeDates() override;
        const Handle<YieldTermStructure>& baseCcyLegDiscountHandle() const;
        const Handle<YieldTermStructure>& quoteCcyLegDiscountHandle() const;

        ext::shared_ptr<IborIndex> baseCcyIdx_;
        ext::shared_ptr<IborIndex> quoteCcyIdx_;
        bool isFxBaseCurrencyCollateralCurrency_;
        bool isBasisOnFxBaseCurrencyLeg_;
        std::optional<Frequency> paymentFrequency_;
        std::optional<Frequency> quoteCcyPaymentFrequency_;
        std::optional<bool> useIndexedCoupons_;
        StubIndexSelection baseStubIndexSelection_;
        StubIndexSelection quoteStubIndexSelection_;

        Schedule baseCcySchedule_;
        Schedule quoteCcySchedule_;
        Leg baseCcyIborLeg_;
        Leg quoteCcyIborLeg_;
    };


    //! Rate helper for bootstrapping over constant-notional cross-currency basis swaps
    /*!
    Unlike marked-to-market cross currency swaps, both notionals
    expressed in base and quote currency remain constant throughout
    the lifetime of the swap.

    Note on used conventions. Consider a currency pair EUR-USD.
    EUR is the base currency, while USD is the quote currency.
    The quote currency indicates the amount to be paid in that
    currency for one unit of base currency.
    Hence, for a cross currency swap we define a base currency
    leg and a quote currency leg. The parameters of the instrument,
    e.g. collateral currency, basis, resetting  or constant notional
    legs are defined relative to what base and quote currencies are.
    For example, in case of EUR-USD basis swaps the collateral is paid
    in quote currency (USD), the basis is given on the base currency
    leg (EUR), etc.

    For more details see:
    N. Moreni, A. Pallavicini (2015)
    FX Modelling in Collateralized Markets: foreign measures, basis curves
    and pricing formulae.
    */
    class ConstNotionalCrossCurrencyBasisSwapRateHelper : public CrossCurrencyBasisSwapRateHelperBase {
      public:
        /*! \param paymentFrequency
                payment frequency of the base-currency leg; if left unset (the
                default) the schedule is derived from the base-currency index tenor.
            \param paymentLag
                coupon payment lag, in days, applied to both legs (default: 0).
                Notional exchanges remain on the effective and maturity dates
                unless \c paymentLagOnNotionalExchanges is set.
            \param quoteCurrencyPaymentFrequency
                payment frequency of the quote-currency leg; if left unset (the
                default) it defaults to \c paymentFrequency, and if that is unset as
                well the schedule is derived from the quote-currency index tenor.
            \param useIndexedCoupons
                if provided, overrides the global IborCoupon setting for both legs.
            \param paymentLagOnNotionalExchanges
                if true, both notional exchanges are lagged by \c paymentLag like
                the coupons: the final exchange settles together with the final
                coupon and the initial exchange falls on the lagged settlement
                date (default: false).
            \param baseStubIndexSelection
                index selection applied to irregular coupons of the base-currency
                leg when it is an Ibor leg (see StubIndexSelection).  The candidate
                indices must use exogenous forwarding curves; the default prices
                broken periods off the leg's own index.
            \param quoteStubIndexSelection
                as baseStubIndexSelection, for the quote-currency leg.
            In both frequency parameters, \c NoFrequency is accepted as a synonym for
            an unset (null) value.
        */
        ConstNotionalCrossCurrencyBasisSwapRateHelper(
            const Handle<Quote>& basis,
            const Period& tenor,
            Natural fixingDays,
            const Calendar& calendar,
            BusinessDayConvention convention,
            bool endOfMonth,
            const ext::shared_ptr<IborIndex>& baseCurrencyIndex,
            const ext::shared_ptr<IborIndex>& quoteCurrencyIndex,
            const Handle<YieldTermStructure>& collateralCurve,
            bool isFxBaseCurrencyCollateralCurrency,
            bool isBasisOnFxBaseCurrencyLeg,
            std::optional<Frequency> paymentFrequency = std::nullopt,
            Integer paymentLag = 0,
            std::optional<Frequency> quoteCurrencyPaymentFrequency = std::nullopt,
            std::optional<bool> useIndexedCoupons = std::nullopt,
            bool paymentLagOnNotionalExchanges = false,
            StubIndexSelection baseStubIndexSelection = {},
            StubIndexSelection quoteStubIndexSelection = {});
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const override;

        const ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap>& swap() const { return swap_; }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        void initializeDates() override;

      private:
        void buildSwap();

        ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap> swap_;
    };


    //! Rate helper for bootstrapping over market-to-market cross-currency basis swaps
    /*!
    Helper for a cross currency swap with resetting notional.
    This means that at each accrual-period boundary the notional on the MtM
    leg is reset to reflect changes in the FX rate - reducing
    the counterparty and FX risk of the structure.

    For more details see:
    N. Moreni, A. Pallavicini (2015)
    FX Modelling in Collateralized Markets: foreign measures, basis curves
    and pricing formulae.
    */
    class MtMCrossCurrencyBasisSwapRateHelper : public CrossCurrencyBasisSwapRateHelperBase {
      public:
        /*! \param paymentFrequency
                payment frequency of the base-currency leg; if left unset (the
                default) the schedule is derived from the base-currency index tenor.
            \param paymentLag
                coupon payment lag, in days, applied to both legs (default: 0).
                Notional exchanges remain on the effective and maturity dates.
            \param quoteCurrencyPaymentFrequency
                payment frequency of the quote-currency leg; if left unset (the
                default) it defaults to \c paymentFrequency, and if that is unset as
                well the schedule is derived from the quote-currency index tenor.
            \param fxResetFixingDays
                number of business days by which each FX fixing precedes its
                accrual-start value date (default: 0).
            \param fxResetFixingCalendar
                calendar used for the FX fixing offset; for a non-zero fixing lag,
                \p calendar is used if this is empty.
            \param useIndexedCoupons
                if provided, overrides the global IborCoupon setting for both legs.
            \param baseStubIndexSelection
                index selection applied to irregular coupons of the base-currency
                leg when it is an Ibor leg (see StubIndexSelection).  The candidate
                indices must use exogenous forwarding curves; the default prices
                broken periods off the leg's own index.
            \param quoteStubIndexSelection
                as baseStubIndexSelection, for the quote-currency leg.
            In both frequency parameters, \c NoFrequency is accepted as a synonym for
            an unset (null) value.
        */
        MtMCrossCurrencyBasisSwapRateHelper(const Handle<Quote>& basis,
                                            const Period& tenor,
                                            Natural fixingDays,
                                            const Calendar& calendar,
                                            BusinessDayConvention convention,
                                            bool endOfMonth,
                                            const ext::shared_ptr<IborIndex>& baseCurrencyIndex,
                                            const ext::shared_ptr<IborIndex>& quoteCurrencyIndex,
                                            const Handle<YieldTermStructure>& collateralCurve,
                                            bool isFxBaseCurrencyCollateralCurrency,
                                            bool isBasisOnFxBaseCurrencyLeg,
                                            bool isFxBaseCurrencyLegResettable,
                                            std::optional<Frequency> paymentFrequency = std::nullopt,
                                            Integer paymentLag = 0,
                                            std::optional<Frequency> quoteCurrencyPaymentFrequency = std::nullopt,
                                            Natural fxResetFixingDays = 0,
                                            Calendar fxResetFixingCalendar = Calendar(),
                                            std::optional<bool> useIndexedCoupons = std::nullopt,
                                            StubIndexSelection baseStubIndexSelection = {},
                                            StubIndexSelection quoteStubIndexSelection = {});
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const override;
        //@}
        //! \name Inspectors
        //@{
        //! the underlying par swap: unit notionals, zero spreads, spot FX = 1
        const ext::shared_ptr<MtMCrossCurrencyBasisSwap>& swap() const { return swap_; }
        //! the number of business days from an FX value date to its fixing date
        Natural fxResetFixingDays() const { return fxResetFixingDays_; }
        //! the calendar used to determine FX fixing dates
        const Calendar& fxResetFixingCalendar() const { return fxResetFixingCalendar_; }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        void initializeDates() override;

      private:
        void buildSwap();

        bool isFxBaseCurrencyLegResettable_;
        Natural fxResetFixingDays_;
        Calendar fxResetFixingCalendar_;
        ext::shared_ptr<MtMCrossCurrencyBasisSwap> swap_;
    };


    //! Rate helper for bootstrapping fixed–vs-floating cross-currency par swaps
    /*!
    This helper represents a par cross-currency swap exchanging a fixed-rate leg
    against a floating-rate leg in a different currency. Since the swap is quoted
    at par, the FX spot cancels out and is not required.

    The collateralOnFixedLeg flag determines which leg is discounted using the provided
    collateral curve, while the other leg’s discount curve is the one being bootstrapped.

    The paymentLag parameter, in days, applies to the coupons of both legs;
    notional exchanges remain on the effective and maturity dates.

    The convention parameter applies to the schedule and payments of both legs,
    and the calendar parameter is used to roll and to pay on both legs.

    If provided, the useIndexedCoupons parameter overrides the global
    IborCoupon setting for the floating leg.

    The floatPaymentFrequency parameter is the payment frequency of the floating
    leg.  If left unset (the default) the schedule is derived from the floating
    index tenor, which is only meaningful for an ibor index; an overnight index
    has no payment frequency of its own, so one must be given explicitly.
    \c NoFrequency is accepted as a synonym for an unset (null) value.

    The floatStubIndexSelection parameter selects the indices used for irregular
    coupons of the floating leg when it is an Ibor leg (see StubIndexSelection).
    */
    class ConstNotionalCrossCurrencySwapRateHelper : public CrossCurrencySwapRateHelperBase {
      public:
        ConstNotionalCrossCurrencySwapRateHelper(
            const Handle<Quote>& fixedRate,
            const Period& tenor,
            Natural fixingDays,
            const Calendar& calendar,
            BusinessDayConvention convention,
            bool endOfMonth,
            Frequency fixedFrequency,
            DayCounter  fixedDayCount,
            const ext::shared_ptr<IborIndex>& floatIndex,
            const Handle<YieldTermStructure>& collateralCurve,
            bool collateralOnFixedLeg,
            Integer paymentLag = 0,
            std::optional<bool> useIndexedCoupons = std::nullopt,
            std::optional<Frequency> floatPaymentFrequency = std::nullopt,
            StubIndexSelection floatStubIndexSelection = {});

        Real impliedQuote() const override;
        void accept(AcyclicVisitor&) override;

        const ext::shared_ptr<ConstNotionalCrossCurrencyFixedVsFloatingSwap>& swap() const {
            return xccySwap_;
        }

      protected:
        void initializeDates() override;
        const Handle<YieldTermStructure>& fixedLegDiscountHandle() const;
        const Handle<YieldTermStructure>& floatingLegDiscountHandle() const;

        Frequency fixedFrequency_;
        DayCounter fixedDayCount_;
        ext::shared_ptr<IborIndex> floatIndex_;
        bool collateralOnFixedLeg_;
        std::optional<bool> useIndexedCoupons_;
        std::optional<Frequency> floatPaymentFrequency_;
        StubIndexSelection floatStubIndexSelection_;

        ext::shared_ptr<ConstNotionalCrossCurrencyFixedVsFloatingSwap> xccySwap_;
    };

}

#endif
