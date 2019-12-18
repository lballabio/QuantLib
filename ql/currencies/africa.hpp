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

    //! Algerian Dinar
    /*! The ISO three-letter code is DZD; the numeric code is 012.
        It is divided into 100 Centimes.

        However, centimes are now obsolete due to their exteremely small value.

        \ingroup currencies
     */
     class DZDCurrency : public Currency {
        public:
         DZDCurrency();
     };

     //! Algolan Kwanza
     /*! The ISO three-letter code is AOA; the numeric code is 973.
         It is divided into 100 Centimos.
      */
      class AOACurrency : public Currency {
      public:
          AOACurrency();
      };

}

#if defined(QL_PATCH_MSVC)
#pragma warning(pop)
#endif

#endif
