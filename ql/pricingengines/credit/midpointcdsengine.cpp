/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 Jose Aparicio
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008, 2009 StatPro Italia srl

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

#include <ql/pricingengines/credit/midpointcdsengine.hpp>
#include <ql/instruments/claim.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>

namespace QuantLib {

    MidPointCdsEngine::MidPointCdsEngine(
                   const Handle<DefaultProbabilityTermStructure>& probability,
                   Real recoveryRate,
                   const Handle<YieldTermStructure>& discountCurve)
    : probability_(probability), recoveryRate_(recoveryRate),
      discountCurve_(discountCurve) {
        registerWith(probability_);
        registerWith(discountCurve_);
    }

    void MidPointCdsEngine::calculate() const {
        QL_REQUIRE(!discountCurve_.empty(),
                   "no discount term structure set");
        QL_REQUIRE(!probability_.empty(),
                   "no probability term structure set");

        Date today = Settings::instance().evaluationDate();
        Date settlementDate = discountCurve_->referenceDate();

        // Upfront Flow NPV. Either we are on-the-run (no flow)
        // or we are forward start
        Real upfPVO1 = 0.0;
        if(!arguments_.upfrontPayment->hasOccurred(settlementDate, true))
            upfPVO1 =
                probability_->survivalProbability(settlementDate) *
                discountCurve_->discount(settlementDate);
        results_.upfrontNPV = upfPVO1 * arguments_.upfrontPayment->amount();

        results_.couponLegNPV  = 0.0;
        results_.defaultLegNPV = 0.0;
        for (Size i=0; i<arguments_.leg.size(); ++i) {
            if (arguments_.leg[i]->hasOccurred(settlementDate))
                continue;

            boost::shared_ptr<FixedRateCoupon> coupon =
                boost::dynamic_pointer_cast<FixedRateCoupon>(arguments_.leg[i]);

            // In order to avoid a few switches, we calculate the NPV
            // of both legs as a positive quantity. We'll give them
            // the right sign at the end.

            Date paymentDate = coupon->date(),
                 startDate = coupon->accrualStartDate(),
                 endDate = coupon->accrualEndDate();
            Date effectiveStartDate =
                (startDate <= today && today <= endDate) ? today : startDate;
            Date defaultDate = // mid-point
                effectiveStartDate + (endDate-effectiveStartDate)/2;

            Probability S = probability_->survivalProbability(paymentDate);
            Probability P = probability_->defaultProbability(
                                                effectiveStartDate,
                                                endDate);

            // on one side, we add the fixed rate payments in case of
            // survival...
            results_.couponLegNPV +=
                S * coupon->amount() *
                discountCurve_->discount(paymentDate);
            // ...possibly including accrual in case of default.
            if (arguments_.settlesAccrual) {
                if (arguments_.paysAtDefaultTime) {
                    results_.couponLegNPV +=
                        P * coupon->accruedAmount(defaultDate) *
                        discountCurve_->discount(defaultDate);
                } else {
                    // pays at the end
                    results_.couponLegNPV +=
                        P * coupon->amount() *
                        discountCurve_->discount(paymentDate);
                }
            }

            // on the other side, we add the payment in case of default.
            Real claim = arguments_.claim->amount(defaultDate,
                                                  arguments_.notional,
                                                  recoveryRate_);
            if (arguments_.paysAtDefaultTime) {
                results_.defaultLegNPV +=
                    P * claim * discountCurve_->discount(defaultDate);
            } else {
                results_.defaultLegNPV +=
                    P * claim * discountCurve_->discount(paymentDate);
            }
        }

        switch (arguments_.side) {
          case Protection::Seller:
            results_.defaultLegNPV *= -1.0;
            break;
          case Protection::Buyer:
            results_.couponLegNPV *= -1.0;
            results_.upfrontNPV   *= -1.0;
            break;
          default:
            QL_FAIL("unknown protection side");
        }

        results_.value =
            results_.defaultLegNPV+results_.couponLegNPV+results_.upfrontNPV;
        results_.errorEstimate = Null<Real>();

        if (results_.couponLegNPV != 0.0) {
            results_.fairSpread =
                -results_.defaultLegNPV*arguments_.spread/results_.couponLegNPV;
        } else {
            results_.fairSpread = Null<Rate>();
        }

        if (results_.upfrontNPV != 0.0) {
            results_.fairUpfront =
                -(results_.defaultLegNPV + results_.couponLegNPV)
                * (*arguments_.upfront) / results_.upfrontNPV;
        } else {
            results_.fairUpfront = Null<Rate>();
        }

        static const Rate basisPoint = 1.0e-4;

        if (arguments_.spread != 0.0) {
            results_.couponLegBPS =
                results_.couponLegNPV*basisPoint/arguments_.spread;
        } else {
            results_.couponLegBPS = Null<Rate>();
        }

        if (arguments_.upfront && *arguments_.upfront != 0.0) {
            results_.upfrontBPS =
                results_.upfrontNPV*basisPoint/(*arguments_.upfront);
        } else {
            results_.upfrontBPS = Null<Rate>();
        }
    }

}

