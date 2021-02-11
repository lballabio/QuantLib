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

#include <ql/pricingengines/swaption/discretizedswaption.hpp>
#include <ql/pricingengines/swap/discretizedswap.hpp>

namespace QuantLib {

    namespace {

        bool withinPreviousWeek(const Date& d1, const Date& d2) {
            return d2 >= d1-7 && d2 <= d1;
        }

        bool withinNextWeek(const Date& d1, const Date& d2) {
            return d2 >= d1 && d2 <= d1+7;
        }

    }

    DiscretizedSwaption::DiscretizedSwaption(const Swaption::arguments& args,
                                             const Date& referenceDate,
                                             const DayCounter& dayCounter)
    : DiscretizedOption(ext::shared_ptr<DiscretizedAsset>(),
                        args.exercise->type(),
                        std::vector<Time>()),
      arguments_(args) {

        exerciseTimes_.resize(arguments_.exercise->dates().size());
        for (Size i=0; i<exerciseTimes_.size(); ++i)
            exerciseTimes_[i] =
                dayCounter.yearFraction(referenceDate,
                                        arguments_.exercise->date(i));

        // Date adjustments can get time vectors out of synch.
        // Here, we try and collapse similar dates which could cause
        // a mispricing.
        for (Size i=0; i<arguments_.exercise->dates().size(); i++) {
            Date exerciseDate = arguments_.exercise->date(i);
            for (Size j=0; j<arguments_.fixedPayDates.size(); j++) {
                if (withinNextWeek(exerciseDate,
                                   arguments_.fixedPayDates[j])
                    // coupons in the future are dealt with below
                    && arguments_.fixedResetDates[j] < referenceDate)
                    arguments_.fixedPayDates[j] = exerciseDate;
            }
            for (auto& fixedResetDate : arguments_.fixedResetDates) {
                if (withinPreviousWeek(exerciseDate, fixedResetDate))
                    fixedResetDate = exerciseDate;
            }
            for (auto& floatingResetDate : arguments_.floatingResetDates) {
                if (withinPreviousWeek(exerciseDate, floatingResetDate))
                    floatingResetDate = exerciseDate;
            }
        }

        Time lastFixedPayment =
            dayCounter.yearFraction(referenceDate,
                                    arguments_.fixedPayDates.back());
        Time lastFloatingPayment =
            dayCounter.yearFraction(referenceDate,
                                    arguments_.floatingPayDates.back());
        lastPayment_ = std::max(lastFixedPayment,lastFloatingPayment);

        underlying_ = ext::shared_ptr<DiscretizedAsset>(
                                            new DiscretizedSwap(arguments_,
                                                                referenceDate,
                                                                dayCounter));
    }

    void DiscretizedSwaption::reset(Size size) {
        underlying_->initialize(method(), lastPayment_);
        DiscretizedOption::reset(size);
    }

}
