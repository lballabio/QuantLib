/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2022 Jonghee Lee

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

/*! \file tonar.hpp
\brief %TONAR index
*/

#ifndef quantlib_tonar_hpp
#define quantlib_tonar_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/japan.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/currencies/asia.hpp>

namespace QuantLib {

    //! %TONAR index
    /*! TONAR (Tokyo Overnight Average Rate) fixed by the BOJ.

        See <https://www3.boj.or.jp/market/en/menu_m.htm>.
    */
    class Tonar : public OvernightIndex {
      public:
        explicit Tonar(const Handle<YieldTermStructure>& h = {})
        : OvernightIndex("Tonar", 0, JPYCurrency(),
                         Japan(),
                         Actual365Fixed(), h) {}
    };

    /*! \deprecated Renamed to Tonar.
                    Deprecated in version 1.40.
    */
    [[deprecated("Renamed to Tonar")]] typedef Tonar Tona;

}

#endif
