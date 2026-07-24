/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/fxresetcashflows.hpp>
#include <ql/pricingengines/swap/discountingmtmcrosscurrencybasisswapengine.hpp>
#include <ql/settings.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

namespace QuantLib {

DiscountingMtMCrossCurrencyBasisSwapEngine::DiscountingMtMCrossCurrencyBasisSwapEngine(
    Currency domesticCcy, const Handle<YieldTermStructure>& domesticCcyDiscountcurve,
    Currency foreignCcy, const Handle<YieldTermStructure>& foreignCcyDiscountcurve,
    const Handle<Quote>& spotFX, std::optional<bool> includeSettlementDateFlows,
    const Date& settlementDate, const Date& npvDate, const Date& spotFXSettleDate)
: domesticCcy_(std::move(domesticCcy)), domesticCcyDiscountcurve_(domesticCcyDiscountcurve),
  foreignCcy_(std::move(foreignCcy)), foreignCcyDiscountcurve_(foreignCcyDiscountcurve),
  spotFX_(spotFX), includeSettlementDateFlows_(includeSettlementDateFlows),
  settlementDate_(settlementDate), npvDate_(npvDate), spotFXSettleDate_(spotFXSettleDate) {

    registerWith(domesticCcyDiscountcurve_);
    registerWith(foreignCcyDiscountcurve_);
    registerWith(spotFX_);
}

void DiscountingMtMCrossCurrencyBasisSwapEngine::calculate() const {
    QL_REQUIRE(!domesticCcyDiscountcurve_.empty() && !foreignCcyDiscountcurve_.empty(),
               "Discounting term structure handle is empty.");
    QL_REQUIRE(!spotFX_.empty(), "FX spot quote handle is empty.");
    QL_REQUIRE(domesticCcyDiscountcurve_->referenceDate() ==
                   foreignCcyDiscountcurve_->referenceDate(),
               "Term structures should have the same reference date.");

    Date referenceDate = domesticCcyDiscountcurve_->referenceDate();
    Date settlementDate = settlementDate_ == Date() ? referenceDate : settlementDate_;
    QL_REQUIRE(settlementDate >= referenceDate,
               "Settlement date (" << settlementDate
                                    << ") cannot be before discount curve reference date ("
                                    << referenceDate << ")");

    Date valuationDate = npvDate_ == Date() ? referenceDate : npvDate_;
    QL_REQUIRE(valuationDate >= referenceDate,
               "NPV date (" << valuationDate
                            << ") cannot be before discount curve reference date ("
                            << referenceDate << ")");

    Date spotFXSettleDate =
        spotFXSettleDate_ == Date() ? referenceDate : spotFXSettleDate_;
    QL_REQUIRE(spotFXSettleDate >= referenceDate,
               "FX settlement date (" << spotFXSettleDate
                                       << ") cannot be before discount curve reference date ("
                                       << referenceDate << ")");

    DiscountFactor domesticFxSettlementDiscount =
        domesticCcyDiscountcurve_->discount(spotFXSettleDate);
    DiscountFactor foreignFxSettlementDiscount =
        foreignCcyDiscountcurve_->discount(spotFXSettleDate);
    QL_REQUIRE(foreignFxSettlementDiscount != 0.0,
               "Discount factor for currency " << foreignCcy_ << " at " << spotFXSettleDate
                                                << " cannot be zero");
    Real referenceDateFx =
        spotFX_->value() * domesticFxSettlementDiscount / foreignFxSettlementDiscount;

    DiscountFactor domesticValuationDiscount =
        domesticCcyDiscountcurve_->discount(valuationDate);
    DiscountFactor foreignValuationDiscount =
        foreignCcyDiscountcurve_->discount(valuationDate);
    QL_REQUIRE(domesticValuationDiscount != 0.0,
               "Discount factor for currency " << domesticCcy_ << " at " << valuationDate
                                                << " cannot be zero");
    Real valuationDateFx =
        referenceDateFx * foreignValuationDiscount / domesticValuationDiscount;

    Size numLegs = arguments_.legs.size();
    bool includeReferenceDateFlows =
        includeSettlementDateFlows_ ? *includeSettlementDateFlows_
                                    : Settings::instance().includeReferenceDateEvents();

    results_.value = 0.0;
    results_.errorEstimate = Null<Real>();
    results_.valuationDate = valuationDate;
    results_.npvDateDiscount = domesticValuationDiscount;
    results_.legNPV.resize(numLegs);
    results_.legBPS.resize(numLegs);
    results_.startDiscounts.resize(numLegs);
    results_.endDiscounts.resize(numLegs);
    results_.inCcyLegNPV.resize(numLegs);
    results_.inCcyLegBPS.resize(numLegs);
    results_.npvDateDiscounts.resize(numLegs);

    for (Size legNo = 0; legNo < numLegs; ++legNo) {
        try {
            Handle<YieldTermStructure> legDiscountCurve, otherLegCurve;
            if (arguments_.currencies[legNo] == domesticCcy_) {
                legDiscountCurve = domesticCcyDiscountcurve_;
                otherLegCurve = foreignCcyDiscountcurve_;
            } else {
                QL_REQUIRE(arguments_.currencies[legNo] == foreignCcy_,
                           "leg ccy (" << arguments_.currencies[legNo]
                                       << ") must be domesticCcy (" << domesticCcy_
                                       << ") or foreignCcy (" << foreignCcy_ << ")");
                legDiscountCurve = foreignCcyDiscountcurve_;
                otherLegCurve = domesticCcyDiscountcurve_;
            }
            results_.npvDateDiscounts[legNo] = legDiscountCurve->discount(valuationDate);

            if (legNo == arguments_.resettingLegIndex) {
                // The resettable leg pays in currency R; the constant-notional
                // leg pays in currency C.  Its cash flows convert at FX rates
                // quoted R per C, which they obtain from a source built from
                // this engine's market data: attach it before discounting.
                const Currency& resettableCcy = arguments_.currencies[legNo];
                const Currency& constantLegCcy =
                    arguments_.currencies[arguments_.constantLegIndex];
                bool spotIsResettablePerConstant = (resettableCcy == domesticCcy_);
                auto fxResetSource = ext::make_shared<FxResetSource>(
                    constantLegCcy, resettableCcy, otherLegCurve, legDiscountCurve, spotFX_,
                    spotIsResettablePerConstant, spotFXSettleDate_);
                for (const auto& cf : arguments_.legs[legNo]) {
                    if (auto coupon = ext::dynamic_pointer_cast<FxResetCoupon>(cf))
                        coupon->setFxResetSource(fxResetSource);
                    else if (auto exchange =
                                 ext::dynamic_pointer_cast<FxResetNotionalExchange>(cf))
                        exchange->setFxResetSource(fxResetSource);
                }
            }

            std::tie(results_.inCcyLegNPV[legNo], results_.inCcyLegBPS[legNo]) =
                CashFlows::npvbps(arguments_.legs[legNo], **legDiscountCurve,
                                  includeReferenceDateFlows, settlementDate, valuationDate);

            results_.inCcyLegNPV[legNo] *= arguments_.payer[legNo];
            results_.inCcyLegBPS[legNo] *= arguments_.payer[legNo];
            results_.legNPV[legNo] = results_.inCcyLegNPV[legNo];
            results_.legBPS[legNo] = results_.inCcyLegBPS[legNo];

            if (arguments_.currencies[legNo] != domesticCcy_) {
                results_.legNPV[legNo] *= valuationDateFx;
                results_.legBPS[legNo] *= valuationDateFx;
            }

            Date startDate = CashFlows::startDate(arguments_.legs[legNo]);
            results_.startDiscounts[legNo] =
                startDate >= referenceDate ? legDiscountCurve->discount(startDate)
                                           : Null<DiscountFactor>();
            Date maturityDate = CashFlows::maturityDate(arguments_.legs[legNo]);
            results_.endDiscounts[legNo] =
                maturityDate >= referenceDate ? legDiscountCurve->discount(maturityDate)
                                              : Null<DiscountFactor>();
            results_.value += results_.legNPV[legNo];
        } catch (std::exception& e) {
            QL_FAIL(io::ordinal(legNo + 1) << " leg: " << e.what());
        }
    }
}

} // namespace QuantLib
