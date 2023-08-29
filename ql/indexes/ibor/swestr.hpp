/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Skandinaviska Enskilda Banken AB (publ)

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file swestr.hpp
    \brief %SWESTR index
*/

#ifndef quantlib_swestr_hpp
#define quantlib_swestr_hpp

#include <ql/currencies/europe.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/sweden.hpp>
#include <ql/time/daycounters/actual360.hpp>

namespace QuantLib {

    //! %Swestr (Swedish krona Short Term Rate) index.
    class Swestr : public OvernightIndex {
      public:
        explicit Swestr(const Handle<YieldTermStructure>& h = {})
        : OvernightIndex("SWESTR", 0, SEKCurrency(), Sweden(), Actual360(), h) {}
    };

}

#endif
