/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Matthias Groncki

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

/*! \file bibor.hpp
    \brief %Bangkok Interbank Offered Rate index
*/

#ifndef quantlib_bibor_hpp
#define quantlib_bibor_hpp

#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    //! %Bibor index
    /*! Bangkok Interbank Offered Rate  fixed by the Bank of Thailand BOT.
    */
    class Bibor : public IborIndex {
      public:
        Bibor(const Period& tenor,
                const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };


    //! 1-week %Bibor index
    class BiborSW : public Bibor {
      public:
        explicit BiborSW(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Bibor(Period(1, Weeks), h) {}
    };


    //! 1-month %Euribor index
    class Bibor1M : public Bibor {
      public:
        explicit Bibor1M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Bibor(Period(1, Months), h) {}
    };

    //! 2-months %Euribor index
    class Bibor2M : public Bibor {
      public:
        explicit Bibor2M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Bibor(Period(2, Months), h) {}
    };

    //! 3-months %Bibor index
    class Bibor3M : public Bibor {
      public:
        explicit Bibor3M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Bibor(Period(3, Months), h) {}
    };

    //! 6-months %Bibor index
    class Bibor6M : public Bibor {
      public:
        explicit Bibor6M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Bibor(Period(6, Months), h) {}
    };

    //! 9-months %Bibor index
    class Bibor9M : public Bibor {
      public:
        explicit Bibor9M(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Bibor(Period(9, Months), h) {}
    };

    //! 1-year %Bibor index
    class Bibor1Y : public Bibor {
      public:
        explicit Bibor1Y(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Bibor(Period(1, Years), h) {}
    };

}

#endif


#ifndef id_44ec1988bf30e9334ed89df139a9b599
#define id_44ec1988bf30e9334ed89df139a9b599
inline bool test_44ec1988bf30e9334ed89df139a9b599(int* i) { return i != 0; }
#endif
