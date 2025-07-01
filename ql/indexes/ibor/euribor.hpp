/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2006 Chiara Fornarola
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano

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

/*! \file euribor.hpp
    \brief %Euribor index
*/

#ifndef quantlib_euribor_hpp
#define quantlib_euribor_hpp

#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    //! %Euribor index
    /*! Euribor rate fixed by the ECB.

        \warning This is the rate fixed by the ECB. Use EurLibor
                 if you're interested in the London fixing by BBA.
    */
    class Euribor : public IborIndex {
      public:
        Euribor(const Period& tenor,
                const Handle<YieldTermStructure>& h = {});
    };

    //! Actual/365 %Euribor index
    /*! Euribor rate adjusted for the mismatch between the actual/360
        convention used for Euribor and the actual/365 convention
        previously used by a few pre-EUR currencies.
    */
    class Euribor365 : public IborIndex {
      public:
        Euribor365(const Period& tenor,
                   const Handle<YieldTermStructure>& h = {});
    };

    //! 1-week %Euribor index
    class Euribor1W : public Euribor {
      public:
        explicit Euribor1W(const Handle<YieldTermStructure>& h = {})
        : Euribor(Period(1, Weeks), h) {}
    };

    //! 1-month %Euribor index
    class Euribor1M : public Euribor {
      public:
        explicit Euribor1M(const Handle<YieldTermStructure>& h = {})
        : Euribor(Period(1, Months), h) {}
    };

    //! 3-months %Euribor index
    class Euribor3M : public Euribor {
      public:
        explicit Euribor3M(const Handle<YieldTermStructure>& h = {})
        : Euribor(Period(3, Months), h) {}
    };

    //! 6-months %Euribor index
    class Euribor6M : public Euribor {
      public:
        explicit Euribor6M(const Handle<YieldTermStructure>& h = {})
        : Euribor(Period(6, Months), h) {}
    };

    //! 1-year %Euribor index
    class Euribor1Y : public Euribor {
      public:
        explicit Euribor1Y(const Handle<YieldTermStructure>& h = {})
        : Euribor(Period(1, Years), h) {}
    };

}

#endif
