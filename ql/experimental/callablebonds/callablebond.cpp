/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Allen Kuo

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

#include <ql/experimental/callablebonds/callablebond.hpp>
#include <ql/experimental/callablebonds/blackcallablebondengine.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/math/solvers1d/brent.hpp>

namespace QuantLib {

    CallableBond::CallableBond(Natural settlementDays,
                               const Schedule& schedule,
                               const DayCounter& paymentDayCounter,
                               const Date& issueDate,
                               const CallabilitySchedule& putCallSchedule)
    : Bond(settlementDays, schedule.calendar(), issueDate),
      paymentDayCounter_(paymentDayCounter), putCallSchedule_(putCallSchedule) {

        maturityDate_ = schedule.dates().back();

        if (!putCallSchedule_.empty()) {
            Date finalOptionDate = Date::minDate();
            for (Size i=0; i<putCallSchedule_.size();++i) {
                finalOptionDate=std::max(finalOptionDate,
                                         putCallSchedule_[i]->date());
            }
            QL_REQUIRE(finalOptionDate <= maturityDate_ ,
                       "Bond cannot mature before last call/put date");
        }

        // derived classes must set cashflows_ and frequency_
    }


    void CallableBond::arguments::validate() const {

        QL_REQUIRE(Bond::arguments::settlementDate != Date(),
                   "null settlement date");

        QL_REQUIRE(redemption != Null<Real>(), "null redemption");
        QL_REQUIRE(redemption >= 0.0,
                   "positive redemption required: "
                   << redemption << " not allowed");

        QL_REQUIRE(callabilityDates.size() == callabilityPrices.size(),
                   "different number of callability dates and prices");
        QL_REQUIRE(couponDates.size() == couponAmounts.size(),
                   "different number of coupon dates and amounts");
    }


    CallableBond::ImpliedVolHelper::ImpliedVolHelper(
                              const CallableBond& bond,
                              Real targetValue)
    : targetValue_(targetValue) {

        vol_ = boost::shared_ptr<SimpleQuote>(new SimpleQuote(0.0));
        bond.blackVolQuote_.linkTo(vol_);

        QL_REQUIRE(bond.blackEngine_,
                   "Must set blackEngine_ to use impliedVolatility");

        engine_ = bond.blackEngine_;
        bond.setupArguments(engine_->getArguments());
        results_ =
            dynamic_cast<const Instrument::results*>(engine_->getResults());
    }


    Real CallableBond::ImpliedVolHelper::operator()(Volatility x) const {
        vol_->setValue(x);
        engine_->calculate(); // get the Black NPV based on vol x
        return results_->value-targetValue_;
    }


    Volatility CallableBond::impliedVolatility(
                              Real targetValue,
                              const Handle<YieldTermStructure>& discountCurve,
                              Real accuracy,
                              Size maxEvaluations,
                              Volatility minVol,
                              Volatility maxVol) const {
        calculate();
        QL_REQUIRE(!isExpired(), "instrument expired");
        Volatility guess = 0.5*(minVol + maxVol);
        blackDiscountCurve_.linkTo(*discountCurve, false);
        ImpliedVolHelper f(*this,targetValue);
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        return solver.solve(f, accuracy, guess, minVol, maxVol);
    }


    CallableFixedRateBond::CallableFixedRateBond(
                              Natural settlementDays,
                              Real faceAmount,
                              const Schedule& schedule,
                              const std::vector<Rate>& coupons,
                              const DayCounter& accrualDayCounter,
                              BusinessDayConvention paymentConvention,
                              Real redemption,
                              const Date& issueDate,
                              const CallabilitySchedule& putCallSchedule)
    : CallableBond(settlementDays, schedule, accrualDayCounter,
                   issueDate, putCallSchedule) {

        frequency_ = schedule.tenor().frequency();

        bool isZeroCouponBond = (coupons.size() == 1 && close(coupons[0], 0.0));

        if (!isZeroCouponBond) {
            cashflows_ =
                FixedRateLeg(schedule)
                .withNotionals(faceAmount)
                .withCouponRates(coupons, accrualDayCounter)
                .withPaymentAdjustment(paymentConvention);

            addRedemptionsToCashflows(std::vector<Real>(1, redemption));
        } else {
            Date redemptionDate = calendar_.adjust(maturityDate_,
                                                   paymentConvention);
            setSingleRedemption(faceAmount, redemption, redemptionDate);
        }

        // used for impliedVolatility() calculation
        boost::shared_ptr<SimpleQuote> dummyVolQuote(new SimpleQuote(0.));
        blackVolQuote_.linkTo(dummyVolQuote);
        blackEngine_ = boost::shared_ptr<PricingEngine>(
                   new BlackCallableFixedRateBondEngine(blackVolQuote_,
                                                        blackDiscountCurve_));
    }


    Real CallableFixedRateBond::accrued(Date settlement) const {

        if (settlement == Date()) settlement = settlementDate();

        const bool IncludeToday = false;
        for (Size i = 0; i<cashflows_.size(); ++i) {
            // the first coupon paying after d is the one we're after
            if (!cashflows_[i]->hasOccurred(settlement,IncludeToday)) {
                boost::shared_ptr<Coupon> coupon =
                    boost::dynamic_pointer_cast<Coupon>(cashflows_[i]);
                if (coupon)
                    // !!!
                    return coupon->accruedAmount(settlement) /
                           notional(settlement) * 100.0;
                else
                    return 0.0;
            }
        }
        return 0.0;
    }


    void CallableFixedRateBond::setupArguments(
                                       PricingEngine::arguments* args) const {

        Bond::setupArguments(args);
        CallableBond::arguments* arguments =
            dynamic_cast<CallableBond::arguments*>(args);

        QL_REQUIRE(arguments != 0, "no arguments given");

        Date settlement = arguments->settlementDate;

        arguments->redemption = redemption()->amount();
        arguments->redemptionDate = redemption()->date();

        const Leg& cfs = cashflows();

        arguments->couponDates.clear();
        arguments->couponDates.reserve(cfs.size()-1);
        arguments->couponAmounts.clear();
        arguments->couponAmounts.reserve(cfs.size()-1);

        for (Size i=0; i<cfs.size()-1; i++) {
            if (!cfs[i]->hasOccurred(settlement, false)) {
                arguments->couponDates.push_back(cfs[i]->date());
                arguments->couponAmounts.push_back(cfs[i]->amount());
            }
        }

        arguments->callabilityPrices.clear();
        arguments->callabilityDates.clear();
        arguments->callabilityPrices.reserve(putCallSchedule_.size());
        arguments->callabilityDates.reserve(putCallSchedule_.size());

        arguments->paymentDayCounter = paymentDayCounter_;
        arguments->frequency = frequency_;

        arguments->putCallSchedule = putCallSchedule_;
        for (Size i=0; i<putCallSchedule_.size(); i++) {
            if (!putCallSchedule_[i]->hasOccurred(settlement, false)) {
                arguments->callabilityDates.push_back(
                                                 putCallSchedule_[i]->date());
                arguments->callabilityPrices.push_back(
                                       putCallSchedule_[i]->price().amount());

                if (putCallSchedule_[i]->price().type()==
                    Callability::Price::Clean) {
                    /* calling accrued() forces accrued interest to be zero
                       if future option date is also coupon date, so that dirty
                       price = clean price. Use here because callability is
                       always applied before coupon in the tree engine.
                    */
                    arguments->callabilityPrices.back() +=
                        this->accrued(putCallSchedule_[i]->date());
                }
            }
        }
    }


    CallableZeroCouponBond::CallableZeroCouponBond(
                              Natural settlementDays,
                              Real faceAmount,
                              const Calendar& calendar,
                              const Date& maturityDate,
                              const DayCounter& dayCounter,
                              BusinessDayConvention paymentConvention,
                              Real redemption,
                              const Date& issueDate,
                              const CallabilitySchedule& putCallSchedule)
    : CallableFixedRateBond(settlementDays,faceAmount,
                            Schedule(issueDate, maturityDate,
                                     Period(Once),
                                     calendar,
                                     paymentConvention,
                                     paymentConvention,
                                     DateGeneration::Backward,
                                     false),
                            std::vector<Rate>(1, 0.0), dayCounter,
                            paymentConvention, redemption,
                            issueDate, putCallSchedule) {}

}

