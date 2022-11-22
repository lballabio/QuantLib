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

#include <ql/experimental/commodities/commoditysettings.hpp>
#include <ql/experimental/commodities/energyvanillaswap.hpp>
#include <utility>

namespace QuantLib {

    EnergyVanillaSwap::EnergyVanillaSwap(bool payer,
                                         const Calendar& calendar,
                                         Money fixedPrice,
                                         UnitOfMeasure fixedPriceUnitOfMeasure,
                                         ext::shared_ptr<CommodityIndex> index,
                                         const Currency& payCurrency,
                                         const Currency& receiveCurrency,
                                         const PricingPeriods& pricingPeriods,
                                         const CommodityType& commodityType,
                                         const ext::shared_ptr<SecondaryCosts>& secondaryCosts,
                                         Handle<YieldTermStructure> payLegTermStructure,
                                         Handle<YieldTermStructure> receiveLegTermStructure,
                                         Handle<YieldTermStructure> discountTermStructure)
    : EnergySwap(
          calendar, payCurrency, receiveCurrency, pricingPeriods, commodityType, secondaryCosts),
      payReceive_(payer ? 1 : 0), fixedPrice_(std::move(fixedPrice)),
      fixedPriceUnitOfMeasure_(std::move(fixedPriceUnitOfMeasure)), index_(std::move(index)),
      payLegTermStructure_(std::move(payLegTermStructure)),
      receiveLegTermStructure_(std::move(receiveLegTermStructure)),
      discountTermStructure_(std::move(discountTermStructure)) {

        QL_REQUIRE(!pricingPeriods_.empty(), "no pricing periods");
        registerWith(index_);
    }

    bool EnergyVanillaSwap::isExpired() const {
        return detail::simple_event(pricingPeriods_.back()->endDate())
               .hasOccurred();
    }

    void EnergyVanillaSwap::performCalculations() const {

        try {
            if (index_->empty()) {
                if (index_->forwardCurveEmpty()) {
                    QL_FAIL("index [" << index_->name()
                            << "] does not have any quotes");
                } else {
                    addPricingError(PricingError::Warning,
                                    "index [" + index_->name() +
                                    "] does not have any quotes; "
                                    "using forward prices from ["
                                    + index_->forwardCurve()->name() + "]");
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
            Real fixedPriceUomConversionFactor =
                calculateUomConversionFactor(
                               pricingPeriods_[0]->quantity().commodityType(),
                               fixedPriceUnitOfMeasure_, baseUnitOfMeasure);
            Real indexUomConversionFactor =
                calculateUomConversionFactor(index_->commodityType(),
                                             index_->unitOfMeasure(),
                                             baseUnitOfMeasure);

            Real fixedPriceFxConversionFactor =
                calculateFxConversionFactor(fixedPrice_.currency(),
                                            baseCurrency, evaluationDate);
            Real indexPriceFxConversionFactor =
                calculateFxConversionFactor(index_->currency(),
                                            baseCurrency, evaluationDate);
            Real payLegFxConversionFactor =
                calculateFxConversionFactor(
                            baseCurrency,
                            payReceive_ > 0 ? payCurrency_ : receiveCurrency_,
                            evaluationDate);
            Real receiveLegFxConversionFactor =
                calculateFxConversionFactor(
                            baseCurrency,
                            payReceive_ > 0 ? receiveCurrency_ : payCurrency_,
                            evaluationDate);

            Date lastQuoteDate = index_->lastQuoteDate();
            if (lastQuoteDate < evaluationDate - 1) {
                std::ostringstream message;
                message << "index [" << index_->name()
                        << "] has last quote date of "
                        << io::iso_date(lastQuoteDate);
                addPricingError(PricingError::Warning, message.str());
            }

            Real totalQuantityAmount = 0;

            // price each period
            for (const auto& pricingPeriod : pricingPeriods_) {
                QL_REQUIRE(pricingPeriod->quantity().amount() != 0, "quantity is zero");

                Integer periodDayCount = 0;

                // get the futures quotes or everything after
                Date periodStartDate =
                    calendar_.adjust(pricingPeriod->startDate());
                for (Date stepDate = periodStartDate;
                     stepDate <= pricingPeriod->endDate();
                     stepDate = calendar_.advance(stepDate, 1*Days)) {

                    bool unrealized = stepDate > evaluationDate;
                    Real quoteValue = 0;

                    if (stepDate <= lastQuoteDate) {
                        quoteValue = index_->price(stepDate);
                    } else {
                        quoteValue = index_->forwardPrice(stepDate);
                    }

                    if (quoteValue == 0) {
                        std::ostringstream message;
                        message << "pay quote value for curve ["
                                << index_->name() << "] is 0 for date "
                                << io::iso_date(stepDate);
                        addPricingError(PricingError::Warning, message.str());
                    }

                    QL_REQUIRE(quoteValue != Null<Real>(),
                               "curve [" << index_->name() <<
                               "] missing value for pricing date: "
                               << stepDate);

                    Real fixedLegPriceValue =
                        fixedPrice_.value() * fixedPriceUomConversionFactor *
                        fixedPriceFxConversionFactor;
                    Real floatingLegPriceValue =
                        quoteValue * indexUomConversionFactor *
                        indexPriceFxConversionFactor;
                    Real payLegPriceValue =
                        payReceive_ > 0 ? fixedLegPriceValue :
                                          floatingLegPriceValue;
                    Real receiveLegPriceValue =
                        payReceive_ > 0 ? floatingLegPriceValue :
                                          fixedLegPriceValue;

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
                    periodDayCount == 0 ? Real(0) :
                                          periodQuantityAmount / periodDayCount;

                Real payLegValue = 0;
                Real receiveLegValue = 0;
                for (auto dpi = dailyPositions_.find(periodStartDate);
                     dpi != dailyPositions_.end() && dpi->first <= pricingPeriod->endDate();
                     ++dpi) {
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
                if (pricingPeriod->paymentDate() >= evaluationDate + 2) {
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
                    ((dDelta * payReceive_) > 0) ? payLegFxConversionFactor :
                                                   receiveLegFxConversionFactor;
                Currency pmtCurrency =
                    ((dDelta * payReceive_) > 0) ? receiveCurrency_ :
                                                   payCurrency_;
                Real pmtDiscountFactor =
                    (dDelta  > 0) ? receiveLegDiscountFactor :
                                    payLegDiscountFactor;

                paymentCashFlows_[pricingPeriod->paymentDate()] =
                    ext::make_shared<CommodityCashFlow>(
                           pricingPeriod->paymentDate(),
                                                 Money(baseCurrency,
                                                       uDelta * discountFactor),
                                                 Money(baseCurrency, uDelta),
                                                 Money(pmtCurrency,
                                                       dDelta * pmtFxConversionFactor),
                                                 Money(pmtCurrency,
                                                       uDelta * pmtFxConversionFactor),
                                                 discountFactor,
                                                 pmtDiscountFactor,
                                                 pricingPeriod->paymentDate() <= evaluationDate);

                calculateSecondaryCostAmounts(
                               pricingPeriods_[0]->quantity().commodityType(),
                               totalQuantityAmount, evaluationDate);

                NPV_ += dDelta;
            }

            QL_REQUIRE(!paymentCashFlows_.empty(), "no cashflows");

            for (SecondaryCostAmounts::const_iterator i =
                     secondaryCostAmounts_.begin();
                 i != secondaryCostAmounts_.end(); ++i) {
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

