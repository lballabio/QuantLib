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
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    void IntegralCDOEngine::calculate() const {
        Date today = Settings::instance().evaluationDate();
   /////      const vector<Date>& dates = arguments_.schedule.dates();

        results_.protectionValue = 0.0;
        results_.premiumValue = 0.0;
        results_.upfrontPremiumValue = 0.0;
        results_.error = 0;
        results_.expectedTrancheLoss.clear();
    ////PP    results_.expectedTrancheLoss.resize(dates.size(), 0.0); ///????????????????????????

        // set remainingBasket_, results_.remainingNotional,
        // vector results_.expectedTrancheLoss for all schedule dates
            results_.xMin = arguments_.basket->attachmentAmount();
            results_.xMax = arguments_.basket->detachmentAmount();
            results_.remainingNotional = results_.xMax - results_.xMin;// compact!

        Real e1 = 0;
        if (!arguments_.normalizedLeg[0]->hasOccurred(today)) // add includeSettlement date flows variable to engine.
//PP        if (arguments_.schedule.dates().front() > today)
//PP            e1 = expectedTrancheLoss (arguments_.schedule.dates()[0]);
     //PP       e1 = arguments_.basket->expectedTrancheLoss(arguments_.normalizedLeg[0]->date());
     // cast to fixed rate coupon rather, it is what it should be, this is not restrictive enough from the contract point of view
            e1 = arguments_.basket->expectedTrancheLoss(
                boost::dynamic_pointer_cast<Coupon>(arguments_.normalizedLeg[0])->accrualEndDate());
        results_.expectedTrancheLoss.push_back(e1);// zero or realized losses!!!!!!!!!!! see below

        for (Size i = 0; i < arguments_.normalizedLeg.size(); i++) {
//PP        for (Size i = 1; i < arguments_.schedule.size(); i++) {
            if(arguments_.normalizedLeg[i]->hasOccurred(today)) {// add includeSettlement date flows variable to engine.
         //PP   Date d2 = arguments_.schedule.dates()[i];
         //PP   if (d2 < today) {
                results_.expectedTrancheLoss.push_back(0.);///// this is ok if talking of the remaining basket, there might be defaults!!!
                continue;
            }

            const boost::shared_ptr<Coupon> coupon =
                boost::dynamic_pointer_cast<Coupon>(arguments_.normalizedLeg[i]);

            Date d1 = coupon->accrualStartDate(); //PP   arguments_.schedule.dates()[i-1];
            Date d2 = coupon->date();// PP xtra

            Date d, d0 = d1;
            Real e2;
            do {
                d = NullCalendar().advance(d0 > today ? d0 : today,
                                           stepSize_);
                if (d > d2) d = d2;

                e2 = arguments_.basket->expectedTrancheLoss(d);

                results_.premiumValue
                    += (results_.remainingNotional - e2)// THIS IS WRONG IF e2 INCLUDES  !!!!!!!!!!!!!!!!!!!!!!!!
                                                                      // PAST LOSSES move it to: (baskt.Notional - e2)
                    * arguments_.runningRate
                    * arguments_.dayCounter.yearFraction(d0, d)
//                    * arguments_.yieldTS->discount (d);
                    * discountCurve_->discount(d);

                // TO DO: Addd default coupon accrual value here-----

                if (e2 < e1) results_.error ++;

                results_.protectionValue
//                    += (e2 - e1) * arguments_.yieldTS->discount (d);
                    += (e2 - e1) * discountCurve_->discount(d);

                d0 = d;
                e1 = e2;
            }
            while (d < d2);
            results_.expectedTrancheLoss.push_back(e2);////////////////////////////PPPPP
        }

        //if (arguments_.schedule.dates().front() >= today)
       /// if (arguments_.normalizedLeg[0]->date() >= today)
        if (!arguments_.normalizedLeg[0]->hasOccurred(today))// add includeSettlement date flows variable to engine.
            results_.upfrontPremiumValue
                = results_.remainingNotional * arguments_.upfrontRate
                    * discountCurve_->discount(boost::dynamic_pointer_cast<Coupon>(
                        arguments_.normalizedLeg[0])->accrualStartDate());

        if (arguments_.side == Protection::Buyer) {
            results_.protectionValue *= -1;
            results_.premiumValue *= -1;
            results_.upfrontPremiumValue *= -1;
        }

        results_.value = results_.premiumValue - results_.protectionValue
            + results_.upfrontPremiumValue;

        results_.errorEstimate = Null<Real>();
    }

}
