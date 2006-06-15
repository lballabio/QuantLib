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
#include <ql/Currencies/europe.hpp>

namespace QuantLib {

    //! %Euribor index
    /*! Euribor rate fixed by the ECB.

        \warning This is the rate fixed by the ECB. Use EURLibor
                 if you're interested in the London fixing by BBA.
    */
    class Euribor : public Xibor {
      public:
        Euribor(Period p,
                const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>(),
                const DayCounter& dc = Actual360())
        : Xibor("Euribor", p, 2, EURCurrency(),
                TARGET(),
                ModifiedFollowing, dc, h) {}
    };

    class EuriborSW : public Euribor {
      public:
        EuriborSW(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(1, Weeks), h, Actual360()) {}
    };

    class Euribor2W : public Euribor {
      public:
        Euribor2W(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(2, Weeks), h, Actual360()) {}
    };

    class Euribor3W : public Euribor {
      public:
        Euribor3W(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(3, Weeks), h, Actual360()) {}
    };

    class Euribor1M : public Euribor {
      public:
        Euribor1M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(1, Months), h, Actual360()) {}
    };

    class Euribor2M : public Euribor {
      public:
        Euribor2M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(2, Months), h, Actual360()) {}
    };

    class Euribor3M : public Euribor {
      public:
        Euribor3M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(3, Months), h, Actual360()) {}
    };

    class Euribor4M : public Euribor {
      public:
        Euribor4M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(4, Months), h, Actual360()) {}
    };

    class Euribor5M : public Euribor {
      public:
        Euribor5M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(5, Months), h, Actual360()) {}
    };

    class Euribor6M : public Euribor {
      public:
        Euribor6M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(6, Months), h, Actual360()) {}
    };

    class Euribor7M : public Euribor {
      public:
        Euribor7M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(7, Months), h, Actual360()) {}
    };

    class Euribor8M : public Euribor {
      public:
        Euribor8M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(8, Months), h, Actual360()) {}
    };

    class Euribor9M : public Euribor {
      public:
        Euribor9M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(9, Months), h, Actual360()) {}
    };

    class Euribor10M : public Euribor {
      public:
        Euribor10M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(10, Months), h, Actual360()) {}
    };

    class Euribor11M : public Euribor {
      public:
        Euribor11M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(11, Months), h, Actual360()) {}
    };

    class Euribor1Y : public Euribor {
      public:
        Euribor1Y(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Euribor(Period(1, Years), h, Actual360()) {}
    };

}


#endif
