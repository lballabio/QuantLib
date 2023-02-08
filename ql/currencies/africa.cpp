/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005 StatPro Italia srl
 Copyright (C) 2016 Quaternion Risk Management Ltd
 Copyright (C) 2023 Skandinaviska Enskilda Banken AB (publ)

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

    // Nigerian Naira
    /* ISO-4217 code -> NGN
     * Corresponding code Number -> 566
     * The NGN ISO code has a currency exponent of 2 ( i.e subdivided into 100 kobo), 100k = 1NGN.
     */
    NGNCurrency::NGNCurrency() {
        static ext::shared_ptr<Data> ngnData(
            new Data("Nigerian Naira", "NGN", 566,
                     "N", "K", 100,
                     Rounding(),
                     "%3% %1N.2f"));
        data_ = ngnData;
    }
    // Tunisian dinar
    TNDCurrency::TNDCurrency() {
        static ext::shared_ptr<Data> tndData(
            new Data("Tunisian dinar", "TND", 788, "TND", "", 1000, Rounding(), "1$.2f %3%"));
        data_ = tndData;
    }
    // Egyptian pound
    EGPCurrency::EGPCurrency() {
        static ext::shared_ptr<Data> egpData(
            new Data("Egyptian pound", "EGP", 818, "EGP", "", 100, Rounding(), "1$.2f %3%"));
        data_ = egpData;
    }

    // Mauritian rupee
    MURCurrency::MURCurrency() {
        static ext::shared_ptr<Data> murData(
            new Data("Mauritian rupee", "MUR", 480, "MUR", "", 100, Rounding(), "1$.2f %3%"));
        data_ = murData;
    }

    // Ugandan shilling
    UGXCurrency::UGXCurrency() {
        static ext::shared_ptr<Data> ugxData(
            new Data("Ugandan shilling", "UGX", 800, "UGX", "", 1, Rounding(), "1$.2f %3%"));
        data_ = ugxData;
    }

    // Zambian kwacha
    ZMWCurrency::ZMWCurrency() {
        static ext::shared_ptr<Data> zmwData(
            new Data("Zambian kwacha", "ZMW", 967, "ZMW", "", 100, Rounding(), "1$.2f %3%"));
        data_ = zmwData;
    }

    // Moroccan dirham
    MADCurrency::MADCurrency() {
        static ext::shared_ptr<Data> madData(
            new Data("Moroccan dirham", "MAD", 504, "MAD", "", 100, Rounding(), "1$.2f %3%"));
        data_ = madData;
    }

    // Kenyan shilling
    KESCurrency::KESCurrency() {
        static ext::shared_ptr<Data> kesData(
            new Data("Kenyan shilling", "KES", 404, "KES", "", 100, Rounding(), "1$.2f %3%"));
        data_ = kesData;
    }

    // Ghanaian cedi
    GHSCurrency::GHSCurrency() {
        static ext::shared_ptr<Data> ghsData(
            new Data("Ghanaian cedi", "GHS", 936, "GHS", "", 100, Rounding(), "1$.2f %3%"));
        data_ = ghsData;
    }

    // Angolan kwanza
    AOACurrency::AOACurrency() {
        static ext::shared_ptr<Data> aoaData(
            new Data("Angolan kwanza", "AOA", 973, "AOA", "", 100, Rounding(), "1$.2f %3%"));
        data_ = aoaData;
    }

    // Ethiopian birr
    ETBCurrency::ETBCurrency() {
        static ext::shared_ptr<Data> etbData(
            new Data("Ethiopian birr", "ETB", 230, "ETB", "", 100, Rounding(), "1$.2f %3%"));
        data_ = etbData;
    }

    // West African CFA franc
    XOFCurrency::XOFCurrency() {
        static ext::shared_ptr<Data> xofData(new Data("West African CFA franc", "XOF", 952, "XOF",
                                                        "", 100, Rounding(), "1$.2f %3%"));
        data_ = xofData;
    }

    
    // Botswanan pula
    BWPCurrency::BWPCurrency() {
        static ext::shared_ptr<Data> bwpData(
            new Data("Botswanan pula", "BWP", 72, "P", "", 100, Rounding(), "1$.2f %3%"));
        data_ = bwpData;
    }

}
