/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Chirag Desai

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

#include <ql/pricingengines/forward/discountingfxforwardengine.hpp>
#include <utility>

namespace QuantLib {

    DiscountingFxForwardEngine::DiscountingFxForwardEngine(
        Handle<YieldTermStructure> currency1DiscountCurve,
        Handle<YieldTermStructure> currency2DiscountCurve,
        Handle<Quote> spotFx,
        const Date& npvDate)
    : currency1DiscountCurve_(std::move(currency1DiscountCurve)),
      currency2DiscountCurve_(std::move(currency2DiscountCurve)),
      spotFx_(std::move(spotFx)),
      npvDate_(npvDate) {
        registerWith(currency1DiscountCurve_);
        registerWith(currency2DiscountCurve_);
        registerWith(spotFx_);
    }

    void DiscountingFxForwardEngine::calculate() const {
        QL_REQUIRE(!currency1DiscountCurve_.empty(), "currency1 discount curve handle is empty");
        QL_REQUIRE(!currency2DiscountCurve_.empty(), "currency2 discount curve handle is empty");
        QL_REQUIRE(!spotFx_.empty(), "spot FX quote handle is empty");

        Date refDate = currency1DiscountCurve_->referenceDate();
        Date refDate2 = currency2DiscountCurve_->referenceDate();
        QL_REQUIRE(refDate == refDate2, "discount curves must have the same reference date, "
                                        "currency1 has "
                                            << refDate << " and currency2 has " << refDate2);

        Date npvDate = npvDate_;
        if (npvDate_ == Date()) {
            npvDate = refDate;
        } else {
            QL_REQUIRE(npvDate >= refDate, "npv date (" << npvDate
                                                        << ") before "
                                                           "discount curve reference date ("
                                                        << refDate << ")");
        }

        results_.value = 0.0;
        results_.errorEstimate = Null<Real>();

        const Date& maturityDate = arguments_.maturityDate;

        // Get the spot FX rate (currency2/currency1)
        Real spotFxRate = spotFx_->value();
        QL_REQUIRE(spotFxRate > 0.0, "spot FX rate must be positive");

        // Get discount factors to maturity
        DiscountFactor df1 = currency1DiscountCurve_->discount(maturityDate);
        DiscountFactor df2 = currency2DiscountCurve_->discount(maturityDate);

        // Get discount factor to NPV date for normalization
        DiscountFactor npvDateDiscount = currency1DiscountCurve_->discount(npvDate);

        // Calculate fair forward rate: F = S * df2 / df1
        // This is the forward rate currency2/currency1
        results_.fairForwardRate = spotFxRate * df2 / df1;

        // Calculate present values of each leg
        // PV of currency1 leg (in currency1)
        Real pv1 = arguments_.nominal1 * df1;

        // PV of currency2 leg (in currency2)
        Real pv2 = arguments_.nominal2 * df2;

        // Convert currency2 PV to currency1 using spot FX rate
        Real pv2InCurrency1 = pv2 / spotFxRate;

        // Calculate NPV based on direction of trade
        // If payCurrency1 is true: pay currency1, receive currency2
        //   NPV = -PV1 + PV2 (in currency1 terms)
        // If payCurrency1 is false: receive currency1, pay currency2
        //   NPV = +PV1 - PV2 (in currency1 terms)
        Real npvInCurrency1;
        if (arguments_.payCurrency1) {
            npvInCurrency1 = -pv1 + pv2InCurrency1;
        } else {
            npvInCurrency1 = pv1 - pv2InCurrency1;
        }

        // Store results
        results_.npvCurrency1 = npvInCurrency1;
        results_.npvCurrency2 = npvInCurrency1 * spotFxRate;

        // NPV normalized to npvDate
        results_.value = npvInCurrency1 / npvDateDiscount;

        // Store additional results for inspection
        results_.additionalResults["spotFx"] = spotFxRate;
        results_.additionalResults["currency1DiscountFactor"] = df1;
        results_.additionalResults["currency2DiscountFactor"] = df2;
        results_.additionalResults["currency1PV"] = pv1;
        results_.additionalResults["currency2PV"] = pv2;
    }

}
