/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl

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

#include <ql/instruments/vanillaswap.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>
#include <ql/yieldtermstructure.hpp>

namespace QuantLib {

    VanillaSwap::VanillaSwap(Type type,
                             Real nominal,
                             const Schedule& fixedSchedule,
                             Rate fixedRate,
                             const DayCounter& fixedDayCount,
                             const Schedule& floatSchedule,
                             const boost::shared_ptr<IborIndex>& index,
                             Spread spread,
                             const DayCounter& floatingDayCount,
                             const Handle<YieldTermStructure>& termStructure)
    : Swap(termStructure, Leg(), Leg()),
      type_(type), fixedRate_(fixedRate), spread_(spread),
      nominal_(nominal)
    {
        BusinessDayConvention convention =
            floatSchedule.businessDayConvention();

        Leg fixedLeg = FixedRateLeg(std::vector<Real>(1,nominal),
                                    fixedSchedule,
                                    std::vector<Rate>(1,fixedRate),
                                    fixedDayCount,
                                    convention);

        Leg floatingLeg = IborLeg(std::vector<Real>(1,nominal),
                                  floatSchedule,
                                  index,
                                  floatingDayCount,
                                  convention,
                                  std::vector<Natural>(1,index->fixingDays()),
                                  std::vector<Real>(1,1.0),
                                  std::vector<Spread>(1,spread));
        boost::shared_ptr<IborCouponPricer> fictitiousPricer(new
            BlackIborCouponPricer(Handle<OptionletVolatilityStructure>()));
        setCouponPricer(floatingLeg, fictitiousPricer);

        Leg::const_iterator i;
        for (i = floatingLeg.begin(); i < floatingLeg.end(); ++i)
            registerWith(*i);

        legs_[0] = fixedLeg;
        legs_[1] = floatingLeg;
        if (type_==Payer) {
            payer_[0]=-1.0;
            payer_[1]=+1.0;
        } else {
            payer_[0]=+1.0;
            payer_[1]=-1.0;
        }
    }

    void VanillaSwap::setupArguments(PricingEngine::arguments* args) const {
        VanillaSwap::arguments* arguments =
            dynamic_cast<VanillaSwap::arguments*>(args);

        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->type = type_;
        arguments->nominal = nominal_;
        // reset in case it's not set later
        arguments->currentFloatingCoupon = Null<Real>();

        Date settlement = discountCurve_->referenceDate();
        DayCounter counter = discountCurve_->dayCounter();

        const Leg& fixedCoupons = fixedLeg();

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
            boost::shared_ptr<IborCoupon> coupon =
                boost::dynamic_pointer_cast<IborCoupon>(floatingCoupons[i]);

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

    void VanillaSwap::performCalculations() const {
        if (engine_) {
            Instrument::performCalculations();
        } else {
            static const Spread basisPoint = 1.0e-4;
            Swap::performCalculations();
            fairRate_ = fixedRate_ - NPV_/(legBPS_[0]/basisPoint);
            fairSpread_ = spread_ - NPV_/(legBPS_[1]/basisPoint);
        }
    }

    void VanillaSwap::fetchResults(const PricingEngine::results* r) const {
        Instrument::fetchResults(r);
        const VanillaSwap::results* results =
            dynamic_cast<const VanillaSwap::results*>(r);
        fairRate_ = results->fairRate;
        fairSpread_ = results->fairSpread;
    }

    void VanillaSwap::arguments::validate() const {
        QL_REQUIRE(nominal != Null<Real>(), "nominal null or not set");
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
