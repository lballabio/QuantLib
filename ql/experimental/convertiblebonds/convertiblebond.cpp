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
#include <ql/experimental/convertiblebonds/convertiblebond.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

namespace QuantLib {

    ConvertibleBond::ConvertibleBond(const ext::shared_ptr<Exercise>&,
                                     Real conversionRatio,
                                     const CallabilitySchedule& callability,
                                     const Date& issueDate,
                                     Natural settlementDays,
                                     const Schedule& schedule,
                                     Real)
    : Bond(settlementDays, schedule.calendar(), issueDate), conversionRatio_(conversionRatio),
      callability_(callability) {

        maturityDate_ = schedule.endDate();

        if (!callability.empty()) {
            QL_REQUIRE(callability.back()->date() <= maturityDate_,
                       "last callability date (" << callability.back()->date()
                                                 << ") later than maturity (" << maturityDate_
                                                 << ")");
        }
    }

    void ConvertibleBond::performCalculations() const {
        option_->setPricingEngine(engine_);
        NPV_ = settlementValue_ = option_->NPV();
        errorEstimate_ = Null<Real>();
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

        option_ = ext::shared_ptr<option>(
            new option(this, exercise, conversionRatio, callability,
                       cashflows_, dayCounter, schedule, issueDate, settlementDays, redemption));
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

        option_ = ext::shared_ptr<option>(
            new option(this, exercise, conversionRatio, callability,
                       cashflows_, dayCounter, schedule, issueDate, settlementDays, redemption));
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

        option_ = ext::shared_ptr<option>(
            new option(this, exercise, conversionRatio, callability,
                       cashflows_, dayCounter, schedule, issueDate, settlementDays, redemption));

        registerWith(index);
    }

    ConvertibleBond::option::option(const ConvertibleBond* bond,
                                    const ext::shared_ptr<Exercise>& exercise,
                                    Real conversionRatio,
                                    CallabilitySchedule callability,
                                    Leg cashflows,
                                    DayCounter dayCounter,
                                    Schedule schedule,
                                    const Date& issueDate,
                                    Natural settlementDays,
                                    Real redemption)
    : OneAssetOption(
          ext::shared_ptr<StrikedTypePayoff>(new PlainVanillaPayoff(
              Option::Call, (bond->notionals()[0]) / 100.0 * redemption / conversionRatio)),
          exercise),
      bond_(bond), conversionRatio_(conversionRatio), callability_(std::move(callability)),
      cashflows_(std::move(cashflows)), dayCounter_(std::move(dayCounter)), issueDate_(issueDate),
      schedule_(std::move(schedule)), settlementDays_(settlementDays), redemption_(redemption) {
        registerWith(
            ext::shared_ptr<ConvertibleBond>(const_cast<ConvertibleBond*>(bond), null_deleter()));
    }


    void ConvertibleBond::option::setupArguments(PricingEngine::arguments* args) const {

        OneAssetOption::setupArguments(args);

        auto* moreArgs = dynamic_cast<ConvertibleBond::option::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");

        moreArgs->conversionRatio = conversionRatio_;

        Date settlement = bond_->settlementDate();

        Size n = callability_.size();
        moreArgs->callabilityDates.clear();
        moreArgs->callabilityTypes.clear();
        moreArgs->callabilityPrices.clear();
        moreArgs->callabilityTriggers.clear();
        moreArgs->callabilityDates.reserve(n);
        moreArgs->callabilityTypes.reserve(n);
        moreArgs->callabilityPrices.reserve(n);
        moreArgs->callabilityTriggers.reserve(n);
        for (Size i = 0; i < n; i++) {
            if (!callability_[i]->hasOccurred(settlement, false)) {
                moreArgs->callabilityTypes.push_back(callability_[i]->type());
                moreArgs->callabilityDates.push_back(callability_[i]->date());
                moreArgs->callabilityPrices.push_back(callability_[i]->price().amount());
                if (callability_[i]->price().type() == Bond::Price::Clean)
                    moreArgs->callabilityPrices.back() +=
                        bond_->accruedAmount(callability_[i]->date());
                ext::shared_ptr<SoftCallability> softCall =
                    ext::dynamic_pointer_cast<SoftCallability>(callability_[i]);
                if (softCall != nullptr)
                    moreArgs->callabilityTriggers.push_back(softCall->trigger());
                else
                    moreArgs->callabilityTriggers.push_back(Null<Real>());
            }
        }

        const Leg& cashflows = bond_->cashflows();

        moreArgs->couponDates.clear();
        moreArgs->couponAmounts.clear();
        for (Size i = 0; i < cashflows.size() - 1; i++) {
            if (!cashflows[i]->hasOccurred(settlement, false)) {
                moreArgs->couponDates.push_back(cashflows[i]->date());
                moreArgs->couponAmounts.push_back(cashflows[i]->amount());
            }
        }

        moreArgs->issueDate = issueDate_;
        moreArgs->settlementDate = settlement;
        moreArgs->settlementDays = settlementDays_;
        moreArgs->redemption = redemption_;
    }


    void ConvertibleBond::option::arguments::validate() const {

        OneAssetOption::arguments::validate();

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

        QL_REQUIRE(couponDates.size() == couponAmounts.size(),
                   "different number of coupon dates and amounts");
    }

}