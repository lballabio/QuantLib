
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

/*! \file eurcurrency.hpp
    \brief European Euro
*/

#ifndef quantlib_eur_currency_hpp
#define quantlib_eur_currency_hpp

#include <ql/currency.hpp>

namespace QuantLib {

    //! European Euro
    /*! The ISO three-letter code is EUR; the numeric code is 978.
        It is divided into 100 cents.

        \ingroup currencies
    */
    class EURCurrency : public Currency {
      public:
        EURCurrency()
        : Currency("European Euro",      // name
                   "EUR",                // ISO 4217 code
                   978,                  // ISO 4217 numeric code
                   "",                   // symbol
                   "",                   // fraction symbol
                   100,                  // fractions per unit
                   ClosestRounding(2),   // preferred rounding
                   Currency())           // triangulation currency
        {}
    };

}


#endif
