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

#include <ql/experimental/credit/integralcdoengine.hpp>

#ifndef QL_PATCH_SOLARIS

#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    void IntegralCDOEngine::calculate() const {
        Date today = Settings::instance().evaluationDate();

        results_.protectionValue = 0.0;
        results_.premiumValue = 0.0;
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
             // cast to fixed rate coupon?
            e1 = arguments_.basket->expectedTrancheLoss(
                ext::dynamic_pointer_cast<Coupon>(
                    arguments_.normalizedLeg[0])->accrualStartDate()); 
        results_.expectedTrancheLoss.push_back(e1);// zero or realized losses?

        for (auto& i : arguments_.normalizedLeg) {
            if (i->hasOccurred(today)) {
                // add includeSettlement date flows variable to engine.
                results_.expectedTrancheLoss.push_back(0.);
                continue;
            }

            const ext::shared_ptr<Coupon> coupon = ext::dynamic_pointer_cast<Coupon>(i);

            Date d1 = coupon->accrualStartDate();
            Date d2 = coupon->date();

            Date d, d0 = d1;
            Real e2;
            do {
                d = NullCalendar().advance(d0 > today ? d0 : today,
                                           stepSize_);
                if (d > d2) d = d2;

                e2 = arguments_.basket->expectedTrancheLoss(d);

                results_.premiumValue
                    // ..check for e2 including past/realized losses
                    += (inceptionTrancheNotional - e2)
                    * arguments_.runningRate
                    * arguments_.dayCounter.yearFraction(d0, d)
                    * discountCurve_->discount(d);

                // TO DO: Addd default coupon accrual value here-----

                if (e2 < e1) results_.error ++;

                results_.protectionValue
                    += (e2 - e1) * discountCurve_->discount(d);

                d0 = d;
                e1 = e2;
            }
            while (d < d2);
            results_.expectedTrancheLoss.push_back(e2);
        }

        // add includeSettlement date flows variable to engine.
        if (!arguments_.normalizedLeg[0]->hasOccurred(today))
            results_.upfrontPremiumValue
                = inceptionTrancheNotional * arguments_.upfrontRate
                    * discountCurve_->discount(
                        ext::dynamic_pointer_cast<Coupon>(
                            arguments_.normalizedLeg[0])->accrualStartDate());

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
