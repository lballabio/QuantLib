/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2016 Fabrice Lecuyer

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

/*! \file nzocr.hpp
\brief %NZOCR index
*/

#ifndef quantlib_nzocr_hpp
#define quantlib_nzocr_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/newzealand.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/currencies/oceania.hpp>

namespace QuantLib {

    //! %Nzocr index
    /*! %Nzocr (New Zealand official cash rate) rate fixed by the RBNZ.

        See <http://www.rbnz.govt.nz/monetary-policy/official-cash-rate-decisions>.
    */
    class Nzocr : public OvernightIndex {
      public:
        explicit Nzocr(const Handle<YieldTermStructure>& h = {})
        : OvernightIndex("Nzocr", 0, NZDCurrency(),
                         NewZealand(),
                         Actual365Fixed(), h) {}
    };

}

#endif
