/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2007 StatPro Italia srl
 Copyright (C) 2022 Ralf Konrad Eckel

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

#include <ql/pricingengines/swap/discretizedswap.hpp>
#include <ql/settings.hpp>
#include <utility>

namespace QuantLib {
    namespace {
        inline bool isResetTimeInPast(const Time& resetTime,
                                      const Time& payTime,
                                      const bool& includeTodaysCashFlows) {
            return (resetTime < 0.0) &&
                   ((payTime > 0.0) || (includeTodaysCashFlows && (payTime == 0.0)));
        }
    }

    DiscretizedSwap::DiscretizedSwap(const VanillaSwap::arguments& args,
                                     const Date& referenceDate,
                                     const DayCounter& dayCounter)
    : DiscretizedSwap(
          args,
          referenceDate,
          dayCounter,
          std::vector<CouponAdjustment>(args.fixedPayDates.size(), CouponAdjustment::pre),
          std::vector<CouponAdjustment>(args.floatingPayDates.size(), CouponAdjustment::pre)) {}

    DiscretizedSwap::DiscretizedSwap(const VanillaSwap::arguments& args,
                                     const Date& referenceDate,
                                     const DayCounter& dayCounter,
                                     std::vector<CouponAdjustment> fixedCouponAdjustments,
                                     std::vector<CouponAdjustment> floatingCouponAdjustments)
    : arguments_(args), fixedCouponAdjustments_(std::move(fixedCouponAdjustments)),
      floatingCouponAdjustments_(std::move(floatingCouponAdjustments)) {
        QL_REQUIRE(
            fixedCouponAdjustments_.size() == arguments_.fixedPayDates.size(),
            "The fixed coupon adjustments must have the same size as the number of fixed coupons.");
        QL_REQUIRE(floatingCouponAdjustments_.size() == arguments_.floatingPayDates.size(),
                   "The floating coupon adjustments must have the same size as the number of "
                   "floating coupons.");

        // NOLINTNEXTLINE(readability-implicit-bool-conversion)
        auto includeTodaysCashFlows = Settings::instance().includeTodaysCashFlows() &&
                                      *Settings::instance().includeTodaysCashFlows(); // NOLINT(bugprone-unchecked-optional-access)

        auto nrOfFixedCoupons = args.fixedResetDates.size();
        fixedResetTimes_.resize(nrOfFixedCoupons);
        fixedPayTimes_.resize(nrOfFixedCoupons);
        fixedResetTimeIsInPast_.resize(nrOfFixedCoupons);
        for (Size i = 0; i < nrOfFixedCoupons; ++i) {
            auto resetTime = dayCounter.yearFraction(referenceDate, args.fixedResetDates[i]);
            auto payTime = dayCounter.yearFraction(referenceDate, args.fixedPayDates[i]);
            auto resetIsInPast = isResetTimeInPast(resetTime, payTime, includeTodaysCashFlows);

            fixedResetTimes_[i] = resetTime;
            fixedPayTimes_[i] = payTime;
            fixedResetTimeIsInPast_[i] = resetIsInPast;
            if (resetIsInPast)
                fixedCouponAdjustments_[i] = CouponAdjustment::post;
        }

        auto nrOfFloatingCoupons = args.floatingResetDates.size();
        floatingResetTimes_.resize(nrOfFloatingCoupons);
        floatingPayTimes_.resize(nrOfFloatingCoupons);
        floatingResetTimeIsInPast_.resize(nrOfFloatingCoupons);
        for (Size i = 0; i < nrOfFloatingCoupons; ++i) {
            auto resetTime = dayCounter.yearFraction(referenceDate, args.floatingResetDates[i]);
            auto payTime = dayCounter.yearFraction(referenceDate, args.floatingPayDates[i]);
            auto resetIsInPast = isResetTimeInPast(resetTime, payTime, includeTodaysCashFlows);

            floatingResetTimes_[i] = resetTime;
            floatingPayTimes_[i] = payTime;
            floatingResetTimeIsInPast_[i] = resetIsInPast;
            if (resetIsInPast)
                floatingCouponAdjustments_[i] = CouponAdjustment::post;
        }
    }

    void DiscretizedSwap::reset(Size size) {
        values_ = Array(size, 0.0);
        adjustValues();
    }

    std::vector<Time> DiscretizedSwap::mandatoryTimes() const {
        std::vector<Time> times;
        for (Real t : fixedResetTimes_) {
            if (t >= 0.0)
                times.push_back(t);
        }
        for (Real t : fixedPayTimes_) {
            if (t >= 0.0)
                times.push_back(t);
        }
        for (Real t : floatingResetTimes_) {
            if (t >= 0.0)
                times.push_back(t);
        }
        for (Real t : floatingPayTimes_) {
            if (t >= 0.0)
                times.push_back(t);
        }
        return times;
    }

    void DiscretizedSwap::preAdjustValuesImpl() {
        // floating payments
        for (Size i = 0; i < floatingResetTimes_.size(); i++) {
            Time t = floatingResetTimes_[i];
            if (floatingCouponAdjustments_[i] == CouponAdjustment::pre && t >= 0.0 && isOnTime(t)) {
                addFloatingCoupon(i);
            }
        }
        // fixed payments
        for (Size i = 0; i < fixedResetTimes_.size(); i++) {
            Time t = fixedResetTimes_[i];
            if (fixedCouponAdjustments_[i] == CouponAdjustment::pre && t >= 0.0 && isOnTime(t)) {
                addFixedCoupon(i);
            }
        }
    }

    void DiscretizedSwap::postAdjustValuesImpl() {
        // floating payments
        for (Size i = 0; i < floatingResetTimes_.size(); i++) {
            Time t = floatingResetTimes_[i];
            if (floatingCouponAdjustments_[i] == CouponAdjustment::post && t >= 0.0 && isOnTime(t)) {
                addFloatingCoupon(i);
            }
        }
        // fixed payments
        for (Size i = 0; i < fixedResetTimes_.size(); i++) {
            Time t = fixedResetTimes_[i];
            if (fixedCouponAdjustments_[i] == CouponAdjustment::post && t >= 0.0 && isOnTime(t)) {
                addFixedCoupon(i);
            }
        }

        // fixed coupons whose reset time is in the past won't be managed
        // in preAdjustValues()
        for (Size i = 0; i < fixedPayTimes_.size(); i++) {
            Time t = fixedPayTimes_[i];
            if (fixedResetTimeIsInPast_[i] && isOnTime(t)) {
                Real fixedCoupon = arguments_.fixedCoupons[i];
                if (arguments_.type == Swap::Payer)
                    values_ -= fixedCoupon;
                else
                    values_ += fixedCoupon;
            }
        }

        // the same applies to floating payments whose rate is already fixed
        for (Size i = 0; i < floatingPayTimes_.size(); i++) {
            Time t = floatingPayTimes_[i];
            if (floatingResetTimeIsInPast_[i] && isOnTime(t)) {
                Real currentFloatingCoupon = arguments_.floatingCoupons[i];
                QL_REQUIRE(currentFloatingCoupon != Null<Real>(),
                           "current floating coupon not given");
                if (arguments_.type == Swap::Payer)
                    values_ += currentFloatingCoupon;
                else
                    values_ -= currentFloatingCoupon;
            }
        }
    }

    void DiscretizedSwap::addFixedCoupon(Size i) {
        DiscretizedDiscountBond bond;
        bond.initialize(method(), fixedPayTimes_[i]);
        bond.rollback(time_);

        Real fixedCoupon = arguments_.fixedCoupons[i];
        for (Size j = 0; j < values_.size(); j++) {
            Real coupon = fixedCoupon * bond.values()[j];
            if (arguments_.type == Swap::Payer)
                values_[j] -= coupon;
            else
                values_[j] += coupon;
        }
    }

    void DiscretizedSwap::addFloatingCoupon(Size i) {
        DiscretizedDiscountBond bond;
        bond.initialize(method(), floatingPayTimes_[i]);
        bond.rollback(time_);

        QL_REQUIRE(arguments_.nominal != Null<Real>(),
                   "non-constant nominals are not supported yet");

        Real nominal = arguments_.nominal;
        Time T = arguments_.floatingAccrualTimes[i];
        Spread spread = arguments_.floatingSpreads[i];
        Real accruedSpread = nominal * T * spread;
        for (Size j = 0; j < values_.size(); j++) {
            Real coupon = nominal * (1.0 - bond.values()[j]) + accruedSpread * bond.values()[j];
            if (arguments_.type == Swap::Payer)
                values_[j] += coupon;
            else
                values_[j] -= coupon;
        }
    }
}
