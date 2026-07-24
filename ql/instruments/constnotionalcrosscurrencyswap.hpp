/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
 Copyright (C) 2025 Paolo D'Elia

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

/*! \file constnotionalcrosscurrencyswap.hpp
    \brief Generic constant-notional cross currency swap
*/

#ifndef quantlib_const_notional_cross_currency_swap_hpp
#define quantlib_const_notional_cross_currency_swap_hpp

#include <ql/instruments/crosscurrencyswap.hpp>

namespace QuantLib {

//! Generic constant-notional cross currency swap
/*! The notional amounts are constant and exchanged at inception and maturity.

    The first leg holds the pay currency cashflows and second leg holds
    the receive currency cashflows.

    \ingroup instruments
*/
class ConstNotionalCrossCurrencySwap : public CrossCurrencySwap {
  public:
    class arguments;
    class results;
    class engine;
    //! inherited constructors
    /*! @see CrossCurrencySwap::CrossCurrencySwap */
    using CrossCurrencySwap::CrossCurrencySwap;
};


class ConstNotionalCrossCurrencySwap::arguments : public CrossCurrencySwap::arguments {};


class ConstNotionalCrossCurrencySwap::results : public CrossCurrencySwap::results {};


class ConstNotionalCrossCurrencySwap::engine
    : public GenericEngine<ConstNotionalCrossCurrencySwap::arguments,
                           ConstNotionalCrossCurrencySwap::results> {};

}

#endif
