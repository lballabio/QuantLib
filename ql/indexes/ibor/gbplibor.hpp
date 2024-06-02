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

/*! \file gbplibor.hpp
    \brief %GBP %LIBOR rate
*/

#ifndef quantlib_gbp_libor_hpp
#define quantlib_gbp_libor_hpp

#include <ql/indexes/ibor/libor.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

    //! %GBP %LIBOR rate
    /*! Pound Sterling LIBOR fixed by ICE.

        See <https://www.theice.com/marketdata/reports/170>.
    */
    class GBPLibor : public Libor {
      public:
        GBPLibor(const Period& tenor,
                 const Handle<YieldTermStructure>& h = {})
        : Libor("GBPLibor", tenor,
                0,
                GBPCurrency(),
                UnitedKingdom(UnitedKingdom::Exchange),
                Actual365Fixed(), h) {}
    };

    //! Base class for the one day deposit ICE %GBP %LIBOR indexes
    class DailyTenorGBPLibor : public DailyTenorLibor {
      public:
        DailyTenorGBPLibor(Natural settlementDays,
                           const Handle<YieldTermStructure>& h = {})
        : DailyTenorLibor("GBPLibor", settlementDays,
                          GBPCurrency(),
                          UnitedKingdom(UnitedKingdom::Exchange),
                          Actual365Fixed(), h) {}
    };

    //! Overnight %GBP %Libor index
    class GBPLiborON : public DailyTenorGBPLibor {
      public:
        explicit GBPLiborON(const Handle<YieldTermStructure>& h = {})
        : DailyTenorGBPLibor(0, h) {}
    };

}

#endif
