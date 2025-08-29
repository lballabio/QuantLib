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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/fixedvsfloatingswap.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    FixedVsFloatingSwap::FixedVsFloatingSwap(Type type,
                                             std::vector<Real> fixedNominals,
                                             Schedule fixedSchedule,
                                             Rate fixedRate,
                                             DayCounter fixedDayCount,
                                             std::vector<Real> floatingNominals,
                                             Schedule floatingSchedule,
                                             ext::shared_ptr<IborIndex> iborIndex,
                                             Spread spread,
                                             DayCounter floatingDayCount,
                                             ext::optional<BusinessDayConvention> paymentConvention,
                                             Integer paymentLag,
                                             const Calendar& paymentCalendar)
    : Swap(2), type_(type), fixedNominals_(std::move(fixedNominals)), fixedSchedule_(std::move(fixedSchedule)),
      fixedRate_(fixedRate), fixedDayCount_(std::move(fixedDayCount)),
      floatingNominals_(std::move(floatingNominals)), floatingSchedule_(std::move(floatingSchedule)),
      iborIndex_(std::move(iborIndex)), spread_(spread), floatingDayCount_(std::move(floatingDayCount)) {

        QL_REQUIRE(iborIndex_, "null floating index provided");

        if (fixedDayCount_ == DayCounter())
            fixedDayCount_ = iborIndex_->dayCounter();

        if (paymentConvention) // NOLINT(readability-implicit-bool-conversion)
            paymentConvention_ = *paymentConvention;
        else
            paymentConvention_ = floatingSchedule_.businessDayConvention();

        legs_[0] = FixedRateLeg(fixedSchedule_)
            .withNotionals(fixedNominals_)
            .withCouponRates(fixedRate_, fixedDayCount_)
            .withPaymentAdjustment(paymentConvention_)
            .withPaymentLag(paymentLag)
            .withPaymentCalendar(paymentCalendar.empty() ?
                                 fixedSchedule_.calendar() :
                                 paymentCalendar);

        // legs_[1] to be built by derived class constructor

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


        // These bools tell us if we can support the old methods nominal() and nominals().
        // There might be false negatives (i.e., if we pass constant vectors of different lengths
        // as fixedNominals and floatingNominals) but we're going to assume that whoever uses the
        // constructor with two vectors is mostly going to use the new methods instead.
        sameNominals_ = std::equal(fixedNominals_.begin(), fixedNominals_.end(),
                                   floatingNominals_.begin(), floatingNominals_.end());
        if (!sameNominals_) {
            constantNominals_ = false;
        } else {
            constantNominals_ = true;
            Real front = fixedNominals_[0];
            for (auto x : fixedNominals_) {
                if (x != front) {
                    constantNominals_ = false;
                    break;
                }
            }
        }
    }

    void FixedVsFloatingSwap::setupArguments(PricingEngine::arguments* args) const {

        Swap::setupArguments(args);

        auto* arguments = dynamic_cast<FixedVsFloatingSwap::arguments*>(args);

        if (arguments == nullptr) // it's a swap engine...
            return;

        arguments->type = type_;

        if (constantNominals_)
            arguments->nominal = nominal();
        else
            arguments->nominal = Null<Real>();

        const Leg& fixedCoupons = fixedLeg();
        Size n = fixedCoupons.size();

        arguments->fixedResetDates = arguments->fixedPayDates = std::vector<Date>(n);
        arguments->fixedNominals = arguments->fixedCoupons = std::vector<Real>(n);

        for (Size i=0; i<n; ++i) {
            auto coupon = ext::dynamic_pointer_cast<FixedRateCoupon>(fixedCoupons[i]);

            arguments->fixedPayDates[i] = coupon->date();
            arguments->fixedResetDates[i] = coupon->accrualStartDate();
            arguments->fixedCoupons[i] = coupon->amount();
            arguments->fixedNominals[i] = coupon->nominal();
        }

        setupFloatingArguments(arguments);
    }

    Rate FixedVsFloatingSwap::fairRate() const {
        calculate();
        QL_REQUIRE(fairRate_ != Null<Rate>(), "result not available");
        return fairRate_;
    }

    Spread FixedVsFloatingSwap::fairSpread() const {
        calculate();
        QL_REQUIRE(fairSpread_ != Null<Spread>(), "result not available");
        return fairSpread_;
    }

    Real FixedVsFloatingSwap::fixedLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[0] != Null<Real>(), "result not available");
        return legBPS_[0];
    }

    Real FixedVsFloatingSwap::floatingLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[1] != Null<Real>(), "result not available");
        return legBPS_[1];
    }

    Real FixedVsFloatingSwap::fixedLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[0] != Null<Real>(), "result not available");
        return legNPV_[0];
    }

    Real FixedVsFloatingSwap::floatingLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[1] != Null<Real>(), "result not available");
        return legNPV_[1];
    }

    void FixedVsFloatingSwap::setupExpired() const {
        Swap::setupExpired();
        legBPS_[0] = legBPS_[1] = 0.0;
        fairRate_ = Null<Rate>();
        fairSpread_ = Null<Spread>();
    }

    void FixedVsFloatingSwap::fetchResults(const PricingEngine::results* r) const {
        static const Spread basisPoint = 1.0e-4;

        Swap::fetchResults(r);

        const auto* results = dynamic_cast<const FixedVsFloatingSwap::results*>(r);
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

    void FixedVsFloatingSwap::arguments::validate() const {
        Swap::arguments::validate();
        QL_REQUIRE(fixedNominals.size() == fixedPayDates.size(),
                   "number of fixed nominals different from "
                   "number of fixed payment dates");
        QL_REQUIRE(fixedResetDates.size() == fixedPayDates.size(),
                   "number of fixed start dates different from "
                   "number of fixed payment dates");
        QL_REQUIRE(fixedPayDates.size() == fixedCoupons.size(),
                   "number of fixed payment dates different from "
                   "number of fixed coupon amounts");
        QL_REQUIRE(floatingNominals.size() == floatingPayDates.size(),
                   "number of floating nominals different from "
                   "number of floating payment dates");
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

    void FixedVsFloatingSwap::results::reset() {
        Swap::results::reset();
        fairRate = Null<Rate>();
        fairSpread = Null<Spread>();
    }

}
