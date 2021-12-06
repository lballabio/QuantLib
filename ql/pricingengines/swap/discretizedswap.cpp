/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2007 StatPro Italia srl

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

#include <ql/pricingengines/swap/discretizedswap.hpp>
#include <ql/settings.hpp>

namespace QuantLib {
    namespace {
        inline bool useCouponInPostAdjust(const Time& resetTime,
                                          const Time& payTime,
                                          const bool& includeTodaysCashFlows) {
            return (resetTime < 0.0) &&
                   ((payTime > 0.0) || (includeTodaysCashFlows && (payTime == 0.0)));
        }
    }

    DiscretizedSwap::DiscretizedSwap(const VanillaSwap::arguments& args,
                                     const Date& referenceDate,
                                     const DayCounter& dayCounter)
    : arguments_(args) {
        // NOLINTNEXTLINE(readability-implicit-bool-conversion)
        includeTodaysCashFlows_ = Settings::instance().includeTodaysCashFlows() &&
                                  *Settings::instance().includeTodaysCashFlows();

        fixedResetTimes_.resize(args.fixedResetDates.size());
        for (Size i=0; i<fixedResetTimes_.size(); ++i)
            fixedResetTimes_[i] =
                dayCounter.yearFraction(referenceDate,
                                        args.fixedResetDates[i]);

        fixedPayTimes_.resize(args.fixedPayDates.size());
        for (Size i=0; i<fixedPayTimes_.size(); ++i)
            fixedPayTimes_[i] =
                dayCounter.yearFraction(referenceDate,
                                        args.fixedPayDates[i]);

        floatingResetTimes_.resize(args.floatingResetDates.size());
        for (Size i=0; i<floatingResetTimes_.size(); ++i)
            floatingResetTimes_[i] =
                dayCounter.yearFraction(referenceDate,
                                        args.floatingResetDates[i]);

        floatingPayTimes_.resize(args.floatingPayDates.size());
        for (Size i=0; i<floatingPayTimes_.size(); ++i)
            floatingPayTimes_[i] =
                dayCounter.yearFraction(referenceDate,
                                        args.floatingPayDates[i]);

        if (args.originalFixedPayDates.size() > 0) {
            originalFixedPayTimes_.resize(args.originalFixedPayDates.size());
            for (Size i = 0; i < originalFixedPayTimes_.size(); ++i)
                originalFixedPayTimes_[i] =
                    dayCounter.yearFraction(referenceDate, args.originalFixedPayDates[i]);
        } else {
            originalFixedPayTimes_.resize(args.fixedPayDates.size());
            for (Size i = 0; i < originalFixedPayTimes_.size(); ++i)
                originalFixedPayTimes_[i] =
                    dayCounter.yearFraction(referenceDate, args.fixedPayDates[i]);
        }
        
        if (args.originalFixedResetDates.size() > 0) {
            originalFixedResetTimes_.resize(args.originalFixedResetDates.size());
            for (Size i = 0; i < originalFixedResetTimes_.size(); ++i)
                originalFixedResetTimes_[i] =
                    dayCounter.yearFraction(referenceDate, args.originalFixedResetDates[i]);
        } else {
            originalFixedResetTimes_.resize(args.fixedResetDates.size());
            for (Size i = 0; i < originalFixedResetTimes_.size(); ++i)
                originalFixedResetTimes_[i] =
                    dayCounter.yearFraction(referenceDate, args.fixedResetDates[i]);
        }
        
        if (args.originalFloatingResetDates.size() > 0) {
            originalFloatingResetTimes_.resize(args.originalFloatingResetDates.size());
            for (Size i = 0; i < originalFloatingResetTimes_.size(); ++i)
                originalFloatingResetTimes_[i] =
                    dayCounter.yearFraction(referenceDate, args.originalFloatingResetDates[i]);
        } else {
            originalFloatingResetTimes_.resize(args.floatingResetDates.size());
            for (Size i = 0; i < originalFloatingResetTimes_.size(); ++i)
                originalFloatingResetTimes_[i] =
                    dayCounter.yearFraction(referenceDate, args.floatingResetDates[i]);
        }
        
    }

    void DiscretizedSwap::reset(Size size) {
        values_ = Array(size, 0.0);
        adjustValues();
    }

    std::vector<Time> DiscretizedSwap::mandatoryTimes() const {
        std::vector<Time> times;
        for (double t : fixedResetTimes_) {
            if (t >= 0.0)
                times.push_back(t);
        }
        for (double t : fixedPayTimes_) {
            if (t >= 0.0)
                times.push_back(t);
        }
        for (double t : floatingResetTimes_) {
            if (t >= 0.0)
                times.push_back(t);
        }
        for (double t : floatingPayTimes_) {
            if (t >= 0.0)
                times.push_back(t);
        }
        return times;
    }

    void DiscretizedSwap::preAdjustValuesImpl() {
        // floating payments
        for (Size i=0; i<originalFloatingResetTimes_.size(); i++) {
            Time t = originalFloatingResetTimes_[i];
            if (t >= 0.0 && isOnTime(t)) {
                DiscretizedDiscountBond bond;
                bond.initialize(method(), floatingPayTimes_[i]);
                bond.rollback(time_);

                Real nominal = arguments_.nominal;
                Time T = arguments_.floatingAccrualTimes[i];
                Spread spread = arguments_.floatingSpreads[i];
                Real accruedSpread = nominal*T*spread;
                for (Size j=0; j<values_.size(); j++) {
                    Real coupon = nominal * (1.0 - bond.values()[j])
                                + accruedSpread * bond.values()[j];
                    if (arguments_.type == Swap::Payer)
                        values_[j] += coupon;
                    else
                        values_[j] -= coupon;
                }
            }
        }
        // fixed payments
        for (Size i=0; i<originalFixedResetTimes_.size(); i++) {
            Time t = originalFixedResetTimes_[i];
            if (t >= 0.0 && isOnTime(t)) {
                DiscretizedDiscountBond bond;
                bond.initialize(method(), originalFixedPayTimes_[i]);
                bond.rollback(time_);

                Real fixedCoupon = arguments_.fixedCoupons[i];
                for (Size j=0; j<values_.size(); j++) {
                    Real coupon = fixedCoupon*bond.values()[j];
                    if (arguments_.type == Swap::Payer)
                        values_[j] -= coupon;
                    else
                        values_[j] += coupon;
                }
            }
        }
    }

    void DiscretizedSwap::postAdjustValuesImpl() {
        // fixed coupons whose reset time is in the past won't be managed
        // in preAdjustValues()
        for (Size i=0; i<originalFixedPayTimes_.size(); i++) {
            Time t = originalFixedPayTimes_[i];
            Time reset = originalFixedResetTimes_[i];
            if (useCouponInPostAdjust(reset, t, includeTodaysCashFlows_) && isOnTime(t)) {
                Real fixedCoupon = arguments_.fixedCoupons[i];
                if (arguments_.type==Swap::Payer)
                    values_ -= fixedCoupon;
                else
                    values_ += fixedCoupon;
            }
        }

        // the same applies to floating payments whose rate is already fixed
        for (Size i=0; i<floatingPayTimes_.size(); i++) {
            Time t = floatingPayTimes_[i];
            Time reset = originalFloatingResetTimes_[i];
            if (useCouponInPostAdjust(reset, t, includeTodaysCashFlows_) && isOnTime(t)) {
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
}
