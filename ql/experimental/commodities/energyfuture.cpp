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
#include <ql/experimental/commodities/energyfuture.hpp>
#include <ql/settings.hpp>
#include <utility>

namespace QuantLib {

    EnergyFuture::EnergyFuture(Integer buySell,
                               Quantity quantity,
                               CommodityUnitCost tradePrice,
                               ext::shared_ptr<CommodityIndex> index,
                               const CommodityType& commodityType,
                               const ext::shared_ptr<SecondaryCosts>& secondaryCosts)
    : EnergyCommodity(commodityType, secondaryCosts), buySell_(buySell),
      quantity_(std::move(quantity)), tradePrice_(std::move(tradePrice)), index_(std::move(index)) {
        registerWith(Settings::instance().evaluationDate());
        registerWith(index_);
    }

    bool EnergyFuture::isExpired() const {
        return false;
    }

    void EnergyFuture::performCalculations() const {

        NPV_ = 0.0;
        additionalResults_.clear();

        Date evaluationDate = Settings::instance().evaluationDate();
        const Currency& baseCurrency =
            CommoditySettings::instance().currency();
        const UnitOfMeasure baseUnitOfMeasure =
            CommoditySettings::instance().unitOfMeasure();

        Real quantityUomConversionFactor =
            calculateUomConversionFactor(
                           quantity_.commodityType(),
                           baseUnitOfMeasure,
                           quantity_.unitOfMeasure()) * index_->lotQuantity();
        Real indexUomConversionFactor =
            calculateUomConversionFactor(index_->commodityType(),
                                         index_->unitOfMeasure(),
                                         baseUnitOfMeasure);
        Real tradePriceUomConversionFactor =
            calculateUomConversionFactor(quantity_.commodityType(),
                                         tradePrice_.unitOfMeasure(),
                                         baseUnitOfMeasure);

        Real tradePriceFxConversionFactor =
            calculateFxConversionFactor(tradePrice_.amount().currency(),
                                        baseCurrency, evaluationDate);
        Real indexPriceFxConversionFactor =
            calculateFxConversionFactor(index_->currency(), baseCurrency,
                                        evaluationDate);

        Real quoteValue = 0;

        Date lastQuoteDate = index_->lastQuoteDate();
        if (lastQuoteDate >= evaluationDate - 1) {
            quoteValue = index_->price(evaluationDate);
        } else {
            quoteValue = index_->forwardPrice(evaluationDate);
            std::ostringstream message;
            message << "curve [" << index_->name()
                    << "] has last quote date of "
                    << io::iso_date(lastQuoteDate)
                    << " using forward price from ["
                    << index_->forwardCurve()->name() << "]";
            addPricingError(PricingError::Warning, message.str());
        }

        QL_REQUIRE(quoteValue != Null<Real>(),
                   "missing quote for [" << index_->name() << "]");

        Real tradePriceValue =
            tradePrice_.amount().value() * tradePriceUomConversionFactor
            * tradePriceFxConversionFactor;
        Real quotePriceValue = quoteValue * indexUomConversionFactor
            * indexPriceFxConversionFactor;

        Real quantityAmount = quantity_.amount() * quantityUomConversionFactor;

        Real delta = (((quotePriceValue - tradePriceValue) * quantityAmount)
                      * index_->lotQuantity()) * buySell_;

        NPV_ = delta;

        calculateSecondaryCostAmounts(quantity_.commodityType(),
                                      quantity_.amount(), evaluationDate);
        for (auto & secondaryCostAmount : secondaryCostAmounts_) {
            Real amount = secondaryCostAmount.second.value();
            NPV_ -= amount;
        }

        // additionalResults_["brokerCommission"] =
        //     -(brokerCommissionValue * quantityAmount);
    }

}

