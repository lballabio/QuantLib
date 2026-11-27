/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Allen Kuo
 Copyright (C) 2021, 2022 Ralf Konrad Eckel

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

#include <ql/experimental/callablebonds/discretizedcallablefixedratebond.hpp>
#include <algorithm>

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

        // With advance notice the actual order of decisions and intervening
        // payments matters. Keep the exact dates, including any immediate
        // rights in the same schedule. Preserve legacy snapping otherwise.
        const bool hasNotice = !args.callabilityNoticeDates.empty() &&
                               args.callabilityNoticeDates != args.callabilityDates;
        callabilityTimes_.resize(args.callabilityDates.size());
        callabilityPaymentTimes_.resize(args.callabilityDates.size());
        for (Size i = 0; i < callabilityTimes_.size(); ++i) {
            const Date callabilityDate = args.callabilityDates[i];
            const Date noticeDate = hasNotice ? args.callabilityNoticeDates[i] : callabilityDate;
            Time callabilityTime = dayCounter.yearFraction(referenceDate, noticeDate);
            callabilityPaymentTimes_[i] = dayCounter.yearFraction(referenceDate, callabilityDate);

            // To avoid mispricing, we snap exercise dates to the closest coupon date.
            for (Size j = 0; !hasNotice && j < couponTimes_.size(); j++) {
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
                     * discount factor including any possible spread e.g. set in the OAS
                     * calculation. */
                    auto spread  = arguments_.spread;
                    auto calcDiscountFactorInclSpread = [&termStructure, spread](Date date) {
                        auto time = termStructure->timeFromReference(date);
                        auto zeroRateInclSpread =
                            termStructure->zeroRate(date, termStructure->dayCounter(), Continuous,
                                                    NoFrequency) +
                            spread;
                        auto df = std::exp(-zeroRateInclSpread * time);
                        return df;
                    };

                    auto dfTillCallDate = calcDiscountFactorInclSpread(callabilityDate);
                    auto dfTillCouponDate = calcDiscountFactorInclSpread(couponDate);
                    adjustedCallabilityPrices_[i] *= dfTillCallDate / dfTillCouponDate;

                    break;
                }
            }

            adjustedCallabilityPrices_[i] *= arguments_.faceAmount / 100.0;
            callabilityTimes_[i] = callabilityTime;
            if (noticeDate == callabilityDate)
                callabilityPaymentTimes_[i] = callabilityTime;
        }

        if (hasNotice) {
            // Distinct dates can share a lattice time (e.g., under 30/360).
            // Within that time roll events back in contractual date order.
            // On the same date retain schedule order for decisions, then add coupons.
            for (Size i = 0; i < callabilityTimes_.size(); ++i)
                noticeScheduleEvents_.push_back(
                    {args.callabilityNoticeDates[i], callabilityTimes_[i], i, false});
            for (Size i = 0; i < couponTimes_.size(); ++i)
                noticeScheduleEvents_.push_back({args.couponDates[i], couponTimes_[i], i, true});
            std::stable_sort(noticeScheduleEvents_.begin(), noticeScheduleEvents_.end(),
                             [](const DatedEvent& a, const DatedEvent& b) { return a.date > b.date; });
            return;
        }

        // not snapped itself, but a snapped neighbour may have moved this coupon ahead
        for (Size i = 0; i < callabilityTimes_.size(); ++i) {
            for (Size j = 0; j < couponTimes_.size(); ++j) {
                if (couponAdjustments_[j] == CouponAdjustment::pre &&
                    args.callabilityDates[i] == args.couponDates[j])
                    adjustedCallabilityPrices_[i] += args.couponAmounts[j];
            }
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
                if (callabilityPaymentTimes_[i] != t)
                    times.push_back(callabilityPaymentTimes_[i]);
            }
        }

        return times;
    }


    void DiscretizedCallableFixedRateBond::preAdjustValuesImpl() {
        for (Size i = 0; i < couponTimes_.size(); i++) {
            if (couponAdjustments_[i] == CouponAdjustment::pre) {
                Time t = couponTimes_[i];
                if (t >= 0.0 && close_enough(t, time())) {
                    addCoupon(i);
                }
            }
        }
    }


    void DiscretizedCallableFixedRateBond::postAdjustValuesImpl() {
        if (!noticeScheduleEvents_.empty()) {
            for (const auto& event : noticeScheduleEvents_) {
                if (event.time >= 0.0 && isOnTime(event.time)) {
                    if (event.isCoupon)
                        addCoupon(event.index);
                    else
                        applyCallability(event.index);
                }
            }
            return;
        }
        for (Size i = 0; i < callabilityTimes_.size(); i++) {
            Time t = callabilityTimes_[i];
            if (t >= 0.0 && close_enough(t, time())) {
                applyCallability(i);
            }
        }
        for (Size i = 0; i < couponTimes_.size(); i++) {
            if (couponAdjustments_[i] == CouponAdjustment::post) {
                Time t = couponTimes_[i];
                if (t >= 0.0 && close_enough(t, time())) {
                    /* Exercise and coupon date matches. */
                    addCoupon(i);
                }
            }
        }
    }


    void DiscretizedCallableFixedRateBond::applyCallability(Size i) {
        // The alternative to continuation is the conditional value, at
        // notice, of the committed strike and coupons through payment. Roll
        // these promised cash flows on the same (possibly spreaded) lattice:
        // the strike is not payable at notice and discount factors are random.
        Array exerciseValues;
        if (!arguments_.callabilityNoticeDates.empty() &&
            arguments_.callabilityNoticeDates[i] < arguments_.callabilityDates[i]) {
            DiscretizedDiscountBond payoff;
            payoff.initialize(method(), callabilityPaymentTimes_[i]);
            payoff.values() *= adjustedCallabilityPrices_[i];
            for (Size j = couponTimes_.size(); j > 0; --j) {
                const Date couponDate = arguments_.couponDates[j - 1];
                if (arguments_.callabilityNoticeDates[i] < couponDate &&
                    couponDate <= arguments_.callabilityDates[i]) {
                    payoff.rollback(couponTimes_[j - 1]);
                    payoff.values() += arguments_.couponAmounts[j - 1];
                }
            }
            // A coupon on the decision date is added to both alternatives
            // by postAdjustValuesImpl, after the call/put comparison.
            payoff.rollback(time());
            exerciseValues = payoff.values();
        }
        auto exercisePrice = [&](Size j) -> Real {
            return exerciseValues.empty() ? adjustedCallabilityPrices_[i] : exerciseValues[j];
        };
        switch (arguments_.callabilityTypes[i]) {
            case Callability::Call:
                for (Size j = 0; j < values_.size(); ++j) {
                    values_[j] = std::min(exercisePrice(j), values_[j]);
                }
                break;
            case Callability::Put:
                for (Size j = 0; j < values_.size(); ++j) {
                    values_[j] = std::max(exercisePrice(j), values_[j]);
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
