/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
 All rights reserved.

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

#include <ql/cashflows/cashflows.hpp>
#include <ql/exchangerate.hpp>
#include <ql/utilities/dataformatters.hpp>

#include <ql/pricingengines/swap/crossccyswapengine.hpp>

namespace QuantLib {

CrossCcySwapEngine::CrossCcySwapEngine(const Currency& ccy1, const Handle<YieldTermStructure>& currency1Discountcurve,
                                       const Currency& ccy2, const Handle<YieldTermStructure>& currency2Discountcurve,
                                       const Handle<Quote>& spotFX, ext::optional<bool> includeSettlementDateFlows,
                                       const Date& settlementDate, const Date& npvDate, const Date& spotFXSettleDate)
    : ccy1_(ccy1), currency1Discountcurve_(currency1Discountcurve), ccy2_(ccy2),
      currency2Discountcurve_(currency2Discountcurve), spotFX_(spotFX),
      includeSettlementDateFlows_(includeSettlementDateFlows), settlementDate_(settlementDate), npvDate_(npvDate),
      spotFXSettleDate_(spotFXSettleDate) {

    registerWith(currency1Discountcurve_);
    registerWith(currency2Discountcurve_);
    registerWith(spotFX_);
}

void CrossCcySwapEngine::calculate() const {

    QL_REQUIRE(!currency1Discountcurve_.empty() && !currency2Discountcurve_.empty(),
               "Discounting term structure handle is empty.");

    QL_REQUIRE(!spotFX_.empty(), "FX spot quote handle is empty.");

    QL_REQUIRE(currency1Discountcurve_->referenceDate() == currency2Discountcurve_->referenceDate(),
               "Term structures should have the same reference date.");
    Date referenceDate = currency1Discountcurve_->referenceDate();
    Date settlementDate = settlementDate_;
    if (settlementDate_ == Date()) {
        settlementDate = referenceDate;
    } else {
        QL_REQUIRE(settlementDate >= referenceDate, "Settlement date (" << settlementDate
                                                                        << ") cannot be before discount curve "
                                                                           "reference date ("
                                                                        << referenceDate << ")");
    }

    Size numLegs = arguments_.legs.size();
    // - Instrument::Results
    if (npvDate_ == Date()) {
        results_.valuationDate = referenceDate;
    } else {
        QL_REQUIRE(npvDate_ >= referenceDate, "NPV date (" << npvDate_
                                                           << ") cannot be before "
                                                              "discount curve reference date ("
                                                           << referenceDate << ")");
        results_.valuationDate = npvDate_;
    }

    Date spotFXSettleDate = spotFXSettleDate_;
    if (spotFXSettleDate_ == Date()) {
        spotFXSettleDate = referenceDate;
    } else {
        QL_REQUIRE(spotFXSettleDate >= referenceDate, "FX settlement date (" << spotFXSettleDate
                                                                        << ") cannot be before discount curve "
                                                                           "reference date ("
                                                                        << referenceDate << ")");
    }
    
    results_.value = 0.0;
    results_.errorEstimate = Null<Real>();
    // - Swap::Results
    results_.legNPV.resize(numLegs);
    results_.legBPS.resize(numLegs);
    results_.startDiscounts.resize(numLegs);
    results_.endDiscounts.resize(numLegs);
    // - CrossCcySwap::Results
    results_.inCcyLegNPV.resize(numLegs);
    results_.inCcyLegBPS.resize(numLegs);
    results_.npvDateDiscounts.resize(numLegs);

    bool includeReferenceDateFlows =
        includeSettlementDateFlows_ ? *includeSettlementDateFlows_ : Settings::instance().includeReferenceDateEvents();

    for (Size legNo = 0; legNo < numLegs; legNo++) {
        try {
            // Choose the correct discount curve for the leg.
            Handle<YieldTermStructure> legDiscountCurve;
            if (arguments_.currencies[legNo] == ccy1_) {
                legDiscountCurve = currency1Discountcurve_;
            } else {
                QL_REQUIRE(arguments_.currencies[legNo] == ccy2_, "leg ccy (" << arguments_.currencies[legNo]
                                                                              << ") must be ccy1 (" << ccy1_
                                                                              << ") or ccy2 (" << ccy2_ << ")");
                legDiscountCurve = currency2Discountcurve_;
            }
            results_.npvDateDiscounts[legNo] = legDiscountCurve->discount(results_.valuationDate);

            // Calculate the NPV and BPS of each leg in its currency.
            std::tie(results_.inCcyLegNPV[legNo], results_.inCcyLegBPS[legNo]) =
                CashFlows::npvbps(arguments_.legs[legNo], **legDiscountCurve, includeReferenceDateFlows, settlementDate,
                                  results_.valuationDate);
            results_.inCcyLegNPV[legNo] *= arguments_.payer[legNo];
            results_.inCcyLegBPS[legNo] *= arguments_.payer[legNo];

            results_.legNPV[legNo] = results_.inCcyLegNPV[legNo];
            results_.legBPS[legNo] = results_.inCcyLegBPS[legNo];

            // Convert to NPV currency if necessary.
            if (arguments_.currencies[legNo] != ccy1_) {
                // results_.legNPV[legNo] *= spotFX_->value();
                // results_.legBPS[legNo] *= spotFX_->value();
				Real spotFXRate = spotFX_->value();
				if (spotFXSettleDate != referenceDate) {
					// Use the parity relation between discount factors and fx rates to compute spotFXRate
					// Generic formula: fx(T1)/fx(T2) = FwdDF_Quote(T1->T2) / FwdDF_Base(T1->T2),
                    // where fx represents the currency ratio Base/Quote
					Real ccy1DF = currency1Discountcurve_->discount(spotFXSettleDate);
					Real ccy2DF = currency2Discountcurve_->discount(spotFXSettleDate);
					QL_REQUIRE(ccy2DF != 0.0, "Discount Factor associated with currency " << ccy2_
                                                 << " at maturity " << spotFXSettleDate << " cannot be zero");
					spotFXRate *= ccy1DF / ccy2DF;
				}
				results_.legNPV[legNo] *= spotFXRate;
				results_.legBPS[legNo] *= spotFXRate;
            }

            // Get start date and end date discount for the leg
            Date startDate = CashFlows::startDate(arguments_.legs[legNo]);
            if (startDate >= currency1Discountcurve_->referenceDate()) {
                results_.startDiscounts[legNo] = legDiscountCurve->discount(startDate);
            } else {
                results_.startDiscounts[legNo] = Null<DiscountFactor>();
            }

            Date maturityDate = CashFlows::maturityDate(arguments_.legs[legNo]);
            if (maturityDate >= currency1Discountcurve_->referenceDate()) {
                results_.endDiscounts[legNo] = legDiscountCurve->discount(maturityDate);
            } else {
                results_.endDiscounts[legNo] = Null<DiscountFactor>();
            }

        } catch (std::exception& e) {
            QL_FAIL(io::ordinal(legNo + 1) << " leg: " << e.what());
        }

        results_.value += results_.legNPV[legNo];
    }
}
} // namespace QuantLib
