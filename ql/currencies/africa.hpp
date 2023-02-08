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

/*! \file africa.hpp
    \brief African currencies

    Data from http://fx.sauder.ubc.ca/currency_table.html
    and http://www.thefinancials.com/vortex/CurrencyFormats.html
*/

#ifndef quantlib_african_currencies_hpp
#define quantlib_african_currencies_hpp

#include <ql/currency.hpp>

#if defined(QL_PATCH_MSVC)
#pragma warning(push)
#pragma warning(disable:4819)
#endif

namespace QuantLib {

    //! South-African rand
    /*! The ISO three-letter code is ZAR; the numeric code is 710.
        It is divided into 100 cents.

        \ingroup currencies
    */
    class ZARCurrency : public Currency {
      public:
        ZARCurrency();
    };

    class NGNCurrency : public Currency {
      public:
        NGNCurrency();
    };
    //! Tunisian dinar
    /*! The ISO three-letter code is TND; the numeric code is 788.
     It is divided into 1000 millim.
     \ingroup currencies
     */
    class TNDCurrency : public Currency {
      public:
        TNDCurrency();
    };

    //! Egyptian pound
    /*! The ISO three-letter code is EGP; the numeric code is 818.
     It is divided into 100 piastres.
     \ingroup currencies
    */
    class EGPCurrency : public Currency {
      public:
        EGPCurrency();
    };

    //! Mauritian rupee
    /*! The ISO three-letter code is MUR; the numeric code is 480.
     It is divided into 100 cents.
     \ingroup currencies
    */
    class MURCurrency : public Currency {
      public:
        MURCurrency();
    };

    //! Ugandan shilling
    /*! The ISO three-letter code is UGX; the numeric code is 800.
    It is the smallest unit.
     \ingroup currencies
    */
    class UGXCurrency : public Currency {
      public:
        UGXCurrency();
    };

    //! Zambian kwacha
    /*! The ISO three-letter code is ZMW; the numeric code is 967.
    It is divided into 100 ngwee.
     \ingroup currencies
    */
    class ZMWCurrency : public Currency {
      public:
        ZMWCurrency();
    };

    //! Moroccan dirham
    /*! The ISO three-letter code is MAD; the numeric code is 504.
     It is divided into 100 santim.
     \ingroup currencies
    */
    class MADCurrency : public Currency {
      public:
        MADCurrency();
    };

    //! Kenyan shilling
    /*! The ISO three-letter code is KES; the numeric code is 404.
     It is divided into 100 cents.
     \ingroup currencies
    */
    class KESCurrency : public Currency {
      public:
        KESCurrency();
    };

    //! Ghanaian cedi
    /*! The ISO three-letter code is GHS; the numeric code is 936.
     It is divided into 100 pesewas.
     \ingroup currencies
    */
    class GHSCurrency : public Currency {
      public:
        GHSCurrency();
    };

    // Angolan kwanza
    /*! The ISO three-letter code is AOA; the numeric code is 973.
     It is divided into 100 cêntimo.
     \ingroup currencies
    */
    class AOACurrency : public Currency {
      public:
        AOACurrency();
    };

    // Ethiopian birr
    /*! The ISO three-letter code is ETB; the numeric code is 230.
     It is divided into 100 santim.
     \ingroup currencies
    */
    class ETBCurrency : public Currency {
      public:
        ETBCurrency();
    };

    // West African CFA franc
    /*! The ISO three-letter code is XOF; the numeric code is 952.
     It is divided into 100 centime.
     \ingroup currencies
    */
    class XOFCurrency : public Currency {
      public:
        XOFCurrency();
    };

    // Botswanan Pula
    /*! The ISO three-letter code is BWP; the numeric code is 72.
     It is divided into 100 thebe.
     \ingroup currencies
    */
    class BWPCurrency : public Currency {
      public:
        BWPCurrency();
    };
}

#if defined(QL_PATCH_MSVC)
#pragma warning(pop)
#endif

#endif
