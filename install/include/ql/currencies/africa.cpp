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

#include <ql/currencies/africa.hpp>

namespace QuantLib {

    // Angolan kwanza
    AOACurrency::AOACurrency() {
        static auto aoaData = ext::make_shared<Data>("Angolan kwanza", "AOA", 973, "AOA", "", 100, Rounding());
        data_ = aoaData;
    }

    // Botswanan pula
    BWPCurrency::BWPCurrency() {
        static auto  bwpData = ext::make_shared<Data>("Botswanan pula", "BWP", 72, "P", "", 100, Rounding());
        data_ = bwpData;
    }

    // Egyptian pound
    EGPCurrency::EGPCurrency() {
        static auto  egpData = ext::make_shared<Data>("Egyptian pound", "EGP", 818, "EGP", "", 100, Rounding());
        data_ = egpData;
    }

    // Ethiopian birr
    ETBCurrency::ETBCurrency() {
        static auto  etbData = ext::make_shared<Data>("Ethiopian birr", "ETB", 230, "ETB", "", 100, Rounding());
        data_ = etbData;
    }

    // Ghanaian cedi
    GHSCurrency::GHSCurrency() {
        static auto  ghsData = ext::make_shared<Data>("Ghanaian cedi", "GHS", 936, "GHS", "", 100, Rounding());
        data_ = ghsData;
    }

    // Kenyan shilling
    KESCurrency::KESCurrency() {
        static auto  kesData = ext::make_shared<Data>("Kenyan shilling", "KES", 404, "KES", "", 100, Rounding());
        data_ = kesData;
    }

    // Moroccan dirham
    MADCurrency::MADCurrency() {
        static auto  madData = ext::make_shared<Data>("Moroccan dirham", "MAD", 504, "MAD", "", 100, Rounding());
        data_ = madData;
    }

    // Mauritian rupee
    MURCurrency::MURCurrency() {
        static auto murData = ext::make_shared<Data>("Mauritian rupee", "MUR", 480, "MUR", "", 100, Rounding());
        data_ = murData;
    }

    // Nigerian Naira
    NGNCurrency::NGNCurrency() {
        static auto ngnData = ext::make_shared<Data>("Nigerian Naira", "NGN", 566, "N", "K", 100, Rounding());
        data_ = ngnData;
    }

    // Tunisian dinar
    TNDCurrency::TNDCurrency() {
        static auto tndData = ext::make_shared<Data>("Tunisian dinar", "TND", 788, "TND", "", 1000, Rounding());
        data_ = tndData;
    }

    // Ugandan shilling
    UGXCurrency::UGXCurrency() {
        static auto ugxData = ext::make_shared<Data>("Ugandan shilling", "UGX", 800, "UGX", "", 1, Rounding());
        data_ = ugxData;
    }

    // West African CFA franc
    XOFCurrency::XOFCurrency() {
        static auto xofData = ext::make_shared<Data>("West African CFA franc", "XOF", 952, "XOF", "", 100, Rounding());
        data_ = xofData;
    }

    // South-African rand
    ZARCurrency::ZARCurrency() {
        static auto zarData = ext::make_shared<Data>("South-African rand", "ZAR", 710, "R", "", 100, Rounding());
        data_ = zarData;
    }

    // Zambian kwacha
    ZMWCurrency::ZMWCurrency() {
        static auto zmwData = ext::make_shared<Data>("Zambian kwacha", "ZMW", 967, "ZMW", "", 100, Rounding());
        data_ = zmwData;
    }

}
