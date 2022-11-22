/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2018 Sebastian Schlenkrich

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

/*! \file swaptioncfs.cpp
    \brief translate swaption into deterministic fixed and float cash flows
*/

#include <ql/experimental/basismodels/swaptioncfs.hpp>
#include <ql/cashflows/coupon.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/exercise.hpp>
#include <ql/settings.hpp>


namespace QuantLib {

    IborLegCashFlows::IborLegCashFlows(const Leg& iborLeg,
                                       const Handle<YieldTermStructure>& discountCurve,
                                       bool contTenorSpread)
    : refDate_(discountCurve->referenceDate()) {
        // we need to find the first coupon for initial payment
        Size floatIdx = 0;
        while (
            (floatIdx + 1 < iborLeg.size()) &&
            (refDate_ > (ext::dynamic_pointer_cast<Coupon>(iborLeg[floatIdx]))->accrualStartDate()))
            ++floatIdx;
        if (refDate_ <= (ext::dynamic_pointer_cast<Coupon>(iborLeg[floatIdx]))
                            ->accrualStartDate()) { // otherwise there is no floating coupon left
            ext::shared_ptr<Coupon> firstFloatCoupon =
                ext::dynamic_pointer_cast<Coupon>(iborLeg[floatIdx]);
            floatLeg_.push_back(ext::shared_ptr<CashFlow>(new SimpleCashFlow(
                firstFloatCoupon->nominal(), firstFloatCoupon->accrualStartDate())));
            // calculate spread payments
            for (Size k = floatIdx; k < iborLeg.size(); ++k) {
                ext::shared_ptr<Coupon> coupon = ext::dynamic_pointer_cast<Coupon>(iborLeg[k]);
                if (!coupon)
                    QL_FAIL("FloatingLeg CashFlow is no Coupon.");
                Date startDate = coupon->accrualStartDate();
                Date endDate = coupon->accrualEndDate();
                Rate liborForwardRate = coupon->rate();
                Rate discForwardRate =
                    (discountCurve->discount(startDate) / discountCurve->discount(endDate) - 1.0) /
                    coupon->accrualPeriod();
                Rate spread;
                Date payDate;
                if (contTenorSpread) {
                    // Db = (1 + Delta L^libor) / (1 + Delta L^ois)
                    // spread (Db - 1) paid at startDate
                    spread = ((1.0 + coupon->accrualPeriod() * liborForwardRate) /
                                  (1.0 + coupon->accrualPeriod() * discForwardRate) -
                              1.0) /
                             coupon->accrualPeriod();
                    payDate = startDate;
                } else {
                    // spread L^libor - L^ois
                    spread = liborForwardRate - discForwardRate;
                    payDate = coupon->date();
                }
                floatLeg_.push_back(ext::shared_ptr<CashFlow>(new FixedRateCoupon(
                    payDate, coupon->nominal(), spread, coupon->dayCounter(), startDate, endDate)));
            } // for ...
              // finally, add the notional at the last date
            ext::shared_ptr<Coupon> lastFloatCoupon =
                ext::dynamic_pointer_cast<Coupon>(iborLeg.back());
            floatLeg_.push_back(ext::shared_ptr<CashFlow>(new SimpleCashFlow(
                -1.0 * lastFloatCoupon->nominal(), lastFloatCoupon->accrualEndDate())));
        } // if ...
        // assemble raw cash flow data...
        Actual365Fixed dc;
        // ... float times/weights
        for (auto& k : floatLeg_)
            floatTimes_.push_back(dc.yearFraction(refDate_, k->date()));
        for (auto& k : floatLeg_)
            floatWeights_.push_back(k->amount());
    }

    SwapCashFlows::SwapCashFlows(const ext::shared_ptr<VanillaSwap>& swap,
                                 const Handle<YieldTermStructure>& discountCurve,
                                 bool contTenorSpread)
    : IborLegCashFlows(swap->floatingLeg(), discountCurve, contTenorSpread) {
        // copy fixed leg coupons
        Leg fixedLeg = swap->fixedLeg();
        for (auto& k : fixedLeg) {
            if (ext::dynamic_pointer_cast<Coupon>(k)->accrualStartDate() >= refDate_)
                fixedLeg_.push_back(k);
        }
        Actual365Fixed dc;
        // ... fixed times/weights
        for (auto& k : fixedLeg_)
            fixedTimes_.push_back(dc.yearFraction(refDate_, k->date()));
        for (auto& k : fixedLeg_)
            fixedWeights_.push_back(k->amount());
        for (auto& k : fixedLeg_) {
            ext::shared_ptr<Coupon> coupon = ext::dynamic_pointer_cast<Coupon>(k);
            if (coupon != nullptr)
                annuityWeights_.push_back(coupon->nominal() * coupon->accrualPeriod());
        }
    }


    // constructor to map a swaption to deterministic fixed and floating leg cash flows
    SwaptionCashFlows::SwaptionCashFlows(const ext::shared_ptr<Swaption>& swaption,
                                         const Handle<YieldTermStructure>& discountCurve,
                                         bool contTenorSpread)
    : SwapCashFlows(swaption->underlyingSwap(), discountCurve, contTenorSpread),
      swaption_(swaption) {
        // assemble raw cash flow data...
        Actual365Fixed dc;
        // ... exercise times
        for (Size k = 0; k < swaption_->exercise()->dates().size(); ++k)
            if (swaption_->exercise()->dates()[k] > refDate_) // consider only future exercise dates
                exerciseTimes_.push_back(
                    dc.yearFraction(refDate_, swaption_->exercise()->dates()[k]));
    }


}
