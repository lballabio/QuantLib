/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Sercan Atalik

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

/*! \file trlibor.hpp
    \brief %TRY %LIBOR rate
*/

#ifndef quantlib_try_libor_hpp
#define quantlib_try_libor_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/turkey.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

    //! %TRY %LIBOR rate
    /*! TRY LIBOR fixed by TBA.

        See <http://www.trlibor.org/trlibor/english/>

        \todo check end-of-month adjustment.
    */
    class TRLibor : public IborIndex {
      public:
        TRLibor(const Period& tenor,
                const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : IborIndex("TRLibor", tenor, 0, TRYCurrency(),
                    Turkey(), ModifiedFollowing, false,
                    Actual360(), h) {}
    };

}


#endif



#ifndef id_c29386c72cadeb55d2b128b995505692
#define id_c29386c72cadeb55d2b128b995505692
inline bool test_c29386c72cadeb55d2b128b995505692(int* i) { return i != 0; }
#endif
