
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

/*! \file demcurrency.hpp
    \brief German mark
*/

#ifndef quantlib_dem_currency_hpp
#define quantlib_dem_currency_hpp

#include <ql/currency.hpp>
#include <ql/Currencies/eurcurrency.hpp>

namespace QuantLib {

    //! Deutsche mark
    /*! The ISO three-letter code is (or rather was) DEM; the numeric
        code was 276.
        It was divided into 100 pfennig.

        \ingroup currencies
    */
    class DEMCurrency : public Currency {
      public:
        DEMCurrency()
        : Currency("Deutsche mark", // name
                   "DEM",           // ISO 4217 code
                   276,             // ISO 4217 numeric code
                   "",              // symbol
                   "",              // fraction symbol
                   100,             // fractions per unit
                   Rounding(),      // preferred rounding
                   EURCurrency())   // triangulation currency
        {}
    };

}


#endif
