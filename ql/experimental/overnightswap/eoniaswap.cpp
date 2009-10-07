/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters

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

#include <ql/experimental/overnightswap/eoniaswap.hpp>
#include <ql/experimental/overnightswap/eoniacoupon.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>

#include <iostream>

namespace QuantLib {

  OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                       Real nominal,
                       // overnight leg
                       const Schedule& overnightSchedule,
                       Rate overnightSpread,
                       const boost::shared_ptr<OvernightIndex>& index,
                       // fixed leg
                       const Schedule& fixedSchedule,
                       Rate fixedRate,
                       const DayCounter& fixedDayCount)
      : Swap(2), type_(type), nominal_(nominal),
        overnightSpread_(overnightSpread), fixedRate_(fixedRate) {

        BusinessDayConvention convention =
            overnightSchedule.businessDayConvention();

        legs_[0] = FixedRateLeg(fixedSchedule)
            .withNotionals(nominal)
            .withCouponRates(fixedRate, fixedDayCount)
            .withPaymentAdjustment(convention);

        legs_[1] = OvernightLeg(overnightSchedule, index)
            .withNotionals(nominal)
            .withPaymentDayCounter(index->dayCounter())
            .withSpreads(overnightSpread)
            .withPaymentAdjustment(overnightSchedule.businessDayConvention());

        for (Size j=0; j<2; ++j) {
            for (Leg::iterator i = legs_[j].begin(); i!= legs_[j].end(); ++i)
                registerWith(*i);
        }

        switch (type_) {
          case Payer:
            payer_[0] = +1.0;
            payer_[1] = -1.0;
            break;
          case Receiver:
            payer_[0] = -1.0;
            payer_[1] = +1.0;
            break;
          default:
            QL_FAIL("Unknown overnight-swap type");
        }
    }

    Spread OvernightIndexedSwap::overnightSpread() const {
        return overnightSpread_;
    }

    Spread OvernightIndexedSwap::fixedRate() const {
        return fixedRate_;
    }

    Real OvernightIndexedSwap::nominal() const {
        return nominal_;
    }

    OvernightIndexedSwap::Type OvernightIndexedSwap::type() const {
        return type_;
    }

    const Leg& OvernightIndexedSwap::fixedLeg() const {
        return legs_[0];
    }

    const Leg& OvernightIndexedSwap::overnightLeg() const {
        return legs_[1];
    }

    Real OvernightIndexedSwap::fixedLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[0] != Null<Real>(), "result not available");
        return legBPS_[0];
    }

    Real OvernightIndexedSwap::fixedLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[0] != Null<Real>(), "result not available");
        return legNPV_[0];
    }

    Real OvernightIndexedSwap::fairRate() const {
        static Spread basisPoint = 1.0e-4;
        calculate();
        return fixedRate_ - NPV_/(fixedLegBPS()/basisPoint);
    }

    Spread OvernightIndexedSwap::fairSpread() const {
        static Spread basisPoint = 1.0e-4;
        calculate();
        return overnightSpread_ - NPV_/(overnightLegBPS()/basisPoint);
    }

    Real OvernightIndexedSwap::overnightLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[1] != Null<Real>(), "result not available");
        return legBPS_[1];
    }

    Real OvernightIndexedSwap::overnightLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[1] != Null<Real>(), "result not available");
        return legNPV_[1];
    }

}
