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
#include <ql/cashflows/coupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/currencies/exchangeratemanager.hpp>
#include <ql/patterns/visitor.hpp>
#include <ql/pricingengines/swap/discountingmtmcrosscurrencybasisswapengine.hpp>
#include <ql/settings.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

namespace QuantLib {

namespace {

    //! Analytic valuation of a mark-to-market (FX-resetting) floating leg.
    /*! Each period accrues on the constant-leg notional converted at the reset FX,
        observed at the accrual start.  A future reset is the forward FX implied by
        the two discount curves (spotResetPerConstant * P_constant/P_resettable);
        an already-fixed reset is read from the exchange-rate repository.  The
        per-period NPV is the redemption of the reset notional plus accrued interest
        at period end, minus the borrowed notional at period start, all on the leg's
        own discount curve.
    */
    class ResettingLegCalculator : public AcyclicVisitor, public Visitor<Coupon> {
      public:
        ResettingLegCalculator(const YieldTermStructure& ownCurve,
                               const YieldTermStructure& otherCurve,
                               Real constantLegNotional,
                               Real spotResetPerConstant,
                               Currency constantLegCurrency,
                               Currency resettableLegCurrency,
                               Integer paymentLag,
                               Calendar paymentCalendar,
                               BusinessDayConvention convention,
                               std::optional<bool> includeSettlementDateFlows,
                               Date settlementDate)
        : ownCurve_(ownCurve), otherCurve_(otherCurve), constantLegNotional_(constantLegNotional),
          spotResetPerConstant_(spotResetPerConstant),
          constantLegCurrency_(std::move(constantLegCurrency)),
          resettableLegCurrency_(std::move(resettableLegCurrency)), paymentLag_(paymentLag),
          paymentCalendar_(std::move(paymentCalendar)), convention_(convention),
          includeSettlementDateFlows_(includeSettlementDateFlows),
          settlementDate_(settlementDate) {}

        void visit(Coupon& c) override {
            Date start = c.accrualStartDate();
            Date end = c.accrualEndDate();

            Date paymentStart, paymentEnd;
            if (paymentLag_ == 0) {
                paymentStart = start;
                paymentEnd = end;
            } else {
                paymentStart = paymentCalendar_.advance(start, paymentLag_, Days, convention_);
                paymentEnd = paymentCalendar_.advance(end, paymentLag_, Days, convention_);
            }

            bool includeStart =
                !SimpleCashFlow(0.0, paymentStart)
                     .hasOccurred(settlementDate_, includeSettlementDateFlows_);
            bool includeEnd =
                !SimpleCashFlow(0.0, paymentEnd)
                     .hasOccurred(settlementDate_, includeSettlementDateFlows_);

            if (!includeStart && !includeEnd)
                return;

            // FX-reset notional for the period: constantLegNotional * FX(reset).
            Real fx;
            if (start >= ownCurve_.referenceDate()) {
                // future reset: forward FX from the two discount curves
                fx = spotResetPerConstant_ * otherCurve_.discount(start) /
                     ownCurve_.discount(start);
            } else {
                // In-progress reset already fixed: obtain the realized conversion from
                // the exchange-rate repository.  Using exchange(), instead of rate(),
                // handles rates stored in either orientation and derived rates.
                ExchangeRate exchangeRate = ExchangeRateManager::instance().lookup(
                    constantLegCurrency_, resettableLegCurrency_, start);
                fx = exchangeRate.exchange(Money(1.0, constantLegCurrency_)).value();
                QL_REQUIRE(fx > 0.0,
                           "FX fixing from " << constantLegCurrency_ << " to "
                                             << resettableLegCurrency_ << " on " << start
                                             << " must be positive");
            }
            Real adjustedNotional = constantLegNotional_ * fx;

            Time accrual = c.accrualPeriod();
            if (includeStart)
                npv_ -= adjustedNotional * ownCurve_.discount(paymentStart);

            if (includeEnd) {
                DiscountFactor discountEnd = ownCurve_.discount(paymentEnd);
                npv_ += adjustedNotional * discountEnd * (1.0 + c.rate() * accrual);
                bps_ += adjustedNotional * discountEnd * accrual;
            }
        }
        Real NPV() const { return npv_; }
        Real BPS() const { return bps_; }

      private:
        const YieldTermStructure& ownCurve_;
        const YieldTermStructure& otherCurve_;
        Real constantLegNotional_;
        Real spotResetPerConstant_;
        Currency constantLegCurrency_;
        Currency resettableLegCurrency_;
        Real npv_ = 0.0;
        Real bps_ = 0.0;
        Integer paymentLag_;
        Calendar paymentCalendar_;
        BusinessDayConvention convention_;
        std::optional<bool> includeSettlementDateFlows_;
        Date settlementDate_;
    };

    // Returns { NPV, dNPV/drate } in the resetting leg's own currency, discounted
    // to the curve reference date.
    std::pair<Real, Real> npvbpsResettingLeg(const Leg& leg,
                                             Real constantLegNotional,
                                             Real spotResetPerConstant,
                                             const Currency& constantLegCurrency,
                                             const Currency& resettableLegCurrency,
                                             Integer paymentLag,
                                             const Calendar& paymentCalendar,
                                             BusinessDayConvention convention,
                                             const std::optional<bool>& includeSettlementDateFlows,
                                             const Date& settlementDate,
                                             const YieldTermStructure& ownCurve,
                                             const YieldTermStructure& otherCurve) {
        ResettingLegCalculator calc(ownCurve, otherCurve, constantLegNotional,
                                    spotResetPerConstant, constantLegCurrency,
                                    resettableLegCurrency, paymentLag, paymentCalendar,
                                    convention, includeSettlementDateFlows, settlementDate);
        for (const auto& cf : leg)
            cf->accept(calc);
        return { calc.NPV(), calc.BPS() };
    }

}

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

    static const Spread basisPoint = 1.0e-4;

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

            const auto& resetData = arguments_.resettingLegData;
            if (legNo == resetData.resettingLegIndex) {
                // The resettable leg pays in currency R; the constant-notional
                // leg pays in currency C.  The reset FX is quoted R per C.
                const Currency& resettableCcy = arguments_.currencies[legNo];
                const Currency& constantLegCcy = arguments_.currencies[resetData.constantLegIndex];
                Real spotResetPerConstant = (resettableCcy == foreignCcy_) ?
                                                1.0 / referenceDateFx :
                                                referenceDateFx;
                auto [npvRef, bpsRef] = npvbpsResettingLeg(
                    arguments_.legs[legNo], resetData.constantLegNotional,
                    spotResetPerConstant, constantLegCcy, resettableCcy, resetData.paymentLag,
                    resetData.paymentCalendar,
                    resetData.paymentConvention, includeReferenceDateFlows, settlementDate,
                    **legDiscountCurve, **otherLegCurve);
                // npvbpsResettingLeg discounts to the reference date and returns
                // dNPV/drate; rescale to the npv date and to a 1bp sensitivity to
                // match the conventions used by CashFlows::npvbps elsewhere.
                Real dfValuation = results_.npvDateDiscounts[legNo];
                results_.inCcyLegNPV[legNo] = npvRef / dfValuation;
                results_.inCcyLegBPS[legNo] = (bpsRef * basisPoint) / dfValuation;
            } else {
                std::tie(results_.inCcyLegNPV[legNo], results_.inCcyLegBPS[legNo]) =
                    CashFlows::npvbps(arguments_.legs[legNo], **legDiscountCurve,
                                      includeReferenceDateFlows, settlementDate, valuationDate);
            }

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
