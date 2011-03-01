/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano

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

#include <ql/instruments/overnightindexedswap.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>

namespace QuantLib {

    OvernightIndexedSwap::OvernightIndexedSwap(
                    Type type,
                    Real nominal,
                    const Schedule& schedule,
                    Rate fixedRate,
                    const DayCounter& fixedDC,
                    const boost::shared_ptr<OvernightIndex>& overnightIndex,
                    Spread spread)
    : Swap(2), type_(type),
      nominals_(std::vector<Real>(1, nominal)),
      paymentFrequency_(schedule.tenor().frequency()),
      fixedRate_(fixedRate), fixedDC_(fixedDC),
      overnightIndex_(overnightIndex), spread_(spread) {

          initialize(schedule);

    }

    OvernightIndexedSwap::OvernightIndexedSwap(
                    Type type,
                    std::vector<Real> nominals,
                    const Schedule& schedule,
                    Rate fixedRate,
                    const DayCounter& fixedDC,
                    const boost::shared_ptr<OvernightIndex>& overnightIndex,
                    Spread spread)
    : Swap(2), type_(type), nominals_(nominals),
      paymentFrequency_(schedule.tenor().frequency()),
      fixedRate_(fixedRate), fixedDC_(fixedDC),
      overnightIndex_(overnightIndex), spread_(spread) {

          initialize(schedule);

    }

    void OvernightIndexedSwap::initialize(const Schedule& schedule) {
        if (fixedDC_==DayCounter())
            fixedDC_ = overnightIndex_->dayCounter();
        legs_[0] = FixedRateLeg(schedule)
            .withNotionals(nominals_)
            .withCouponRates(fixedRate_, fixedDC_);

        legs_[1] = OvernightLeg(schedule, overnightIndex_)
            .withNotionals(nominals_)
            .withSpreads(spread_);

        for (Size j=0; j<2; ++j) {
            for (Leg::iterator i = legs_[j].begin(); i!= legs_[j].end(); ++i)
                registerWith(*i);
        }

        switch (type_) {
          case Payer:
            payer_[0] = -1.0;
            payer_[1] = +1.0;
            break;
          case Receiver:
            payer_[0] = +1.0;
            payer_[1] = -1.0;
            break;
          default:
            QL_FAIL("Unknown overnight-swap type");
        }
    }

    Real OvernightIndexedSwap::fairRate() const {
        static Spread basisPoint = 1.0e-4;
        calculate();
        return fixedRate_ - NPV_/(fixedLegBPS()/basisPoint);
    }

    Spread OvernightIndexedSwap::fairSpread() const {
        static Spread basisPoint = 1.0e-4;
        calculate();
        return spread_ - NPV_/(overnightLegBPS()/basisPoint);
    }

    Real OvernightIndexedSwap::fixedLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[0] != Null<Real>(), "result not available");
        return legBPS_[0];
    }

    Real OvernightIndexedSwap::overnightLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[1] != Null<Real>(), "result not available");
        return legBPS_[1];
    }

    Real OvernightIndexedSwap::fixedLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[0] != Null<Real>(), "result not available");
        return legNPV_[0];
    }

    Real OvernightIndexedSwap::overnightLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[1] != Null<Real>(), "result not available");
        return legNPV_[1];
    }

}
