/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2006 Chiara Fornarola

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

/*! \file euribor.hpp
    \brief %Euribor index
*/

#ifndef quantlib_euribor_hpp
#define quantlib_euribor_hpp

#include <ql/Indexes/iborindex.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/DayCounters/actual365fixed.hpp>
#include <ql/Currencies/europe.hpp>

namespace QuantLib {

    //! %Euribor index
    /*! Euribor rate fixed by the ECB.

        \warning This is the rate fixed by the ECB. Use EurLibor
                 if you're interested in the London fixing by BBA.
    */
    class Euribor : public IborIndex {
      public:
        Euribor(const Period& tenor,
                const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>(),
                BusinessDayConvention convention = ModifiedFollowing,
                bool endOfMonth = true)
        : IborIndex("Euribor", tenor,
                2, // settlementDays
                EURCurrency(), TARGET(),
                convention, endOfMonth, Actual360(), h) {}
    };


    //! Actual/365 %Euribor index
    /*! Euribor rate adjusted for the mismatch between the actual/360
        convention used for Euribor and the actual/365 convention
        previously used by a few pre-EUR currencies.
    */
    class Euribor365 : public IborIndex {
      public:
        Euribor365(const Period& tenor,
                   const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>(),
                   BusinessDayConvention convention = ModifiedFollowing,
                   bool endOfMonth = true)
        : IborIndex("Euribor", tenor,
                2, // settlementDays
                EURCurrency(), TARGET(),
                convention, endOfMonth, Actual365Fixed(), h) {}
    };


    class WeeklyTenorEuribor : public Euribor {
      public:
        WeeklyTenorEuribor(const Period& tenor,
                const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(tenor, h, Following, false) {}
    };

    class MonthlyTenorEuribor : public Euribor {
      public:
        MonthlyTenorEuribor(const Period& tenor,
                const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(tenor, h, ModifiedFollowing, true) {}
    };

    //! 1-week %Euribor index
    class EuriborSW : public WeeklyTenorEuribor {
      public:
        EuriborSW(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : WeeklyTenorEuribor(Period(1, Weeks), h) {}
    };

    //! 2-weeks %Euribor index
    class Euribor2W : public WeeklyTenorEuribor {
      public:
        Euribor2W(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : WeeklyTenorEuribor(Period(2, Weeks), h) {}
    };

    //! 3-weeks %Euribor index
    class Euribor3W : public WeeklyTenorEuribor {
      public:
        Euribor3W(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : WeeklyTenorEuribor(Period(3, Weeks), h) {}
    };

    //! 1-month %Euribor index
    class Euribor1M : public MonthlyTenorEuribor {
      public:
        Euribor1M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor(Period(1, Months), h) {}
    };

    //! 2-months %Euribor index
    class Euribor2M : public MonthlyTenorEuribor {
      public:
        Euribor2M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor(Period(2, Months), h) {}
    };

    //! 3-months %Euribor index
    class Euribor3M : public MonthlyTenorEuribor {
      public:
        Euribor3M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor(Period(3, Months), h) {}
    };

    //! 4-months %Euribor index
    class Euribor4M : public MonthlyTenorEuribor {
      public:
        Euribor4M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor(Period(4, Months), h) {}
    };

    //! 5-months %Euribor index
    class Euribor5M : public MonthlyTenorEuribor {
      public:
        Euribor5M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor(Period(5, Months), h) {}
    };

    //! 6-months %Euribor index
    class Euribor6M : public MonthlyTenorEuribor {
      public:
        Euribor6M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor(Period(6, Months), h) {}
    };

    //! 7-months %Euribor index
    class Euribor7M : public MonthlyTenorEuribor {
      public:
        Euribor7M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor(Period(7, Months), h) {}
    };

    //! 8-months %Euribor index
    class Euribor8M : public MonthlyTenorEuribor {
      public:
        Euribor8M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor(Period(8, Months), h) {}
    };

    //! 9-months %Euribor index
    class Euribor9M : public MonthlyTenorEuribor {
      public:
        Euribor9M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor(Period(9, Months), h) {}
    };

    //! 10-months %Euribor index
    class Euribor10M : public MonthlyTenorEuribor {
      public:
        Euribor10M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor(Period(10, Months), h) {}
    };

    //! 11-months %Euribor index
    class Euribor11M : public MonthlyTenorEuribor {
      public:
        Euribor11M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor(Period(11, Months), h) {}
    };

    //! 1-year %Euribor index
    class Euribor1Y : public MonthlyTenorEuribor {
      public:
        Euribor1Y(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor(Period(1, Years), h) {}
    };


    class WeeklyTenorEuribor365 : public Euribor365 {
      public:
        WeeklyTenorEuribor365(const Period& tenor,
                const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor365(tenor, h, Following, false) {}
    };

    class MonthlyTenorEuribor365 : public Euribor365 {
      public:
        MonthlyTenorEuribor365(const Period& tenor,
                const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor365(tenor, h, ModifiedFollowing, true) {}
    };

    //! 1-week %Euribor365 index
    class Euribor365_SW : public WeeklyTenorEuribor365 {
      public:
        Euribor365_SW(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : WeeklyTenorEuribor365(Period(1, Weeks), h) {}
    };

    //! 2-weeks %Euribor365 index
    class Euribor365_2W : public WeeklyTenorEuribor365 {
      public:
        Euribor365_2W(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : WeeklyTenorEuribor365(Period(2, Weeks), h) {}
    };

    //! 3-weeks %Euribor365 index
    class Euribor365_3W : public WeeklyTenorEuribor365 {
      public:
        Euribor365_3W(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : WeeklyTenorEuribor365(Period(3, Weeks), h) {}
    };

    //! 1-month %Euribor365 index
    class Euribor365_1M : public MonthlyTenorEuribor365 {
      public:
        Euribor365_1M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor365(Period(1, Months), h) {}
    };

    //! 2-months %Euribor365 index
    class Euribor365_2M : public MonthlyTenorEuribor365 {
      public:
        Euribor365_2M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor365(Period(2, Months), h) {}
    };

    //! 3-months %Euribor365 index
    class Euribor365_3M : public MonthlyTenorEuribor365 {
      public:
        Euribor365_3M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor365(Period(3, Months), h) {}
    };

    //! 4-months %Euribor365 index
    class Euribor365_4M : public MonthlyTenorEuribor365 {
      public:
        Euribor365_4M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor365(Period(4, Months), h) {}
    };

    //! 5-months %Euribor365 index
    class Euribor365_5M : public MonthlyTenorEuribor365 {
      public:
        Euribor365_5M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor365(Period(5, Months), h) {}
    };

    //! 6-months %Euribor365 index
    class Euribor365_6M : public MonthlyTenorEuribor365 {
      public:
        Euribor365_6M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor365(Period(6, Months), h) {}
    };

    //! 7-months %Euribor365 index
    class Euribor365_7M : public MonthlyTenorEuribor365 {
      public:
        Euribor365_7M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor365(Period(7, Months), h) {}
    };

    //! 8-months %Euribor365 index
    class Euribor365_8M : public MonthlyTenorEuribor365 {
      public:
        Euribor365_8M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor365(Period(8, Months), h) {}
    };

    //! 9-months %Euribor365 index
    class Euribor365_9M : public MonthlyTenorEuribor365 {
      public:
        Euribor365_9M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor365(Period(9, Months), h) {}
    };

    //! 10-months %Euribor365 index
    class Euribor365_10M : public MonthlyTenorEuribor365 {
      public:
        Euribor365_10M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor365(Period(10, Months), h) {}
    };

    //! 11-months %Euribor365 index
    class Euribor365_11M : public MonthlyTenorEuribor365 {
      public:
        Euribor365_11M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor365(Period(11, Months), h) {}
    };

    //! 1-year %Euribor365 index
    class Euribor365_1Y : public MonthlyTenorEuribor365 {
      public:
        Euribor365_1Y(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEuribor365(Period(1, Years), h) {}
    };

}


#endif
