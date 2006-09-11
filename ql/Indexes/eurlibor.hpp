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

#include <ql/Indexes/libor.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/Currencies/europe.hpp>

namespace QuantLib {

    //! %EUR %LIBOR rate
    /*! Euro LIBOR fixed by BBA.

        See <http://www.bba.org.uk/bba/jsp/polopoly.jsp?d=225&a=1414>.

        \warning This is the rate fixed in London by BBA. Use Euribor if
                 you're interested in the fixing by the ECB.
    */
    class Eurlibor : public Libor {
      public:
        Eurlibor(const Period& tenor,
                 const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>(),
                 BusinessDayConvention convention = MonthEndReference,
                 Integer settlementDays = 2)
        : Libor("Eurlibor", tenor, settlementDays, EURCurrency(),
                TARGET(), TARGET(),
                convention, Actual360(), h) {}
    };
    class WeeklyTenorEurlibor : public Eurlibor {
      public:
        WeeklyTenorEurlibor(const Period& tenor,
                const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Eurlibor(tenor, h, Following) {}
    };

    class MonthlyTenorEurlibor : public Eurlibor {
      public:
        MonthlyTenorEurlibor(const Period& tenor,
                const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Eurlibor(tenor, h, MonthEndReference) {}
    };

    //! 1-week %Eurlibor index
    class EurliborSW : public WeeklyTenorEurlibor {
      public:
        EurliborSW(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : WeeklyTenorEurlibor(Period(1, Weeks), h) {}
    };

    //! 2-weeks %Euribor index
    class Eurlibor2W : public WeeklyTenorEurlibor {
      public:
        Eurlibor2W(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : WeeklyTenorEurlibor(Period(2, Weeks), h) {}
    };

   
    //! 1-month %Eurlibor index
    class Eurlibor1M : public MonthlyTenorEurlibor {
      public:
        Eurlibor1M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEurlibor(Period(1, Months), h) {}
    };

    //! 2-months %Eurlibor index
    class Eurlibor2M : public MonthlyTenorEurlibor {
      public:
        Eurlibor2M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEurlibor(Period(2, Months), h) {}
    };

    //! 3-months %Eurlibor index
    class Eurlibor3M : public MonthlyTenorEurlibor {
      public:
        Eurlibor3M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEurlibor(Period(3, Months), h) {}
    };

    //! 4-months %Eurlibor index
    class Eurlibor4M : public MonthlyTenorEurlibor {
      public:
        Eurlibor4M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEurlibor(Period(4, Months), h) {}
    };

    //! 5-months %Eurlibor index
    class Eurlibor5M : public MonthlyTenorEurlibor {
      public:
        Eurlibor5M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEurlibor(Period(5, Months), h) {}
    };

    //! 6-months %Eurlibor index
    class Eurlibor6M : public MonthlyTenorEurlibor {
      public:
        Eurlibor6M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEurlibor(Period(6, Months), h) {}
    };

    //! 7-months %Eurlibor index
    class Eurlibor7M : public MonthlyTenorEurlibor{
      public:
        Eurlibor7M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEurlibor(Period(7, Months), h) {}
    };

    //! 8-months %Eurlibor index
    class Eurlibor8M : public MonthlyTenorEurlibor {
      public:
        Eurlibor8M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEurlibor(Period(8, Months), h) {}
    };

    //! 9-months %Eurlibor index
    class Eurlibor9M : public MonthlyTenorEurlibor {
      public:
        Eurlibor9M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEurlibor(Period(9, Months), h) {}
    };

    //! 10-months %Eurlibor index
    class Eurlibor10M : public MonthlyTenorEurlibor {
      public:
        Eurlibor10M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEurlibor(Period(10, Months), h) {}
    };

    //! 11-months %Eurlibor index
    class Eurlibor11M : public MonthlyTenorEurlibor {
      public:
        Eurlibor11M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEurlibor(Period(11, Months), h) {}
    };

    //! 1-year %Eurlibor index
    class Eurlibor1Y : public MonthlyTenorEurlibor {
      public:
        Eurlibor1Y(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : MonthlyTenorEurlibor(Period(1, Years), h) {}
    };


}


#endif
