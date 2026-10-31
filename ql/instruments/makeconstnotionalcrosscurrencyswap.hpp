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

/*! \file makeconstnotionalcrosscurrencyswap.hpp
    \brief Helper class to instantiate a constant-notional cross-currency swap
           from two already-built legs.
*/

#ifndef quantlib_make_const_notional_cross_currency_swap_hpp
#define quantlib_make_const_notional_cross_currency_swap_hpp

#include <ql/instruments/constnotionalcrosscurrencyswap.hpp>
#include <ql/optional.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way to instantiate a generic
        constant-notional cross-currency swap out of two already-built legs
        (e.g. an \c IborLeg, \c OvernightLeg, or \c FixedRateLeg, including any
        notional-exchange cash flows the user wants attached) and to wire up
        the resulting instrument to a \c DiscountingConstNotionalCrossCurrencySwapEngine.

        The pay leg is the first leg (payer) and the receive leg is the second
        leg (receiver), following the convention of \c CrossCurrencySwap.

        For a more convenient way of building the two legs from a tenor and a
        pair of indices, see \c MakeConstNotionalCrossCurrencyBasisSwap and
        \c MakeConstNotionalCrossCurrencyFixedVsFloatingSwap.
    */
    class MakeConstNotionalCrossCurrencySwap {
      public:
        MakeConstNotionalCrossCurrencySwap(Leg payLeg,
                                            Currency payCurrency,
                                            Leg recLeg,
                                            Currency recCurrency);

        operator ConstNotionalCrossCurrencySwap() const;
        operator ext::shared_ptr<ConstNotionalCrossCurrencySwap>() const;

        //! discount curve used for the pay-currency leg
        MakeConstNotionalCrossCurrencySwap&
        withPayDiscountCurve(const Handle<YieldTermStructure>& payDiscountCurve);
        //! discount curve used for the receive-currency leg
        MakeConstNotionalCrossCurrencySwap&
        withRecDiscountCurve(const Handle<YieldTermStructure>& recDiscountCurve);
        /*! spot FX rate, expressed as units of the receive currency per unit
            of the pay currency, quoted for settlement on the npv date (unless
            \c withSpotFXSettleDate is used).
        */
        MakeConstNotionalCrossCurrencySwap& withFxSpot(const Handle<Quote>& spotFX);

        MakeConstNotionalCrossCurrencySwap&
        withIncludeSettlementDateFlows(bool includeSettlementDateFlows);
        MakeConstNotionalCrossCurrencySwap& withSettlementDate(const Date& settlementDate);
        MakeConstNotionalCrossCurrencySwap& withNpvDate(const Date& npvDate);
        MakeConstNotionalCrossCurrencySwap& withSpotFXSettleDate(const Date& spotFXSettleDate);

        //! overrides any engine that would otherwise be built from the curves/spot above
        MakeConstNotionalCrossCurrencySwap&
        withPricingEngine(const ext::shared_ptr<PricingEngine>& engine);

      private:
        Leg payLeg_, recLeg_;
        Currency payCurrency_, recCurrency_;

        Handle<YieldTermStructure> payDiscountCurve_, recDiscountCurve_;
        Handle<Quote> spotFX_;
        std::optional<bool> includeSettlementDateFlows_;
        Date settlementDate_, npvDate_, spotFXSettleDate_;

        ext::shared_ptr<PricingEngine> engine_;
    };

}

#endif
