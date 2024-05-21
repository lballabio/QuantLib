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

/*! \file audlibor.hpp
    \brief %AUD %LIBOR rate
*/

#ifndef quantlib_aud_libor_hpp
#define quantlib_aud_libor_hpp

#include <ql/indexes/ibor/libor.hpp>
#include <ql/time/calendars/australia.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/currencies/oceania.hpp>

namespace QuantLib {

    //! %AUD %LIBOR rate
    /*! Australian Dollar LIBOR discontinued as of 2013.
    */
    class AUDLibor : public Libor {
      public:
        AUDLibor(const Period& tenor,
                 Handle<YieldTermStructure> h = {})
        : Libor("AUDLibor", tenor,
                2,
                AUDCurrency(),
                Australia(),
                Actual360(), std::move(h)) {}
    };

}

#endif
