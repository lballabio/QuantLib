/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2009, 2014 Jose Aparicio

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

#include <ql/experimental/credit/midpointcdoengine.hpp>

#ifndef QL_PATCH_SOLARIS

#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    void MidPointCDOEngine::calculate() const {
        Date today = Settings::instance().evaluationDate();

        results_.premiumValue = 0.0;
        results_.protectionValue = 0.0;
        results_.upfrontPremiumValue = 0.0;
        results_.error = 0;
        results_.expectedTrancheLoss.clear();
        // todo Should be remaining when considering realized loses
        results_.xMin = arguments_.basket->attachmentAmount();
        results_.xMax = arguments_.basket->detachmentAmount();
        results_.remainingNotional = results_.xMax - results_.xMin;
        const Real inceptionTrancheNotional = 
            arguments_.basket->trancheNotional();

        // compute expected loss at the beginning of first relevant period
        Real e1 = 0;
        // todo add includeSettlement date flows variable to engine.
        if (!arguments_.normalizedLeg[0]->hasOccurred(today))
            // Notice that since there might be a gap between the end of 
            // acrrual and payment dates and today be in between
            // the tranche loss on that date might not be contingent but 
            // realized:
            e1 = arguments_.basket->expectedTrancheLoss(
                std::dynamic_pointer_cast<Coupon>(
                    arguments_.normalizedLeg[0])->accrualStartDate());
        results_.expectedTrancheLoss.push_back(e1);
        //'e1'  should contain the existing loses.....? use remaining amounts?
        for (auto& i : arguments_.normalizedLeg) {
            if (i->hasOccurred(today)) {
                results_.expectedTrancheLoss.push_back(0.);
                continue;
            }
            std::shared_ptr<Coupon> coupon = std::dynamic_pointer_cast<Coupon>(i);
            Date paymentDate = coupon->date();
            Date startDate = std::max(coupon->accrualStartDate(),
                                      discountCurve_->referenceDate());
            Date endDate = coupon->accrualEndDate();
            // we assume the loss within the period took place on this date:
            Date defaultDate = startDate + (endDate-startDate)/2;

            Real e2 = arguments_.basket->expectedTrancheLoss(endDate);
            results_.expectedTrancheLoss.push_back(e2);
            results_.premiumValue += 
                ((inceptionTrancheNotional - e2) / inceptionTrancheNotional)
                * coupon->amount()
                * discountCurve_->discount(paymentDate);
            // default flows:
            const Real discount = discountCurve_->discount(defaultDate);

            /* Accrual removed till the argument flag is implemented
            // pays accrued on defaults' date
            results_.premiumValue += coupon->accruedAmount(defaultDate)
                * discount * (e2 - e1) / inceptionTrancheNotional;
            */
            results_.protectionValue += discount * (e2 - e1);
            /* use it in a future version for coherence with the integral engine
            * arguments_.leverageFactor;
            */
            e1 = e2;
        }

        //\todo treat upfron tnow as in the new CDS (see March 2014)
        // add includeSettlement date flows variable to engine ?
        if (!arguments_.normalizedLeg[0]->hasOccurred(today))
            results_.upfrontPremiumValue 
                = inceptionTrancheNotional * arguments_.upfrontRate 
                    * discountCurve_->discount(
                        std::dynamic_pointer_cast<Coupon>(
                            arguments_.normalizedLeg[0])->accrualStartDate());
            /* use it in a future version for coherence with the integral engine
                arguments_.leverageFactor * ;
            */
        if (arguments_.side == Protection::Buyer) {
            results_.protectionValue *= -1;
            results_.premiumValue *= -1;
            results_.upfrontPremiumValue *= -1;
        }
        results_.value = results_.premiumValue - results_.protectionValue
            + results_.upfrontPremiumValue;
        results_.errorEstimate = Null<Real>();
        // Fair spread GIVEN the upfront
        Real fairSpread = 0.;
        if (results_.premiumValue != 0.0) {
            fairSpread =
                -(results_.protectionValue + results_.upfrontPremiumValue)
                  *arguments_.runningRate/results_.premiumValue;
        }

        results_.additionalResults["fairPremium"] = fairSpread;
        results_.additionalResults["premiumLegNPV"] = 
            Real(results_.premiumValue + results_.upfrontPremiumValue);
        results_.additionalResults["protectionLegNPV"] = 
            results_.protectionValue;
    }

}

#endif
