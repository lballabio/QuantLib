/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2010 Andre Miemiec

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

#include <ql/experimental/swaptions/irregularswap.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    IrregularSwap::IrregularSwap(
                     Type type,
                     const Leg& fixLeg,
                     const Leg& floatLeg)
    : Swap(2), type_(type) {


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
            QL_FAIL("Unknown Irregular-swap type");
        }

        //Fixed leg
        legs_[0] = fixLeg;

        for (auto i = legs_[0].begin(); i < legs_[0].end(); ++i)
            registerWith(*i);
        
        //Floating Leg
        legs_[1] = floatLeg;

        for (auto i = legs_[1].begin(); i < legs_[1].end(); ++i)
            registerWith(*i);

    }


    void IrregularSwap::setupArguments(PricingEngine::arguments* args) const {

    //Debug: to be done

        Swap::setupArguments(args);

        auto* arguments = dynamic_cast<IrregularSwap::arguments*>(args);

        if (arguments == nullptr) // it's a swap engine...
            return;

        arguments->type = type_;
        //arguments->nominal = nominal_;

        
        const Leg& fixedCoupons = fixedLeg();

        arguments->fixedResetDates = arguments->fixedPayDates = std::vector<Date>(fixedCoupons.size());
        arguments->fixedNominals   = arguments->fixedCoupons  = std::vector<Real>(fixedCoupons.size());


        for (Size i=0; i<fixedCoupons.size(); ++i) {
            ext::shared_ptr<FixedRateCoupon> coupon =
                ext::dynamic_pointer_cast<FixedRateCoupon>(fixedCoupons[i]);

            arguments->fixedPayDates[i]   = coupon->date();
            arguments->fixedResetDates[i] = coupon->accrualStartDate();
            arguments->fixedCoupons[i]    = coupon->amount();
            arguments->fixedNominals[i]   = coupon->nominal(); 
        }


        const Leg& floatingCoupons = floatingLeg();

        arguments->floatingResetDates   = arguments->floatingPayDates 
                                        = arguments->floatingFixingDates = std::vector<Date>(floatingCoupons.size());
        arguments->floatingAccrualTimes = std::vector<Time>(floatingCoupons.size());
        arguments->floatingSpreads      = std::vector<Spread>(floatingCoupons.size());
        arguments->floatingNominals     = arguments->floatingCoupons = std::vector<Real>(floatingCoupons.size());

        for (Size i=0; i<floatingCoupons.size(); ++i) {
            ext::shared_ptr<IborCoupon> coupon =
                ext::dynamic_pointer_cast<IborCoupon>(floatingCoupons[i]);

            arguments->floatingResetDates[i]   = coupon->accrualStartDate();
            arguments->floatingPayDates[i]     = coupon->date();

            arguments->floatingFixingDates[i]  = coupon->fixingDate();
            arguments->floatingAccrualTimes[i] = coupon->accrualPeriod();
            arguments->floatingSpreads[i]      = coupon->spread();
            arguments->floatingNominals[i]     = coupon->nominal();

            try {
                arguments->floatingCoupons[i] = coupon->amount();
            } catch (Error&) {
                arguments->floatingCoupons[i] = Null<Real>();
            }
        }
        
    }


    Rate IrregularSwap::fairRate() const {
        calculate();
        QL_REQUIRE(fairRate_ != Null<Rate>(), "result not available");
        return fairRate_;
    }

    Spread IrregularSwap::fairSpread() const {
        calculate();
        QL_REQUIRE(fairSpread_ != Null<Spread>(), "result not available");
        return fairSpread_;
    }

    Real IrregularSwap::fixedLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[0] != Null<Real>(), "result not available");
        return legBPS_[0];
    }

    Real IrregularSwap::floatingLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[1] != Null<Real>(), "result not available");
        return legBPS_[1];
    }

    Real IrregularSwap::fixedLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[0] != Null<Real>(), "result not available");
        return legNPV_[0];
    }

    Real IrregularSwap::floatingLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[1] != Null<Real>(), "result not available");
        return legNPV_[1];
    }

    void IrregularSwap::setupExpired() const {
        Swap::setupExpired();
        legBPS_[0] = legBPS_[1] = 0.0;
        fairRate_ = Null<Rate>();
        fairSpread_ = Null<Spread>();
    }

    void IrregularSwap::fetchResults(const PricingEngine::results* r) const {
        Swap::fetchResults(r);

        const auto* results = dynamic_cast<const IrregularSwap::results*>(r);
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
                fairRate_ = 0.0; // Debug: legs_[0]->fixedRate_ - NPV_/(legBPS_[0]/basisPoint);
        }
        if (fairSpread_ == Null<Spread>()) {
            // ditto
            if (legBPS_[1] != Null<Real>())
                fairSpread_ = 0.0; //DEBUG: spread_ - NPV_/(legBPS_[1]/basisPoint);
        }
    }

    void IrregularSwap::arguments::validate() const {

        Swap::arguments::validate();

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

    void IrregularSwap::results::reset() {
        Swap::results::reset();
        fairRate = Null<Rate>();
        fairSpread = Null<Spread>();
    }

}
