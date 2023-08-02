/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2007 Ferdinando Ametrano

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

#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/optional.hpp>
#include <utility>

namespace QuantLib {

    VanillaSwap::VanillaSwap(Type type,
                             Real nominal,
                             Schedule fixedSchedule,
                             Rate fixedRate,
                             DayCounter fixedDayCount,
                             Schedule floatSchedule,
                             ext::shared_ptr<IborIndex> iborIndex,
                             Spread spread,
                             DayCounter floatingDayCount,
                             ext::optional<BusinessDayConvention> paymentConvention,
                             ext::optional<bool> useIndexedCoupons,
                             const ext::shared_ptr<FloatingRateCouponPricer>& pricer)
    : Swap(2), type_(type), nominal_(nominal), fixedSchedule_(std::move(fixedSchedule)),
      fixedRate_(fixedRate), fixedDayCount_(std::move(fixedDayCount)),
      floatingSchedule_(std::move(floatSchedule)), iborIndex_(std::move(iborIndex)),
      spread_(spread), floatingDayCount_(std::move(floatingDayCount)) {

        if (paymentConvention) // NOLINT(readability-implicit-bool-conversion)
            paymentConvention_ = *paymentConvention;
        else
            paymentConvention_ = floatingSchedule_.businessDayConvention();

        legs_[0] = FixedRateLeg(fixedSchedule_)
            .withNotionals(nominal_)
            .withCouponRates(fixedRate_, fixedDayCount_)
            .withPaymentAdjustment(paymentConvention_);

        legs_[1] = IborLeg(floatingSchedule_, iborIndex_, pricer)
            .withNotionals(nominal_)
            .withPaymentDayCounter(floatingDayCount_)
            .withPaymentAdjustment(paymentConvention_)
            .withSpreads(spread_)
            .withIndexedCoupons(useIndexedCoupons);
        for (Leg::const_iterator i = legs_[1].begin(); i < legs_[1].end(); ++i)
            registerWith(*i);

        switch (type_) {
          case Payer:
            payer_[0] = -1.0;
            payer_[1] = +1.0;
            break;
          case Receiver:
            payer_[0] = +1.0;
            payer_[1] = -1.0;
            break;
          default:
            QL_FAIL("Unknown vanilla-swap type");
        }
    }

    void VanillaSwap::setupArguments(PricingEngine::arguments* args) const {

        Swap::setupArguments(args);

        auto* arguments = dynamic_cast<VanillaSwap::arguments*>(args);

        if (arguments == nullptr) // it's a swap engine...
            return;

        arguments->type = type_;
        arguments->nominal = nominal_;

        const Leg& fixedCoupons = fixedLeg();

        arguments->fixedResetDates = arguments->fixedPayDates =
            std::vector<Date>(fixedCoupons.size());
        arguments->fixedCoupons = std::vector<Real>(fixedCoupons.size());

        for (Size i=0; i<fixedCoupons.size(); ++i) {
            ext::shared_ptr<FixedRateCoupon> coupon =
                ext::dynamic_pointer_cast<FixedRateCoupon>(fixedCoupons[i]);

            arguments->fixedPayDates[i] = coupon->date();
            arguments->fixedResetDates[i] = coupon->accrualStartDate();
            arguments->fixedCoupons[i] = coupon->amount();
        }

        const Leg& floatingCoupons = floatingLeg();

        arguments->floatingResetDates = arguments->floatingPayDates =
            arguments->floatingFixingDates =
            std::vector<Date>(floatingCoupons.size());
        arguments->floatingAccrualTimes =
            std::vector<Time>(floatingCoupons.size());
        arguments->floatingSpreads =
            std::vector<Spread>(floatingCoupons.size());
        arguments->floatingCoupons = std::vector<Real>(floatingCoupons.size());
        for (Size i=0; i<floatingCoupons.size(); ++i) {
            ext::shared_ptr<IborCoupon> coupon =
                ext::dynamic_pointer_cast<IborCoupon>(floatingCoupons[i]);

            arguments->floatingResetDates[i] = coupon->accrualStartDate();
            arguments->floatingPayDates[i] = coupon->date();

            arguments->floatingFixingDates[i] = coupon->fixingDate();
            arguments->floatingAccrualTimes[i] = coupon->accrualPeriod();
            arguments->floatingSpreads[i] = coupon->spread();
            try {
                arguments->floatingCoupons[i] = coupon->amount();
            } catch (Error&) {
                arguments->floatingCoupons[i] = Null<Real>();
            }
        }
    }

    Rate VanillaSwap::fairRate() const {
        calculate();
        QL_REQUIRE(fairRate_ != Null<Rate>(), "result not available");
        return fairRate_;
    }

    Spread VanillaSwap::fairSpread() const {
        calculate();
        QL_REQUIRE(fairSpread_ != Null<Spread>(), "result not available");
        return fairSpread_;
    }

    Real VanillaSwap::fixedLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[0] != Null<Real>(), "result not available");
        return legBPS_[0];
    }

    Real VanillaSwap::floatingLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[1] != Null<Real>(), "result not available");
        return legBPS_[1];
    }

    Real VanillaSwap::fixedLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[0] != Null<Real>(), "result not available");
        return legNPV_[0];
    }

    Real VanillaSwap::floatingLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[1] != Null<Real>(), "result not available");
        return legNPV_[1];
    }

    void VanillaSwap::setupExpired() const {
        Swap::setupExpired();
        legBPS_[0] = legBPS_[1] = 0.0;
        fairRate_ = Null<Rate>();
        fairSpread_ = Null<Spread>();
    }

    void VanillaSwap::fetchResults(const PricingEngine::results* r) const {
        static const Spread basisPoint = 1.0e-4;

        Swap::fetchResults(r);

        const auto* results = dynamic_cast<const VanillaSwap::results*>(r);
        if (results != nullptr) { // might be a swap engine, so no error is thrown
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

    void VanillaSwap::arguments::validate() const {
        Swap::arguments::validate();
        QL_REQUIRE(nominal != Null<Real>(), "nominal null or not set");
        QL_REQUIRE(fixedResetDates.size() == fixedPayDates.size(),
                   "number of fixed start dates different from "
                   "number of fixed payment dates");
        QL_REQUIRE(fixedPayDates.size() == fixedCoupons.size(),
                   "number of fixed payment dates different from "
                   "number of fixed coupon amounts");
        QL_REQUIRE(floatingResetDates.size() == floatingPayDates.size(),
                   "number of floating start dates different from "
                   "number of floating payment dates");
        QL_REQUIRE(floatingFixingDates.size() == floatingPayDates.size(),
                   "number of floating fixing dates different from "
                   "number of floating payment dates");
        QL_REQUIRE(floatingAccrualTimes.size() == floatingPayDates.size(),
                   "number of floating accrual Times different from "
                   "number of floating payment dates");
        QL_REQUIRE(floatingSpreads.size() == floatingPayDates.size(),
                   "number of floating spreads different from "
                   "number of floating payment dates");
        QL_REQUIRE(floatingPayDates.size() == floatingCoupons.size(),
                   "number of floating payment dates different from "
                   "number of floating coupon amounts");
    }

    void VanillaSwap::results::reset() {
        Swap::results::reset();
        fairRate = Null<Rate>();
        fairSpread = Null<Spread>();
    }

}
