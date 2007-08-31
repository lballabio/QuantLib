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
    : Swap(discountCurve, Leg(), Leg()),
      spread_(spread),
      bondCleanPrice_(bondCleanPrice) {

        Schedule schedule = floatSch;
        if (floatSch.empty()) {
            Date refDate = discountCurve->referenceDate();
            schedule = Schedule(bond->settlementDate(refDate),
                                bond->maturityDate(),
                                index->tenor(),
                                index->fixingCalendar(),
                                index->businessDayConvention(),
                                index->businessDayConvention(),
                                true,
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

        legs_[1] = IborLeg(nominals,
                           schedule,
                           index,
                           floatingDayCounter,
                           paymentAdjustment,
                           std::vector<Natural>(1,fixingDays),
                           gearings, spreads);

        boost::shared_ptr<IborCouponPricer> fictitiousPricer(new
            BlackIborCouponPricer(Handle<CapletVolatilityStructure>()));
        setCouponPricer(legs_[1], fictitiousPricer);

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
    }

    void AssetSwap::setupArguments(PricingEngine::arguments* args) const {
        AssetSwap::arguments* arguments =
            dynamic_cast<AssetSwap::arguments*>(args);

        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->nominal = nominal_;
        // reset in case it's not set later
        arguments->currentFloatingCoupon = Null<Real>();

        Date settlement = discountCurve_->referenceDate();
        DayCounter counter = discountCurve_->dayCounter();
        const Leg& fixedCoupons = bondLeg();

        arguments->fixedResetTimes = arguments->fixedPayTimes =
            std::vector<Time>(fixedCoupons.size());
        arguments->fixedCoupons = std::vector<Real>(fixedCoupons.size());

        for (Size i=0; i<fixedCoupons.size(); ++i) {
            boost::shared_ptr<FixedRateCoupon> coupon =
                boost::dynamic_pointer_cast<FixedRateCoupon>(fixedCoupons[i]);

            Time time = counter.yearFraction(settlement, coupon->date());
            arguments->fixedPayTimes[i] = time;
            time = counter.yearFraction(settlement,
                                        coupon->accrualStartDate());
            arguments->fixedResetTimes[i] = time;
            arguments->fixedCoupons[i] = coupon->amount();
        }

        const Leg& floatingCoupons = floatingLeg();

        arguments->floatingResetTimes = arguments->floatingPayTimes =
            arguments->floatingFixingTimes = arguments->floatingAccrualTimes =
            std::vector<Time>(floatingCoupons.size());
        arguments->floatingSpreads =
            std::vector<Spread>(floatingCoupons.size());

        for (Size i=0; i<floatingCoupons.size(); ++i) {
            boost::shared_ptr<FloatingRateCoupon> coupon =
                boost::dynamic_pointer_cast<FloatingRateCoupon>(
                                                          floatingCoupons[i]);

            Date resetDate = coupon->accrualStartDate(); // already adjusted
            Time resetTime = counter.yearFraction(settlement, resetDate);
            arguments->floatingResetTimes[i] = resetTime;
            Time paymentTime =
                counter.yearFraction(settlement, coupon->date());
            arguments->floatingPayTimes[i] = paymentTime;
            Time floatingFixingTime =
                counter.yearFraction(settlement, coupon->fixingDate());
            arguments->floatingFixingTimes[i] = floatingFixingTime;
            arguments->floatingAccrualTimes[i] = coupon->accrualPeriod();
            arguments->floatingSpreads[i] = coupon->spread();
            if (resetTime < 0.0 && paymentTime >= 0.0)
                arguments->currentFloatingCoupon = coupon->amount();
        }
    }

    Spread AssetSwap::fairSpread() const {
        calculate();
        QL_REQUIRE(fairSpread_ != Null<Spread>(), "fairSpread not available");
        return fairSpread_;
    }

    Real AssetSwap::floatingLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[1] != Null<Real>(), "floatingLegBPS not available");
        return legBPS_[1];
    }

    Real AssetSwap::fairPrice() const {
        calculate();
        QL_REQUIRE(fairPrice_ != Null<Real>(), "fairPrice not available");
        return fairPrice_;
    }
    void AssetSwap::setupExpired() const {
        Swap::setupExpired();
        legBPS_[0] = legBPS_[1] = 0.0;
        fairSpread_ = Null<Spread>();
        fairPrice_= Null<Real>();
    }

    void AssetSwap::performCalculations() const {
        if (engine_) {
            Instrument::performCalculations();
        } else {
            static const Spread basisPoint = 1.0e-4;
            Swap::performCalculations();
            fairSpread_ = spread_ - NPV_/(legBPS_[1]/basisPoint);

            // handle when upfrontDate_ is in the past
            if (upfrontDate_<discountCurve_->referenceDate())
                fairPrice_ = Null<Real>();
            else
                fairPrice_= bondCleanPrice_ - NPV_/(nominal_/100.0)/
                                discountCurve_->discount(upfrontDate_);
        }
    }

    void AssetSwap::fetchResults(const PricingEngine::results* r) const {
        Instrument::fetchResults(r);
        const AssetSwap::results* results =
            dynamic_cast<const AssetSwap::results*>(r);
        fairSpread_ = results->fairSpread;
        fairPrice_= results->fairPrice;
    }

    void AssetSwap::arguments::validate() const {
        QL_REQUIRE(nominal != Null<Real>(),
                   "nominal null or not set");
        QL_REQUIRE(fixedResetTimes.size() == fixedPayTimes.size(),
                   "number of fixed start times different from "
                   "number of fixed payment times");
        QL_REQUIRE(fixedPayTimes.size() == fixedCoupons.size(),
                   "number of fixed payment times different from "
                   "number of fixed coupon amounts");
        QL_REQUIRE(floatingResetTimes.size() == floatingPayTimes.size(),
                   "number of floating start times different from "
                   "number of floating payment times");
        QL_REQUIRE(floatingFixingTimes.size() == floatingPayTimes.size(),
                   "number of floating fixing times different from "
                   "number of floating payment times");
        QL_REQUIRE(floatingAccrualTimes.size() == floatingPayTimes.size(),
                   "number of floating accrual times different from "
                   "number of floating payment times");
        QL_REQUIRE(floatingSpreads.size() == floatingPayTimes.size(),
                   "number of floating spreads different from "
                   "number of floating payment times");
        QL_REQUIRE(currentFloatingCoupon != Null<Real>() || // unless...
                   floatingResetTimes.empty() ||
                   floatingResetTimes[0] >= 0.0,
                   "current floating coupon null or not set");
    }

}
