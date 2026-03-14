/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005 StatPro Italia srl
 Copyright (C) 2016 Quaternion Risk Management Ltd

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

#include <ql/currencies/asia.hpp>

namespace QuantLib {

    // Bangladesh taka
    /* The ISO three-letter code is BDT; the numeric code is 50.
       It is divided in 100 paisa.
    */
    BDTCurrency::BDTCurrency() {
        static auto bdtData = ext::make_shared<Data>("Bangladesh taka", "BDT", 50, "Bt", "", 100, Rounding());
        data_ = bdtData;
    }

    // Chinese yuan
    /* The ISO three-letter code is CNY; the numeric code is 156.
       It is divided in 100 fen.
    */
    CNYCurrency::CNYCurrency() {
        static auto cnyData = ext::make_shared<Data>("Chinese yuan", "CNY", 156, "Y", "", 100, Rounding());
        data_ = cnyData;
    }

    // Hong Kong dollar
    /* The ISO three-letter code is HKD; the numeric code is 344.
      It is divided in 100 cents.
    */
    HKDCurrency::HKDCurrency() {
        static auto hkdData = ext::make_shared<Data>("Hong Kong dollar", "HKD", 344, "HK$", "", 100, Rounding());
        data_ = hkdData;
    }

    // Indonesian Rupiah
    /* The ISO three-letter code is IDR; the numeric code is 360.
       It is divided in 100 sen.
    */
    IDRCurrency::IDRCurrency() {
        static auto idrData = ext::make_shared<Data>("Indonesian Rupiah", "IDR", 360, "Rp", "", 100, Rounding());
        data_ = idrData;
    }

    // Israeli shekel
    /* The ISO three-letter code is ILS; the numeric code is 376.
      It is divided in 100 agorot.
    */
    ILSCurrency::ILSCurrency() {
        static auto ilsData = ext::make_shared<Data>("Israeli shekel", "ILS", 376, "NIS", "", 100, Rounding());
        data_ = ilsData;
    }

    // Indian rupee
    /* The ISO three-letter code is INR; the numeric code is 356.
       It is divided in 100 paise.
    */
    INRCurrency::INRCurrency() {
        static auto inrData = ext::make_shared<Data>("Indian rupee", "INR", 356, "Rs", "", 100, Rounding());
        data_ = inrData;
    }

    // Iraqi dinar
    /* The ISO three-letter code is IQD; the numeric code is 368.
       It is divided in 1000 fils.
    */
    IQDCurrency::IQDCurrency() {
        static auto iqdData = ext::make_shared<Data>("Iraqi dinar", "IQD", 368, "ID", "", 1000, Rounding());
        data_ = iqdData;
    }

    // Iranian rial
    /* The ISO three-letter code is IRR; the numeric code is 364.
       It has no subdivisions.
    */
    IRRCurrency::IRRCurrency() {
        static auto irrData = ext::make_shared<Data>("Iranian rial", "IRR", 364, "Rls", "", 1, Rounding());
        data_ = irrData;
    }

    // Japanese yen
    /* The ISO three-letter code is JPY; the numeric code is 392.
       It is divided into 100 sen.
    */
    JPYCurrency::JPYCurrency() {
        static auto jpyData = ext::make_shared<Data>("Japanese yen", "JPY", 392, "\xA5", "", 100, Rounding());
        data_ = jpyData;
    }

    // South-Korean won
    /* The ISO three-letter code is KRW; the numeric code is 410.
       It is divided in 100 chon.
    */
    KRWCurrency::KRWCurrency() {
        static auto krwData = ext::make_shared<Data>("South-Korean won", "KRW", 410, "W", "", 100, Rounding());
        data_ = krwData;
    }

    // Kuwaiti dinar
    /* The ISO three-letter code is KWD; the numeric code is 414.
       It is divided in 1000 fils.
    */
    KWDCurrency::KWDCurrency() {
        static auto kwdData = ext::make_shared<Data>("Kuwaiti dinar", "KWD", 414, "KD", "", 1000, Rounding());
        data_ = kwdData;
    }
 
    // Kazakstani Tenge
    /* The ISO three-letter code is KZT; the numeric code is 398.
    It is divided in 100 tijin.
    */
    KZTCurrency::KZTCurrency() {
        static auto kztData = ext::make_shared<Data>("Kazakstanti Tenge", "KZT", 398, "Kzt", "", 100, Rounding());
        data_ = kztData;
    }

    // Malaysian Ringgit
    /* The ISO three-letter code is MYR; the numeric code is 458.
       It is divided in 100 sen.
    */
    MYRCurrency::MYRCurrency() {
        static auto myrData = ext::make_shared<Data>("Malaysian Ringgit", "MYR", 458, "RM", "", 100, Rounding());
        data_ = myrData;
    }
    
    // Nepal rupee
    /* The ISO three-letter code is NPR; the numeric code is 524.
       It is divided in 100 paise.
    */
    NPRCurrency::NPRCurrency() {
        static auto nprData = ext::make_shared<Data>("Nepal rupee", "NPR", 524, "NRs", "", 100, Rounding());
        data_ = nprData;
    }

    // Pakistani rupee
    /* The ISO three-letter code is PKR; the numeric code is 586.
       It is divided in 100 paisa.
    */
    PKRCurrency::PKRCurrency() {
        static auto pkrData = ext::make_shared<Data>("Pakistani rupee", "PKR", 586, "Rs", "", 100, Rounding());
        data_ = pkrData;
    }

    // Saudi riyal
    /* The ISO three-letter code is SAR; the numeric code is 682.
       It is divided in 100 halalat.
    */
    SARCurrency::SARCurrency() {
        static auto sarData = ext::make_shared<Data>("Saudi riyal", "SAR", 682, "SRls", "", 100, Rounding());
        data_ = sarData;
    }

    // %Singapore dollar
    /* The ISO three-letter code is SGD; the numeric code is 702.
       It is divided in 100 cents.
    */
    SGDCurrency::SGDCurrency() {
        static auto sgdData = ext::make_shared<Data>("Singapore dollar", "SGD", 702, "S$", "", 100, Rounding());
        data_ = sgdData;
    }

    // Thai baht
    /* The ISO three-letter code is THB; the numeric code is 764.
       It is divided in 100 stang.
    */
    THBCurrency::THBCurrency() {
        static auto thbData = ext::make_shared<Data>("Thai baht", "THB", 764, "Bht", "", 100, Rounding());
        data_ = thbData;
    }

    // %Taiwan dollar
    /* The ISO three-letter code is TWD; the numeric code is 901.
       It is divided in 100 cents.
    */
    TWDCurrency::TWDCurrency() {
        static auto twdData = ext::make_shared<Data>("Taiwan dollar", "TWD", 901, "NT$", "", 100, Rounding());
        data_ = twdData;
    }

    // Vietnamese Dong
    /* The ISO three-letter code is VND; the numeric code is 704.
       It was divided in 100 xu.
    */
    VNDCurrency::VNDCurrency() {
        static auto twdData = ext::make_shared<Data>("Vietnamese Dong", "VND", 704, "", "", 100, Rounding());
        data_ = twdData;
    }

    // Qatari riyal
    QARCurrency::QARCurrency() {
        static auto qarData = ext::make_shared<Data>("Qatari riyal", "QAR", 634, "QAR", "", 100, Rounding());
        data_ = qarData;
    }

    // Bahraini dinar
    BHDCurrency::BHDCurrency() {
        static auto bhdData = ext::make_shared<Data>("Bahraini dinar", "BHD", 48, "BHD", "", 1000, Rounding());
        data_ = bhdData;
    }

    // Omani rial
    OMRCurrency::OMRCurrency() {
        static auto omrData = ext::make_shared<Data>("Omani rial", "OMR", 512, "OMR", "", 1000, Rounding());
        data_ = omrData;
    }

    // Jordanian dinar
    JODCurrency::JODCurrency() {
        static auto jodData = ext::make_shared<Data>("Jordanian dinar", "JOD", 400, "JOD", "", 1000, Rounding());
        data_ = jodData;
    }

    // United Arab Emirates dirham
    AEDCurrency::AEDCurrency() {
        static auto aedData = ext::make_shared<Data>("United Arab Emirates dirham", "AED", 784, "AED", "", 100, Rounding());
        data_ = aedData;
    }

    // Philippine peso
    PHPCurrency::PHPCurrency() {
        static auto phpData = ext::make_shared<Data>("Philippine peso", "PHP", 608, "PHP", "", 100, Rounding());
        data_ = phpData;
    }

    // Chinese yuan (Hong Kong)
    CNHCurrency::CNHCurrency() {
        static auto cnhData = ext::make_shared<Data>("Chinese yuan (Hong Kong)", "CNH", 156, "CNH", "", 100, Rounding());
        data_ = cnhData;
    }

    // Sri Lankan rupee
    LKRCurrency::LKRCurrency() {
        static auto lkrData = ext::make_shared<Data>("Sri Lankan rupee", "LKR", 144, "LKR", "", 100, Rounding());
        data_ = lkrData;
    }

}

