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
#include <cmath>
#include <limits>

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
            }
        }

        return times;
    }


    void DiscretizedCallableFixedRateBond::preAdjustValuesImpl() {
        checkGridTimes();
        for (Size i = 0; i < couponTimes_.size(); i++) {
            if (couponAdjustments_[i] == CouponAdjustment::pre) {
                Time t = couponTimes_[i];
                if (t >= 0.0 && isOnGridTime(t, couponGridTimes_, i)) {
                    addCoupon(i);
                }
            }
        }
    }


    void DiscretizedCallableFixedRateBond::postAdjustValuesImpl() {
        checkGridTimes();
        for (Size i = 0; i < callabilityTimes_.size(); i++) {
            Time t = callabilityTimes_[i];
            if (t >= 0.0 && isOnGridTime(t, callabilityGridTimes_, i)) {
                applyCallability(i);
            }
        }
        for (Size i = 0; i < couponTimes_.size(); i++) {
            if (couponAdjustments_[i] == CouponAdjustment::post) {
                Time t = couponTimes_[i];
                if (t >= 0.0 && isOnGridTime(t, couponGridTimes_, i)) {
                    /* Exercise and coupon date matches. */
                    addCoupon(i);
                }
            }
        }
    }


    void DiscretizedCallableFixedRateBond::checkGridTimes() {
        const Lattice* lattice = &(*method());
        // Recompute also if the lattice is the same object but has been rebuilt
        // on a different grid in the meantime: prices/thresholds are unaffected,
        // but the node values are not.
        const TimeGrid& grid = method()->timeGrid();
        bool sameGrid = lattice == gridTimesMethod_ &&
                        ((grid.empty() && gridTimes_.empty()) ||
                         (!grid.empty() && !gridTimes_.empty() && grid[0] == gridTimes_.front() &&
                          grid[grid.size() - 1] == gridTimes_.back() &&
                          grid.size() == gridTimes_.size()));
        if (lattice == gridTimesMethod_ && sameGrid &&
            couponGridTimes_.size() == couponTimes_.size() &&
            callabilityGridTimes_.size() == callabilityTimes_.size())
            return;
        gridTimes_.assign(grid.begin(), grid.end());
        couponGridTimes_.assign(couponTimes_.size(), std::numeric_limits<Time>::quiet_NaN());
        callabilityGridTimes_.assign(callabilityTimes_.size(), std::numeric_limits<Time>::quiet_NaN());
        gridTimesMethod_ = lattice;
    }


    bool DiscretizedCallableFixedRateBond::isOnGridTime(Time t,
                                                        std::vector<Time>& gridTimes,
                                                        Size i) {
        if (std::isnan(gridTimes[i])) {
            // Same as isOnTime(), but the expensive lookup is done once per lattice.
            const TimeGrid& grid = method()->timeGrid();
            gridTimes[i] = grid[grid.index(t)];
        }
        return close_enough(gridTimes[i], time());
    }


    void DiscretizedCallableFixedRateBond::applyCallability(Size i) {
        Size j;
        switch (arguments_.callabilityTypes[i]) {
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
