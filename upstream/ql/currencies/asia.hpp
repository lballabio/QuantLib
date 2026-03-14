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

/*! \file asia.hpp
    \brief Asian currencies

    Data from http://fx.sauder.ubc.ca/currency_table.html
    and http://www.thefinancials.com/vortex/CurrencyFormats.html
*/

#ifndef quantlib_asian_currencies_hpp
#define quantlib_asian_currencies_hpp

#include <ql/currency.hpp>

#if defined(QL_PATCH_MSVC)
#pragma warning(push)
#pragma warning(disable:4819)
#endif

namespace QuantLib {

    //! Bangladesh taka
    /*! The ISO three-letter code is BDT; the numeric code is 50.
        It is divided in 100 paisa.

        \ingroup currencies
    */
    class BDTCurrency : public Currency {
      public:
        BDTCurrency();
    };

    //! Chinese yuan
    /*! The ISO three-letter code is CNY; the numeric code is 156.
        It is divided in 100 fen.

        \ingroup currencies
    */
    class CNYCurrency : public Currency {
      public:
        CNYCurrency();
    };

    //! Hong Kong dollar
    /*! The ISO three-letter code is HKD; the numeric code is 344.
        It is divided in 100 cents.

        \ingroup currencies
    */
    class HKDCurrency : public Currency {
      public:
        HKDCurrency();
    };

    //! Indonesian Rupiah
    /*! The ISO three-letter code is IDR; the numeric code is 360.
        It is divided in 100 sen.

        \ingroup currencies
    */
    class IDRCurrency : public Currency {
      public:
        IDRCurrency();
    };

    //! Israeli shekel
    /*! The ISO three-letter code is ILS; the numeric code is 376.
        It is divided in 100 agorot.

        \ingroup currencies
    */
    class ILSCurrency : public Currency {
      public:
        ILSCurrency();
    };

    //! Indian rupee
    /*! The ISO three-letter code is INR; the numeric code is 356.
        It is divided in 100 paise.

        \ingroup currencies
    */
    class INRCurrency : public Currency {
      public:
        INRCurrency();
    };

    //! Iraqi dinar
    /*! The ISO three-letter code is IQD; the numeric code is 368.
        It is divided in 1000 fils.

        \ingroup currencies
    */
    class IQDCurrency : public Currency {
      public:
        IQDCurrency();
    };

    //! Iranian rial
    /*! The ISO three-letter code is IRR; the numeric code is 364.
        It has no subdivisions.

        \ingroup currencies
    */
    class IRRCurrency : public Currency {
      public:
        IRRCurrency();
    };

    //! Japanese yen
    /*! The ISO three-letter code is JPY; the numeric code is 392.
        It is divided into 100 sen.

        \ingroup currencies
    */
    class JPYCurrency : public Currency {
      public:
        JPYCurrency();
    };

    //! South-Korean won
    /*! The ISO three-letter code is KRW; the numeric code is 410.
        It is divided in 100 chon.

        \ingroup currencies
    */
    class KRWCurrency : public Currency {
      public:
        KRWCurrency();
    };

    //! Kuwaiti dinar
    /*! The ISO three-letter code is KWD; the numeric code is 414.
        It is divided in 1000 fils.

        \ingroup currencies
    */
    class KWDCurrency : public Currency {
      public:
        KWDCurrency();
    };
 
    class KZTCurrency : public Currency {
      public:
       KZTCurrency();
    };

    //! Malaysian Ringgit
    /*! The ISO three-letter code is MYR; the numeric code is 458.
        It is divided in 100 sen.

        \ingroup currencies
    */
    class MYRCurrency : public Currency {
      public:
        MYRCurrency();
    };

    //! Nepal rupee
    /*! The ISO three-letter code is NPR; the numeric code is 524.
        It is divided in 100 paise.

        \ingroup currencies
    */
    class NPRCurrency : public Currency {
      public:
        NPRCurrency();
    };

    //! Pakistani rupee
    /*! The ISO three-letter code is PKR; the numeric code is 586.
        It is divided in 100 paisa.

        \ingroup currencies
    */
    class PKRCurrency : public Currency {
      public:
        PKRCurrency();
    };

    //! Saudi riyal
    /*! The ISO three-letter code is SAR; the numeric code is 682.
        It is divided in 100 halalat.

        \ingroup currencies
    */
    class SARCurrency : public Currency {
      public:
        SARCurrency();
    };

    //! %Singapore dollar
    /*! The ISO three-letter code is SGD; the numeric code is 702.
        It is divided in 100 cents.

        \ingroup currencies
    */
    class SGDCurrency : public Currency {
      public:
        SGDCurrency();
    };

    //! Thai baht
    /*! The ISO three-letter code is THB; the numeric code is 764.
        It is divided in 100 stang.

        \ingroup currencies
    */
    class THBCurrency : public Currency {
      public:
        THBCurrency();
    };

    //! %Taiwan dollar
    /*! The ISO three-letter code is TWD; the numeric code is 901.
        It is divided in 100 cents.

        \ingroup currencies
    */
    class TWDCurrency : public Currency {
      public:
        TWDCurrency();
    };

    //! Vietnamese Dong
    /*! The ISO three-letter code is VND; the numeric code is 704.
        It was divided in 100 xu.

        \ingroup currencies
    */
    class VNDCurrency : public Currency {
      public:
        VNDCurrency();
    };

    //! Qatari riyal
    /*! The ISO three-letter code is QAR; the numeric code is 634.
     It is divided into 100 diram.
     \ingroup currencies
    */
    class QARCurrency : public Currency {
      public:
        QARCurrency();
    };

    //! Bahraini dinar
    /*! The ISO three-letter code is BHD; the numeric code is 048.
     It is divided into 1000 fils.
     \ingroup currencies
    */
    class BHDCurrency : public Currency {
      public:
        BHDCurrency();
    };

    //! Omani rial
    /*! The ISO three-letter code is OMR; the numeric code is 512.
     It is divided into 1000 baisa.
     \ingroup currencies
     */
    class OMRCurrency : public Currency {
      public:
        OMRCurrency();
    };

    //! Jordanian dinar
    /*! The ISO three-letter code is JOD; the numeric code is 400.
     It is divided into 100 qirshes.
     \ingroup currencies
     */
    class JODCurrency : public Currency {
      public:
        JODCurrency();
    };

    //! United Arab Emirates dirham
    /*! The ISO three-letter code is AED; the numeric code is 784.
     It is divided into 100 fils.
     \ingroup currencies
     */
    class AEDCurrency : public Currency {
      public:
        AEDCurrency();
    };

    //! Philippine peso
    /*! The ISO three-letter code is PHP; the numeric code is 608.
     It is divided into 100 centavo.
     \ingroup currencies
     */
    class PHPCurrency : public Currency {
      public:
        PHPCurrency();
    };

    //! Chinese yuan (Hong Kong)
    /*! The ISO three-letter code is CNH; there is no numeric code.
     It is divided in 100 fen.
     \ingroup currencies
    */
    class CNHCurrency : public Currency {
      public:
        CNHCurrency();
    };

    //! Sri Lankan rupee
    /*! The ISO three-letter code is LKR; there numeric code is 144.
     It is divided into 100 cents.
     \ingroup currencies
    */
    class LKRCurrency : public Currency {
      public:
        LKRCurrency();
    };

}

#if defined(QL_PATCH_MSVC)
#pragma warning(pop)
#endif

#endif
