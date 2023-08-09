/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Peter Caspers

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

/*! \file simplifynotificationgraph.hpp
    \brief utility functions to reduce number of notifications sent by observables
*/

#ifndef quantlib_simplify_notification_graph
#define quantlib_simplify_notification_graph

#include <ql/cashflow.hpp>
#include <ql/instrument.hpp>
#include <ql/instruments/bond.hpp>
#include <ql/instruments/swap.hpp>


namespace QuantLib {

    //! Utility function to optimize the observability graph of an instrument
    /*! This function unregisters the given instrument from the given cashflows and
        instead registers with the observables of the cashflows. This is safe to do if

        - the coupon pricers of the cashflows are set before the function is called and never
          updated afterwards
        - the cashflows are not themselves originating notifications, i.e. they only pass through
          notifications from their observables (which is usually the case)
        - the set of cashflows does not dynamically change (usually satisfied as well)

        If unregisterCoupons is set to true, all given cashflows are in addition unregistered from
        all their observables. This can be done

        - if the coupons are not asked for results directly
        - if deepUpdate() is called on the instrument before retrieving a result; to determine
          whether the result might have changed, isCalculated() can be called on the instrument.

       There are overloads of this function for specific instrument types like Swap, Bond.
    */
    void simplifyNotificationGraph(Instrument& instrument,
                                   const Leg& leg,
                                   const bool unregisterCoupons = false);

    //! Utility function to opimize the observability graph of a swap
    void simplifyNotificationGraph(Swap& swap, const bool unregisterCoupons = false);

    //! Utility function to opimize the observability graph of a bond
    void simplifyNotificationGraph(Bond& bond, const bool unregisterCoupons = false);

}

#endif
