/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Marcin Rybacki
 Copyright (C) 2025 Uzair Beg

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

#include <ql/termstructures/yield/ratehelpers.hpp>

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
                                        Integer paymentLag);

        void initializeDatesFromLegs(const Leg& firstLeg, const Leg& secondLeg);

        Period tenor_;
        Natural fixingDays_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        bool endOfMonth_;
        Integer paymentLag_;

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
                                             Frequency paymentFrequency = NoFrequency,
                                             Integer paymentLag = 0);

        void initializeDates() override;
        const Handle<YieldTermStructure>& baseCcyLegDiscountHandle() const;
        const Handle<YieldTermStructure>& quoteCcyLegDiscountHandle() const;

        ext::shared_ptr<IborIndex> baseCcyIdx_;
        ext::shared_ptr<IborIndex> quoteCcyIdx_;
        bool isFxBaseCurrencyCollateralCurrency_;
        bool isBasisOnFxBaseCurrencyLeg_;
        Frequency paymentFrequency_;

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
            Frequency paymentFrequency = NoFrequency,
            Integer paymentLag = 0);
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
    };


    //! Rate helper for bootstrapping over market-to-market cross-currency basis swaps
    /*!
    Helper for a cross currency swap with resetting notional.
    This means that at each interest payment the notional on the MtM
    leg is being reset to reflect the changes in the FX rate - reducing
    the counterparty and FX risk of the structure.

    For more details see:
    N. Moreni, A. Pallavicini (2015)
    FX Modelling in Collateralized Markets: foreign measures, basis curves
    and pricing formulae.
    */
    class MtMCrossCurrencyBasisSwapRateHelper : public CrossCurrencyBasisSwapRateHelperBase {
      public:
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
                                            Frequency paymentFrequency = NoFrequency,
                                            Integer paymentLag = 0);
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:
        bool isFxBaseCurrencyLegResettable_;
    };


    //! Rate helper for bootstrapping fixed–vs-floating cross-currency par swaps
    /*!
    This helper represents a par cross-currency swap exchanging a fixed-rate leg
    against a floating-rate leg in a different currency. Since the swap is quoted
    at par, the FX spot cancels out and is not required.

    The collateralOnFixedLeg flag determines which leg is discounted using the provided
    collateral curve, while the other leg’s discount curve is the one being bootstrapped.
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
            const DayCounter& fixedDayCount,
            const ext::shared_ptr<IborIndex>& floatIndex,
            Handle<YieldTermStructure> collateralCurve,
            bool collateralOnFixedLeg,
            Frequency floatingFrequency = NoFrequency,
            Integer paymentLag = 0);

        Real impliedQuote() const override;
        void accept(AcyclicVisitor&) override;

      protected:
        void initializeDates() override;
        const Handle<YieldTermStructure>& fixedLegDiscountHandle() const;
        const Handle<YieldTermStructure>& floatingLegDiscountHandle() const;

        Frequency fixedFrequency_;
        Frequency floatingFrequency_;
        DayCounter fixedDayCount_;
        ext::shared_ptr<IborIndex> floatIndex_;
        bool collateralOnFixedLeg_;

        Leg fixedLeg_;
        Leg floatLeg_;
    };

}

#endif
