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

    // CFA Franc BCEAO
    /* The ISO three-letter code is XOF; the numeric code is 952.
       It is divided into 100 centimes.
    */
    XOFCurrency::XOFCurrency(){
        static ext::shared_ptr<Data> xofData(
                                new Data("CFA Franc BCEAO", "XOF", 952,
                                        "CFA", "c", 100,
                                        Rounding(), "%3% %1$.2f"));
    }

    // Botswana Pula
    /* This ISO three-letter code is BWP; the numeric code is 072.
       It is divided into 100 Thebe
     */
    BWPCurrency::BWPCurrency(){
        static ext::shared_ptr<Data> bwpData(
                                new Data("Botswanan Pula", "BWP", 072,
                                        "P", "", 100,
                                        Rounding(), "%3% %1$.2f"));
    }
    // Burundi Franc
    /* This ISO three-letter code is BIF; the numeric code is 108.
       It is divided into 100 Centimes
     */
    BIFCurrency::BIFCurrency(){
        static ext::shared_ptr<Data> bifData(
                                new Data("Burundi Franc", "BIF", 108,
                                        "FBu", "", 100,
                                        Rounding(), "%3% %1$.2f"));
    }

    // CFA Franc BEAC
    /* This ISO three-letter code is XAF; the numeric code is 950.
       It is divided into 100 Centimes
     */
    XAFCurrency::XAFCurrency(){
        static ext::shared_ptr<Data> xafData(
                                    new Data("CFA Franc BEAC", "XAF", 950,
                                            "FCFA", "c", 100,
                                            Rounding(), "%3% %1$.2f"));
    }

}
