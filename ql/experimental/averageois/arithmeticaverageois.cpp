/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Stefano Fondi

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

#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/cashflows/overnightindexedcouponpricer.hpp>
#include <ql/experimental/averageois/arithmeticaverageois.hpp>
#include <utility>

namespace QuantLib {

    QL_DEPRECATED_DISABLE_WARNING

    ArithmeticAverageOIS::ArithmeticAverageOIS(Type type,
                                               Real nominal,
                                               Schedule fixedLegSchedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               ext::shared_ptr<OvernightIndex> overnightIndex,
                                               Schedule overnightLegSchedule,
                                               Spread spread,
                                               Real meanReversionSpeed,
                                               Real volatility,
                                               bool byApprox)
    : Swap(2), type_(type), nominals_(std::vector<Real>(1, nominal)),
      fixedLegPaymentFrequency_(fixedLegSchedule.tenor().frequency()),
      overnightLegPaymentFrequency_(overnightLegSchedule.tenor().frequency()),
      fixedRate_(fixedRate), fixedDC_(std::move(fixedDC)),
      overnightIndex_(std::move(overnightIndex)), spread_(spread), byApprox_(byApprox),
      mrs_(meanReversionSpeed), vol_(volatility) {

        initialize(std::move(fixedLegSchedule), std::move(overnightLegSchedule));
    }

    ArithmeticAverageOIS::ArithmeticAverageOIS(Type type,
                                               std::vector<Real> nominals,
                                               Schedule fixedLegSchedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               ext::shared_ptr<OvernightIndex> overnightIndex,
                                               Schedule overnightLegSchedule,
                                               Spread spread,
                                               Real meanReversionSpeed,
                                               Real volatility,
                                               bool byApprox)
    : Swap(2), type_(type), nominals_(std::move(nominals)),
      fixedLegPaymentFrequency_(fixedLegSchedule.tenor().frequency()),
      overnightLegPaymentFrequency_(overnightLegSchedule.tenor().frequency()),
      fixedRate_(fixedRate), fixedDC_(std::move(fixedDC)),
      overnightIndex_(std::move(overnightIndex)), spread_(spread), byApprox_(byApprox),
      mrs_(meanReversionSpeed), vol_(volatility) {

        initialize(std::move(fixedLegSchedule), std::move(overnightLegSchedule));
    }

    void ArithmeticAverageOIS::initialize(Schedule fixedLegSchedule,
                                          Schedule overnightLegSchedule) {
        if (fixedDC_==DayCounter())
            fixedDC_ = overnightIndex_->dayCounter();
        legs_[0] = FixedRateLeg(std::move(fixedLegSchedule))
            .withNotionals(nominals_)
            .withCouponRates(fixedRate_, fixedDC_);

        legs_[1] = OvernightLeg(std::move(overnightLegSchedule), overnightIndex_)
            .withNotionals(nominals_)
            .withSpreads(spread_);

        ext::shared_ptr<FloatingRateCouponPricer> arithmeticPricer(
                new ArithmeticAveragedOvernightIndexedCouponPricer(mrs_, vol_, byApprox_));

        for (auto& i : legs_[1]) {
            ext::shared_ptr<OvernightIndexedCoupon> c =
                ext::dynamic_pointer_cast<OvernightIndexedCoupon>(i);
            c->setPricer(arithmeticPricer);
        }

        for (Size j=0; j<2; ++j) {
            for (auto& i : legs_[j])
                registerWith(i);
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

    Real ArithmeticAverageOIS::fairRate() const {
        static Spread basisPoint = 1.0e-4;
        calculate();
        return fixedRate_ - NPV_/(fixedLegBPS()/basisPoint);
    }

    Spread ArithmeticAverageOIS::fairSpread() const {
        static Spread basisPoint = 1.0e-4;
        calculate();
        return spread_ - NPV_/(overnightLegBPS()/basisPoint);
    }

    Real ArithmeticAverageOIS::fixedLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[0] != Null<Real>(), "result not available");
        return legBPS_[0];
    }

    Real ArithmeticAverageOIS::overnightLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[1] != Null<Real>(), "result not available");
        return legBPS_[1];
    }

    Real ArithmeticAverageOIS::fixedLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[0] != Null<Real>(), "result not available");
        return legNPV_[0];
    }

    Real ArithmeticAverageOIS::overnightLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[1] != Null<Real>(), "result not available");
        return legNPV_[1];
    }

    QL_DEPRECATED_ENABLE_WARNING

}
