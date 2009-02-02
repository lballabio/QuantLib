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

namespace QuantLib {

  EoniaSwap::EoniaSwap(Type type,
                       Real nominal,
                       // Eonia leg
                       const Schedule& eoniaSchedule,
                       Rate eoniaSpread,
                       const boost::shared_ptr<Eonia>& index,
                       // fixed leg
                       const Schedule& fixedSchedule,
                       Rate fixedRate,
                       const DayCounter& fixedDayCount)
      : Swap(2), type_(type), nominal_(nominal),
        eoniaSpread_(eoniaSpread), fixedRate_(fixedRate) {

        BusinessDayConvention convention =
            eoniaSchedule.businessDayConvention();

        legs_[0] = FixedRateLeg(fixedSchedule, fixedDayCount)
            .withNotionals(nominal)
            .withCouponRates(fixedRate)
            .withPaymentAdjustment(convention);

        legs_[1] = EoniaLeg(eoniaSchedule, index)
            .withNotionals(nominal)
            .withPaymentDayCounter(index->dayCounter())
            .withPaymentAdjustment(eoniaSchedule.businessDayConvention());

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
            QL_FAIL("Unknown Eonia-swap type");
        }
    }

    Spread EoniaSwap::eoniaSpread() const {
        return eoniaSpread_;
    }

    Spread EoniaSwap::fixedRate() const {
        return fixedRate_;
    }

    Real EoniaSwap::nominal() const {
        return nominal_;
    }

    EoniaSwap::Type EoniaSwap::type() const {
        return type_;
    }

    const Leg& EoniaSwap::fixedLeg() const {
        return legs_[0];
    }

    const Leg& EoniaSwap::eoniaLeg() const {
        return legs_[1];
    }

    Real EoniaSwap::fixedLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[0] != Null<Real>(), "result not available");
        return legBPS_[0];
    }

    Real EoniaSwap::fixedLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[0] != Null<Real>(), "result not available");
        return legNPV_[0];
    }

    Real EoniaSwap::fairRate() const {
        static Spread basisPoint = 1.0e-4;
        calculate();
        return fixedRate_ - NPV_/(fixedLegBPS()/basisPoint);
    }

    Spread EoniaSwap::fairEoniaSpread() const {
        static Spread basisPoint = 1.0e-4;
        calculate();
        return eoniaSpread_ - NPV_/(eoniaLegBPS()/basisPoint);
    }

    Real EoniaSwap::eoniaLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[1] != Null<Real>(), "result not available");
        return legBPS_[1];
    }

    Real EoniaSwap::eoniaLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[1] != Null<Real>(), "result not available");
        return legNPV_[1];
    }

}
