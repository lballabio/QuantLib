/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Theo Boafo
 Copyright (C) 2006, 2007 StatPro Italia srl

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

#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/exercise.hpp>
#include <ql/instruments/bonds/convertiblebonds.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

namespace QuantLib {

    ConvertibleBond::ConvertibleBond(const ext::shared_ptr<Exercise>& exercise,
                                     Real conversionRatio,
                                     const CallabilitySchedule& callability,
                                     const Date& issueDate,
                                     Natural settlementDays,
                                     const Schedule& schedule,
                                     Real redemption)
    : Bond(settlementDays, schedule.calendar(), issueDate), exercise_(exercise),
      conversionRatio_(conversionRatio), callability_(callability), redemption_(redemption) {

        maturityDate_ = schedule.endDate();

        if (!callability.empty()) {
            QL_REQUIRE(callability.back()->date() <= maturityDate_,
                       "last callability date (" << callability.back()->date()
                                                 << ") later than maturity (" << maturityDate_
                                                 << ")");
        }
    }


    ConvertibleZeroCouponBond::ConvertibleZeroCouponBond(const ext::shared_ptr<Exercise>& exercise,
                                                         Real conversionRatio,
                                                         const CallabilitySchedule& callability,
                                                         const Date& issueDate,
                                                         Natural settlementDays,
                                                         const DayCounter& dayCounter,
                                                         const Schedule& schedule,
                                                         Real redemption)
    : ConvertibleBond(exercise,
                      conversionRatio,
                      callability,
                      issueDate,
                      settlementDays,
                      schedule,
                      redemption) {

        cashflows_ = Leg();

        // !!! notional forcibly set to 100
        setSingleRedemption(100.0, redemption, maturityDate_);
    }


    ConvertibleFixedCouponBond::ConvertibleFixedCouponBond(
        const ext::shared_ptr<Exercise>& exercise,
        Real conversionRatio,
        const CallabilitySchedule& callability,
        const Date& issueDate,
        Natural settlementDays,
        const std::vector<Rate>& coupons,
        const DayCounter& dayCounter,
        const Schedule& schedule,
        Real redemption,
        const Period& exCouponPeriod,
        const Calendar& exCouponCalendar,
        const BusinessDayConvention exCouponConvention,
        bool exCouponEndOfMonth)
    : ConvertibleBond(exercise,
                      conversionRatio,
                      callability,
                      issueDate,
                      settlementDays,
                      schedule,
                      redemption) {

        // !!! notional forcibly set to 100
        cashflows_ = FixedRateLeg(schedule)
                         .withNotionals(100.0)
                         .withCouponRates(coupons, dayCounter)
                         .withPaymentAdjustment(schedule.businessDayConvention())
                         .withExCouponPeriod(exCouponPeriod, exCouponCalendar, exCouponConvention,
                                             exCouponEndOfMonth);

        addRedemptionsToCashflows(std::vector<Real>(1, redemption));

        QL_ENSURE(redemptions_.size() == 1, "multiple redemptions created");
    }


    ConvertibleFloatingRateBond::ConvertibleFloatingRateBond(
        const ext::shared_ptr<Exercise>& exercise,
        Real conversionRatio,
        const CallabilitySchedule& callability,
        const Date& issueDate,
        Natural settlementDays,
        const ext::shared_ptr<IborIndex>& index,
        Natural fixingDays,
        const std::vector<Spread>& spreads,
        const DayCounter& dayCounter,
        const Schedule& schedule,
        Real redemption,
        const Period& exCouponPeriod,
        const Calendar& exCouponCalendar,
        const BusinessDayConvention exCouponConvention,
        bool exCouponEndOfMonth)
    : ConvertibleBond(exercise,
                      conversionRatio,
                      callability,
                      issueDate,
                      settlementDays,
                      schedule,
                      redemption) {

        // !!! notional forcibly set to 100
        cashflows_ = IborLeg(schedule, index)
                         .withPaymentDayCounter(dayCounter)
                         .withNotionals(100.0)
                         .withPaymentAdjustment(schedule.businessDayConvention())
                         .withFixingDays(fixingDays)
                         .withSpreads(spreads)
                         .withExCouponPeriod(exCouponPeriod, exCouponCalendar, exCouponConvention,
                                             exCouponEndOfMonth);

        addRedemptionsToCashflows(std::vector<Real>(1, redemption));

        QL_ENSURE(redemptions_.size() == 1, "multiple redemptions created");

        registerWith(index);
    }

    void ConvertibleBond::setupArguments(PricingEngine::arguments* arguments) const {
        auto* args = dynamic_cast<ConvertibleBond::arguments*>(arguments);
        QL_REQUIRE(args != nullptr, "wrong argument type");

        args->exercise = exercise_;
        args->conversionRatio = conversionRatio_;

        Date settlement = settlementDate();

        Size n = callability_.size();
        args->callabilityDates.clear();
        args->callabilityTypes.clear();
        args->callabilityPrices.clear();
        args->callabilityTriggers.clear();
        args->callabilityDates.reserve(n);
        args->callabilityTypes.reserve(n);
        args->callabilityPrices.reserve(n);
        args->callabilityTriggers.reserve(n);
        for (Size i = 0; i < n; i++) {
            if (!callability_[i]->hasOccurred(settlement, false)) {
                args->callabilityTypes.push_back(callability_[i]->type());
                args->callabilityDates.push_back(callability_[i]->date());
                args->callabilityPrices.push_back(callability_[i]->price().amount());
                if (callability_[i]->price().type() == Bond::Price::Clean)
                    args->callabilityPrices.back() +=
                        accruedAmount(callability_[i]->date());
                ext::shared_ptr<SoftCallability> softCall =
                    ext::dynamic_pointer_cast<SoftCallability>(callability_[i]);
                if (softCall != nullptr)
                    args->callabilityTriggers.push_back(softCall->trigger());
                else
                    args->callabilityTriggers.push_back(Null<Real>());
            }
        }

        args->cashflows = cashflows();

        args->issueDate = issueDate_;
        args->settlementDate = settlement;
        args->settlementDays = settlementDays_;
        args->redemption = redemption_;
    }


    void ConvertibleBond::arguments::validate() const {

        QL_REQUIRE(exercise, "no exercise given");
        QL_REQUIRE(conversionRatio != Null<Real>(), "null conversion ratio");
        QL_REQUIRE(conversionRatio > 0.0,
                   "positive conversion ratio required: " << conversionRatio << " not allowed");

        QL_REQUIRE(redemption != Null<Real>(), "null redemption");
        QL_REQUIRE(redemption >= 0.0,
                   "positive redemption required: " << redemption << " not allowed");

        QL_REQUIRE(settlementDate != Date(), "null settlement date");

        QL_REQUIRE(settlementDays != Null<Natural>(), "null settlement days");

        QL_REQUIRE(callabilityDates.size() == callabilityTypes.size(),
                   "different number of callability dates and types");
        QL_REQUIRE(callabilityDates.size() == callabilityPrices.size(),
                   "different number of callability dates and prices");
        QL_REQUIRE(callabilityDates.size() == callabilityTriggers.size(),
                   "different number of callability dates and triggers");

        QL_REQUIRE(!cashflows.empty(), "no cashflows given");
    }

}
