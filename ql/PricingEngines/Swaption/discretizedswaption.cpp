
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/PricingEngines/Swaption/discretizedswaption.hpp>

namespace QuantLib {

    void DiscretizedSwap::preAdjustValuesImpl() {
        for (Size i=0; i<arguments_.floatingResetTimes.size(); i++) {
            Time t = arguments_.floatingResetTimes[i];
            if (t >= 0.0 && isOnTime(t)) {
                DiscretizedDiscountBond bond;
                bond.initialize(method(), arguments_.floatingPayTimes[i]);
                bond.rollback(time_);

                Real nominal = arguments_.nominal;
                for (Size j=0; j<values_.size(); j++) {
                    Real coupon = nominal*(1.0 - bond.values()[j]);
                    if (arguments_.payFixed)
                        values_[j] += coupon;
                    else
                        values_[j] -= coupon;
                }
            }
        }
    }

    void DiscretizedSwap::postAdjustValuesImpl() {
        for (Size i=0; i<arguments_.fixedPayTimes.size(); i++) {
            Time t = arguments_.fixedPayTimes[i];
            if (t >= 0.0 && isOnTime(t)) {
                Real fixedCoupon = arguments_.fixedCoupons[i];
                if (arguments_.payFixed)
                    values_ -= fixedCoupon;
                else
                    values_ += fixedCoupon;
            }
        }
        if (arguments_.currentFloatingCoupon != Null<Real>()) {
            for (Size i=0; i<arguments_.floatingPayTimes.size(); i++) {
                Time t = arguments_.floatingPayTimes[i];
                if (t >= 0.0 && isOnTime(t)) {
                    if (arguments_.floatingResetTimes[i] < 0.0) {
                        if (arguments_.payFixed)
                            values_ += arguments_.currentFloatingCoupon;
                        else
                            values_ -= arguments_.currentFloatingCoupon;
                        break;
                    }
                }
            }
        }
    }


    namespace {

        bool withinPreviousWeek(Time t1, Time t2) {
            static const Time dt = 1.0/52;
            return t1-dt <= t2 && t2 <= t1;
        }

        bool withinNextWeek(Time t1, Time t2) {
            static const Time dt = 1.0/52;
            return t1 <= t2 && t2 <= t1+dt;
        }

    }

    DiscretizedSwaption::DiscretizedSwaption(const Swaption::arguments& args)
    : DiscretizedOption(boost::shared_ptr<DiscretizedAsset>(),
                        args.exercise->type(),
                        args.stoppingTimes),
      arguments_(args) {

        // Date adjustments can get time vectors out of synch.
        // Here, we try and collapse similar dates.
        for (Size i=0; i<arguments_.stoppingTimes.size(); i++) {
            Time exercise = arguments_.stoppingTimes[i];
            for (Size j=0; j<arguments_.fixedPayTimes.size(); j++) {
                if (withinNextWeek(exercise, arguments_.fixedPayTimes[j]))
                    arguments_.fixedPayTimes[j] = exercise;
            }
            for (Size k=0; k<arguments_.floatingResetTimes.size(); k++) {
                if (withinPreviousWeek(exercise,
                                       arguments_.floatingResetTimes[k]))
                    arguments_.floatingResetTimes[k] = exercise;
            }
        }

        underlying_ = boost::shared_ptr<DiscretizedAsset>(
                                             new DiscretizedSwap(arguments_));
    }

    void DiscretizedSwaption::reset(Size size) {
        Time lastFixedPay = arguments_.fixedPayTimes.back();
        Time lastFloatPay = arguments_.floatingPayTimes.back();
        underlying_->initialize(method(),std::max(lastFixedPay,lastFloatPay));
        DiscretizedOption::reset(size);
    }

}
