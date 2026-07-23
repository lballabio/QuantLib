/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer
 Copyright (C) 2026 Sergio Araujo

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
#include <ql/instruments/overnightindexedswap.hpp>
#include <utility>

namespace QuantLib {

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               Real nominal,
                                               const Schedule& schedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                               Spread spread,
                                               Integer paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod,
                                               Natural lookbackDays,
                                               Natural lockoutDays,
                                               bool applyObservationShift,
                                               const std::optional<Integer>& roundingPrecision)
    : OvernightIndexedSwap(type,
                           std::vector<Real>(1, nominal),
                           schedule,
                           fixedRate,
                           std::move(fixedDC),
                           overnightIndex,
                           spread,
                           paymentLag,
                           paymentAdjustment,
                           paymentCalendar,
                           telescopicValueDates,
                           averagingMethod,
                           lookbackDays,
                           lockoutDays,
                           applyObservationShift,
                           roundingPrecision) {}

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               const std::vector<Real>& nominals,
                                               const Schedule& schedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                               Spread spread,
                                               Integer paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod,
                                               Natural lookbackDays,
                                               Natural lockoutDays,
                                               bool applyObservationShift,
                                               const std::optional<Integer>& roundingPrecision)
    : OvernightIndexedSwap(type,
                           nominals,
                           schedule,
                           fixedRate,
                           std::move(fixedDC),
                           nominals,
                           schedule,
                           overnightIndex,
                           spread,
                           paymentLag,
                           paymentAdjustment,
                           paymentCalendar,
                           telescopicValueDates,
                           averagingMethod,
                           lookbackDays,
                           lockoutDays,
                           applyObservationShift,
                           roundingPrecision) {}

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               Real nominal,
                                               Schedule fixedSchedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               Schedule overnightSchedule,
                                               const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                               Spread spread,
                                               Integer paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod,
                                               Natural lookbackDays,
                                               Natural lockoutDays,
                                               bool applyObservationShift,
                                               const std::optional<Integer>& roundingPrecision)
    : OvernightIndexedSwap(type,
                           std::vector<Real>(1, nominal),
                           std::move(fixedSchedule),
                           fixedRate,
                           std::move(fixedDC),
                           std::vector<Real>(1, nominal),
                           std::move(overnightSchedule),
                           overnightIndex,
                           spread,
                           paymentLag,
                           paymentAdjustment,
                           paymentCalendar,
                           telescopicValueDates,
                           averagingMethod,
                           lookbackDays,
                           lockoutDays,
                           applyObservationShift,
                           roundingPrecision) {}

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               std::vector<Real> fixedNominals,
                                               Schedule fixedSchedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               const std::vector<Real>& overnightNominals,
                                               Schedule overnightSchedule,
                                               const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                               Spread spread,
                                               Integer paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod,
                                               Natural lookbackDays,
                                               Natural lockoutDays,
                                               bool applyObservationShift,
                                               const std::optional<Integer>& roundingPrecision)
    : FixedVsFloatingSwap(type, std::move(fixedNominals), std::move(fixedSchedule), fixedRate, std::move(fixedDC),
                          overnightNominals, std::move(overnightSchedule), overnightIndex,
                          spread, DayCounter(), paymentAdjustment, paymentLag, paymentCalendar),
                          overnightIndex_(overnightIndex),
                          paymentLag_(paymentLag), paymentCalendar_(paymentCalendar),
                          telescopicValueDates_(telescopicValueDates),
                          averagingMethod_(averagingMethod),
                          lookbackDays_(lookbackDays), lockoutDays_(lockoutDays),
                          applyObservationShift_(applyObservationShift),
                          roundingPrecision_(roundingPrecision) {
        OvernightLeg leg(floatingSchedule(), overnightIndex_);
        leg
                .withNotionals(overnightNominals)
                .withSpreads(spread)
                .withTelescopicValueDates(telescopicValueDates)
                .withPaymentLag(paymentLag)
                .withPaymentAdjustment(paymentAdjustment)
                .withPaymentCalendar(paymentCalendar.empty() ?
                                     floatingSchedule().calendar() :
                                     paymentCalendar)
                .withAveragingMethod(averagingMethod_)
                .withLookbackDays(lookbackDays_)
                .withLockoutDays(lockoutDays_)
                .withObservationShift(applyObservationShift_);
        if (roundingPrecision_.has_value()) {
            QL_REQUIRE(*roundingPrecision_ >= 0 && *roundingPrecision_ <= 16,
                       "rounding precision (" << *roundingPrecision_ <<
                       ") must be between 0 and 16");
            leg.withRoundingPrecision(*roundingPrecision_);
        }
        legs_[1] = leg;
        for (const auto& c : legs_[1])
            registerWith(c);
    }

    void OvernightIndexedSwap::setupFloatingArguments(arguments* args) const {
        const Leg& floatingCoupons = floatingLeg();
        Size n = floatingCoupons.size();

        args->floatingResetDates = args->floatingPayDates = args->floatingFixingDates = std::vector<Date>(n);
        args->floatingAccrualTimes = std::vector<Time>(n);
        args->floatingSpreads = std::vector<Spread>(n);
        args->floatingCoupons = args->floatingNominals = std::vector<Real>(n);

        for (Size i=0; i<n; ++i) {
            auto coupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(floatingCoupons[i]);

            args->floatingResetDates[i] = coupon->accrualStartDate();
            args->floatingPayDates[i] = coupon->date();
            args->floatingNominals[i] = coupon->nominal();

            args->floatingFixingDates[i] = coupon->fixingDate();
            args->floatingAccrualTimes[i] = coupon->accrualPeriod();
            args->floatingSpreads[i] = coupon->spread();
            try {
                args->floatingCoupons[i] = coupon->amount();
            } catch (Error&) {
                args->floatingCoupons[i] = Null<Real>();
            }
        }
    }

}
