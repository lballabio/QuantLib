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

#include <ql/instruments/makeconstnotionalcrosscurrencyswap.hpp>
#include <ql/pricingengines/swap/discountingconstnotionalcrosscurrencyswapengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <utility>

namespace QuantLib {

    MakeConstNotionalCrossCurrencySwap::MakeConstNotionalCrossCurrencySwap(
        Leg payLeg, Currency payCurrency, Leg recLeg, Currency recCurrency)
    : payLeg_(std::move(payLeg)), recLeg_(std::move(recLeg)),
      payCurrency_(std::move(payCurrency)), recCurrency_(std::move(recCurrency)) {}

    MakeConstNotionalCrossCurrencySwap::operator ConstNotionalCrossCurrencySwap() const {
        ext::shared_ptr<ConstNotionalCrossCurrencySwap> swap = *this;
        return *swap;
    }

    MakeConstNotionalCrossCurrencySwap::
    operator ext::shared_ptr<ConstNotionalCrossCurrencySwap>() const {

        auto swap = ext::make_shared<ConstNotionalCrossCurrencySwap>(payLeg_, payCurrency_,
                                                                      recLeg_, recCurrency_);

        if (engine_ != nullptr) {
            swap->setPricingEngine(engine_);
        } else if (!payDiscountCurve_.empty() || !recDiscountCurve_.empty()) {
            QL_REQUIRE(!payDiscountCurve_.empty() && !recDiscountCurve_.empty(),
                       "both discount curves are needed to build the pricing engine, but only "
                       "the "
                           << (payDiscountCurve_.empty() ? recCurrency_.code() :
                                                           payCurrency_.code())
                           << " one was given; use withPayDiscountCurve() and "
                              "withRecDiscountCurve()");
            QL_REQUIRE(!spotFX_.empty() || payCurrency_ == recCurrency_,
                       "an FX spot quote is required to build the pricing engine for a "
                           << payCurrency_.code() << "/" << recCurrency_.code()
                           << " swap; use withFxSpot() or supply a custom engine with "
                              "withPricingEngine()");
            Handle<Quote> engineSpotFX = spotFX_.empty() ? makeQuoteHandle(1.0) : spotFX_;
            auto builtEngine = ext::make_shared<DiscountingConstNotionalCrossCurrencySwapEngine>(
                recCurrency_, recDiscountCurve_, payCurrency_, payDiscountCurve_, engineSpotFX,
                includeSettlementDateFlows_, settlementDate_, npvDate_, spotFXSettleDate_);
            swap->setPricingEngine(builtEngine);
        }

        return swap;
    }

    MakeConstNotionalCrossCurrencySwap& MakeConstNotionalCrossCurrencySwap::withPayDiscountCurve(
        const Handle<YieldTermStructure>& payDiscountCurve) {
        payDiscountCurve_ = payDiscountCurve;
        return *this;
    }

    MakeConstNotionalCrossCurrencySwap& MakeConstNotionalCrossCurrencySwap::withRecDiscountCurve(
        const Handle<YieldTermStructure>& recDiscountCurve) {
        recDiscountCurve_ = recDiscountCurve;
        return *this;
    }

    MakeConstNotionalCrossCurrencySwap&
    MakeConstNotionalCrossCurrencySwap::withFxSpot(const Handle<Quote>& spotFX) {
        spotFX_ = spotFX;
        return *this;
    }

    MakeConstNotionalCrossCurrencySwap&
    MakeConstNotionalCrossCurrencySwap::withIncludeSettlementDateFlows(
        bool includeSettlementDateFlows) {
        includeSettlementDateFlows_ = includeSettlementDateFlows;
        return *this;
    }

    MakeConstNotionalCrossCurrencySwap&
    MakeConstNotionalCrossCurrencySwap::withSettlementDate(const Date& settlementDate) {
        settlementDate_ = settlementDate;
        return *this;
    }

    MakeConstNotionalCrossCurrencySwap&
    MakeConstNotionalCrossCurrencySwap::withNpvDate(const Date& npvDate) {
        npvDate_ = npvDate;
        return *this;
    }

    MakeConstNotionalCrossCurrencySwap&
    MakeConstNotionalCrossCurrencySwap::withSpotFXSettleDate(const Date& spotFXSettleDate) {
        spotFXSettleDate_ = spotFXSettleDate;
        return *this;
    }

    MakeConstNotionalCrossCurrencySwap& MakeConstNotionalCrossCurrencySwap::withPricingEngine(
        const ext::shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

}
