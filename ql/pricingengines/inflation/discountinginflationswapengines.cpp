/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2009 StatPro Italia srl

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

#include <ql/pricingengines/inflation/discountinginflationswapengines.hpp>
#include <ql/cashflows/simplecashflow.hpp>

namespace QuantLib {

    DiscountingYoYInflationSwapEngine::DiscountingYoYInflationSwapEngine(
                     const Handle<YieldTermStructure>& discountCurve,
                     const Handle<YoYInflationTermStructure>& inflationCurve,
                     boost::optional<bool> includeSettlementDateFlows)
    : discountCurve_(discountCurve), inflationCurve_(inflationCurve),
      includeSettlementDateFlows_(includeSettlementDateFlows) {
        registerWith(discountCurve_);
        registerWith(inflationCurve_);
    }

    void DiscountingYoYInflationSwapEngine::calculate() const {
        // Rates for instruments always look at earlier values paid later.
        Real nom = 0.0;
        Real infl = 0.0;
        Real frac;

        Date referenceDate = discountCurve_->referenceDate();
        for (Size i=0; i<arguments_.paymentDates.size(); i++) {
            Date couponPayDate = arguments_.paymentDates[i];
            // Just to use the CashFlow machinery. There has to be a
            // better way...
            SimpleCashFlow payment(0.0, couponPayDate);
            if (!payment.hasOccurred(referenceDate,
                                     includeSettlementDateFlows_)) {
                if (i==0) {
                    frac = arguments_.dayCounter.yearFraction(referenceDate,
                                                              couponPayDate);
                } else {
                    if (referenceDate > arguments_.paymentDates[i-1])
                        frac = arguments_.dayCounter.yearFraction(
                                                referenceDate, couponPayDate);
                    else
                        frac = arguments_.dayCounter.yearFraction(
                                 arguments_.paymentDates[i-1], couponPayDate);
                }

                nom += frac * discountCurve_->discount(couponPayDate);
                infl += frac * inflationCurve_->yoyRate(
                    arguments_.calendar.adjust(couponPayDate - arguments_.lag,
                                               arguments_.bdc)) *
                    discountCurve_->discount(couponPayDate);
            }
        }

        results_.value = nom*arguments_.fixedRate - infl;
        results_.errorEstimate = 0.0;
        results_.fairRate = infl/nom;
    }



    DiscountingZeroInflationSwapEngine::DiscountingZeroInflationSwapEngine(
                    const Handle<YieldTermStructure>& discountCurve,
                    const Handle<ZeroInflationTermStructure>& inflationCurve,
                    boost::optional<bool> includeSettlementDateFlows)
    : discountCurve_(discountCurve), inflationCurve_(inflationCurve),
      includeSettlementDateFlows_(includeSettlementDateFlows) {
        registerWith(discountCurve_);
        registerWith(inflationCurve_);
    }

    void DiscountingZeroInflationSwapEngine::calculate() const {

        results_.fairRate =
            inflationCurve_->zeroRate(arguments_.maturity - arguments_.lag);


        Date referenceDate = discountCurve_->referenceDate();
        SimpleCashFlow payment(0.0, arguments_.maturity);
        if (!payment.hasOccurred(referenceDate,
                                 includeSettlementDateFlows_)) {
            // discount is relative to the payment date, not the
            // observation date.
            Real T = arguments_.dayCounter.yearFraction(
                                        inflationCurve_->baseDate(),
                                        arguments_.maturity - arguments_.lag);
            results_.value = discountCurve_->discount(arguments_.maturity) *
                (std::pow(1.0 + arguments_.fixedRate, T) -
                 std::pow(1.0 + results_.fairRate, T));
        } else {
            results_.value = 0.0;
        }
        results_.errorEstimate = 0.0;
    }

}

