/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
 All rights reserved.

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file ql/pricingengines/swap/crossccyswapengine.hpp
    \brief Cross currency swap engine

        \ingroup engines
*/

#ifndef quantlib_cross_ccy_swap_engine_hpp
#define quantlib_cross_ccy_swap_engine_hpp

#include <ql/handle.hpp>
#include <ql/instruments/crossccyswap.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/optional.hpp>

namespace QuantLib {

//! Cross currency swap engine

/*! This class implements an engine for pricing swaps comprising legs that
    involve two currencies. The npv is expressed in domesticCcy. The given currencies
    domesticCcy and foreignCcy are matched to the correct swap legs. The evaluation date is the
    reference date of either discounting curve (which must be equal).

   \ingroup engines
*/
class CrossCcySwapEngine : public CrossCcySwap::engine {
public:
    //! \name Constructors
    //@{
    /*! \param domesticCcy
               Currency 1
        \param domesitcCcyDiscountCurve
               Discount curve for cash flows in currency 1
        \param foreignCcy
               Currency 2
        \param foreignCcyDiscountCurve
               Discount curve for cash flows in currency 2
        \param spotFX
               The market spot rate quote, given as units of domesticCcy
               for one unit of foreignCcy. The spot rate must be given
               w.r.t. a settlement equal to the npv date.
        \param includeSettlementDateFlows, settlementDate
               If includeSettlementDateFlows is true (false), cashflows
               on the settlementDate are (not) included in the NPV.
               If not given the settlement date is set to the
               npv date.
        \param npvDate
               Discount to this date. If not given the npv date
               is set to the evaluation date
        \param spotFXSettleDate
               FX conversion as of this date if specified explicitly
    */
    CrossCcySwapEngine(const Currency& domesticCcy, const Handle<YieldTermStructure>& domesitcCcyDiscountCurve,
                       const Currency& foreignCcy, const Handle<YieldTermStructure>& foreignCcyDiscountCurve,
                       const Handle<Quote>& spotFX, ext::optional<bool> includeSettlementDateFlows = ext::nullopt,
                       const Date& settlementDate = Date(), const Date& npvDate = Date(), const Date& spotFXSettleDate = Date());
    //@}

    //! \name PricingEngine interface
    //@{
    void calculate() const override;
    //@}

    //! \name Inspectors
    //@{
    const Handle<YieldTermStructure>& domesitcCcyDiscountCurve() const { return domesticCcyDiscountcurve_; }
    const Handle<YieldTermStructure>& foreignCcyDiscountCurve() const { return foreignCcyDiscountcurve_; }

    const Currency& currency1() const { return domesticCcy_; }
    const Currency& currency2() const { return foreignCcy_; }

    const Handle<Quote>& spotFX() const { return spotFX_; }
    //@}

private:
    Currency domesticCcy_;
    Handle<YieldTermStructure> domesticCcyDiscountcurve_;
    Currency foreignCcy_;
    Handle<YieldTermStructure> foreignCcyDiscountcurve_;
    Handle<Quote> spotFX_;
    ext::optional<bool> includeSettlementDateFlows_;
    Date settlementDate_;
    Date npvDate_;
    Date spotFXSettleDate_;
};
} // namespace QuantLib

#endif
