/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall

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

#include <ql/experimental/commodities/energybasisswap.hpp>
#include <ql/experimental/commodities/commoditysettings.hpp>

namespace QuantLib {

    EnergyBasisSwap::EnergyBasisSwap(
                    const Calendar& calendar,
                    const boost::shared_ptr<CommodityIndex>& spreadIndex,
                    const boost::shared_ptr<CommodityIndex>& payIndex,
                    const boost::shared_ptr<CommodityIndex>& receiveIndex,
                    bool spreadToPayLeg,
                    const Currency& payCurrency,
                    const Currency& receiveCurrency,
                    const PricingPeriods& pricingPeriods,
                    const CommodityUnitCost& basis,
                    const CommodityType& commodityType,
                    const boost::shared_ptr<SecondaryCosts>& secondaryCosts,
                    const Handle<YieldTermStructure>& payLegTermStructure,
                    const Handle<YieldTermStructure>& receiveLegTermStructure,
                    const Handle<YieldTermStructure>& discountTermStructure)
    : EnergySwap(calendar, payCurrency, receiveCurrency,
                 pricingPeriods, commodityType, secondaryCosts),
      spreadIndex_(spreadIndex), payIndex_(payIndex),
      receiveIndex_(receiveIndex), spreadToPayLeg_(spreadToPayLeg),
      basis_(basis), payLegTermStructure_(payLegTermStructure),
      receiveLegTermStructure_(receiveLegTermStructure),
      discountTermStructure_(discountTermStructure) {
        QL_REQUIRE(pricingPeriods_.size() > 0, "no payment dates");
        registerWith(spreadIndex_);
        registerWith(payIndex_);
        registerWith(receiveIndex_);
    }

    void EnergyBasisSwap::performCalculations() const {

        try {

            if (payIndex_->empty()) {
                if (payIndex_->forwardCurveEmpty()) {
                    QL_FAIL("index [" + payIndex_->name() +
                            "] does not have any quotes or forward prices");
                } else {
                    addPricingError(PricingError::Warning,
                                    "index [" + payIndex_->name() +
                                    "] does not have any quotes; "
                                    "using forward prices from [" +
                                    payIndex_->forwardCurve()->name() + "]");
                }
            }
            if (receiveIndex_->empty()) {
                if (receiveIndex_->forwardCurveEmpty()) {
                    QL_FAIL("index [" + receiveIndex_->name() +
                            "] does not have any quotes or forward prices");
                } else {
                    addPricingError(PricingError::Warning,
                                    "index [" + receiveIndex_->name() +
                                    "] does not have any quotes; "
                                    "using forward prices from [" +
                                    receiveIndex_->forwardCurve()->name() +
                                    "]");
                }
            }

            NPV_ = 0.0;
            additionalResults_.clear();
            dailyPositions_.clear();
            paymentCashFlows_.clear();

            Date evaluationDate = Settings::instance().evaluationDate();

            const Currency& baseCurrency =
                CommoditySettings::instance().currency();
            const UnitOfMeasure baseUnitOfMeasure =
                CommoditySettings::instance().unitOfMeasure();

            Real quantityUomConversionFactor =
                calculateUomConversionFactor(
                               pricingPeriods_[0]->quantity().commodityType(),
                               baseUnitOfMeasure,
                               pricingPeriods_[0]->quantity().unitOfMeasure());
            Real payIndexUomConversionFactor =
                calculateUomConversionFactor(payIndex_->commodityType(),
                                             payIndex_->unitOfMeasure(),
                                             baseUnitOfMeasure);
            Real receiveIndexUomConversionFactor =
                calculateUomConversionFactor(receiveIndex_->commodityType(),
                                             receiveIndex_->unitOfMeasure(),
                                             baseUnitOfMeasure);

            Real payIndexFxConversionFactor =
                calculateFxConversionFactor(payIndex_->currency(),
                                            baseCurrency, evaluationDate);
            Real receiveIndexFxConversionFactor =
                calculateFxConversionFactor(receiveIndex_->currency(),
                                            baseCurrency, evaluationDate);
            Real payLegFxConversionFactor =
                calculateFxConversionFactor(baseCurrency, payCurrency_,
                                            evaluationDate);
            Real receiveLegFxConversionFactor =
                calculateFxConversionFactor(baseCurrency, receiveCurrency_,
                                            evaluationDate);

            Real basisUomConversionFactor =
                calculateUomConversionFactor(
                               pricingPeriods_[0]->quantity().commodityType(),
                               basis_.unitOfMeasure(), baseUnitOfMeasure);
            Real basisFxConversionFactor =
                calculateFxConversionFactor(baseCurrency,
                                            basis_.amount().currency(),
                                            evaluationDate);

            Real basisValue = basis_.amount().value() *
                basisUomConversionFactor * basisFxConversionFactor;

            Date lastPayIndexQuoteDate = payIndex_->lastQuoteDate();
            Date lastReceiveIndexQuoteDate = receiveIndex_->lastQuoteDate();

            if (lastPayIndexQuoteDate < evaluationDate - 1) {
                std::ostringstream message;
                message << "index [" << payIndex_->name()
                        << "] has last quote date of "
                        << io::iso_date(lastPayIndexQuoteDate);
                addPricingError(PricingError::Warning, message.str());
            }
            if (lastReceiveIndexQuoteDate < evaluationDate - 1) {
                std::ostringstream message;
                message << "index [" << receiveIndex_->name()
                        << "] has last quote date of "
                        << io::iso_date(lastReceiveIndexQuoteDate);
                addPricingError(PricingError::Warning, message.str());
            }

            Date lastQuoteDate = std::min(lastPayIndexQuoteDate,
                                          lastReceiveIndexQuoteDate);

            Real totalQuantityAmount = 0;

            // price each period
            for (PricingPeriods::const_iterator pi = pricingPeriods_.begin();
                 pi != pricingPeriods_.end(); pi++) {
                const boost::shared_ptr<PricingPeriod>& pricingPeriod = *pi;

                Integer periodDayCount = 0;

                // get the index quotes
                Date periodStartDate =
                    calendar_.adjust(pricingPeriod->startDate());
                for (Date stepDate = periodStartDate;
                     stepDate <= pricingPeriod->endDate();
                     stepDate = calendar_.advance(stepDate, 1*Days)) {

                    bool unrealized = stepDate > evaluationDate;
                    Real payQuoteValue = 0;
                    Real receiveQuoteValue = 0;

                    if (stepDate <= lastQuoteDate) {
                        payQuoteValue = payIndex_->price(stepDate);
                        receiveQuoteValue = receiveIndex_->price(stepDate);
                    } else {
                        payQuoteValue = payIndex_->forwardPrice(stepDate);
                        receiveQuoteValue =
                            receiveIndex_->forwardPrice(stepDate);
                    }

                    if (payQuoteValue == 0) {
                        std::ostringstream message;
                        message << "pay quote value for curve ["
                                << payIndex_->name() << "] is 0 for date "
                                << io::iso_date(stepDate);
                        addPricingError(PricingError::Warning, message.str());
                    }
                    if (receiveQuoteValue == 0) {
                        std::ostringstream message;
                        message << "receive quote value for curve ["
                                << receiveIndex_->name() << "] is 0 for date "
                                << io::iso_date(stepDate);
                        addPricingError(PricingError::Warning, message.str());
                    }

                    QL_REQUIRE(payQuoteValue != Null<Real>(),
                               "curve [" << payIndex_->name() <<
                               "] missing value for pricing date: "
                               << stepDate);
                    QL_REQUIRE(receiveQuoteValue != Null<Real>(),
                               "curve [" << receiveIndex_->name() <<
                               "] missing value for pricing date: "
                               << stepDate);

                    Real payLegPriceValue =
                        payQuoteValue * payIndexUomConversionFactor *
                        payIndexFxConversionFactor;
                    Real receiveLegPriceValue =
                        receiveQuoteValue * receiveIndexUomConversionFactor *
                        receiveIndexFxConversionFactor;

                    if (spreadToPayLeg_)
                        payLegPriceValue += basisValue;
                    else
                        receiveLegPriceValue += basisValue;

                    dailyPositions_[stepDate] =
                        EnergyDailyPosition(stepDate, payLegPriceValue,
                                            receiveLegPriceValue, unrealized);
                    periodDayCount++;
                }

                Real periodQuantityAmount =
                    pricingPeriod->quantity().amount() *
                    quantityUomConversionFactor;
                totalQuantityAmount += periodQuantityAmount;

                Real avgDailyQuantityAmount =
                    periodDayCount == 0 ? 0 :
                                          periodQuantityAmount / periodDayCount;

                Real payLegValue = 0;
                Real receiveLegValue = 0;
                for (std::map<Date, EnergyDailyPosition>::iterator dpi =
                         dailyPositions_.find(periodStartDate);
                     dpi != dailyPositions_.end() &&
                         dpi->first <= pricingPeriod->endDate(); dpi++) {
                    EnergyDailyPosition& dailyPosition = dpi->second;
                    dailyPosition.quantityAmount = avgDailyQuantityAmount;
                    dailyPosition.riskDelta =
                        (-dailyPosition.payLegPrice + dailyPosition.receiveLegPrice) * avgDailyQuantityAmount;
                    payLegValue += -dailyPosition.payLegPrice * avgDailyQuantityAmount;
                    receiveLegValue += dailyPosition.receiveLegPrice * avgDailyQuantityAmount;
                }

                Real discountFactor = 1;
                Real payLegDiscountFactor = 1;
                Real receiveLegDiscountFactor = 1;
                if (pricingPeriod->paymentDate() >= evaluationDate + 2 /* settlement days*/) {
                    discountFactor =
                        discountTermStructure_->discount(
                                                pricingPeriod->paymentDate());
                    payLegDiscountFactor =
                        payLegTermStructure_->discount(
                                                pricingPeriod->paymentDate());
                    receiveLegDiscountFactor =
                        receiveLegTermStructure_->discount(
                                                pricingPeriod->paymentDate());
                }

                Real uDelta = receiveLegValue + payLegValue;
                Real dDelta = (receiveLegValue * receiveLegDiscountFactor) +
                    (payLegValue * payLegDiscountFactor);
                Real pmtFxConversionFactor =
                    (dDelta > 0) ? payLegFxConversionFactor : receiveLegFxConversionFactor;
                Currency pmtCurrency =
                    (dDelta  > 0) ? receiveCurrency_ : payCurrency_;
                Real pmtDiscountFactor =
                    (dDelta  > 0) ? receiveLegDiscountFactor : payLegDiscountFactor;

                paymentCashFlows_[pricingPeriod->paymentDate()] =
                    boost::shared_ptr<CommodityCashFlow> (
                           new CommodityCashFlow(pricingPeriod->paymentDate(),
                                                 Money(baseCurrency,
                                                       uDelta * discountFactor),
                                                 Money(baseCurrency, uDelta),
                                                 Money(pmtCurrency,
                                                       dDelta * pmtFxConversionFactor),
                                                 Money(pmtCurrency,
                                                       uDelta * pmtFxConversionFactor),
                                                 discountFactor,
                                                 pmtDiscountFactor,
                                                 pricingPeriod->paymentDate() <= evaluationDate));

                calculateSecondaryCostAmounts(
                               pricingPeriods_[0]->quantity().commodityType(),
                               totalQuantityAmount, evaluationDate);

                NPV_ += dDelta;
            }

            QL_REQUIRE(paymentCashFlows_.size() > 0, "no cashflows");

            for (SecondaryCostAmounts::const_iterator i =
                     secondaryCostAmounts_.begin();
                 i != secondaryCostAmounts_.end(); i++) {
                Real amount = i->second.value();
                NPV_ -= amount;
            }

            additionalResults_["dailyPositions"] = dailyPositions_;
          
        } catch (const QuantLib::Error& e) {
            addPricingError(PricingError::Error, e.what());
            throw;
        } catch (const std::exception& e) {
            addPricingError(PricingError::Error, e.what());
            throw;
        }
    }

}

