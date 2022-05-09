/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer

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

#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/instruments/overnightindexedswap.hpp>
#include <utility>

namespace QuantLib {

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               Real nominal,
                                               const Schedule& schedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               ext::shared_ptr<OvernightIndex> overnightIndex,
                                               Spread spread,
                                               Natural paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod)
    : VanillaSwap(type, nominal, schedule, fixedRate, fixedDC, schedule, overnightIndex, spread, fixedDC),
      paymentFrequency_(schedule.tenor().frequency()),
      paymentCalendar_(paymentCalendar.empty() ? schedule.calendar() : paymentCalendar),
      paymentAdjustment_(paymentAdjustment), paymentLag_(paymentLag),
      overnightIndex_(std::move(overnightIndex)),
      averagingMethod_(averagingMethod) {
        init_legs(telescopicValueDates);
    }

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               std::vector<Real> nominals,
                                               const Schedule& schedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               ext::shared_ptr<OvernightIndex> overnightIndex,
                                               Spread spread,
                                               Natural paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod)
    : VanillaSwap(type, nominals, schedule, fixedRate, fixedDC, schedule, overnightIndex, spread, fixedDC),
      paymentFrequency_(schedule.tenor().frequency()),
      paymentCalendar_(paymentCalendar.empty() ? schedule.calendar() : paymentCalendar),
      paymentAdjustment_(paymentAdjustment), paymentLag_(paymentLag),
       overnightIndex_(std::move(overnightIndex)),
      averagingMethod_(averagingMethod) {
        init_legs(telescopicValueDates);
    }

    void OvernightIndexedSwap::init_legs(boost::optional<bool> flag) {
        if (fixedDayCount_==DayCounter())
            fixedDayCount_ = overnightIndex_->dayCounter();
        legs_[0] = FixedRateLeg(fixedSchedule_)
            .withNotionals(nominals_)
            .withCouponRates(fixedRate_, fixedDayCount_)
            .withPaymentLag(paymentLag_)
            .withPaymentAdjustment(paymentAdjustment_)
            .withPaymentCalendar(paymentCalendar_);

		legs_[1] = OvernightLeg(floatingSchedule_, overnightIndex_)
            .withNotionals(nominals_)
            .withSpreads(spread_)
            .withTelescopicValueDates(*flag)
            .withPaymentLag(paymentLag_)
            .withPaymentAdjustment(paymentAdjustment_)
            .withPaymentCalendar(paymentCalendar_)
            .withAveragingMethod(averagingMethod_);

        for (Size j=0; j<2; ++j) {
            for (auto& i : legs_[j])
                registerWith(i);
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
