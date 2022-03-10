/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021, 2022 Ralf Konrad Eckel

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
#include <ql/pricingengines/swap/discretizedswap.hpp>
#include <ql/pricingengines/swaption/discretizedswaption2.hpp>

namespace QuantLib {

    namespace {

        bool withinPreviousWeek(const Date& d1, const Date& d2) { return d2 >= d1 - 7 && d2 <= d1; }

        bool withinNextWeek(const Date& d1, const Date& d2) { return d2 >= d1 && d2 <= d1 + 7; }

        bool withinOneWeek(const Date& d1, const Date& d2) {
            return withinPreviousWeek(d1, d2) || withinNextWeek(d1, d2);
        }
    }

    DiscretizedSwaption2::DiscretizedSwaption2(const Swaption::arguments& args,
                                               const Date& referenceDate,
                                               const DayCounter& dayCounter)
    : DiscretizedOption(
          ext::shared_ptr<DiscretizedAsset>(), args.exercise->type(), std::vector<Time>()),
      arguments_(args) {

        // Date adjustments can get time vectors out of synch.
        // Here, we try and collapse similar dates which could cause
        // a mispricing.
        Swaption::arguments snappedArgs;
        prepareSwaptionWithSnappedDates(
            arguments_, referenceDate, dayCounter, snappedArgs, fixedResetTimes_, fixedPayTimes_,
            fixedCouponAdjustments_, floatingResetTimes_, floatingPayTimes_,
            floatCouponAdjustments_, preCouponAdjustments_, postCouponAdjustments_);

        exerciseTimes_.resize(snappedArgs.exercise->dates().size());
        for (Size i = 0; i < exerciseTimes_.size(); ++i)
            exerciseTimes_[i] =
                dayCounter.yearFraction(referenceDate, snappedArgs.exercise->date(i));

        Time lastFixedPayment =
            dayCounter.yearFraction(referenceDate, snappedArgs.fixedPayDates.back());
        Time lastFloatingPayment =
            dayCounter.yearFraction(referenceDate, snappedArgs.floatingPayDates.back());
        lastPayment_ = std::max(lastFixedPayment, lastFloatingPayment);

        underlying_ = ext::shared_ptr<DiscretizedAsset>(
            new DiscretizedSwap(snappedArgs, referenceDate, dayCounter));
    }

    void DiscretizedSwaption2::reset(Size size) {
        underlying_->initialize(method(), lastPayment_);
        underlying_->partialRollback(time());
        values_ = underlying_->values();
        QL_ASSERT(size == values_.size(), "We should have rolled back to the right size by now.");
    }

    std::vector<Time> DiscretizedSwaption2::mandatoryTimes() const {
        std::vector<Time> mandatoryTimes;

        mandatoryTimes.insert(mandatoryTimes.end(), fixedResetTimes_.begin(),
                              fixedResetTimes_.end());
        mandatoryTimes.insert(mandatoryTimes.end(), fixedPayTimes_.begin(), fixedPayTimes_.end());
        mandatoryTimes.insert(mandatoryTimes.end(), floatingResetTimes_.begin(),
                              floatingResetTimes_.end());
        mandatoryTimes.insert(mandatoryTimes.end(), floatingPayTimes_.begin(),
                              floatingPayTimes_.end());
        mandatoryTimes.insert(mandatoryTimes.end(), exerciseTimes_.begin(), exerciseTimes_.end());

        std::sort(mandatoryTimes.begin(), mandatoryTimes.end());
        mandatoryTimes.erase(std::unique(mandatoryTimes.begin(), mandatoryTimes.end()),
                             mandatoryTimes.end());

        return mandatoryTimes;
    }

    void DiscretizedSwaption2::preAdjustValuesImpl() {}

    void DiscretizedSwaption2::postAdjustValuesImpl() {}

    void DiscretizedSwaption2::applyCallability(Size i) {}

    void DiscretizedSwaption2::addFixedCoupon(Size i) { values_ += arguments_.fixedCoupons[i]; }

    void DiscretizedSwaption2::addFloatCoupon(Size i) {}

    void DiscretizedSwaption2::prepareSwaptionWithSnappedDates(
        const Swaption::arguments& args,
        const Date& referenceDate,
        const DayCounter& dayCounter,
        PricingEngine::arguments& snappedArgs,
        std::vector<Time>& fixedResetTimes,
        std::vector<Time>& fixedPayTimes,
        std::vector<CouponAdjustment>& fixedCouponAdjustments,
        std::vector<Time>& floatingResetTimes,
        std::vector<Time>& floatingPayTimes,
        std::vector<CouponAdjustment>& floatCouponAdjustments,
        std::vector<Real>& preCouponAdjustments,
        std::vector<Real>& postCouponAdjustments) {

        std::vector<Date> fixedDates = args.swap->fixedSchedule().dates();
        std::vector<Date> floatDates = args.swap->floatingSchedule().dates();

        fixedCouponAdjustments = std::vector<CouponAdjustment>(fixedDates.size() - 1);
        floatCouponAdjustments = std::vector<CouponAdjustment>(floatDates.size() - 1);

        preCouponAdjustments = std::vector<Real>(args.exercise->dates().size(), 0.0);
        postCouponAdjustments = std::vector<Real>(args.exercise->dates().size(), 0.0);

        for (const auto& exerciseDate : args.exercise->dates()) {
            for (Size j = 0; j < fixedDates.size() - 1; j++) {
                auto& fixedDate = fixedDates[j];
                if (withinOneWeek(exerciseDate, fixedDate) && exerciseDate != fixedDate) {
                    fixedDate = exerciseDate;
                    fixedCouponAdjustments[j] = withinNextWeek(exerciseDate, fixedDate) ?
                                                    CouponAdjustment::pre :
                                                    CouponAdjustment::post;
                }
            }

            for (Size j = 0; j < floatDates.size() - 1; j++) {
                auto& floatDate = floatDates[j];
                if (withinOneWeek(exerciseDate, floatDate)) {
                    floatDate = exerciseDate;
                    floatCouponAdjustments[j] = withinNextWeek(exerciseDate, floatDate) ?
                                                    CouponAdjustment::pre :
                                                    CouponAdjustment::post;
                }
            }
        }

        auto getTimes = [&referenceDate, &dayCounter](const std::vector<Date>& dates,
                                                      std::vector<Time>& resetTimes,
                                                      std::vector<Time>& payTimes) {
            std::vector<Time> times;
            std::transform(dates.begin(), dates.end(), std::back_inserter(times),
                           [&referenceDate, &dayCounter](const auto date) -> Time {
                               return dayCounter.yearFraction(referenceDate, date);
                           });

            resetTimes = times;
            resetTimes.pop_back();

            payTimes = times;
            payTimes.erase(payTimes.begin());
        };

        getTimes(fixedDates, fixedResetTimes, fixedPayTimes);
        getTimes(floatDates, floatingResetTimes, floatingPayTimes);

        Schedule snappedFixedSchedule(fixedDates);
        Schedule snappedFloatSchedule(floatDates);

        auto snappedSwap = ext::make_shared<VanillaSwap>(
            args.swap->type(), args.swap->nominal(), snappedFixedSchedule, args.swap->fixedRate(),
            args.swap->fixedDayCount(), snappedFloatSchedule, args.swap->iborIndex(),
            args.swap->spread(), args.swap->floatingDayCount(), args.swap->paymentConvention());

        Swaption snappedSwaption(snappedSwap, args.exercise, args.settlementType,
                                 args.settlementMethod);

        snappedSwaption.setupArguments(&snappedArgs);
    }
}
