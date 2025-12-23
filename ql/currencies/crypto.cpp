/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005 StatPro Italia srl

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

/*
    Data from http://fx.sauder.ubc.ca/currency_table.html
    and http://www.thefinancials.com/vortex/CurrencyFormats.html
*/

#include <ql/currencies/crypto.hpp>

namespace QuantLib {

    // Bitcoin
    /* https://bitcoin.org/
    */
    BTCCurrency::BTCCurrency() {
        static auto btcData = ext::make_shared<Data>("Bitcoin", "BTC", 10000, "BTC", "", 100000, Rounding());
        data_ = btcData;
    }

    //! Ethereum
    /*! https://www.ethereum.org/
    */
    ETHCurrency::ETHCurrency() {
        static auto ethData = ext::make_shared<Data>("Ethereum", "ETH", 10001, "ETH", "", 100000, Rounding());
        data_ = ethData;
    }

    //! Ethereum Classic
    /*! https://ethereumclassic.github.io/
    */
    ETCCurrency::ETCCurrency() {
        static auto etcData = ext::make_shared<Data>("Ethereum Classic", "ETC", 10002, "ETC", "", 100000, Rounding());
        data_ = etcData;
    }

    //! Bitcoin Cash
    /*! https://www.bitcoincash.org/
    */
    BCHCurrency::BCHCurrency() {
        static auto bchData = ext::make_shared<Data>("Bitcoin Cash", "BCH", 10003, "BCH", "", 100000, Rounding());
        data_ = bchData;
    }

    //! Ripple
    /*! https://ripple.com/
    */
    XRPCurrency::XRPCurrency() {
        static auto xrpData = ext::make_shared<Data>("Ripple", "XRP", 10004, "XRP", "", 100000, Rounding());
        data_ = xrpData;
    }

    //! Litecoin
    /*! https://litecoin.com/
    */
    LTCCurrency::LTCCurrency() {
        static auto ltcData = ext::make_shared<Data>("Litecoin", "LTC", 10005, "LTC", "", 100000, Rounding());
        data_ = ltcData;
    }

    //! Dash coin
    /*! https://www.dash.org/
    */
    DASHCurrency::DASHCurrency() {
        static auto dashData = ext::make_shared<Data>("Dash coin", "DASH", 10006, "DASH", "", 100000, Rounding());
        data_ = dashData;
    }

    //! Zcash
    /*! https://z.cash/
    */
    ZECCurrency::ZECCurrency() {
        static auto zecData = ext::make_shared<Data>("Zcash", "ZEC", 10007, "ZEC", "", 100000, Rounding());
        data_ = zecData;
    }

}

