/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
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

/*! \file eurlibor.hpp
    \brief %EUR %LIBOR rate
*/

#ifndef quantlib_eur_libor_hpp
#define quantlib_eur_libor_hpp

#include <ql/indexes/libor.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

    //! %EUR %LIBOR rate
    /*! Euro LIBOR fixed by BBA.

        See <http://www.bba.org.uk/bba/jsp/polopoly.jsp?d=225&a=1414>.

        \warning This is the rate fixed in London by BBA. Use Euribor if
                 you're interested in the fixing by the ECB.
    */
    class EURLibor : public Libor {
      public:
        EURLibor(const Period& tenor,
                 const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>(),
                 BusinessDayConvention convention = ModifiedFollowing,
                 bool endOfMonth = true,
                 Natural settlementDays = 2)
        : Libor("EURLibor", tenor, settlementDays, EURCurrency(),
                TARGET(), TARGET(),
                convention, endOfMonth, Actual360(), h) {}
    };
    class WeeklyTenorEURLibor : public EURLibor {
      public:
        WeeklyTenorEURLibor(const Period& tenor,
                const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : EURLibor(tenor, h, Following, false) {}
    };

    class MonthlyTenorEURLibor : public EURLibor {
      public:
        MonthlyTenorEURLibor(const Period& tenor,
                const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : EURLibor(tenor, h, ModifiedFollowing, true) {}
    };

    //! 1-week %EURLibor index
    class EURLiborSW : public WeeklyTenorEURLibor {
      public:
        EURLiborSW(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : WeeklyTenorEURLibor(Period(1, Weeks), h) {}
    };

    //! 2-weeks %Euribor index
    class EURLibor2W : public WeeklyTenorEURLibor {
      public:
        EURLibor2W(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : WeeklyTenorEURLibor(Period(2, Weeks), h) {}
    };

   
    //! 1-month %EURLibor index
    class EURLibor1M : public MonthlyTenorEURLibor {
      public:
        EURLibor1M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEURLibor(Period(1, Months), h) {}
    };

    //! 2-months %EURLibor index
    class EURLibor2M : public MonthlyTenorEURLibor {
      public:
        EURLibor2M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEURLibor(Period(2, Months), h) {}
    };

    //! 3-months %EURLibor index
    class EURLibor3M : public MonthlyTenorEURLibor {
      public:
        EURLibor3M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEURLibor(Period(3, Months), h) {}
    };

    //! 4-months %EURLibor index
    class EURLibor4M : public MonthlyTenorEURLibor {
      public:
        EURLibor4M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEURLibor(Period(4, Months), h) {}
    };

    //! 5-months %EURLibor index
    class EURLibor5M : public MonthlyTenorEURLibor {
      public:
        EURLibor5M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEURLibor(Period(5, Months), h) {}
    };

    //! 6-months %EURLibor index
    class EURLibor6M : public MonthlyTenorEURLibor {
      public:
        EURLibor6M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEURLibor(Period(6, Months), h) {}
    };

    //! 7-months %EURLibor index
    class EURLibor7M : public MonthlyTenorEURLibor{
      public:
        EURLibor7M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEURLibor(Period(7, Months), h) {}
    };

    //! 8-months %EURLibor index
    class EURLibor8M : public MonthlyTenorEURLibor {
      public:
        EURLibor8M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEURLibor(Period(8, Months), h) {}
    };

    //! 9-months %EURLibor index
    class EURLibor9M : public MonthlyTenorEURLibor {
      public:
        EURLibor9M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEURLibor(Period(9, Months), h) {}
    };

    //! 10-months %EURLibor index
    class EURLibor10M : public MonthlyTenorEURLibor {
      public:
        EURLibor10M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEURLibor(Period(10, Months), h) {}
    };

    //! 11-months %EURLibor index
    class EURLibor11M : public MonthlyTenorEURLibor {
      public:
        EURLibor11M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEURLibor(Period(11, Months), h) {}
    };

    //! 1-year %EURLibor index
    class EURLibor1Y : public MonthlyTenorEURLibor {
      public:
        EURLibor1Y(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEURLibor(Period(1, Years), h) {}
    };


}


#endif
