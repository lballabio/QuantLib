/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Roland Lichters
 Copyright (C) 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/instruments/bmaswap.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/averagebmacoupon.hpp>

namespace QuantLib {

    BMASwap::BMASwap(Type type,
                     Real nominal,
                     // Libor leg
                     Schedule liborSchedule,
                     Real liborFraction,
                     Spread liborSpread,
                     const ext::shared_ptr<IborIndex>& liborIndex,
                     const DayCounter& liborDayCount,
                     // BMA leg
                     Schedule bmaSchedule,
                     const ext::shared_ptr<BMAIndex>& bmaIndex,
                     const DayCounter& bmaDayCount)
    : Swap(2), type_(type), nominal_(nominal),
      liborFraction_(liborFraction), liborSpread_(liborSpread)  {

        BusinessDayConvention convention =
            liborSchedule.businessDayConvention();

        legs_[0] = IborLeg(std::move(liborSchedule), liborIndex)
            .withNotionals(nominal)
            .withPaymentDayCounter(liborDayCount)
            .withPaymentAdjustment(convention)
            .withFixingDays(liborIndex->fixingDays())
            .withGearings(liborFraction)
            .withSpreads(liborSpread);

        auto bmaConvention = bmaSchedule.businessDayConvention();

        legs_[1] = AverageBMALeg(std::move(bmaSchedule), bmaIndex)
            .withNotionals(nominal)
            .withPaymentDayCounter(bmaDayCount)
            .withPaymentAdjustment(bmaConvention);

        for (Size j=0; j<2; ++j) {
            for (auto& i : legs_[j])
                registerWith(i);
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
            QL_FAIL("Unknown BMA-swap type");
        }
    }

    Real BMASwap::liborFraction() const {
        return liborFraction_;
    }

    Spread BMASwap::liborSpread() const {
        return liborSpread_;
    }

    Real BMASwap::nominal() const {
        return nominal_;
    }

    Swap::Type BMASwap::type() const {
        return type_;
    }

    const Leg& BMASwap::liborLeg() const {
        return legs_[0];
    }

    const Leg& BMASwap::bmaLeg() const {
        return legs_[1];
    }


    Real BMASwap::liborLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[0] != Null<Real>(), "result not available");
        return legBPS_[0];
    }

    Real BMASwap::liborLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[0] != Null<Real>(), "result not available");
        return legNPV_[0];
    }

    Real BMASwap::fairLiborFraction() const {
        static Spread basisPoint = 1.0e-4;

        Real spreadNPV = (liborSpread_/basisPoint)*liborLegBPS();
        Real pureLiborNPV = liborLegNPV() - spreadNPV;
        QL_REQUIRE(pureLiborNPV != 0.0,
                   "result not available (null libor NPV)");
        return -liborFraction_ * (bmaLegNPV() + spreadNPV) / pureLiborNPV;
    }

    Spread BMASwap::fairLiborSpread() const {
        static Spread basisPoint = 1.0e-4;

        return liborSpread_ - NPV()/(liborLegBPS()/basisPoint);
    }

    Real BMASwap::bmaLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[1] != Null<Real>(), "result not available");
        return legBPS_[1];
    }

    Real BMASwap::bmaLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[1] != Null<Real>(), "result not available");
        return legNPV_[1];
    }

}
