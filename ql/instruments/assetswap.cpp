/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Chiara Fornarola
 Copyright (C) 2007, 2009, 2011 Ferdinando Ametrano
 Copyright (C) 2007, 2009 StatPro Italia srl

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

#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/instruments/assetswap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <utility>

using std::vector;

namespace QuantLib {

    AssetSwap::AssetSwap(bool parSwap,
                         ext::shared_ptr<Bond> bond,
                         Real bondCleanPrice,
                         Real nonParRepayment,
                         Real gearing,
                         const ext::shared_ptr<IborIndex>& iborIndex,
                         Spread spread,
                         const DayCounter& floatingDayCounter,
                         Date dealMaturity,
                         bool payBondCoupon)
    : AssetSwap(payBondCoupon, std::move(bond), bondCleanPrice, iborIndex, spread,
                Schedule(), floatingDayCounter, parSwap, gearing,
                nonParRepayment, dealMaturity) {}

    AssetSwap::AssetSwap(bool payBondCoupon,
                         ext::shared_ptr<Bond> bond,
                         Real bondCleanPrice,
                         const ext::shared_ptr<IborIndex>& iborIndex,
                         Spread spread,
                         Schedule floatSchedule,
                         const DayCounter& floatingDayCounter,
                         bool parSwap,
                         Real gearing,
                         Real nonParRepayment,
                         Date dealMaturity)
    : Swap(2), bond_(std::move(bond)), bondCleanPrice_(bondCleanPrice),
      nonParRepayment_(nonParRepayment), spread_(spread), parSwap_(parSwap) {

        auto overnight = ext::dynamic_pointer_cast<OvernightIndex>(iborIndex);
        if (overnight) {
            QL_REQUIRE(!floatSchedule.empty(),
                       "floating schedule is needed when using an overnight index");
        }

        Schedule schedule = floatSchedule.empty()
            ? Schedule(bond_->settlementDate(),
                       bond_->maturityDate(),
                       iborIndex->tenor(),
                       iborIndex->fixingCalendar(),
                       iborIndex->businessDayConvention(),
                       iborIndex->businessDayConvention(),
                       DateGeneration::Backward,
                       false) // endOfMonth
            : std::move(floatSchedule);

        if (dealMaturity == Date())
            dealMaturity = schedule.back();
        QL_REQUIRE(dealMaturity <= schedule.back(),
                   "deal maturity " << dealMaturity <<
                   " cannot be later than (adjusted) bond maturity " <<
                   schedule.back());
        QL_REQUIRE(dealMaturity > schedule.front(),
                   "deal maturity " << dealMaturity <<
                   " must be later than swap start date " <<
                   schedule.front());

        // the following might become an input parameter
        BusinessDayConvention paymentAdjustment = Following;

        Date finalDate = schedule.calendar().adjust(
            dealMaturity, paymentAdjustment);
        schedule = schedule.until(finalDate);

        // bondCleanPrice must be the (forward) clean price
        // at the floating schedule start date
        upfrontDate_ = schedule.startDate();
        Real dirtyPrice = bondCleanPrice_ + bond_->accruedAmount(upfrontDate_);

        Real notional = bond_->notional(upfrontDate_);
        /* In the market asset swap, the bond is purchased in return for
           payment of the full price. The notional of the floating leg is
           then scaled by the full price. */
        if (!parSwap_)
            notional *= dirtyPrice/100.0;

        /******** Bond leg ********/

        const Leg& bondLeg = bond_->cashflows();
        QL_REQUIRE(!bondLeg.empty(), "no cashflows from bond");

        bool includeOnUpfrontDate = false; // a cash flow on the upfront
                                           // date must be discarded

        // add coupons for the time being, not the redemption
        Leg::const_iterator i;
        for (i = bondLeg.begin(); i < bondLeg.end()-1 && (*i)->date()<=dealMaturity; ++i) {
            if (!(*i)->hasOccurred(upfrontDate_, includeOnUpfrontDate))
                legs_[0].push_back(*i);
        }

        // if we're skipping a cashflow before the redemption
        // and it's a coupon, then add the accrued coupon.
        if (i < bondLeg.end()-1) {
            auto c = ext::dynamic_pointer_cast<Coupon>(*i);
            if (c != nullptr) {
                Real accruedAmount = c->accruedAmount(dealMaturity);
                auto accruedCoupon =
                    ext::make_shared<SimpleCashFlow>(accruedAmount, finalDate);
                legs_[0].push_back(accruedCoupon);
            }
        }

        // add the redemption, or whatever the final payment is
        if (nonParRepayment_ == Null<Real>()) {
            auto redemption = bondLeg.back();
            auto finalFlow =
                ext::make_shared<SimpleCashFlow>(redemption->amount(), finalDate);
            legs_[0].push_back(finalFlow);
            nonParRepayment_ = 100.0;
        } else {
            auto finalFlow =
                ext::make_shared<SimpleCashFlow>(nonParRepayment_, finalDate);
            legs_[0].push_back(finalFlow);
        }

        /******** Floating leg ********/

        if (overnight) {
            legs_[1] =
                OvernightLeg(schedule, overnight)
                .withNotionals(notional)
                .withPaymentAdjustment(paymentAdjustment)
                .withGearings(gearing)
                .withSpreads(spread)
                .withPaymentDayCounter(floatingDayCounter);
        } else {
            legs_[1] =
                IborLeg(std::move(schedule), iborIndex)
                .withNotionals(notional)
                .withPaymentAdjustment(paymentAdjustment)
                .withGearings(gearing)
                .withSpreads(spread)
                .withPaymentDayCounter(floatingDayCounter);
        }

        if (parSwap_) {
            // upfront
            Real upfront = (dirtyPrice-100.0)/100.0 * notional;
            auto upfrontCashFlow =
                ext::make_shared<SimpleCashFlow>(upfront, upfrontDate_);
            legs_[1].insert(legs_[1].begin(), upfrontCashFlow);
            // backpayment (accounts for non-par redemption, if any)
            Real backPayment = notional;
            auto backPaymentCashFlow =
                ext::make_shared<SimpleCashFlow>(backPayment, finalDate);
            legs_[1].push_back(backPaymentCashFlow);
        } else {
            // final notional exchange
            auto finalCashFlow =
                ext::make_shared<SimpleCashFlow>(notional, finalDate);
            legs_[1].push_back(finalCashFlow);
        }

        /******** registration and sides ********/

        for (const auto& leg: legs_)
            for (const auto& c: leg)
                registerWith(c);

        if (payBondCoupon) {
            payer_[0]=-1.0;
            payer_[1]=+1.0;
        } else {
            payer_[0]=+1.0;
            payer_[1]=-1.0;
        }
    }

    void AssetSwap::setupArguments(PricingEngine::arguments* args) const {

        Swap::setupArguments(args);

        auto* arguments = dynamic_cast<AssetSwap::arguments*>(args);

        if (arguments == nullptr) // it's a swap engine...
            return;

        const Leg& fixedCoupons = bondLeg();

        arguments->fixedResetDates = arguments->fixedPayDates =
            vector<Date>(fixedCoupons.size());
        arguments->fixedCoupons = vector<Real>(fixedCoupons.size());

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
            vector<Date>(floatingCoupons.size());
        arguments->floatingAccrualTimes =
            vector<Time>(floatingCoupons.size());
        arguments->floatingSpreads =
            vector<Spread>(floatingCoupons.size());

        for (Size i=0; i<floatingCoupons.size(); ++i) {
            ext::shared_ptr<FloatingRateCoupon> coupon =
                ext::dynamic_pointer_cast<FloatingRateCoupon>(floatingCoupons[i]);

            arguments->floatingResetDates[i] = coupon->accrualStartDate();
            arguments->floatingPayDates[i] = coupon->date();
            arguments->floatingFixingDates[i] = coupon->fixingDate();
            arguments->floatingAccrualTimes[i] = coupon->accrualPeriod();
            arguments->floatingSpreads[i] = coupon->spread();
        }
    }

    Spread AssetSwap::fairSpread() const {
        static const Spread basisPoint = 1.0e-4;
        calculate();
        if (fairSpread_ != Null<Spread>()) {
            return fairSpread_;
        } else if (legBPS_.size() > 1 && legBPS_[1] != Null<Spread>()) {
            fairSpread_ = spread_ - NPV_/legBPS_[1]*basisPoint;
            return fairSpread_;
        } else {
            QL_FAIL("fair spread not available");
        }
    }

    Real AssetSwap::floatingLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_.size() > 1 && legBPS_[1] != Null<Real>(),
                   "floating-leg BPS not available");
        return legBPS_[1];
    }

    Real AssetSwap::floatingLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_.size() > 1 && legNPV_[1] != Null<Real>(),
                   "floating-leg NPV not available");
        return legNPV_[1];
    }

    Real AssetSwap::fairCleanPrice() const {
        calculate();
        if (fairCleanPrice_ != Null<Real>()) {
            return fairCleanPrice_;
        } else {
            QL_REQUIRE(startDiscounts_[1]!=Null<DiscountFactor>(),
                       "fair clean price not available for seasoned deal");
            Real notional = bond_->notional(upfrontDate_);
            if (parSwap_) {
                fairCleanPrice_ = bondCleanPrice_ - payer_[1] *
                    NPV_*npvDateDiscount_/startDiscounts_[1]/(notional/100.0);
            } else {
                Real accruedAmount = bond_->accruedAmount(upfrontDate_);
                Real dirtyPrice = bondCleanPrice_ + accruedAmount;
                Real fairDirtyPrice = - legNPV_[0]/legNPV_[1] * dirtyPrice;
                fairCleanPrice_ = fairDirtyPrice - accruedAmount;
            }

            return fairCleanPrice_;
        }
    }

    Real AssetSwap::fairNonParRepayment() const {
        calculate();
        if (fairNonParRepayment_ != Null<Real>()) {
            return fairNonParRepayment_;
        } else {
            QL_REQUIRE(endDiscounts_[1]!=Null<DiscountFactor>(),
                       "fair non par repayment not available for expired leg");
            Real notional = bond_->notional(upfrontDate_);
            fairNonParRepayment_ = nonParRepayment_ - payer_[0] *
                NPV_*npvDateDiscount_/endDiscounts_[1]/(notional/100.0);
            return fairNonParRepayment_;
        }
    }

    void AssetSwap::setupExpired() const {
        Swap::setupExpired();
        fairSpread_ = Null<Spread>();
        fairCleanPrice_ = Null<Real>();
        fairNonParRepayment_ = Null<Real>();
    }

    void AssetSwap::fetchResults(const PricingEngine::results* r) const {
        Swap::fetchResults(r);
        const auto* results = dynamic_cast<const AssetSwap::results*>(r);
        if (results != nullptr) {
            fairSpread_ = results->fairSpread;
            fairCleanPrice_= results->fairCleanPrice;
            fairNonParRepayment_= results->fairNonParRepayment;
        } else {
            fairSpread_ = Null<Spread>();
            fairCleanPrice_ = Null<Real>();
            fairNonParRepayment_ = Null<Real>();
        }
    }

    void AssetSwap::arguments::validate() const {
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
                   "number of floating accrual times different from "
                   "number of floating payment dates");
        QL_REQUIRE(floatingSpreads.size() == floatingPayDates.size(),
                   "number of floating spreads different from "
                   "number of floating payment dates");
    }

    void AssetSwap::results::reset() {
        Swap::results::reset();
        fairSpread = Null<Spread>();
        fairCleanPrice = Null<Real>();
        fairNonParRepayment = Null<Real>();
    }

}
