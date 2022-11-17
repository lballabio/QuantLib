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

/*! \file thbfix.hpp
    \brief %THB %THBFIX rate
*/

#ifndef quantlib_thbfix_hpp
#define quantlib_thbfix_hpp

#include <ql/indexes/ibor/libor.hpp>
#include <ql/time/calendars/thailand.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/currencies/asia.hpp>

namespace QuantLib {

    //! %THB %THBFIX rate
    /*! THBFIX

    THB interest rate implied by USD/THB foreign exchange swaps

    The Swap Offer Rate represents the cost of
    borrowing a currency synthetically by borrowing USD
    for the same tenor and using the foreign exchange swap
    offer rate on the offer side to swap out the USD in return
    for the foreign currency.

    Fixing is based on average FX Forward rates from 21 banks
    and the USD Libor Fixing.

    Fixing is published at 11:00 am BKK time
    */
    class THBFIX : public IborIndex {
      public:
        THBFIX(const Period& tenor,
               const Handle<YieldTermStructure>& h = {})
        : IborIndex("THBFIX", tenor,
                    2,
                    THBCurrency(),
                    Thailand(),
                    ModifiedFollowing, true,
                    Actual365Fixed(), h) {}
    };
}

#endif
