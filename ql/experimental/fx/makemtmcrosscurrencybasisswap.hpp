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

/*! \file makemtmcrosscurrencybasisswap.hpp
    \brief Helper class to instantiate mark-to-market cross-currency basis swaps
*/

#ifndef quantlib_make_mtm_cross_currency_basis_swap_hpp
#define quantlib_make_mtm_cross_currency_basis_swap_hpp

#include <ql/cashflows/rateaveraging.hpp>
#include <ql/experimental/fx/mtmcrosscurrencybasisswap.hpp>
#include <ql/optional.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/dategenerationrule.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way to instantiate a
        mark-to-market (resetting-notional) cross-currency basis swap and to
        wire it up to a \c DiscountingMtMCrossCurrencyBasisSwapEngine.

        The FX-base leg is priced/quoted in the currency of \c fxBaseIndex and
        the FX-quote leg in the currency of \c fxQuoteIndex, following the
        naming convention of \c MtMCrossCurrencyBasisSwap. The FX spot rate
        is expressed as units of the fx-quote currency per unit of the
        fx-base currency, and is used both to derive the fx-quote notional
        from the fx-base notional (unless explicitly overridden) and to build
        the pricing engine. It is required whenever the two legs are in
        different currencies, unless both the fx-quote notional and the
        pricing engine are given explicitly.

        Payment dates default to the business-day convention of the
        corresponding index, as in \c MakeVanillaSwap.
    */
    class MakeMtMCrossCurrencyBasisSwap {
      public:
        MakeMtMCrossCurrencyBasisSwap(const Period& swapTenor,
                                      ext::shared_ptr<IborIndex> fxBaseIndex,
                                      ext::shared_ptr<IborIndex> fxQuoteIndex,
                                      bool isFxBaseCurrencyLegResettable,
                                      Real fxBaseNominal = 1.0,
                                      const Period& forwardStart = 0 * Days);

        operator MtMCrossCurrencyBasisSwap() const;
        operator ext::shared_ptr<MtMCrossCurrencyBasisSwap>() const;

        MakeMtMCrossCurrencyBasisSwap& payFxBaseCurrency(bool flag = true);
        MakeMtMCrossCurrencyBasisSwap& receiveFxBaseCurrency(bool flag = true);
        MakeMtMCrossCurrencyBasisSwap& withType(MtMCrossCurrencyBasisSwap::Type type);

        MakeMtMCrossCurrencyBasisSwap& withFxBaseNominal(Real n);
        MakeMtMCrossCurrencyBasisSwap& withFxQuoteNominal(Real n);
        /*! units of the fx-quote currency for one unit of the fx-base
            currency; used to derive the fx-quote notional if
            \c withFxQuoteNominal was not called, and to build the pricing
            engine. */
        MakeMtMCrossCurrencyBasisSwap& withFxSpot(const Handle<Quote>& spotFX);

        MakeMtMCrossCurrencyBasisSwap& withFxBaseCurrency(const Currency& ccy);
        MakeMtMCrossCurrencyBasisSwap& withFxQuoteCurrency(const Currency& ccy);

        MakeMtMCrossCurrencyBasisSwap& withSettlementDays(Natural settlementDays);
        MakeMtMCrossCurrencyBasisSwap& withEffectiveDate(const Date& effectiveDate);
        MakeMtMCrossCurrencyBasisSwap& withTerminationDate(const Date& terminationDate);
        MakeMtMCrossCurrencyBasisSwap& withCalendar(const Calendar& cal);
        MakeMtMCrossCurrencyBasisSwap& withFxBaseCalendar(const Calendar& cal);
        MakeMtMCrossCurrencyBasisSwap& withFxQuoteCalendar(const Calendar& cal);
        MakeMtMCrossCurrencyBasisSwap& withPaymentConvention(BusinessDayConvention bdc);
        MakeMtMCrossCurrencyBasisSwap& withFxBasePaymentConvention(BusinessDayConvention bdc);
        MakeMtMCrossCurrencyBasisSwap& withFxQuotePaymentConvention(BusinessDayConvention bdc);
        MakeMtMCrossCurrencyBasisSwap& withRule(DateGeneration::Rule r);
        MakeMtMCrossCurrencyBasisSwap& withEndOfMonth(bool flag = true);
        //! payment frequency for the fx-base leg; defaults to the fx-base index tenor
        MakeMtMCrossCurrencyBasisSwap& withFxBaseFrequency(Frequency f);
        //! payment frequency for the fx-quote leg; defaults to the fx-quote index tenor
        MakeMtMCrossCurrencyBasisSwap& withFxQuoteFrequency(Frequency f);

        MakeMtMCrossCurrencyBasisSwap& withFxBaseSpread(Spread sp);
        MakeMtMCrossCurrencyBasisSwap& withFxQuoteSpread(Spread sp);
        MakeMtMCrossCurrencyBasisSwap& withFxBaseGearing(Real g);
        MakeMtMCrossCurrencyBasisSwap& withFxQuoteGearing(Real g);
        MakeMtMCrossCurrencyBasisSwap& withFxBasePaymentLag(Integer lag);
        MakeMtMCrossCurrencyBasisSwap& withFxQuotePaymentLag(Integer lag);
        MakeMtMCrossCurrencyBasisSwap& withIndexedCoupons(const std::optional<bool>& b = true);

        MakeMtMCrossCurrencyBasisSwap& withFxResetFixingDays(Natural fixingDays);
        MakeMtMCrossCurrencyBasisSwap& withFxResetFixingCalendar(const Calendar& cal);

        //! \name overnight-index-only leg conventions
        //@{
        MakeMtMCrossCurrencyBasisSwap& withFxBaseCompoundSpread(bool flag = true);
        MakeMtMCrossCurrencyBasisSwap& withFxQuoteCompoundSpread(bool flag = true);
        MakeMtMCrossCurrencyBasisSwap& withFxBaseLookbackDays(Natural lookbackDays);
        MakeMtMCrossCurrencyBasisSwap& withFxQuoteLookbackDays(Natural lookbackDays);
        MakeMtMCrossCurrencyBasisSwap& withFxBaseLockoutDays(Natural lockoutDays);
        MakeMtMCrossCurrencyBasisSwap& withFxQuoteLockoutDays(Natural lockoutDays);
        MakeMtMCrossCurrencyBasisSwap& withFxBaseObservationShift(bool flag = true);
        MakeMtMCrossCurrencyBasisSwap& withFxQuoteObservationShift(bool flag = true);
        MakeMtMCrossCurrencyBasisSwap&
        withFxBaseAveragingMethod(RateAveraging::Type averagingMethod);
        MakeMtMCrossCurrencyBasisSwap&
        withFxQuoteAveragingMethod(RateAveraging::Type averagingMethod);
        MakeMtMCrossCurrencyBasisSwap& withTelescopicValueDates(bool flag = true);
        //@}

        MakeMtMCrossCurrencyBasisSwap&
        withFxBaseDiscountCurve(const Handle<YieldTermStructure>& fxBaseDiscountCurve);
        MakeMtMCrossCurrencyBasisSwap&
        withFxQuoteDiscountCurve(const Handle<YieldTermStructure>& fxQuoteDiscountCurve);
        MakeMtMCrossCurrencyBasisSwap&
        withPricingEngine(const ext::shared_ptr<PricingEngine>& engine);

      private:
        Period swapTenor_;
        ext::shared_ptr<IborIndex> fxBaseIndex_, fxQuoteIndex_;
        bool isFxBaseCurrencyLegResettable_;
        Real fxBaseNominal_;
        Period forwardStart_;

        MtMCrossCurrencyBasisSwap::Type type_ = MtMCrossCurrencyBasisSwap::Type::PayFxBaseCurrency;
        Real fxQuoteNominal_ = Null<Real>();
        Handle<Quote> fxSpot_;

        Currency fxBaseCurrency_, fxQuoteCurrency_;

        Natural settlementDays_ = Null<Natural>();
        Date effectiveDate_, terminationDate_;
        Calendar calendar_, fxBaseCalendar_, fxQuoteCalendar_;
        BusinessDayConvention fxBasePaymentConvention_, fxQuotePaymentConvention_;
        DateGeneration::Rule rule_ = DateGeneration::Backward;
        bool endOfMonth_ = false;
        std::optional<Frequency> fxBaseFrequency_, fxQuoteFrequency_;

        Spread fxBaseSpread_ = 0.0, fxQuoteSpread_ = 0.0;
        Real fxBaseGearing_ = 1.0, fxQuoteGearing_ = 1.0;
        Integer fxBasePaymentLag_ = 0, fxQuotePaymentLag_ = 0;
        std::optional<bool> useIndexedCoupons_;

        Natural fxResetFixingDays_ = 0;
        Calendar fxResetFixingCalendar_;

        bool fxBaseCompoundSpread_ = false, fxQuoteCompoundSpread_ = false;
        Natural fxBaseLookbackDays_ = Null<Natural>(), fxQuoteLookbackDays_ = Null<Natural>();
        bool fxBaseObservationShift_ = false, fxQuoteObservationShift_ = false;
        Natural fxBaseLockoutDays_ = 0, fxQuoteLockoutDays_ = 0;
        RateAveraging::Type fxBaseAveragingMethod_ = RateAveraging::Compound,
                            fxQuoteAveragingMethod_ = RateAveraging::Compound;
        bool telescopicValueDates_ = false;

        Handle<YieldTermStructure> fxBaseDiscountCurve_, fxQuoteDiscountCurve_;
        ext::shared_ptr<PricingEngine> engine_;
    };

}

#endif
