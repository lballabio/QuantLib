/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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

/*! \file jpylibor.hpp
    \brief %JPY %LIBOR rate
*/

#ifndef quantlib_jpy_libor_hpp
#define quantlib_jpy_libor_hpp

#include <ql/indexes/ibor/libor.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/calendars/japan.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/currencies/asia.hpp>

namespace QuantLib {

    //! %JPY %LIBOR rate
    /*! Japanese Yen LIBOR fixed by ICE.

        See <https://www.theice.com/marketdata/reports/170>.

        \warning This is the rate fixed in London by ICE. Use TIBOR if
                 you're interested in the Tokio fixing.
    */
    class JPYLibor : public Libor {
      public:
        JPYLibor(const Period& tenor,
                 const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Libor("JPYLibor", tenor,
                2,
                JPYCurrency(),
                Japan(),
                Actual360(), h) {}
    };

    //! base class for the one day deposit ICE %JPY %LIBOR indexes
    class DailyTenorJPYLibor : public DailyTenorLibor {
      public:
        DailyTenorJPYLibor(Natural settlementDays,
                           const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : DailyTenorLibor("JPYLibor", settlementDays,
                          JPYCurrency(),
                          Japan(),
                          Actual360(), h) {}
    };

}


#endif


#ifndef id_95fcc15ad396908daa0c1ae6d3029b22
#define id_95fcc15ad396908daa0c1ae6d3029b22
inline bool test_95fcc15ad396908daa0c1ae6d3029b22(int* i) { return i != 0; }
#endif
