
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

/*! \file itlcurrency.hpp
    \brief Italian lira
*/

#ifndef quantlib_itl_currency_hpp
#define quantlib_itl_currency_hpp

#include <ql/currency.hpp>
#include <ql/Currencies/eurcurrency.hpp>

namespace QuantLib {

    //! Italian lira
    /*! The ISO three-letter code was ITL; the numeric code was 380.
        It had no subdivisions.

        \ingroup currencies
    */
    class ITLCurrency : public Currency {
      public:
        ITLCurrency()
        : Currency("Italian lira",   // name
                   "ITL",            // ISO 4217 code
                   380,              // ISO 4217 numeric code
                   "L",              // symbol
                   "",               // fraction symbol
                   1,                // fractions per unit
                   Rounding(),       // preferred rounding
                   EURCurrency())    // triangulation currency
        {}
    };

}


#endif
