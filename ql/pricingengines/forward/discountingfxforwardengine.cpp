/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Chirag Desai

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
        Handle<YieldTermStructure> sourceCurrencyDiscountCurve,
        Handle<YieldTermStructure> targetCurrencyDiscountCurve,
        Handle<Quote> spotFx)
    : sourceCurrencyDiscountCurve_(std::move(sourceCurrencyDiscountCurve)),
      targetCurrencyDiscountCurve_(std::move(targetCurrencyDiscountCurve)),
      spotFx_(std::move(spotFx)) {
        registerWith(sourceCurrencyDiscountCurve_);
        registerWith(targetCurrencyDiscountCurve_);
        registerWith(spotFx_);
    }

    void DiscountingFxForwardEngine::calculate() const {
        QL_REQUIRE(!sourceCurrencyDiscountCurve_.empty(),
                   "source currency discount curve handle is empty");
        QL_REQUIRE(!targetCurrencyDiscountCurve_.empty(),
                   "target currency discount curve handle is empty");
        QL_REQUIRE(!spotFx_.empty(), "spot FX quote handle is empty");

        results_.value = 0.0;
        results_.errorEstimate = Null<Real>();

        const Date& maturityDate = arguments_.maturityDate;
        const Date& settlementDate = arguments_.settlementDate;

        // Validate that curve reference dates are on or before settlement date
        const Date sourceRefDate = sourceCurrencyDiscountCurve_->referenceDate();
        const Date targetRefDate = targetCurrencyDiscountCurve_->referenceDate();
        QL_REQUIRE(sourceRefDate <= settlementDate,
                   "source currency discount curve reference date (" << sourceRefDate
                   << ") must be on or before settlement date (" << settlementDate << ")");
        QL_REQUIRE(targetRefDate <= settlementDate,
                   "target currency discount curve reference date (" << targetRefDate
                   << ") must be on or before settlement date (" << settlementDate << ")");

        // Get the spot FX rate (targetCurrency/sourceCurrency)
        const Real spotFxRate = spotFx_->value();
        QL_REQUIRE(spotFxRate > 0.0, "spot FX rate must be positive");

        // Get discount factors from settlement to maturity
        const DiscountFactor dfSource = sourceCurrencyDiscountCurve_->discount(maturityDate) /
                                        sourceCurrencyDiscountCurve_->discount(settlementDate);
        const DiscountFactor dfTarget = targetCurrencyDiscountCurve_->discount(maturityDate) /
                                        targetCurrencyDiscountCurve_->discount(settlementDate);
        const DiscountFactor dfSourceSettlement =
            sourceCurrencyDiscountCurve_->discount(settlementDate);
        const DiscountFactor dfTargetSettlement =
            targetCurrencyDiscountCurve_->discount(settlementDate);

        // Calculate fair forward rate: F = S * dfSource / dfTarget
        // This is the forward rate targetCurrency/sourceCurrency
        results_.fairForwardRate = spotFxRate * dfSource / dfTarget;

        // Calculate settlement-date present values of each leg
        // PV of source currency leg (in source currency)
        const Real pvSource = arguments_.sourceNominal * dfSource;

        // PV of target currency leg (in target currency)
        const Real pvTarget = arguments_.targetNominal * dfTarget;

        // Convert target currency PV to source currency using spot FX rate
        const Real pvTargetInSourceCurrency = pvTarget / spotFxRate;

        // Calculate NPV based on direction of trade
        // If paySourceCurrency is true: pay source currency, receive target currency
        //   NPV = -PVSource + PVTarget (in source currency terms)
        // If paySourceCurrency is false: receive source currency, pay target currency
        //   NPV = +PVSource - PVTarget (in source currency terms)
        const Real npvAtSettlementInSourceCurrency =
            arguments_.paySourceCurrency ?
                -pvSource + pvTargetInSourceCurrency :
                pvSource - pvTargetInSourceCurrency;

        const Real npvInSourceCurrency =
            npvAtSettlementInSourceCurrency * dfSourceSettlement;
        const Real npvInTargetCurrency =
            npvAtSettlementInSourceCurrency * spotFxRate * dfTargetSettlement;

        // Store results - NPV is as of the curve reference date
        results_.value = npvInSourceCurrency;
        results_.npvSourceCurrency = npvInSourceCurrency;
        results_.npvTargetCurrency = npvInTargetCurrency;

        // Store additional results for inspection
        results_.additionalResults["spotFx"] = spotFxRate;
        results_.additionalResults["sourceCurrencyDiscountFactor"] = dfSource;
        results_.additionalResults["targetCurrencyDiscountFactor"] = dfTarget;
        results_.additionalResults["sourceCurrencySettlementDiscountFactor"] =
            dfSourceSettlement;
        results_.additionalResults["targetCurrencySettlementDiscountFactor"] =
            dfTargetSettlement;
        results_.additionalResults["sourceCurrencyPV"] = pvSource;
        results_.additionalResults["targetCurrencyPV"] = pvTarget;
    }

}
