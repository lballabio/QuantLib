
/*
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

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

namespace QuantLib {

    //! Argentinian peso
    /*! The ISO three-letter code is ARS; the numeric code is 32.
        It is divided in 100 centavos.

        ingroup currencies
    */
    class ARSCurrency : public Currency {
      public:
        ARSCurrency()
        : Currency("Argentinian peso",
                   "ARS",
                   32,
                   "",
                   "",
                   100,
                   Rounding(),
                   Currency())
        {}
    };

    //! Brazilian real
    /*! The ISO three-letter code is BRL; the numeric code is 986.
        It is divided in 100 centavos.

        ingroup currencies
    */
    class BRLCurrency : public Currency {
      public:
        BRLCurrency()
        : Currency("Brazilian real",
                   "BRL",
                   986,
                   "R$",
                   "",
                   100,
                   Rounding(),
                   Currency())
        {}
    };

    //! Canadian dollar
    /*! The ISO three-letter code is CAD; the numeric code is 124.
        It is divided into 100 cents.

        \ingroup currencies
    */
    class CADCurrency : public Currency {
      public:
        CADCurrency()
        : Currency("Canadian dollar",
                   "CAD",
                   124,
                   "Can$",
                   "",
                   100,
                   Rounding(),
                   Currency())
        {}
    };

    //! Chilean peso
    /*! The ISO three-letter code is CLP; the numeric code is 152.
        It is divided in 100 centavos.

        ingroup currencies
    */
    class CLPCurrency : public Currency {
      public:
        CLPCurrency()
        : Currency("Chilean peso",
                   "CLP",
                   152,
                   "Ch$",
                   "",
                   100,
                   Rounding(),
                   Currency())
        {}
    };

    //! Colombian peso
    /*! The ISO three-letter code is COP; the numeric code is 170.
        It is divided in 100 centavos.

        ingroup currencies
    */
    class COPCurrency : public Currency {
      public:
        COPCurrency()
        : Currency("Colombian peso",
                   "COP",
                   170,
                   "Col$",
                   "",
                   100,
                   Rounding(),
                   Currency())
        {}
    };

    //! Mexican peso
    /*! The ISO three-letter code is MXN; the numeric code is 484.
        It is divided in 100 centavos.

        ingroup currencies
    */
    class MXNCurrency : public Currency {
      public:
        MXNCurrency()
        : Currency("Mexican peso",
                   "MXN",
                   484,
                   "Mex$",
                   "",
                   100,
                   Rounding(),
                   Currency())
        {}
    };

    //! Trinidad & Tobago dollar
    /*! The ISO three-letter code is TTD; the numeric code is 780.
        It is divided in 100 cents.

        ingroup currencies
    */
    class TTDCurrency : public Currency {
      public:
        TTDCurrency()
        : Currency("Trinidad & Tobago dollar",
                   "TTD",
                   780,
                   "TT$",
                   "",
                   100,
                   Rounding(),
                   Currency())
        {}
    };

    //! U.S. dollar
    /*! The ISO three-letter code is USD; the numeric code is 840.
        It is divided in 100 cents.

        \ingroup currencies
    */
    class USDCurrency : public Currency {
      public:
        USDCurrency()
        : Currency("U.S. dollar",
                   "USD",
                   840,
                   "$",
                   "\xA2",
                   100,
                   Rounding(),
                   Currency())
        {}
    };

    //! Venezuelan bolivar
    /*! The ISO three-letter code is VEB; the numeric code is 862.
        It is divided in 100 centimos.

        ingroup currencies
    */
    class VEBCurrency : public Currency {
      public:
        VEBCurrency()
        : Currency("Venezuelan bolivar",
                   "VEB",
                   862,
                   "Bs",
                   "",
                   100,
                   Rounding(),
                   Currency())
        {}
    };

}


#endif
