/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Zain Mughal

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

#include <ql/cashflows/multipleresetscoupon.hpp>
#include <ql/instruments/multipleresetsswap.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        Schedule buildCouponSchedule(const Schedule& fullResetSchedule, Size resetsPerCoupon) {
            Size n = (fullResetSchedule.size() - 1) / resetsPerCoupon;
            std::vector<Date> dates(n + 1);
            for (Size i = 0; i <= n; ++i)
                dates[i] = fullResetSchedule[i * resetsPerCoupon];
            return Schedule(dates);
        }

    }

    MultipleResetsSwap::MultipleResetsSwap(
        Type type, Real nominal,
        const Schedule& fixedSchedule, Rate fixedRate, DayCounter fixedDayCount,
        Schedule fullResetSchedule,
        const ext::shared_ptr<IborIndex>& iborIndex, Size resetsPerCoupon,
        Spread spread, RateAveraging::Type averagingMethod,
        ext::optional<BusinessDayConvention> paymentConvention,
        Integer paymentLag, const Calendar& paymentCalendar)
    : FixedVsFloatingSwap(type,
          std::vector<Real>(fixedSchedule.size() - 1, nominal),
          fixedSchedule, fixedRate, std::move(fixedDayCount),
          std::vector<Real>((fullResetSchedule.size() - 1) / resetsPerCoupon, nominal),
          buildCouponSchedule(fullResetSchedule, resetsPerCoupon),
          iborIndex, spread, iborIndex->dayCounter(),
          paymentConvention, paymentLag, paymentCalendar),
      fullResetSchedule_(std::move(fullResetSchedule)),
      resetsPerCoupon_(resetsPerCoupon),
      averagingMethod_(averagingMethod) {

        QL_REQUIRE((fullResetSchedule_.size() - 1) % resetsPerCoupon_ == 0,
                   "number of reset periods (" << fullResetSchedule_.size() - 1
                   << ") is not a multiple of resetsPerCoupon ("
                   << resetsPerCoupon_ << ")");

        legs_[1] = MultipleResetsLeg(fullResetSchedule_, this->iborIndex(), resetsPerCoupon_)
            .withNotionals(floatingNominals())
            .withRateSpreads(spread)
            .withAveragingMethod(averagingMethod_)
            .withPaymentAdjustment(this->paymentConvention())
            .withPaymentLag(paymentLag)
            .withPaymentCalendar(paymentCalendar.empty() ?
                                 fullResetSchedule_.calendar() : paymentCalendar);

        for (auto& cf : legs_[1])
            registerWith(cf);
    }

    void MultipleResetsSwap::setupFloatingArguments(arguments* args) const {
        const Leg& leg = floatingLeg();
        Size n = leg.size();
        args->floatingResetDates = args->floatingPayDates = std::vector<Date>(n);
        args->floatingFixingDates = std::vector<Date>(n);
        args->floatingNominals = std::vector<Real>(n);
        args->floatingAccrualTimes = std::vector<Time>(n);
        args->floatingSpreads = std::vector<Spread>(n);
        args->floatingCoupons = std::vector<Real>(n);
        for (Size i = 0; i < n; ++i) {
            auto coupon = ext::dynamic_pointer_cast<MultipleResetsCoupon>(leg[i]);
            args->floatingResetDates[i] = coupon->accrualStartDate();
            args->floatingPayDates[i] = coupon->date();
            args->floatingFixingDates[i] = coupon->fixingDate();
            args->floatingNominals[i] = coupon->nominal();
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
