
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

/*! \file audcurrency.hpp
    \brief Australian dollar
*/

#ifndef quantlib_aud_currency_hpp
#define quantlib_aud_currency_hpp

#include <ql/currency.hpp>

namespace QuantLib {

    //! Australian dollar
    /*! The ISO three-letter code is AUD; the numeric code is 36.
        It is divided into 100 cents.

        \ingroup currencies
    */
    class AUDCurrency : public Currency {
      public:
        AUDCurrency()
        : Currency("Australian dollar", // name
                   "AUD",               // ISO 4217 code
                   36,                  // ISO 4217 numeric code
                   "A$",                // symbol
                   "",                  // fraction symbol
                   100,                 // fractions per unit
                   Rounding(),          // preferred rounding
                   Currency())          // triangulation currency
        {}
    };

}


#endif
