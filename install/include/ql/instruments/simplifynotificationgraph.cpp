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

#include <ql/instruments/simplifynotificationgraph.hpp>

namespace QuantLib {


    void simplifyNotificationGraph(Instrument& instrument, const Leg& leg, bool unregisterCoupons) {
        for (auto const& coupon : leg) {
            instrument.unregisterWith(coupon);
            instrument.registerWithObservables(coupon);
            if (unregisterCoupons) {
                coupon->unregisterWithAll();
            }
        }
    }

    void simplifyNotificationGraph(Swap& swap, const bool unregisterCoupons) {
        for (auto const& leg : swap.legs())
            simplifyNotificationGraph(swap, leg, unregisterCoupons);
    }

    void simplifyNotificationGraph(Bond& bond, const bool unregisterCoupons) {
        simplifyNotificationGraph(bond, bond.cashflows(), unregisterCoupons);
    }


}
