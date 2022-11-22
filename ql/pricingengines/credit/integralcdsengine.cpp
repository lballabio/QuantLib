/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008, 2009 StatPro Italia srl
 Copyright (C) 2009 Jose Aparicio

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

#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/instruments/claim.hpp>
#include <ql/pricingengines/credit/integralcdsengine.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    IntegralCdsEngine::IntegralCdsEngine(const Period& step,
                                         Handle<DefaultProbabilityTermStructure> probability,
                                         Real recoveryRate,
                                         Handle<YieldTermStructure> discountCurve,
                                         const boost::optional<bool>& includeSettlementDateFlows)
    : integrationStep_(step), probability_(std::move(probability)), recoveryRate_(recoveryRate),
      discountCurve_(std::move(discountCurve)),
      includeSettlementDateFlows_(includeSettlementDateFlows) {
        registerWith(probability_);
        registerWith(discountCurve_);
    }

    void IntegralCdsEngine::calculate() const {
        QL_REQUIRE(integrationStep_ != Period(),
                   "null period set");
        QL_REQUIRE(!discountCurve_.empty(),
                   "no discount term structure set");
        QL_REQUIRE(!probability_.empty(),
                   "no probability term structure set");

        Date today = Settings::instance().evaluationDate();
        Date settlementDate = discountCurve_->referenceDate();

        // Upfront amount.
        Real upfPVO1 = 0.0;
        results_.upfrontNPV = 0.0;
        if (!arguments_.upfrontPayment->hasOccurred(
            settlementDate, includeSettlementDateFlows_)) {
            upfPVO1 = discountCurve_->discount(arguments_.upfrontPayment->date());
            results_.upfrontNPV = upfPVO1 * arguments_.upfrontPayment->amount();
        }

        // Accrual rebate.
        results_.accrualRebateNPV = 0.;
        // NOLINTNEXTLINE(readability-implicit-bool-conversion)
        if (arguments_.accrualRebate &&
            !arguments_.accrualRebate->hasOccurred(settlementDate, includeSettlementDateFlows_)) {
            results_.accrualRebateNPV =
                discountCurve_->discount(arguments_.accrualRebate->date()) *
                arguments_.accrualRebate->amount();
        }

        results_.couponLegNPV = 0.0;
        results_.defaultLegNPV = 0.0;
        for (Size i=0; i<arguments_.leg.size(); ++i) {
            if (arguments_.leg[i]->hasOccurred(settlementDate,
                                               includeSettlementDateFlows_))
                continue;

            ext::shared_ptr<FixedRateCoupon> coupon =
                ext::dynamic_pointer_cast<FixedRateCoupon>(arguments_.leg[i]);

            // In order to avoid a few switches, we calculate the NPV
            // of both legs as a positive quantity. We'll give them
            // the right sign at the end.

            Date paymentDate = coupon->date(),
                 startDate = (i == 0 ? arguments_.protectionStart :
                                       coupon->accrualStartDate()),
                 endDate = coupon->accrualEndDate();
            Date effectiveStartDate =
                (startDate <= today && today <= endDate) ? today : startDate;
            Real couponAmount = coupon->amount();

            Probability S = probability_->survivalProbability(paymentDate);

            // On one side, we add the fixed rate payments in case of
            // survival.
            results_.couponLegNPV +=
                S * couponAmount * discountCurve_->discount(paymentDate);

            // On the other side, we add the payment (and possibly the
            // accrual) in case of default.

            Period step = integrationStep_;
            Date d0 = effectiveStartDate;
            Date d1 = std::min(d0 + step, endDate);
            Probability P0 = probability_->defaultProbability(d0);
            DiscountFactor endDiscount = discountCurve_->discount(paymentDate);
            do {
                DiscountFactor B =
                    arguments_.paysAtDefaultTime ?
                    discountCurve_->discount(d1) :
                    endDiscount;

                Probability P1 = probability_->defaultProbability(d1);
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
                                                      recoveryRate_);
                results_.defaultLegNPV += claim * B * dP;

                // setup for next time around the loop
                P0 = P1;
                d0 = d1;
                d1 = std::min(d0 + step, endDate);
            } while (d0 < endDate);
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
                -results_.defaultLegNPV*arguments_.spread
                    /(results_.couponLegNPV + results_.accrualRebateNPV);
        } else {
            results_.fairSpread = Null<Rate>();
        }

        if (upfPVO1 > 0.0) {
            results_.fairUpfront =
                -upfrontSign*(results_.defaultLegNPV + results_.couponLegNPV +
                    results_.accrualRebateNPV)
                / (upfPVO1 * arguments_.notional);
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

        // NOLINTNEXTLINE(readability-implicit-bool-conversion)
        if (arguments_.upfront && *arguments_.upfront != 0.0) {
            results_.upfrontBPS =
                results_.upfrontNPV*basisPoint/(*arguments_.upfront);
        } else {
            results_.upfrontBPS = Null<Rate>();
        }
    }

}
