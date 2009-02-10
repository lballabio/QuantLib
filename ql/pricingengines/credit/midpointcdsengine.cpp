/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Jose Aparicio
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl

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
                              const Issuer& issuer,
                              const Handle<YieldTermStructure>& discountCurve)
    : issuer_(issuer), discountCurve_(discountCurve) {
        registerWith(issuer_);
        registerWith(discountCurve_);
    }

    void MidPointCdsEngine::calculate() const {
        QL_REQUIRE(!discountCurve_.empty(),
                   "no discount term structure set");
        QL_REQUIRE(!issuer_.defaultProbability().empty(),
                   "no probability term structure set");

        const Handle<DefaultProbabilityTermStructure>& probabilityCurve =
            issuer_.defaultProbability();

        Date today = Settings::instance().evaluationDate();
        Date settlementDate = discountCurve_->referenceDate();

        results_.couponLegNPV = 0.0;
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

            Probability S = probabilityCurve->survivalProbability(paymentDate);
            Probability P =
                probabilityCurve->defaultProbability(effectiveStartDate,
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
                                                  issuer_.recoveryRate());
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
            break;
          default:
            QL_FAIL("unknown protection side");
        }

        results_.value = results_.defaultLegNPV + results_.couponLegNPV;
        results_.errorEstimate = Null<Real>();

        if (results_.couponLegNPV != 0.0) {
            results_.fairSpread =
                -results_.defaultLegNPV*arguments_.spread/results_.couponLegNPV;
        } else {
            results_.fairSpread = Null<Rate>();
        }

        if (arguments_.spread != 0.0) {
            static const Rate basisPoint = 1.0e-4;
            results_.couponLegBPS =
                results_.couponLegNPV*basisPoint/arguments_.spread;
        } else {
            results_.couponLegBPS = Null<Rate>();
        }
    }

}

