/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005 StatPro Italia srl

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
        static ext::shared_ptr<Data> btcData(
                                    new Data("Bitcoin", "BTC", 10000,
                                             "BTC", "", 100000,
                                             Rounding(),
                                             "%3% %1$.5f"));
        data_ = btcData;
    }

    //! Ethereum
    /*! https://www.ethereum.org/
    */
    ETHCurrency::ETHCurrency() {
        static ext::shared_ptr<Data> ethData(
                                      new Data("Ethereum", "ETH", 10001,
                                               "ETH", "", 100000,
                                               Rounding(),
                                               "%3% %1$.5f"));
        data_ = ethData;
    }

    //! Ethereum Classic
    /*! https://ethereumclassic.github.io/
    */
    ETCCurrency::ETCCurrency() {
        static ext::shared_ptr<Data> etcData(
                                  new Data("Ethereum Classic", "ETC", 10002,
                                           "ETC", "", 100000,
                                           Rounding(),
                                           "%3% %1$.5f"));
        data_ = etcData;
    }

    //! Bitcoin Cash
    /*! https://www.bitcoincash.org/
    */
    BCHCurrency::BCHCurrency() {
        static ext::shared_ptr<Data> bchData(
                                    new Data("Bitcoin Cash", "BCH", 10003,
                                             "BCH", "", 100000,
                                             Rounding(),
                                             "%3% %1$.5f"));
        data_ = bchData;
    }

    //! Ripple
    /*! https://ripple.com/
    */
    XRPCurrency::XRPCurrency() {
        static ext::shared_ptr<Data> xrpData(
                                    new Data("Ripple", "XRP", 10004,
                                             "XRP", "", 100000,
                                             Rounding(),
                                             "%3% %1$.5f"));
        data_ = xrpData;
    }

    //! Litecoin
    /*! https://litecoin.com/
    */
    LTCCurrency::LTCCurrency() {
            static ext::shared_ptr<Data> ltcData(
                                          new Data("Litecoin", "LTC", 10005,
                                                   "LTC", "", 100000,
                                                   Rounding(),
                                                   "%3% %1$.5f"));
            data_ = ltcData;
        }

    //! Dash coin
    /*! https://www.dash.org/
    */
    DASHCurrency::DASHCurrency() {
        static ext::shared_ptr<Data> dashData(
                                       new Data("Dash coin", "DASH", 10006,
                                                "DASH", "", 100000,
                                                Rounding(),
                                                "%2% %1$.5f"));
        data_ = dashData;
    }

    //! Zcash
    /*! https://z.cash/
    */
    ZECCurrency::ZECCurrency() {
        static ext::shared_ptr<Data> zecData(
                                      new Data("Zcash", "ZEC", 10007,
                                               "ZEC", "", 100000,
                                               Rounding(),
                                               "%3% %1$.5f"));
        data_ = zecData;
    }
}

