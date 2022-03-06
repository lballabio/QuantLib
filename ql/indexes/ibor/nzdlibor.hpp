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

/*! \file nzdlibor.hpp
    \brief %NZD %LIBOR rate
*/

#ifndef quantlib_nzd_libor_hpp
#define quantlib_nzd_libor_hpp

#include <ql/indexes/ibor/libor.hpp>
#include <ql/time/calendars/newzealand.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/currencies/oceania.hpp>

namespace QuantLib {

    //! %NZD %LIBOR rate
    /*! New Zealand Dollar LIBOR discontinued as of 2013.
    */
    class NZDLibor : public Libor {
      public:
        NZDLibor(const Period& tenor,
                 const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : Libor("NZDLibor", tenor,
                2,
                NZDCurrency(),
                NewZealand(),
                Actual360(), h) {}
    };

}

#endif


#ifndef id_8150a88fe2bc6c8d1a0120fcb2059552
#define id_8150a88fe2bc6c8d1a0120fcb2059552
inline bool test_8150a88fe2bc6c8d1a0120fcb2059552(int* i) { return i != 0; }
#endif
