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

#include <ql/currencies/africa.hpp>

namespace QuantLib {

    // South-African rand
    /* The ISO three-letter code is ZAR; the numeric code is 710.
       It is divided into 100 cents.
    */
    ZARCurrency::ZARCurrency() {
        static ext::shared_ptr<Data> zarData(
                                new Data("South-African rand", "ZAR", 710,
                                         "R", "", 100,
                                         Rounding(),
                                         "%3% %1$.2f"));
        data_ = zarData;
    }

    // Algerian Dinar
    /* The ISO three-letter code is DZD; the numeric code is 012.
       It is divided into 100 centimes.
    */
    DZDCurrency::DZDCurrency(){
        static ext::shared_ptr<Data> dzdData(
                                new Data("Algerian Dinar", "DZD", 012,
                                        "DA", "", 100,
                                        Rounding(), "%3% %1$.2f"));
        data_ = dzdData;
    }

    // Angolan Kwanza
    /* The ISO three-letter code is AOA; the numeric code is 973.
       It is divided into 100 Centimos.
    */
    AOACurrency::AOACurrency(){
        static ext::shared_ptr<Data> aoaData(
                                new Data("Angolan Kwanza", "AOA", 973,
                                        "Kz", "", 100,
                                        Rounding(), "%3% %1$.2f"));
    }

}
