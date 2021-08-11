/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Allen Kuo
 Copyright (C) 2021 Ralf Konrad Eckel

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

#include <ql/experimental/callablebonds/discretizedcallablefixedratebond.hpp>

namespace QuantLib {

    namespace {

        bool withinNextWeek(Time t1, Time t2) {
            static const Time dt = 1.0 / 52;
            return t1 <= t2 && t2 <= t1 + dt;
        }

    }


    DiscretizedCallableFixedRateBond::DiscretizedCallableFixedRateBond(
        const CallableBond::arguments& args, const Handle<YieldTermStructure>& termStructure)
    : arguments_(args), adjustedCallabilityPrices_(args.callabilityPrices) {

        auto dayCounter = termStructure->dayCounter();
        auto referenceDate = termStructure->referenceDate();

        redemptionTime_ = dayCounter.yearFraction(referenceDate, args.redemptionDate);

        /* By default the coupon adjustment should take place in
         * DiscretizedCallableFixedRateBond::postAdjustValuesImpl(). */
        couponAdjustments_ =
            std::vector<CouponAdjustment>(args.couponDates.size(), CouponAdjustment::post);

        couponTimes_.resize(args.couponDates.size());
        for (Size i = 0; i < couponTimes_.size(); ++i) {
            couponTimes_[i] = dayCounter.yearFraction(referenceDate, args.couponDates[i]);
        }

        callabilityTimes_.resize(args.callabilityDates.size());
        for (Size i = 0; i < callabilityTimes_.size(); ++i) {
            const Date callabilityDate = args.callabilityDates[i];
            Time callabilityTime = dayCounter.yearFraction(referenceDate, args.callabilityDates[i]);

            // To avoid mispricing, we snap exercise dates to the closest coupon date.
            for (Size j = 0; j < couponTimes_.size(); j++) {
                const Time couponTime = couponTimes_[j];
                const Date couponDate = args.couponDates[j];

                if (withinNextWeek(callabilityTime, couponTime) && callabilityDate < couponDate) {
                    // Snap the exercise date.
                    callabilityTime = couponTime;

                    /* The order of events must be changed here. In
                     * DiscretizedCallableFixedRateBond::postAdjustValuesImpl() the callability is
                     * done before adding of the coupon. However from the
                     * DiscretizedAsset::rollback(Time to) perspective the coupon must be added
                     * before the callability as it is later in time. */
                    couponAdjustments_[j] = CouponAdjustment::pre;

                    /* We snapped the callabilityTime so we need to take into account the missing
                     * discount factor. */
                    auto discountTillCallDate = termStructure->discount(callabilityDate);
                    auto discountTillCouponDate = termStructure->discount(couponDate);
                    adjustedCallabilityPrices_[i] *= discountTillCallDate / discountTillCouponDate;

                    break;
                }
            }

            callabilityTimes_[i] = callabilityTime;
        }
    }


    void DiscretizedCallableFixedRateBond::reset(Size size) {
        values_ = Array(size, arguments_.redemption);
        adjustValues();
    }


    std::vector<Time> DiscretizedCallableFixedRateBond::mandatoryTimes() const {
        std::vector<Time> times;
        Time t;
        Size i;

        t = redemptionTime_;
        if (t >= 0.0) {
            times.push_back(t);
        }

        for (i = 0; i < couponTimes_.size(); i++) {
            t = couponTimes_[i];
            if (t >= 0.0) {
                times.push_back(t);
            }
        }

        for (i = 0; i < callabilityTimes_.size(); i++) {
            t = callabilityTimes_[i];
            if (t >= 0.0) {
                times.push_back(t);
            }
        }

        return times;
    }


    void DiscretizedCallableFixedRateBond::preAdjustValuesImpl() {
        for (Size i = 0; i < couponTimes_.size(); i++) {
            if (couponAdjustments_[i] == CouponAdjustment::pre) {
                Time t = couponTimes_[i];
                if (t >= 0.0 && isOnTime(t)) {
                    addCoupon(i);
                }
            }
        }
    }


    void DiscretizedCallableFixedRateBond::postAdjustValuesImpl() {
        for (Size i = 0; i < callabilityTimes_.size(); i++) {
            Time t = callabilityTimes_[i];
            if (t >= 0.0 && isOnTime(t)) {
                applyCallability(i);
            }
        }
        for (Size i = 0; i < couponTimes_.size(); i++) {
            if (couponAdjustments_[i] == CouponAdjustment::post) {
                Time t = couponTimes_[i];
                if (t >= 0.0 && isOnTime(t)) {
                    /* Exercise and coupon date matches. */
                    addCoupon(i);
                }
            }
        }
    }


    void DiscretizedCallableFixedRateBond::applyCallability(Size i) {
        Size j;
        switch (arguments_.putCallSchedule[i]->type()) {
            case Callability::Call:
                for (j = 0; j < values_.size(); j++) {
                    values_[j] = std::min(adjustedCallabilityPrices_[i], values_[j]);
                }
                break;
            case Callability::Put:
                for (j = 0; j < values_.size(); j++) {
                    values_[j] = std::max(values_[j], adjustedCallabilityPrices_[i]);
                }
                break;
            default:
                QL_FAIL("unknown callability type");
        }
    }


    void DiscretizedCallableFixedRateBond::addCoupon(Size i) {
        values_ += arguments_.couponAmounts[i];
    }

}
