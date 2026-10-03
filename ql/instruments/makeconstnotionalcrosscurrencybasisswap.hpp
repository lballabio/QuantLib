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

/*! \file makeconstnotionalcrosscurrencybasisswap.hpp
    \brief Helper class to instantiate constant-notional cross-currency basis swaps
*/

#ifndef quantlib_make_const_notional_cross_currency_basis_swap_hpp
#define quantlib_make_const_notional_cross_currency_basis_swap_hpp

#include <ql/cashflows/rateaveraging.hpp>
#include <ql/instruments/constnotionalcrosscurrencybasisswap.hpp>
#include <ql/optional.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/dategenerationrule.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way to instantiate a
        constant-notional, float-vs-float cross-currency basis swap (both legs
        built from a tenor and an index) and to wire it up to a
        \c DiscountingConstNotionalCrossCurrencySwapEngine.

        The pay leg is priced/quoted in the currency of \c payIndex and the
        receive leg in the currency of \c recIndex, following the naming
        convention of \c ConstNotionalCrossCurrencyBasisSwap. The FX spot
        rate is expressed as units of the receive currency per unit of the
        pay currency, and is used both to derive the receive notional from
        the pay notional (unless explicitly overridden) and to build the
        pricing engine. It is required whenever the two legs are in
        different currencies, unless both the receive notional and the
        pricing engine are given explicitly.

        Payment dates default to the business-day convention of the
        corresponding index, as in \c MakeVanillaSwap.
    */
    class MakeConstNotionalCrossCurrencyBasisSwap {
      public:
        MakeConstNotionalCrossCurrencyBasisSwap(const Period& swapTenor,
                                                 ext::shared_ptr<IborIndex> payIndex,
                                                 ext::shared_ptr<IborIndex> recIndex,
                                                 Real payNominal = 1.0,
                                                 const Period& forwardStart = 0 * Days);

        operator ConstNotionalCrossCurrencyBasisSwap() const;
        operator ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap>() const;

        MakeConstNotionalCrossCurrencyBasisSwap& withPayNominal(Real n);
        MakeConstNotionalCrossCurrencyBasisSwap& withRecNominal(Real n);
        /*! units of the receive currency for one unit of the pay currency;
            used to derive the receive notional if \c withRecNominal was not
            called, and to build the pricing engine. */
        MakeConstNotionalCrossCurrencyBasisSwap& withFxSpot(const Handle<Quote>& spotFX);

        MakeConstNotionalCrossCurrencyBasisSwap& withPayCurrency(const Currency& ccy);
        MakeConstNotionalCrossCurrencyBasisSwap& withRecCurrency(const Currency& ccy);

        MakeConstNotionalCrossCurrencyBasisSwap& withSettlementDays(Natural settlementDays);
        MakeConstNotionalCrossCurrencyBasisSwap& withEffectiveDate(const Date& effectiveDate);
        MakeConstNotionalCrossCurrencyBasisSwap& withTerminationDate(const Date& terminationDate);
        MakeConstNotionalCrossCurrencyBasisSwap& withCalendar(const Calendar& cal);
        MakeConstNotionalCrossCurrencyBasisSwap& withPayCalendar(const Calendar& cal);
        MakeConstNotionalCrossCurrencyBasisSwap& withRecCalendar(const Calendar& cal);
        MakeConstNotionalCrossCurrencyBasisSwap& withPaymentConvention(BusinessDayConvention bdc);
        MakeConstNotionalCrossCurrencyBasisSwap&
        withPayPaymentConvention(BusinessDayConvention bdc);
        MakeConstNotionalCrossCurrencyBasisSwap&
        withRecPaymentConvention(BusinessDayConvention bdc);
        MakeConstNotionalCrossCurrencyBasisSwap& withRule(DateGeneration::Rule r);
        MakeConstNotionalCrossCurrencyBasisSwap& withEndOfMonth(bool flag = true);
        //! payment frequency for the pay leg; defaults to the pay index tenor
        MakeConstNotionalCrossCurrencyBasisSwap& withPayFrequency(Frequency f);
        //! payment frequency for the rec leg; defaults to the rec index tenor
        MakeConstNotionalCrossCurrencyBasisSwap& withRecFrequency(Frequency f);

        MakeConstNotionalCrossCurrencyBasisSwap& withPaySpread(Spread sp);
        MakeConstNotionalCrossCurrencyBasisSwap& withRecSpread(Spread sp);
        MakeConstNotionalCrossCurrencyBasisSwap& withPayGearing(Real g);
        MakeConstNotionalCrossCurrencyBasisSwap& withRecGearing(Real g);
        MakeConstNotionalCrossCurrencyBasisSwap& withPayPaymentLag(Integer lag);
        MakeConstNotionalCrossCurrencyBasisSwap& withRecPaymentLag(Integer lag);
        MakeConstNotionalCrossCurrencyBasisSwap&
        withIndexedCoupons(const std::optional<bool>& b = true);

        //! \name overnight-index-only leg conventions
        //@{
        MakeConstNotionalCrossCurrencyBasisSwap& withPayCompoundSpread(bool flag = true);
        MakeConstNotionalCrossCurrencyBasisSwap& withRecCompoundSpread(bool flag = true);
        MakeConstNotionalCrossCurrencyBasisSwap& withPayLookbackDays(Natural lookbackDays);
        MakeConstNotionalCrossCurrencyBasisSwap& withRecLookbackDays(Natural lookbackDays);
        MakeConstNotionalCrossCurrencyBasisSwap& withPayLockoutDays(Natural lockoutDays);
        MakeConstNotionalCrossCurrencyBasisSwap& withRecLockoutDays(Natural lockoutDays);
        MakeConstNotionalCrossCurrencyBasisSwap& withPayObservationShift(bool flag = true);
        MakeConstNotionalCrossCurrencyBasisSwap& withRecObservationShift(bool flag = true);
        MakeConstNotionalCrossCurrencyBasisSwap&
        withPayAveragingMethod(RateAveraging::Type averagingMethod);
        MakeConstNotionalCrossCurrencyBasisSwap&
        withRecAveragingMethod(RateAveraging::Type averagingMethod);
        MakeConstNotionalCrossCurrencyBasisSwap& withTelescopicValueDates(bool flag = true);
        //@}

        MakeConstNotionalCrossCurrencyBasisSwap&
        withPayDiscountCurve(const Handle<YieldTermStructure>& payDiscountCurve);
        MakeConstNotionalCrossCurrencyBasisSwap&
        withRecDiscountCurve(const Handle<YieldTermStructure>& recDiscountCurve);
        MakeConstNotionalCrossCurrencyBasisSwap&
        withPricingEngine(const ext::shared_ptr<PricingEngine>& engine);

      private:
        Period swapTenor_;
        ext::shared_ptr<IborIndex> payIndex_, recIndex_;
        Real payNominal_;
        Period forwardStart_;

        Real recNominal_ = Null<Real>();
        Handle<Quote> fxSpot_;

        Currency payCurrency_, recCurrency_;

        Natural settlementDays_ = Null<Natural>();
        Date effectiveDate_, terminationDate_;
        Calendar calendar_, payCalendar_, recCalendar_;
        BusinessDayConvention payConvention_, recConvention_;
        DateGeneration::Rule rule_ = DateGeneration::Backward;
        bool endOfMonth_ = false;
        std::optional<Frequency> payFrequency_, recFrequency_;

        Spread paySpread_ = 0.0, recSpread_ = 0.0;
        Real payGearing_ = 1.0, recGearing_ = 1.0;
        Integer payPaymentLag_ = 0, recPaymentLag_ = 0;
        std::optional<bool> useIndexedCoupons_;

        bool payCompoundSpread_ = false, recCompoundSpread_ = false;
        Natural payLookbackDays_ = Null<Natural>(), recLookbackDays_ = Null<Natural>();
        bool payObservationShift_ = false, recObservationShift_ = false;
        Natural payLockoutDays_ = 0, recLockoutDays_ = 0;
        RateAveraging::Type payAveragingMethod_ = RateAveraging::Compound,
                            recAveragingMethod_ = RateAveraging::Compound;
        bool telescopicValueDates_ = false;

        Handle<YieldTermStructure> payDiscountCurve_, recDiscountCurve_;
        ext::shared_ptr<PricingEngine> engine_;
    };

}

#endif
