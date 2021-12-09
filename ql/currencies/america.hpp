/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2008 StatPro Italia srl

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

/*! \file america.hpp
    \brief American currencies

    Data from http://fx.sauder.ubc.ca/currency_table.html
    and http://www.thefinancials.com/vortex/CurrencyFormats.html
*/

#ifndef quantlib_american_currencies_hpp
#define quantlib_american_currencies_hpp

#include <ql/currency.hpp>

#if defined(QL_PATCH_MSVC)
#pragma warning(push)
#pragma warning(disable:4819)
#endif

namespace QuantLib {

    //! Argentinian peso
    /*! The ISO three-letter code is ARS; the numeric code is 32.
        It is divided in 100 centavos.

        \ingroup currencies
    */
    class ARSCurrency : public Currency {
      public:
        ARSCurrency();
    };

    //! Brazilian real
    /*! The ISO three-letter code is BRL; the numeric code is 986.
        It is divided in 100 centavos.

        \ingroup currencies
    */
    class BRLCurrency : public Currency {
      public:
        BRLCurrency();
    };

    //! Canadian dollar
    /*! The ISO three-letter code is CAD; the numeric code is 124.
        It is divided into 100 cents.

        \ingroup currencies
    */
    class CADCurrency : public Currency {
      public:
        CADCurrency();
    };

    //! Chilean peso
    /*! The ISO three-letter code is CLP; the numeric code is 152.
        It is divided in 100 centavos.

        \ingroup currencies
    */
    class CLPCurrency : public Currency {
      public:
        CLPCurrency();
    };

    //! Colombian peso
    /*! The ISO three-letter code is COP; the numeric code is 170.
        It is divided in 100 centavos.

        \ingroup currencies
    */
    class COPCurrency : public Currency {
      public:
        COPCurrency();
    };

    //! Mexican peso
    /*! The ISO three-letter code is MXN; the numeric code is 484.
        It is divided in 100 centavos.

        \ingroup currencies
    */
    class MXNCurrency : public Currency {
      public:
        MXNCurrency();
    };

    //! Peruvian nuevo sol
    /*! The ISO three-letter code is PEN; the numeric code is 604.
        It is divided in 100 centimos.

        \ingroup currencies
    */
    class PENCurrency : public Currency {
      public:
        PENCurrency();
    };

    //! Peruvian inti
    /*! The ISO three-letter code was PEI.
        It was divided in 100 centimos. A numeric code is not available;
        as per ISO 3166-1, we assign 998 as a user-defined code.

        Obsoleted by the nuevo sol since July 1991.

        \ingroup currencies
    */
    class PEICurrency : public Currency {
      public:
        PEICurrency();
    };

    //! Peruvian sol
    /*! The ISO three-letter code was PEH. A numeric code is not available;
        as per ISO 3166-1, we assign 999 as a user-defined code.
        It was divided in 100 centavos.

        Obsoleted by the inti since February 1985.

        \ingroup currencies
    */
    class PEHCurrency : public Currency {
      public:
        PEHCurrency();
    };

    //! Trinidad & Tobago dollar
    /*! The ISO three-letter code is TTD; the numeric code is 780.
        It is divided in 100 cents.

        \ingroup currencies
    */
    class TTDCurrency : public Currency {
      public:
        TTDCurrency();
    };

    //! U.S. dollar
    /*! The ISO three-letter code is USD; the numeric code is 840.
        It is divided in 100 cents.

        \ingroup currencies
    */
    class USDCurrency : public Currency {
      public:
        USDCurrency();
    };

    //! Venezuelan bolivar
    /*! The ISO three-letter code is VEB; the numeric code is 862.
        It is divided in 100 centimos.

        \ingroup currencies
    */
    class VEBCurrency : public Currency {
      public:
        VEBCurrency();
    };
    //! Mexican Unidad de Inversion
    /*! The ISO three-letter code is MXV; the numeric code is 979.
     A unit of account used in Mexico.
     \ingroup currencies
    */
    class MXVCurrency : public Currency {
      public:
        MXVCurrency();
    };

    //! Unidad de Valor Real
    /*! The ISO three-letter code is COU; the numeric code is 970.
     A unit of account used in Colombia.
     \ingroup currencies
    */
    class COUCurrency : public Currency {
      public:
        COUCurrency();
    };

    //! Unidad de Fomento (funds code)
    /*! The ISO three-letter code is CLF; the numeric code is 990.
     A unit of account used in Chile.
     \ingroup currencies
     */
    class CLFCurrency : public Currency {
      public:
        CLFCurrency();
    };

}

#if defined(QL_PATCH_MSVC)
#pragma warning(pop)
#endif

#endif
