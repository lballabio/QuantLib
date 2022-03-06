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

/*! \file chflibor.hpp
    \brief %CHF %LIBOR rate
*/

#ifndef quantlib_chf_libor_hpp
#define quantlib_chf_libor_hpp

#include <ql/indexes/ibor/libor.hpp>
#include <ql/time/calendars/switzerland.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

    //! %CHF %LIBOR rate
    /*! Swiss Franc LIBOR fixed by ICE.

        See <https://www.theice.com/marketdata/reports/170>.

        \warning This is the rate fixed in London by BBA. Use ZIBOR if
                 you're interested in the Zurich fixing.
    */
    class CHFLibor : public Libor {
      public:
        CHFLibor(const Period& tenor,
                 const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Libor("CHFLibor", tenor,
                2,
                CHFCurrency(),
                Switzerland(),
                Actual360(), h) {}
    };

    //! base class for the one day deposit BBA %CHF %LIBOR indexes
    class DailyTenorCHFLibor : public DailyTenorLibor {
      public:
        DailyTenorCHFLibor(Natural settlementDays,
                           const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : DailyTenorLibor("CHFLibor", settlementDays,
                          CHFCurrency(),
                          Switzerland(),
                          Actual360(), h) {}
    };

}

#endif


#ifndef id_0fa49841e7977f8819c06d3305c07f86
#define id_0fa49841e7977f8819c06d3305c07f86
inline bool test_0fa49841e7977f8819c06d3305c07f86(int* i) { return i != 0; }
#endif
