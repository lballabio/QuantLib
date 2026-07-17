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

/*! \file discountingmtmcrosscurrencybasisswapengine.hpp
    \brief Discounting engine for mark-to-market cross-currency basis swaps
*/

#ifndef quantlib_discounting_mtm_cross_currency_basis_swap_engine_hpp
#define quantlib_discounting_mtm_cross_currency_basis_swap_engine_hpp

#include <ql/handle.hpp>
#include <ql/instruments/mtmcrosscurrencybasisswap.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <optional>

namespace QuantLib {

//! Discounting mark-to-market cross-currency basis swap engine
/*! Prices a MtMCrossCurrencyBasisSwap by discounting each leg on its own
    currency curve and converting to \p domesticCcy through the FX spot.

    The constant-notional leg is discounted as usual (its notional-exchange
    cash flows are already part of the leg).  The resettable leg is valued
    analytically: each period's notional is the constant-leg notional times the
    FX rate observed at the period's reset.  A future reset is projected as a
    forward FX from the two discount curves, so no explicit FX-reset cash flows
    are needed; an already-fixed reset (the in-progress period of a seasoned
    swap) is read from \c ExchangeRateManager.

    \ingroup engines
*/
class DiscountingMtMCrossCurrencyBasisSwapEngine : public MtMCrossCurrencyBasisSwap::engine {
  public:
    /*! \param domesticCcy                Domestic (NPV) currency.
        \param domesticCcyDiscountCurve   Discount curve for domestic-currency flows.
        \param foreignCcy                 Foreign currency.
        \param foreignCcyDiscountCurve    Discount curve for foreign-currency flows.
        \param spotFX                     Units of \p domesticCcy per unit of
                                          \p foreignCcy, quoted for settlement on
                                          \p spotFXSettleDate (or the curves'
                                          reference date if none is given).
        \param includeSettlementDateFlows,settlementDate
               If includeSettlementDateFlows is true (false), cash flows on the
               settlementDate are (not) included in the NPV.  The settlement date
               defaults to the curves' reference date.
        \param npvDate                    Discount to this date; defaults to the
                                          curves' reference date.
        \param spotFXSettleDate           Settlement date of the FX quote if given.
    */
    DiscountingMtMCrossCurrencyBasisSwapEngine(
        Currency domesticCcy, const Handle<YieldTermStructure>& domesticCcyDiscountCurve,
        Currency foreignCcy, const Handle<YieldTermStructure>& foreignCcyDiscountCurve,
        const Handle<Quote>& spotFX,
        std::optional<bool> includeSettlementDateFlows = std::nullopt,
        const Date& settlementDate = Date(), const Date& npvDate = Date(),
        const Date& spotFXSettleDate = Date());

    //! \name PricingEngine interface
    //@{
    void calculate() const override;
    //@}

    //! \name Inspectors
    //@{
    const Handle<YieldTermStructure>& domesticCcyDiscountCurve() const {
        return domesticCcyDiscountcurve_;
    }
    const Handle<YieldTermStructure>& foreignCcyDiscountCurve() const {
        return foreignCcyDiscountcurve_;
    }
    const Currency& domesticCurrency() const { return domesticCcy_; }
    const Currency& foreignCurrency() const { return foreignCcy_; }
    const Handle<Quote>& spotFX() const { return spotFX_; }
    //@}

  private:
    Currency domesticCcy_;
    Handle<YieldTermStructure> domesticCcyDiscountcurve_;
    Currency foreignCcy_;
    Handle<YieldTermStructure> foreignCcyDiscountcurve_;
    Handle<Quote> spotFX_;
    std::optional<bool> includeSettlementDateFlows_;
    Date settlementDate_;
    Date npvDate_;
    Date spotFXSettleDate_;
};

}

#endif
