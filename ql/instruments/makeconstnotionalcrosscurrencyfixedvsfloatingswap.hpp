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

/*! \file makeconstnotionalcrosscurrencyfixedvsfloatingswap.hpp
    \brief Helper class to instantiate constant-notional cross-currency
           fixed-vs-floating swaps
*/

#ifndef quantlib_make_const_notional_cross_currency_fixed_vs_floating_swap_hpp
#define quantlib_make_const_notional_cross_currency_fixed_vs_floating_swap_hpp

#include <ql/cashflows/rateaveraging.hpp>
#include <ql/instruments/constnotionalcrosscurrencyfixedvsfloatingswap.hpp>
#include <ql/optional.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <ql/time/daycounter.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way to instantiate a
        constant-notional cross-currency fixed-vs-floating swap and to wire
        it up to a \c DiscountingConstNotionalCrossCurrencySwapEngine.

        If the fixed rate is left unspecified (the default, \c Null<Rate>()),
        it is solved for as the fair rate of the swap once a pricing engine
        (or a pair of discount curves and an FX spot) has been supplied,
        mirroring the at-market convenience of \c MakeVanillaSwap.

        The FX spot rate is expressed as units of the floating currency per
        unit of the fixed currency, so that the fixed notional is the
        floating one divided by the spot (unless explicitly overridden); it
        is also used to build the pricing engine. It is required whenever
        the two legs are in different currencies, unless both the fixed
        notional and the pricing engine are given explicitly.
    */
    class MakeConstNotionalCrossCurrencyFixedVsFloatingSwap {
      public:
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap(
            const Period& swapTenor,
            Currency fixedCurrency,
            ext::shared_ptr<IborIndex> floatIndex,
            Rate fixedRate = Null<Rate>(),
            Real floatNominal = 1.0,
            const Period& forwardStart = 0 * Days);

        operator ConstNotionalCrossCurrencyFixedVsFloatingSwap() const;
        operator ext::shared_ptr<ConstNotionalCrossCurrencyFixedVsFloatingSwap>() const;

        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap& payFixed(bool flag = true);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap& receiveFixed(bool flag = true);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap& withType(Swap::Type type);

        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap& withFixedNominal(Real n);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap& withFloatNominal(Real n);
        /*! units of the floating currency for one unit of the fixed
            currency; the floating notional is divided by it to derive the
            fixed notional if \c withFixedNominal was not called, and it is
            also used to build the pricing engine. */
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap& withFxSpot(const Handle<Quote>& spotFX);

        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap& withFloatCurrency(const Currency& ccy);

        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withSettlementDays(Natural settlementDays);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withEffectiveDate(const Date& effectiveDate);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withTerminationDate(const Date& terminationDate);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap& withCalendar(const Calendar& cal);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withRule(DateGeneration::Rule r);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap& withEndOfMonth(bool flag = true);

        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap& withFixedFrequency(Frequency f);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap& withFixedDayCount(const DayCounter& dc);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withFixedPaymentConvention(BusinessDayConvention bdc);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withFixedPaymentLag(Natural lag);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withFixedPaymentCalendar(const Calendar& cal);

        //! defaults to the floating index tenor
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap& withFloatFrequency(Frequency f);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap& withFloatDayCount(const DayCounter& dc);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap& withFloatSpread(Spread sp);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withFloatPaymentConvention(BusinessDayConvention bdc);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withFloatPaymentLag(Natural lag);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withFloatPaymentCalendar(const Calendar& cal);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withIndexedCoupons(const std::optional<bool>& b = true);

        //! \name overnight-index-only floating leg conventions
        //@{
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withFloatCompoundSpread(bool flag = true);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withFloatLookbackDays(Natural lookbackDays);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withFloatLockoutDays(Natural lockoutDays);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withFloatObservationShift(bool flag = true);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withFloatAveragingMethod(RateAveraging::Type averagingMethod);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withTelescopicValueDates(bool flag = true);
        //@}

        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withFixedDiscountCurve(const Handle<YieldTermStructure>& fixedDiscountCurve);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withFloatDiscountCurve(const Handle<YieldTermStructure>& floatDiscountCurve);
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
        withPricingEngine(const ext::shared_ptr<PricingEngine>& engine);

      private:
        Period swapTenor_;
        Currency fixedCurrency_;
        ext::shared_ptr<IborIndex> floatIndex_;
        Rate fixedRate_;
        Real floatNominal_;
        Period forwardStart_;

        Swap::Type type_ = Swap::Payer;
        Real fixedNominal_ = Null<Real>();
        Handle<Quote> fxSpot_;
        Currency floatCurrency_;

        Natural settlementDays_ = Null<Natural>();
        Date effectiveDate_, terminationDate_;
        Calendar calendar_;
        DateGeneration::Rule rule_ = DateGeneration::Backward;
        bool endOfMonth_ = false;

        std::optional<Frequency> fixedFrequency_;
        DayCounter fixedDayCount_;
        BusinessDayConvention fixedPaymentConvention_ = ModifiedFollowing;
        Natural fixedPaymentLag_ = 0;
        Calendar fixedPaymentCalendar_;

        std::optional<Frequency> floatFrequency_;
        DayCounter floatDayCount_;
        Spread floatSpread_ = 0.0;
        std::optional<BusinessDayConvention> floatPaymentConvention_;
        Natural floatPaymentLag_ = 0;
        Calendar floatPaymentCalendar_;
        std::optional<bool> useIndexedCoupons_;

        bool floatCompoundSpread_ = false;
        Natural floatLookbackDays_ = Null<Natural>();
        bool floatObservationShift_ = false;
        Natural floatLockoutDays_ = 0;
        RateAveraging::Type floatAveragingMethod_ = RateAveraging::Compound;
        bool telescopicValueDates_ = false;

        Handle<YieldTermStructure> fixedDiscountCurve_, floatDiscountCurve_;
        ext::shared_ptr<PricingEngine> engine_;
    };

}

#endif
