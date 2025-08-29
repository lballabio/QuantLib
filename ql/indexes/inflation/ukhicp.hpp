/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2022 Skandinaviska Enskilda Banken AB (publ)

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

/*! \file ukhicp.hpp
    \brief UK HICP index
*/

#ifndef quantlib_ukhicp_hpp
#define quantlib_ukhicp_hpp

#include <ql/currencies/europe.hpp>
#include <ql/indexes/inflationindex.hpp>

namespace QuantLib {

    //! UK HICP index
    class UKHICP : public ZeroInflationIndex {
      public:
        explicit UKHICP(const Handle<ZeroInflationTermStructure>& ts = {})
        : ZeroInflationIndex(
              "HICP", UKRegion(), false, Monthly, Period(1, Months), GBPCurrency(), ts) {}
    };
}

#endif
