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
                                               const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                               Spread spread,
                                               Integer paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod)
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
                           averagingMethod) {}

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
                                               RateAveraging::Type averagingMethod)
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
                           averagingMethod) {}

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               Real nominal,
                                               const Schedule& fixedSchedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               const Schedule& overnightSchedule,
                                               const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                               Spread spread,
                                               Integer paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod)
    : OvernightIndexedSwap(type,
                           std::vector<Real>(1, nominal),
                           fixedSchedule,
                           fixedRate,
                           std::move(fixedDC),
                           std::vector<Real>(1, nominal),
                           overnightSchedule,
                           overnightIndex,
                           spread,
                           paymentLag,
                           paymentAdjustment,
                           paymentCalendar,
                           telescopicValueDates,
                           averagingMethod) {}

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               std::vector<Real> fixedNominals,
                                               Schedule fixedSchedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               const std::vector<Real>& overnightNominals,
                                               const Schedule& overnightSchedule,
                                               const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                               Spread spread,
                                               Integer paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod)
    : FixedVsFloatingSwap(type, std::move(fixedNominals), std::move(fixedSchedule), fixedRate, std::move(fixedDC),
                          overnightNominals, overnightSchedule, overnightIndex,
                          spread, DayCounter(), ext::nullopt, paymentLag, paymentCalendar),
      overnightIndex_(overnightIndex), averagingMethod_(averagingMethod) {

        legs_[1] =
            OvernightLeg(overnightSchedule, overnightIndex_)
                .withNotionals(overnightNominals)
                .withSpreads(spread)
                .withTelescopicValueDates(telescopicValueDates)
                .withPaymentLag(paymentLag)
                .withPaymentAdjustment(paymentAdjustment)
                .withPaymentCalendar(paymentCalendar.empty() ?
                                     overnightSchedule.calendar() :
                                     paymentCalendar)
                .withAveragingMethod(averagingMethod_);
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
