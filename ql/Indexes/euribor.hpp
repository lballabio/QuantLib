/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/Indexes/xibor.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/DayCounters/actual365fixed.hpp>
#include <ql/Currencies/europe.hpp>

namespace QuantLib {

    //! %Euribor index
    /*! Euribor rate fixed by the ECB.

        \warning This is the rate fixed by the ECB. Use EURLibor
                 if you're interested in the London fixing by BBA.
    */
    class Euribor : public Xibor {
      public:
        #ifndef QL_DISABLE_DEPRECATED
        /*! \deprecated use the Period-based constructor */
        Euribor(Integer n, TimeUnit units,
                const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>(),
                const DayCounter& dc = Actual360())
        : Xibor("Euribor", n, units, 2, EURCurrency(),
                TARGET(),
                ModifiedFollowing, dc, h) {}
        #endif
        Euribor(const Period& tenor,
                const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Xibor("Euribor", tenor, 2, EURCurrency(),
                TARGET(),
                ModifiedFollowing, Actual360(), h) {}
    };

    //! Actual/365 %Euribor index
    /*! Euribor rate adjusted for the mismatch between the actual/360
        convention used for Euribor and the actual/365 convention
        previously used by a few pre-EUR currencies.
    */
    class Euribor365 : public Xibor {
      public:
        Euribor365(const Period& tenor,
                   const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Xibor("Euribor", tenor, 2, EURCurrency(),
                TARGET(),
                ModifiedFollowing, Actual365Fixed(), h) {}
    };


    //! 1-week %Euribor index
    class EuriborSW : public Euribor {
      public:
        EuriborSW(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(1, Weeks), h) {}
    };

    //! 2-weeks %Euribor index
    class Euribor2W : public Euribor {
      public:
        Euribor2W(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(2, Weeks), h) {}
    };

    //! 3-weeks %Euribor index
    class Euribor3W : public Euribor {
      public:
        Euribor3W(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(3, Weeks), h) {}
    };

    //! 1-month %Euribor index
    class Euribor1M : public Euribor {
      public:
        Euribor1M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(1, Months), h) {}
    };

    //! 2-months %Euribor index
    class Euribor2M : public Euribor {
      public:
        Euribor2M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(2, Months), h) {}
    };

    //! 3-months %Euribor index
    class Euribor3M : public Euribor {
      public:
        Euribor3M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(3, Months), h) {}
    };

    //! 4-months %Euribor index
    class Euribor4M : public Euribor {
      public:
        Euribor4M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(4, Months), h) {}
    };

    //! 5-months %Euribor index
    class Euribor5M : public Euribor {
      public:
        Euribor5M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(5, Months), h) {}
    };

    //! 6-months %Euribor index
    class Euribor6M : public Euribor {
      public:
        Euribor6M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(6, Months), h) {}
    };

    //! 7-months %Euribor index
    class Euribor7M : public Euribor {
      public:
        Euribor7M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(7, Months), h) {}
    };

    //! 8-months %Euribor index
    class Euribor8M : public Euribor {
      public:
        Euribor8M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(8, Months), h) {}
    };

    //! 9-months %Euribor index
    class Euribor9M : public Euribor {
      public:
        Euribor9M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(9, Months), h) {}
    };

    //! 10-months %Euribor index
    class Euribor10M : public Euribor {
      public:
        Euribor10M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(10, Months), h) {}
    };

    //! 11-months %Euribor index
    class Euribor11M : public Euribor {
      public:
        Euribor11M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(11, Months), h) {}
    };

    //! 1-year %Euribor index
    class Euribor1Y : public Euribor {
      public:
        Euribor1Y(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(1, Years), h) {}
    };

}


#endif
