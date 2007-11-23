/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2006, 2007 Chiara Fornarola
 Copyright (C) 2007 StatPro Italia srl

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
                         const Handle<YieldTermStructure>& discountCurve,
                         const Schedule& floatSch,
                         const DayCounter& floatingDayCounter,
                         bool parSwap)
    : Swap(Leg(), Leg()),
      spread_(spread),
      bondCleanPrice_(bondCleanPrice),
      discountCurve_(discountCurve) {

        Schedule schedule = floatSch;
        if (floatSch.empty()) {
            Date refDate = discountCurve_->referenceDate();
            schedule = Schedule(bond->settlementDate(refDate),
                                bond->maturityDate(),
                                index->tenor(),
                                index->fixingCalendar(),
                                index->businessDayConvention(),
                                index->businessDayConvention(),
                                DateGeneration::Backward,
                                index->endOfMonth());
        }

        // what if this date is not a business day??
        // we are assuming it is a business day!
        upfrontDate_ = schedule.startDate();
        Real dirtyPrice = bondCleanPrice_ +
                          bond->accruedAmount(upfrontDate_);

        /* In the market asset swap, the bond is purchased in return for
           payment of the full price. The notional of the floating leg is
           then scaled by the full price, and the resulting value of the
           asset swap spread is different. */
        if (parSwap) nominal_ = bond->faceAmount();
        else         nominal_ = dirtyPrice/100*bond->faceAmount();

        // the following should/might be input parameters
        // -------------------------------------------------
        BusinessDayConvention paymentAdjustment = Following;
        Natural fixingDays = index->fixingDays();
        std::vector<Real> nominals(1, nominal_);
        std::vector<Real> gearings(1, 1.0);
        std::vector<Spread> spreads(1, spread);
        // -------------------------------------------------

        legs_[1] = IborLeg(schedule, index)
            .withNotionals(nominals)
            .withPaymentDayCounter(floatingDayCounter)
            .withPaymentAdjustment(paymentAdjustment)
            .withFixingDays(fixingDays)
            .withGearings(gearings)
            .withSpreads(spreads);

        for (Leg::const_iterator i=legs_[1].begin(); i<legs_[1].end(); ++i)
            registerWith(*i);

        const Leg& bondLeg = bond->cashflows();
        for (Leg::const_iterator i=bondLeg.begin(); i<bondLeg.end(); ++i) {
            if (!(*i)->hasOccurred(upfrontDate_))
                legs_[0].push_back(*i);
        }

        QL_REQUIRE(!legs_[0].empty(),
                   "empty bond leg to start with");

        // what happen if floatSchedule.endDate() < bond->maturityDate() ??

        // special flows
        if (parSwap) {
            // upfront on the floating leg
            Real upfront = (dirtyPrice-100.0)/100.0*nominal_;
            // we are assuming upfrontDate_ is a business day
            // see above!
            boost::shared_ptr<CashFlow> upfrontCashFlow (new
                SimpleCashFlow(upfront, upfrontDate_));
            legs_[1].insert(legs_[1].begin(), upfrontCashFlow);
            // backpayment on the floating leg
            // (accounts for non-par redemption, if any)
            Real backPayment = nominal_;
            Date backPaymentDate = schedule.calendar().adjust(
                schedule.endDate(), paymentAdjustment);
            boost::shared_ptr<CashFlow> backPaymentCashFlow (new
                SimpleCashFlow(backPayment, backPaymentDate));
            legs_[1].push_back(backPaymentCashFlow);
        } else {
            // final nominal exchange
            Real finalFlow = (dirtyPrice)/100.0*bond->faceAmount();
            // we are assuming bond->maturityDate() == schedule.endDate()
            Date finalDate = schedule.calendar().adjust(
                schedule.endDate(), paymentAdjustment);
                //bond->maturityDate(), paymentAdjustment);
            boost::shared_ptr<CashFlow> finalCashFlow (new
                SimpleCashFlow(finalFlow, finalDate));
            legs_[1].push_back(finalCashFlow);
        }

        QL_REQUIRE(!legs_[0].empty(), "empty bond leg");
        for (Leg::const_iterator i=legs_[0].begin(); i<legs_[0].end(); ++i)
            registerWith(*i);

        // handle when termination date is earlier than
        // bond maturity date

        if (payFixedRate) {
            payer_[0]=-1.0;
            payer_[1]=+1.0;
        } else {
            payer_[0]=+1.0;
            payer_[1]=-1.0;
        }

        setPricingEngine(boost::shared_ptr<PricingEngine>(
                                  new DiscountingSwapEngine(discountCurve_)));
    }

    void AssetSwap::setupArguments(PricingEngine::arguments* args) const {

        Swap::setupArguments(args);

        AssetSwap::arguments* arguments =
            dynamic_cast<AssetSwap::arguments*>(args);

        if (!arguments)  // it's a swap engine...
            return;

        arguments->nominal = nominal_;
        arguments->settlementDate = discountCurve_->referenceDate();

        // reset in case it's not set later
        arguments->currentFloatingCoupon = Null<Real>();

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
            if (coupon->accrualStartDate() < arguments->settlementDate
                && coupon->date() >= arguments->settlementDate)
                arguments->currentFloatingCoupon = coupon->amount();
        }
    }

    Spread AssetSwap::fairSpread() const {
        calculate();
        QL_REQUIRE(fairSpread_ != Null<Spread>(), "fair spread not available");
        return fairSpread_;
    }

    Real AssetSwap::floatingLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_.size() > 1 && legBPS_[1] != Null<Real>(),
                   "floating-leg BPS not available");
        return legBPS_[1];
    }

    Real AssetSwap::fairPrice() const {
        calculate();
        QL_REQUIRE(fairPrice_ != Null<Real>(), "fair price not available");
        return fairPrice_;
    }
    void AssetSwap::setupExpired() const {
        Swap::setupExpired();
        fairSpread_ = Null<Spread>();
        fairPrice_= Null<Real>();
    }

    void AssetSwap::performCalculations() const {
        static const Spread basisPoint = 1.0e-4;

        Swap::performCalculations();

        if (fairSpread_ == Null<Spread>()) {
            // the engine didn't provide it; calculate from BPS if available
            if (legBPS_.size() > 1 && legBPS_[1] != Null<Spread>())
                fairSpread_ = spread_ - NPV_/(legBPS_[1]/basisPoint);
        }

        if (fairPrice_ == Null<Real>()) {
            if (upfrontDate_ >= discountCurve_->referenceDate())
                fairPrice_= bondCleanPrice_ - NPV_/(nominal_/100.0)/
                                discountCurve_->discount(upfrontDate_);
        }
    }

    void AssetSwap::fetchResults(const PricingEngine::results* r) const {
        Instrument::fetchResults(r);
        const AssetSwap::results* results =
            dynamic_cast<const AssetSwap::results*>(r);
        if (results) {
            fairSpread_ = results->fairSpread;
            fairPrice_= results->fairPrice;
        } else {
            fairSpread_ = Null<Spread>();
            fairPrice_= Null<Real>();
        }
    }

    void AssetSwap::arguments::validate() const {
        QL_REQUIRE(nominal != Null<Real>(),
                   "nominal null or not set");
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
        QL_REQUIRE(currentFloatingCoupon != Null<Real>() || // unless...
                   floatingResetDates.empty() ||
                   floatingResetDates[0] >= settlementDate,
                   "current floating coupon null or not set");
    }

    void AssetSwap::results::reset() {
        Instrument::results::reset();
        fairSpread = Null<Spread>();
        fairPrice = Null<Real>();
    }

}

