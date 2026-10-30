/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Pratyush Patel 

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file shir.hpp
    \brief %SHIR index
*/

#ifndef quantlib_shir_hpp
#define quantlib_shir_hpp

#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    //! %SHIR index.
    /*! Shekel Overnight Interest Rate (SHIR), published by the Bank of
        Israel, replacing the Telbor rate in interest rate derivative
        transactions. SHIR serves as the overnight interest rate for
        that day (same-day fixing).
        See <https://www.boi.org.il/en/economic-roles/financial-markets/shir/>.
    */
    class Shir : public OvernightIndex {
      public:
        explicit Shir(const Handle<YieldTermStructure>& h = {});
    };

}

#endif