/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 Chris Kenyon
 Copyright (C) 2009 StatPro Italia srl

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

#include <ql/instruments/yearonyearinflationswap.hpp>
#include <ql/time/schedule.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/yoyinflationcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    YearOnYearInflationSwap::
    YearOnYearInflationSwap(
                            Type type,
                            Real nominal,
                            const Schedule& fixedSchedule,
                            Rate fixedRate,
                            const DayCounter& fixedDayCount,
                            const Schedule& yoySchedule,
                            const boost::shared_ptr<YoYInflationIndex>& yoyIndex,
                            const Period& observationLag,
                            Spread spread,
                            const DayCounter& yoyDayCount,
                            const Calendar& paymentCalendar,
                            BusinessDayConvention paymentConvention)
    : Swap(2), type_(type), nominal_(nominal),
    fixedSchedule_(fixedSchedule), fixedRate_(fixedRate),
    fixedDayCount_(fixedDayCount),
    yoySchedule_(yoySchedule), yoyIndex_(yoyIndex),
    observationLag_(observationLag),
    spread_(spread),
    yoyDayCount_(yoyDayCount), paymentCalendar_(paymentCalendar),
    paymentConvention_(paymentConvention)
    {
        // N.B. fixed leg gets its calendar from the schedule!
        Leg fixedLeg = FixedRateLeg(fixedSchedule_)
        .withNotionals(nominal_)
        .withCouponRates(fixedRate_, fixedDayCount_) // Simple compounding by default
        .withPaymentAdjustment(paymentConvention_);

        Leg yoyLeg = yoyInflationLeg(yoySchedule_, paymentCalendar_, yoyIndex_, observationLag_)
        .withNotionals(nominal_)
        .withPaymentDayCounter(yoyDayCount_)
        .withPaymentAdjustment(paymentConvention_)
        .withSpreads(spread_);

        Leg::const_iterator i;
        for (i = yoyLeg.begin(); i < yoyLeg.end(); ++i)
            registerWith(*i);

        legs_[0] = fixedLeg;
        legs_[1] = yoyLeg;
        if (type_==Payer) {
            payer_[0] = -1.0;
            payer_[1] = +1.0;
        } else {
            payer_[0] = +1.0;
            payer_[1] = -1.0;
        }
    }


     void YearOnYearInflationSwap::setupArguments(PricingEngine::arguments* args) const {

        Swap::setupArguments(args);

        YearOnYearInflationSwap::arguments* arguments =
        dynamic_cast<YearOnYearInflationSwap::arguments*>(args);

        if (!arguments)  // it's a swap engine...
            return;

        arguments->type = type_;
        arguments->nominal = nominal_;

        const Leg& fixedCoupons = fixedLeg();

        arguments->fixedResetDates = arguments->fixedPayDates =
        std::vector<Date>(fixedCoupons.size());
        arguments->fixedCoupons = std::vector<Real>(fixedCoupons.size());

        for (Size i=0; i<fixedCoupons.size(); ++i) {
            boost::shared_ptr<FixedRateCoupon> coupon =
            boost::dynamic_pointer_cast<FixedRateCoupon>(fixedCoupons[i]);

            arguments->fixedPayDates[i] = coupon->date();
            arguments->fixedResetDates[i] = coupon->accrualStartDate();
            arguments->fixedCoupons[i] = coupon->amount();
        }

        const Leg& yoyCoupons = yoyLeg();

        arguments->yoyResetDates = arguments->yoyPayDates =
        arguments->yoyFixingDates =
        std::vector<Date>(yoyCoupons.size());
        arguments->yoyAccrualTimes =
        std::vector<Time>(yoyCoupons.size());
        arguments->yoySpreads =
        std::vector<Spread>(yoyCoupons.size());
        arguments->yoyCoupons = std::vector<Real>(yoyCoupons.size());
        for (Size i=0; i<yoyCoupons.size(); ++i) {
            boost::shared_ptr<YoYInflationCoupon> coupon =
            boost::dynamic_pointer_cast<YoYInflationCoupon>(yoyCoupons[i]);

            arguments->yoyResetDates[i] = coupon->accrualStartDate();
            arguments->yoyPayDates[i] = coupon->date();

            arguments->yoyFixingDates[i] = coupon->fixingDate();
            arguments->yoyAccrualTimes[i] = coupon->accrualPeriod();
            arguments->yoySpreads[i] = coupon->spread();
            try {
                arguments->yoyCoupons[i] = coupon->amount();
            } catch (Error&) {
                arguments->yoyCoupons[i] = Null<Real>();
            }
        }
    }


    Rate YearOnYearInflationSwap::fairRate() const {
        calculate();
        QL_REQUIRE(fairRate_ != Null<Rate>(), "result not available");
        return fairRate_;
    }

    Spread YearOnYearInflationSwap::fairSpread() const {
        calculate();
        QL_REQUIRE(fairSpread_ != Null<Spread>(), "result not available");
        return fairSpread_;
    }


    Real YearOnYearInflationSwap::fixedLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[0] != Null<Real>(), "result not available");
        return legNPV_[0];
    }

    Real YearOnYearInflationSwap::yoyLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[1] != Null<Real>(), "result not available");
        return legNPV_[1];
    }

    void YearOnYearInflationSwap::setupExpired() const {
        Swap::setupExpired();
        legBPS_[0] = legBPS_[1] = 0.0;
        fairRate_ = Null<Rate>();
        fairSpread_ = Null<Spread>();
    }

    void YearOnYearInflationSwap::fetchResults(const PricingEngine::results* r) const {
        static const Spread basisPoint = 1.0e-4;

        // copy from VanillaSwap
        // works because similarly simple instrument
        // that we always expect to be priced with a swap engine

        Swap::fetchResults(r);

        const YearOnYearInflationSwap::results* results =
        dynamic_cast<const YearOnYearInflationSwap::results*>(r);
        if (results) { // might be a swap engine, so no error is thrown
            fairRate_ = results->fairRate;
            fairSpread_ = results->fairSpread;
        } else {
            fairRate_ = Null<Rate>();
            fairSpread_ = Null<Spread>();
        }

        if (fairRate_ == Null<Rate>()) {
            // calculate it from other results
            if (legBPS_[0] != Null<Real>())
                fairRate_ = fixedRate_ - NPV_/(legBPS_[0]/basisPoint);
        }
        if (fairSpread_ == Null<Spread>()) {
            // ditto
            if (legBPS_[1] != Null<Real>())
                fairSpread_ = spread_ - NPV_/(legBPS_[1]/basisPoint);
        }

    }

    void YearOnYearInflationSwap::arguments::validate() const {
        Swap::arguments::validate();
        QL_REQUIRE(nominal != Null<Real>(), "nominal null or not set");
        QL_REQUIRE(fixedResetDates.size() == fixedPayDates.size(),
                   "number of fixed start dates different from "
                   "number of fixed payment dates");
        QL_REQUIRE(fixedPayDates.size() == fixedCoupons.size(),
                   "number of fixed payment dates different from "
                   "number of fixed coupon amounts");
        QL_REQUIRE(yoyResetDates.size() == yoyPayDates.size(),
                   "number of yoy start dates different from "
                   "number of yoy payment dates");
        QL_REQUIRE(yoyFixingDates.size() == yoyPayDates.size(),
                   "number of yoy fixing dates different from "
                   "number of yoy payment dates");
        QL_REQUIRE(yoyAccrualTimes.size() == yoyPayDates.size(),
                   "number of yoy accrual Times different from "
                   "number of yoy payment dates");
        QL_REQUIRE(yoySpreads.size() == yoyPayDates.size(),
                   "number of yoy spreads different from "
                   "number of yoy payment dates");
        QL_REQUIRE(yoyPayDates.size() == yoyCoupons.size(),
                   "number of yoy payment dates different from "
                   "number of yoy coupon amounts");
    }

    void YearOnYearInflationSwap::results::reset() {
        Swap::results::reset();
        fairRate = Null<Rate>();
        fairSpread = Null<Spread>();
    }

    std::ostream& operator<<(std::ostream& out,
                             YearOnYearInflationSwap::Type t) {
        switch (t) {
            case YearOnYearInflationSwap::Payer:
                return out << "Payer";
            case YearOnYearInflationSwap::Receiver:
                return out << "Receiver";
            default:
                QL_FAIL("unknown VanillaSwap::Type(" << Integer(t) << ")");
        }
    }

}

