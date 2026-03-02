/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
 Copyright (C) 2025 Paolo D'Elia
 All rights reserved.

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

/*! \file crossccyswap.hpp
    \brief Swap instrument with legs involving two currencies

        \ingroup instruments
*/

#ifndef quantlib_cross_ccy_swap_hpp
#define quantlib_cross_ccy_swap_hpp

#include <ql/currency.hpp>
#include <ql/instruments/swap.hpp>

namespace QuantLib {

//! Cross currency swap
/*! The first leg holds the pay currency cashflows and second leg holds
    the receive currency cashflows.

            \ingroup instruments
*/
class CrossCcySwap : public QuantLib::Swap {
public:
    class arguments;
    class results;
    class engine;
    //! \name Constructors
    //@{
    //! Constructs a cross-currency swap with two legs and their respective currencies
    /*!
        First leg is paid and the second is received.

        \param firstLeg The sequence of cash flows for the first leg of the swap.
        \param firstLegCcy The currency in which the first leg's cash flows are denominated.
        \param secondLeg The sequence of cash flows for the second leg of the swap.
        \param secondLegCcy The currency in which the second leg's cash flows are denominated.

        \note The notional amounts, payment schedules, and other details of each leg must be
            set up in the provided Leg objects before constructing the swap.
    */
    CrossCcySwap(const Leg& firstLeg, const Currency& firstLegCcy, const Leg& secondLeg, const Currency& secondLegCcy);
    //! Constructs a cross-currency swap with multiple legs and their respective currencies
    /*!
        Initializes a cross-currency swap with an arbitrary number of legs, each specified
        by a sequence of cash flows (Leg) and associated with its own currency. The payer vector
        determines the direction of each leg (payer or receiver).

        \param legs A vector of cash flow sequences, one for each leg of the swap.
        \param payer A vector of booleans indicating the direction of each leg:
                    \c true for payer, \c false for receiver.
        \param currencies A vector of currencies, one for each leg, specifying the currency
                        in which the corresponding leg's cash flows are denominated.

        \note The sizes of the \p legs, \p payer, and \p currencies vectors must all be equal.
        \warning The notional amounts, payment schedules, and other details of each leg must be
                set up in the provided Leg objects before constructing the swap.
    */
    CrossCcySwap(const std::vector<Leg>& legs, const std::vector<bool>& payer, const std::vector<Currency>& currencies);
    //@}
    //! \name Instrument interface
    //@{
    void setupArguments(PricingEngine::arguments* args) const override;
    void fetchResults(const PricingEngine::results*) const override;
    //@}
    //! \name Additional interface
    //@{
    const Currency& legCurrency(Size j) const {
        QL_REQUIRE(j < legs_.size(), "leg# " << j << " doesn't exist!");
        return currencies_[j];
    }
    Real inCcyLegBPS(Size j) const {
        QL_REQUIRE(j < legs_.size(), "leg# " << j << " doesn't exist!");
        calculate();
        return inCcyLegBPS_[j];
    }
    Real inCcyLegNPV(Size j) const {
        QL_REQUIRE(j < legs_.size(), "leg #" << j << " doesn't exist!");
        calculate();
        return inCcyLegNPV_[j];
    }
    DiscountFactor npvDateDiscounts(Size j) const {
        QL_REQUIRE(j < legs_.size(), "leg #" << j << " doesn't exist!");
        calculate();
        return npvDateDiscounts_[j];
    }
    //@}
protected:
    //! \name Constructors
    //@{
    /*! This constructor can be used by derived classes that will
        build their legs themselves.
    */
    explicit CrossCcySwap(Size legs);
    //@}
    //! \name Instrument interface
    //@{
    void setupExpired() const override;
    //@}

    std::vector<Currency> currencies_;

private:
    mutable std::vector<Real> inCcyLegNPV_;
    mutable std::vector<Real> inCcyLegBPS_;
    mutable std::vector<DiscountFactor> npvDateDiscounts_;
};

//! \ingroup instruments
class CrossCcySwap::arguments : public Swap::arguments {
public:
    std::vector<Currency> currencies;
    void validate() const override;
};

//! \ingroup instruments
class CrossCcySwap::results : public Swap::results {
public:
    std::vector<Real> inCcyLegNPV;
    std::vector<Real> inCcyLegBPS;
    std::vector<DiscountFactor> npvDateDiscounts;
    void reset() override;
};

//! \ingroup instruments
class CrossCcySwap::engine : public GenericEngine<CrossCcySwap::arguments, CrossCcySwap::results> {};
} // namespace QuantLib

#endif
