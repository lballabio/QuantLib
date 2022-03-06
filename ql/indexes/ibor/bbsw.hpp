/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2016 Fabrice Lecuyer

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

/*! \file bbsw.hpp
    \brief %Bbsw index
*/

#ifndef quantlib_bbsw_hpp
#define quantlib_bbsw_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/australia.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/currencies/oceania.hpp>

namespace QuantLib {

    //! %Bbsw index
    /*! Bbsw rate fixed by AFMA.

        See <http://www.afma.com.au/data/BBSW>.
    */
    class Bbsw : public IborIndex {
      public:
        Bbsw(const Period& tenor,
             const Handle<YieldTermStructure>& h =
                             Handle<YieldTermStructure>())
        : IborIndex("Bbsw", tenor,
                    0, // settlement days
                    AUDCurrency(), Australia(),
                    HalfMonthModifiedFollowing, true,
                    Actual365Fixed(), h) {
            QL_REQUIRE(this->tenor().units() != Days,
                       "for daily tenors (" << this->tenor() <<
                       ") dedicated DailyTenor constructor must be used");
        }
    };

    //! 1-month %Bbsw index
    class Bbsw1M : public Bbsw {
      public:
        explicit Bbsw1M(const Handle<YieldTermStructure>& h =
                               Handle<YieldTermStructure>())
        : Bbsw(Period(1, Months), h) {}
    };

    //! 2-months %Bbsw index
    class Bbsw2M : public Bbsw {
      public:
        explicit Bbsw2M(const Handle<YieldTermStructure>& h =
                               Handle<YieldTermStructure>())
        : Bbsw(Period(2, Months), h) {}
    };

    //! 3-months %Bbsw index
    class Bbsw3M : public Bbsw {
      public:
        explicit Bbsw3M(const Handle<YieldTermStructure>& h =
                               Handle<YieldTermStructure>())
        : Bbsw(Period(3, Months), h) {}
    };

    //! 4-months %Bbsw index
    class Bbsw4M : public Bbsw {
      public:
        explicit Bbsw4M(const Handle<YieldTermStructure>& h =
                               Handle<YieldTermStructure>())
        : Bbsw(Period(4, Months), h) {}
    };

    //! 5-months %Bbsw index
    class Bbsw5M : public Bbsw {
      public:
        explicit Bbsw5M(const Handle<YieldTermStructure>& h =
                               Handle<YieldTermStructure>())
        : Bbsw(Period(5, Months), h) {}
    };

    //! 6-months %Bbsw index
    class Bbsw6M : public Bbsw {
      public:
        explicit Bbsw6M(const Handle<YieldTermStructure>& h =
                               Handle<YieldTermStructure>())
        : Bbsw(Period(6, Months), h) {}
    };

}

#endif


#ifndef id_0cc1002404269199fb8464222e515754
#define id_0cc1002404269199fb8464222e515754
inline bool test_0cc1002404269199fb8464222e515754(int* i) { return i != 0; }
#endif
