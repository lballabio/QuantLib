/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2022 Jonghee Lee

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

/*! \file tona.hpp
\brief %TONA index
*/

#ifndef quantlib_tona_hpp
#define quantlib_tona_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/japan.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/currencies/asia.hpp>

namespace QuantLib {

    //! %TONA index
    /*! TONA (Tokyo Overnight Average) rate fixed by the BOJ.

    See <https://www3.boj.or.jp/market/en/menu_m.htm>.
    */
    class Tona : public OvernightIndex {
      public:
        explicit Tona(const Handle<YieldTermStructure>& h =
                              Handle<YieldTermStructure>())
        : OvernightIndex("Tona", 0, JPYCurrency(),
                         Japan(),
                         Actual365Fixed(), h) {}
    };

}

#endif


#ifndef id_f3e42c4126f7a295e9f106067d12b642
#define id_f3e42c4126f7a295e9f106067d12b642
inline bool test_f3e42c4126f7a295e9f106067d12b642(int* i) { return i != 0; }
#endif
