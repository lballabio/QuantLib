
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

/*! \file gbpcurrency.hpp
    \brief British pound sterling
*/

#ifndef quantlib_gbp_currency_hpp
#define quantlib_gbp_currency_hpp

#include <ql/currency.hpp>

namespace QuantLib {

    //! British pound sterling
    /*! The ISO three-letter code is GBP; the numeric code is 826.
        It is divided into 100 pence.

        \ingroup currencies
    */
    class GBPCurrency : public Currency {
      public:
        GBPCurrency()
        : Currency("British pound sterling",   // name
                   "GBP",                      // ISO 4217 code
                   826,                        // ISO 4217 numeric code
                   "\xA3",                     // symbol
                   "p",                        // fraction symbol
                   100,                        // fractions per unit
                   Rounding(),                 // preferred rounding
                   Currency())                 // triangulation currency
        {}
    };

}


#endif
