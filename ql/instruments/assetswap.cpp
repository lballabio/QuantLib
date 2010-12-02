/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Chiara Fornarola
 Copyright (C) 2007, 2009 Ferdinando Ametrano
 Copyright (C) 2007, 2009 StatPro Italia srl

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

#include <ql/time/schedule.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/instruments/assetswap.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>

namespace QuantLib {

    AssetSwap::AssetSwap(bool payFixedRate,
                         const boost::shared_ptr<Bond>& bond,
                         Real bondCleanPrice,
                         const boost::shared_ptr<IborIndex>& index,
                         Spread spread,
                         const Schedule& floatSch,
                         const DayCounter& floatingDayCounter,
                         bool parSwap)
    : Swap(2), bond_(bond), bondCleanPrice_(bondCleanPrice),
      spread_(spread), parSwap_(parSwap) {

        Schedule schedule = floatSch;
        if (floatSch.empty()) {
            bool endOfMonth = false;
            schedule = Schedule(bond_->settlementDate(),
                                bond_->maturityDate(),
                                index->tenor(),
                                index->fixingCalendar(),
                                index->businessDayConvention(),
                                index->businessDayConvention(),
                                DateGeneration::Backward,
                                endOfMonth);
        }

        // the following might become an input parameter
        BusinessDayConvention paymentAdjustment = Following;

        Date finalDate = schedule.calendar().adjust(
            schedule.endDate(), paymentAdjustment);
        Date adjBondMaturityDate = schedule.calendar().adjust(
            bond_->maturityDate(), paymentAdjustment);

        QL_REQUIRE(finalDate==adjBondMaturityDate,
                   "adjusted schedule end date (" <<
                   finalDate <<
                   ") must be equal to adjusted bond maturity date (" <<
                   adjBondMaturityDate << ")");

        // bondCleanPrice must be the (forward) clean price
        // at the floating schedule start date
        upfrontDate_ = schedule.startDate();
        Real dirtyPrice = bondCleanPrice_ +
                          bond_->accruedAmount(upfrontDate_);

        Real notional = bond_->notional(upfrontDate_);
        /* In the market asset swap, the bond is purchased in return for
           payment of the full price. The notional of the floating leg is
           then scaled by the full price. */
        if (!parSwap_)
            notional *= dirtyPrice/100.0;

        if (floatingDayCounter==DayCounter())
            legs_[1] = IborLeg(schedule, index)
                .withNotionals(notional)
                .withPaymentAdjustment(paymentAdjustment)
                .withSpreads(spread);
        else
            legs_[1] = IborLeg(schedule, index)
                .withNotionals(notional)
                .withPaymentDayCounter(floatingDayCounter)
                .withPaymentAdjustment(paymentAdjustment)
                .withSpreads(spread);

        for (Leg::const_iterator i=legs_[1].begin(); i<legs_[1].end(); ++i)
            registerWith(*i);

        const Leg& bondLeg = bond_->cashflows();
        for (Leg::const_iterator i=bondLeg.begin(); i<bondLeg.end(); ++i) {
            // whatever might be the choice for the discounting engine
            // bond flows on upfrontDate_ must be discarded
            bool upfrontDateBondFlows = false;
            if (!(*i)->hasOccurred(upfrontDate_, upfrontDateBondFlows))
                legs_[0].push_back(*i);
        }

        QL_REQUIRE(!legs_[0].empty(),
                   "empty bond leg to start with");

        // special flows
        if (parSwap_) {
            // upfront on the floating leg
            Real upfront = (dirtyPrice-100.0)/100.0*notional;
            boost::shared_ptr<CashFlow> upfrontCashFlow (new
                SimpleCashFlow(upfront, upfrontDate_));
            legs_[1].insert(legs_[1].begin(), upfrontCashFlow);
            // backpayment on the floating leg
            // (accounts for non-par redemption, if any)
            Real backPayment = notional;
            boost::shared_ptr<CashFlow> backPaymentCashFlow (new
                SimpleCashFlow(backPayment, finalDate));
            legs_[1].push_back(backPaymentCashFlow);
        } else {
            // final notional exchange
            boost::shared_ptr<CashFlow> finalCashFlow (new
                SimpleCashFlow(notional, finalDate));
            legs_[1].push_back(finalCashFlow);
        }

        QL_REQUIRE(!legs_[0].empty(), "empty bond leg");
        for (Leg::const_iterator i=legs_[0].begin(); i<legs_[0].end(); ++i)
            registerWith(*i);

        if (payFixedRate) {
            payer_[0]=-1.0;
            payer_[1]=+1.0;
        } else {
            payer_[0]=+1.0;
            payer_[1]=-1.0;
        }
    }

    void AssetSwap::setupArguments(PricingEngine::arguments* args) const {

        Swap::setupArguments(args);

        AssetSwap::arguments* arguments =
            dynamic_cast<AssetSwap::arguments*>(args);

        if (!arguments)  // it's a swap engine...
            return;

        const Leg& fixedCoupons = bondLeg();

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

        const Leg& floatingCoupons = floatingLeg();

        arguments->floatingResetDates = arguments->floatingPayDates =
            arguments->floatingFixingDates =
            std::vector<Date>(floatingCoupons.size());
        arguments->floatingAccrualTimes =
            std::vector<Time>(floatingCoupons.size());
        arguments->floatingSpreads =
            std::vector<Spread>(floatingCoupons.size());

        for (Size i=0; i<floatingCoupons.size(); ++i) {
            boost::shared_ptr<FloatingRateCoupon> coupon =
                boost::dynamic_pointer_cast<FloatingRateCoupon>(
                                                          floatingCoupons[i]);

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

    Real AssetSwap::fairCleanPrice() const {
        calculate();
        if (fairCleanPrice_ != Null<Real>()) {
            return fairCleanPrice_;
        } else {
            std::vector<DiscountFactor> dfs;
            DiscountFactor npvDateDiscount;
            try {
                dfs = result<std::vector<DiscountFactor> >("startDiscounts");
                npvDateDiscount = result<DiscountFactor>("npvDateDiscount");
            } catch (...) {
                QL_FAIL("fair clean price not available");
            }

            QL_REQUIRE(dfs[1]!=Null<DiscountFactor>(),
                       "fair clean price not available for seasoned deal");
            Real notional = bond_->notional(upfrontDate_);
            if (parSwap_) {
                fairCleanPrice_ = bondCleanPrice_-NPV_*npvDateDiscount/dfs[1]/(notional/100.0);
            } else {
                Real accruedAmount = bond_->accruedAmount(upfrontDate_);
                Real dirtyPrice = bondCleanPrice_ + accruedAmount;
                Real fairDirtyPrice = - legNPV_[0]/legNPV_[1] * dirtyPrice;
                fairCleanPrice_ = fairDirtyPrice - accruedAmount;
            }

            return fairCleanPrice_;
        }
    }

    void AssetSwap::setupExpired() const {
        Swap::setupExpired();
        fairSpread_ = Null<Spread>();
        fairCleanPrice_= Null<Real>();
    }

    void AssetSwap::fetchResults(const PricingEngine::results* r) const {
        Swap::fetchResults(r);
        const AssetSwap::results* results =
            dynamic_cast<const AssetSwap::results*>(r);
        if (results) {
            fairSpread_ = results->fairSpread;
            fairCleanPrice_= results->fairCleanPrice;
        } else {
            fairSpread_ = Null<Spread>();
            fairCleanPrice_= Null<Real>();
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
    }

}
