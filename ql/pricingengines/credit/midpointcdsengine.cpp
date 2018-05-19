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
                   const Handle<YieldTermStructure>& discountCurve,
                   boost::optional<bool> includeSettlementDateFlows)
    : probability_(probability), recoveryRate_(recoveryRate),
      discountCurve_(discountCurve),
      includeSettlementDateFlows_(includeSettlementDateFlows) {
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

        // Upfront Flow NPV and accrual rebate NPV. Either we are on-the-run (no flow)
        // or we are forward start
        //

        // date determining the probability survival so we have to pay
        //   the upfront flows (did not knock out)
        Date effectiveProtectionStart =
            arguments_.protectionStart > probability_->referenceDate() ?
                arguments_.protectionStart : probability_->referenceDate();

        Probability nonKnockOut =
            probability_->survivalProbability(effectiveProtectionStart);

        Real upfPVO1 = 0.0;
        results_.upfrontNPV = 0.0;
        if (!arguments_.upfrontPayment->hasOccurred(
                                               settlementDate,
                                               includeSettlementDateFlows_)) {
            upfPVO1 =
                nonKnockOut *
                discountCurve_->discount(arguments_.upfrontPayment->date());
            if(arguments_.upfrontPayment->amount() != 0.) {
                results_.upfrontNPV =
                    upfPVO1 * arguments_.upfrontPayment->amount();
            }
        }

        results_.accrualRebateNPV = 0.;
        if(arguments_.accrualRebate &&
           arguments_.accrualRebate->amount() != 0. &&
           !arguments_.accrualRebate->hasOccurred(
                                              settlementDate,
                                              includeSettlementDateFlows_)) {
            results_.accrualRebateNPV = nonKnockOut *
                discountCurve_->discount(arguments_.accrualRebate->date()) *
                arguments_.accrualRebate->amount();

        }

        results_.couponLegNPV  = 0.0;
        results_.defaultLegNPV = 0.0;
        for (Size i=0; i<arguments_.leg.size(); ++i) {
            if (arguments_.leg[i]->hasOccurred(settlementDate,
                                               includeSettlementDateFlows_))
                continue;

            boost::shared_ptr<FixedRateCoupon> coupon =
                boost::dynamic_pointer_cast<FixedRateCoupon>(arguments_.leg[i]);

            // In order to avoid a few switches, we calculate the NPV
            // of both legs as a positive quantity. We'll give them
            // the right sign at the end.

            Date paymentDate = coupon->date(),
                 startDate = coupon->accrualStartDate(),
                 endDate = coupon->accrualEndDate();
            // this is the only point where it might not coincide
            if (i==0)
                startDate = arguments_.protectionStart;
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

        Real upfrontSign = 1.0;
        switch (arguments_.side) {
          case Protection::Seller:
            results_.defaultLegNPV *= -1.0;
			results_.accrualRebateNPV *= -1.0;
            break;
          case Protection::Buyer:
            results_.couponLegNPV *= -1.0;
            results_.upfrontNPV   *= -1.0;
            upfrontSign = -1.0;
            break;
          default:
            QL_FAIL("unknown protection side");
        }

        results_.value =
            results_.defaultLegNPV + results_.couponLegNPV +
            results_.upfrontNPV + results_.accrualRebateNPV;
        results_.errorEstimate = Null<Real>();

        if (results_.couponLegNPV != 0.0) {
            results_.fairSpread =
                -results_.defaultLegNPV*arguments_.spread/
                    (results_.couponLegNPV + results_.accrualRebateNPV);
        } else {
            results_.fairSpread = Null<Rate>();
        }

        Real upfrontSensitivity = upfPVO1 * arguments_.notional;
        if (upfrontSensitivity != 0.0) {
            results_.fairUpfront =
                -upfrontSign*(results_.defaultLegNPV + results_.couponLegNPV +
                    results_.accrualRebateNPV)
                / upfrontSensitivity;
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
