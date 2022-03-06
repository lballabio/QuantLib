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

/*! \file commoditypricinghelpers.hpp
    \brief Commodity pricing helpers
*/

#ifndef quantlib_commodity_pricing_helpers_hpp
#define quantlib_commodity_pricing_helpers_hpp

#include <ql/experimental/commodities/energycommodity.hpp>
#include <ql/experimental/commodities/paymentterm.hpp>
#include <ql/experimental/commodities/pricingperiod.hpp>
#include <ql/experimental/commodities/unitofmeasureconversionmanager.hpp>
#include <ql/currencies/exchangeratemanager.hpp>

namespace QuantLib {

    //! commodity index helper
    class CommodityPricingHelper {
      public:
        CommodityPricingHelper();

        static Real calculateFxConversionFactor(const Currency& fromCurrency,
                                                const Currency& toCurrency,
                                                const Date& evaluationDate);

        static Real calculateUomConversionFactor(
                                       const CommodityType& commodityType,
                                       const UnitOfMeasure& fromUnitOfMeasure,
                                       const UnitOfMeasure& toUnitOfMeasure);

        static Real calculateUnitCost(const CommodityType& commodityType,
                                      const CommodityUnitCost& unitCost,
                                      const Currency& baseCurrency,
                                      const UnitOfMeasure& baseUnitOfMeasure,
                                      const Date& evaluationDate);

        static void createPricingPeriods(
                          Date startDate, Date endDate,
                          const Quantity& quantity,
                          EnergyCommodity::DeliverySchedule deliverySchedule,
                          EnergyCommodity::QuantityPeriodicity qtyPeriodicity,
                          const PaymentTerm& paymentTerm,
                          PricingPeriods& pricingPeriods);
    };


    inline Real CommodityPricingHelper::calculateUomConversionFactor(
                                       const CommodityType& commodityType,
                                       const UnitOfMeasure& fromUnitOfMeasure,
                                       const UnitOfMeasure& toUnitOfMeasure) {
        if (toUnitOfMeasure != fromUnitOfMeasure) {
            UnitOfMeasureConversion uomConv =
                UnitOfMeasureConversionManager::instance().lookup(
                           commodityType, fromUnitOfMeasure, toUnitOfMeasure);
            return uomConv.conversionFactor();
        }

        return 1;
    }

    inline Real CommodityPricingHelper::calculateFxConversionFactor(
                                                 const Currency& fromCurrency,
                                                 const Currency& toCurrency,
                                                 const Date& evaluationDate) {
        if (fromCurrency != toCurrency) {
            ExchangeRate exchRate =
                ExchangeRateManager::instance().lookup(fromCurrency,
                                                       toCurrency,
                                                       evaluationDate,
                                                       ExchangeRate::Direct);
            if (fromCurrency != exchRate.source())
                return (Real)1 / exchRate.rate();
            return exchRate.rate();
        }
        return 1;
    }

    inline Real CommodityPricingHelper::calculateUnitCost(
                                       const CommodityType& commodityType,
                                       const CommodityUnitCost& unitCost,
                                       const Currency& baseCurrency,
                                       const UnitOfMeasure& baseUnitOfMeasure,
                                       const Date& evaluationDate) {
        if (unitCost.amount().value() != 0) {
            Real unitCostUomConversionFactor =
                calculateUomConversionFactor(commodityType,
                                             unitCost.unitOfMeasure(),
                                             baseUnitOfMeasure);
            Real unitCostFxConversionFactor =
                calculateFxConversionFactor(unitCost.amount().currency(),
                                            baseCurrency, evaluationDate);
            return unitCost.amount().value() * unitCostUomConversionFactor
                 * unitCostFxConversionFactor;
        }
        return 0;
    }

}

#endif


#ifndef id_f96dd53a0398fb7e8e303cc98885a9f7
#define id_f96dd53a0398fb7e8e303cc98885a9f7
inline bool test_f96dd53a0398fb7e8e303cc98885a9f7(const int* i) {
    return i != nullptr;
}
#endif
