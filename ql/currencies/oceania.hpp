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

/*! \file oceania.hpp
    \brief Oceanian currencies

    Data from http://fx.sauder.ubc.ca/currency_table.html
    and http://www.thefinancials.com/vortex/CurrencyFormats.html
*/

#ifndef quantlib_oceanian_currencies_hpp
#define quantlib_oceanian_currencies_hpp

#include <ql/currency.hpp>

#if defined(QL_PATCH_MSVC)
#pragma warning(push)
#pragma warning(disable:4819)
#endif

namespace QuantLib {

    //! Australian dollar
    /*! The ISO three-letter code is AUD; the numeric code is 36.
        It is divided into 100 cents.

        \ingroup currencies
    */
    class AUDCurrency : public Currency {
      public:
        AUDCurrency();
    };

    //! New Zealand dollar
    /*! The ISO three-letter code is NZD; the numeric code is 554.
        It is divided in 100 cents.

        \ingroup currencies
    */
    class NZDCurrency : public Currency {
      public:
        NZDCurrency();
    };

}

#if defined(QL_PATCH_MSVC)
#pragma warning(pop)
#endif

#endif


#ifndef id_8f8f429f292be155a8795613f8a852de
#define id_8f8f429f292be155a8795613f8a852de
inline bool test_8f8f429f292be155a8795613f8a852de(int* i) { return i != 0; }
#endif
