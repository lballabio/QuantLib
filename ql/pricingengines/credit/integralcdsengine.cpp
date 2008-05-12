/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/pricingengines/credit/integralcdsengine.hpp>
#include <ql/instruments/claim.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>

namespace QuantLib {

    IntegralCdsEngine::IntegralCdsEngine(
                              const Period& step,
                              const Issuer& issuer,
                              const Handle<YieldTermStructure>& discountCurve)
    : integrationStep_(step), issuer_(issuer), discountCurve_(discountCurve) {
        registerWith(issuer_.defaultProbability());
        registerWith(discountCurve_);
    }

    void IntegralCdsEngine::calculate() const {
        QL_REQUIRE(integrationStep_ != Period(),
                   "null period set");
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
            Real couponAmount = coupon->amount();

            Probability S = probabilityCurve->survivalProbability(paymentDate);

            // On one side, we add the fixed rate payments in case of
            // survival.
            results_.couponLegNPV +=
                S * couponAmount * discountCurve_->discount(paymentDate);

            // On the other side, we add the payment (and possibly the
            // accrual) in case of default.

            Period step = integrationStep_;
            Date d0 = effectiveStartDate;
            Date d1 = std::min(d0 + step, endDate);
            Probability P0 = probabilityCurve->defaultProbability(d0);
            DiscountFactor endDiscount = discountCurve_->discount(paymentDate);
            do {
                DiscountFactor B =
                    arguments_.paysAtDefaultTime ?
                    discountCurve_->discount(d1) :
                    endDiscount;

                Probability P1 = probabilityCurve->defaultProbability(d1);
                Probability dP = P1 - P0;

                // accrual...
                if (arguments_.settlesAccrual) {
                    if (arguments_.paysAtDefaultTime)
                        results_.couponLegNPV +=
                            coupon->accruedAmount(d1) * B * dP;
                    else
                        results_.couponLegNPV +=
                            couponAmount * B * dP;
                }

                // ...and claim.
                Real claim = arguments_.claim->amount(d1,
                                                      arguments_.notional,
                                                      issuer_.recoveryRate());
                results_.defaultLegNPV += claim * B * dP;

                // setup for next time around the loop
                P0 = P1;
                d0 = d1;
                d1 = std::min(d0 + step, endDate);
            } while (d0 < endDate);
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

