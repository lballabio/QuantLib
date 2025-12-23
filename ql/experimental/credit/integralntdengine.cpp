/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

#include <ql/experimental/credit/integralntdengine.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/experimental/credit/basket.hpp>
#include <numeric>

namespace QuantLib {

    void IntegralNtdEngine::calculate() const {
        Date today = Settings::instance().evaluationDate();

        results_.errorEstimate = Null<Real>();
        results_.value = 0.0;
        results_.premiumValue = 0.0;
        results_.upfrontPremiumValue = 0.;
        Real accrualValue = 0.0;
        Real claimValue = 0.0;
        Date d0;
        /* Given the expense of probsBeingNthEvent both in integrable and 
        monte carlo algorithms this engine tests who to call.
        Warning: This is not entirely a basket property but of the model too.
        The basket has to have all notionals equal but it is the model which
        determines the recovery; having all the market recoveries equal is not
        enough since we might be using a loss model which is stochastic in the
        recovery rates.
        */
        bool basketIsHomogeneous = true;// hardcoded by now

        for (auto& i : arguments_.premiumLeg) {
            ext::shared_ptr<FixedRateCoupon> coupon = ext::dynamic_pointer_cast<FixedRateCoupon>(i);
            Date d = i->date();
            if (d > discountCurve_->referenceDate()) {
                /*
                std::vector<Probability> probsTriggering =
                    arguments_.basket->probsBeingNthEvent(arguments_.ntdOrder, 
                        d);
                Probability defaultProb = 
                    std::accumulate(probsTriggering.begin(), 
                    probsTriggering.end(), Real(0.));
                // OVERKILL???? 1-probAtLeastNEvents is enough

*/
                // prob of contract not having been triggered by date of payment
                Probability probNonTriggered = 
                    1. - arguments_.basket->probAtLeastNEvents(
                        arguments_.ntdOrder, d);

                results_.premiumValue +=
                    i->amount() * discountCurve_->discount(d) * probNonTriggered;
                ////   * (1.0 - defaultProb);

                if (coupon->accrualStartDate() >= 
                    discountCurve_->referenceDate())
                    d = coupon->accrualStartDate();
                else
                    d = discountCurve_->referenceDate();

                // do steps of specified size
                d0 = d;
                Period stepSize = integrationStepSize_;
/*
                probsTriggering =
                    arguments_.basket->probsBeingNthEvent(arguments_.ntdOrder, 
                    ///////REDUNDANT?
                        d0);
                Probability defProb0 = std::accumulate(probsTriggering.begin(), 
                ///OVERKILL????
                    probsTriggering.end(), Real(0.));
*/
                Probability defProb0 = arguments_.basket->probAtLeastNEvents(
                        arguments_.ntdOrder, d0);
                std::vector<Probability> probsTriggering, probsTriggering1;
                do {
                    DiscountFactor disc = discountCurve_->discount(d);

                    Probability defProb1;
                    if(basketIsHomogeneous) {//take test out of the while loop
                        defProb1 = arguments_.basket->probAtLeastNEvents(
                            arguments_.ntdOrder, d);
                        claimValue -= (defProb1-defProb0)
                            * arguments_.basket->claim()->amount(d, 
                                arguments_.notional, 
                                arguments_.basket->recoveryRate(d, 0))
                            * disc;

                    }else{
                        probsTriggering1 =
                            arguments_.basket->probsBeingNthEvent(
                                arguments_.ntdOrder, d);
                        defProb1 = std::accumulate(probsTriggering1.begin(), 
                            probsTriggering1.end(), Real(0.));
                        /*Recoveries might differ along names, depending on 
                        which name is triggering the contract the loss will be 
                        different  
                        There is an issue here; MC engines can still be used 
                        since the prob of triggering the contract can be 
                        extracted from the simulation from the 
                        probsBeingNthEvent statistic. Yet, when the RR is 
                        stochastic the realized value of the RR is the expected 
                        one subject/conditional to the contract being triggered;
                        not simply the expected value. For this reason the MC 
                        can not be used through the statistic but has to consume
                        the simulations directly.
                        */
                        for(Size iName=0; 
                            iName<arguments_.basket->remainingSize(); 
                            iName++) 
                        {
                            claimValue -= (probsTriggering1[iName]-
                                probsTriggering[iName])
                                * arguments_.basket->claim()->amount(d, 
                                    arguments_.notional,// [iName]! 
                                    arguments_.basket->recoveryRate(d, iName))
                                * disc;
                        }
                        probsTriggering = probsTriggering1;
                    }

                    Probability dcfdd = defProb1 - defProb0;
                    defProb0 = defProb1;

                    if (arguments_.settlePremiumAccrual)
                        accrualValue += coupon->accruedAmount(d)*disc*dcfdd;

                    d0 = d;
                    d = d0 + stepSize;
                    // reduce step size ?
                    if (stepSize != 1*Days && d > coupon->accrualEndDate()) {
                        stepSize = 1*Days;
                        d = d0 + stepSize;
                    }
                }
                while (d <= coupon->accrualEndDate());
            }
        }

        // The upfront might be due before the curve ref date...
        if (!arguments_.premiumLeg[0]->hasOccurred(today))
            results_.upfrontPremiumValue =
                arguments_.basket->remainingNotional() 
                    * arguments_.upfrontRate
                    * discountCurve_->discount(
                        ext::dynamic_pointer_cast<FixedRateCoupon>(
                            arguments_.premiumLeg[0])->accrualStartDate());
        if (arguments_.side == Protection::Buyer) {
            results_.premiumValue *= -1;
            accrualValue *= -1;
            claimValue *= -1;
            results_.upfrontPremiumValue *= -1;
        }

        results_.value = results_.premiumValue + accrualValue + claimValue + 
            results_.upfrontPremiumValue;

        results_.fairPremium = -arguments_.premiumRate * claimValue 
            / (results_.premiumValue + accrualValue);
        // alternatively use results buffers and omit locals.
        results_.protectionValue = claimValue;

        results_.additionalResults["fairPremium"] = results_.fairPremium;
        results_.additionalResults["premiumLegNPV"] = 
            Real(results_.premiumValue + results_.upfrontPremiumValue);
        results_.additionalResults["protectionLegNPV"] = 
            results_.protectionValue;
    }

}
