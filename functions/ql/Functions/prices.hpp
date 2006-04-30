/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Katiuscia Manzoni

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_functions_prices_h
#define quantlib_functions_prices_h

#include <ql/types.hpp>

namespace QuantLib {

    //! Price types
    //! These types specify the algorithm used to provide the mid-equivalent price.

    enum PriceType {
         Bid,          /*!< Bid price. */
         Ask,          /*!< Ask price. */
         Last,         /*!< Last price. */
         Close,        /*!< Close price. */
         Mid,          /*!< Mid price, calculated as the arithmetic 
                            average of bid and ask prices. */
         MidEquivalent /*!< Mid equivalent price, calculated as
                            a) the arithmetic average of bid and ask prices
                            when both available; b) either the bid or the 
                            ask price if any of them is available;
                            c) the last price; or d) tha close price. */
    };

    /*! return the mid equivalent price, i.e. the mid if available,
        or a suitable substitute if the proper mid is not available
    */
    Real midEquivalent(const Real bid,
                       const Real ask,
                       const Real last,
                       const Real close);
}

#endif

