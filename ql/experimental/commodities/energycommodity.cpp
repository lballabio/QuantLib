/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall

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

#include <ql/currencies/exchangeratemanager.hpp>
#include <ql/experimental/commodities/commoditysettings.hpp>
#include <ql/experimental/commodities/energycommodity.hpp>
#include <ql/experimental/commodities/unitofmeasureconversionmanager.hpp>
#include <iomanip>
#include <utility>

namespace QuantLib {

    EnergyDailyPosition::EnergyDailyPosition(const Date& date,
                                             Real payLegPrice,
                                             Real receiveLegPrice,
                                             bool unrealized)
    : date(date), quantityAmount(0), payLegPrice(payLegPrice),
      receiveLegPrice(receiveLegPrice), unrealized(unrealized) {}

    std::ostream& operator<<(std::ostream& out,
                             const EnergyDailyPositions& dailyPositions) {
        out << std::setw(12) << std::left << "positions"
            << std::setw(12) << std::right << "pay"
            << std::setw(12) << std::right << "receive"
            << std::setw(10) << std::right << "qty"
            << std::setw(14) << std::right << "delta"
            << std::setw(10) << std::right << "open" << std::endl;

        for (const auto& i : dailyPositions) {
            const EnergyDailyPosition& dailyPosition = i.second;
            out << std::setw(4) << io::iso_date(i.first) << "  " << std::setw(12) << std::right
                << std::fixed << std::setprecision(6) << dailyPosition.payLegPrice << std::setw(12)
                << std::right << std::fixed << std::setprecision(6) << dailyPosition.receiveLegPrice
                << std::setw(10) << std::right << std::fixed << std::setprecision(2)
                << dailyPosition.quantityAmount << std::setw(14) << std::right << std::fixed
                << std::setprecision(2) << dailyPosition.riskDelta << std::setw(10) << std::right
                << std::fixed << std::setprecision(2)
                << (dailyPosition.unrealized ? dailyPosition.quantityAmount : 0) << std::endl;
        }

        return out;
    }





    void EnergyCommodity::setupArguments(PricingEngine::arguments* args) const {
        auto* arguments = dynamic_cast<EnergyCommodity::arguments*>(args);
        QL_REQUIRE(arguments != nullptr, "wrong argument type");
        //arguments->legs = legs_;
        //arguments->payer = payer_;
    }

    void EnergyCommodity::fetchResults(const PricingEngine::results* r) const {
        Instrument::fetchResults(r);
        const auto* results = dynamic_cast<const EnergyCommodity::results*>(r);
        QL_REQUIRE(results != nullptr, "wrong result type");
    }

    EnergyCommodity::EnergyCommodity(CommodityType commodityType,
                                     const ext::shared_ptr<SecondaryCosts>& secondaryCosts)
    : Commodity(secondaryCosts), commodityType_(std::move(commodityType)) {}

    const CommodityType& EnergyCommodity::commodityType() const {
        return commodityType_;
    }


    Real EnergyCommodity::calculateUomConversionFactor(
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

    Real EnergyCommodity::calculateFxConversionFactor(
                                                 const Currency& fromCurrency,
                                                 const Currency& toCurrency,
                                                 const Date& evaluationDate) {
        if (fromCurrency != toCurrency) {
            ExchangeRate exchRate = ExchangeRateManager::instance().lookup(
                                   fromCurrency, toCurrency,
                                   evaluationDate /*, ExchangeRate::Direct*/);
            if (fromCurrency == exchRate.target())
                return 1.0 / exchRate.rate();
            return exchRate.rate();
        }
        return 1;
    }

    Real EnergyCommodity::calculateUnitCost(const CommodityType& commodityType,
                                            const CommodityUnitCost& unitCost,
                                            const Date& evaluationDate) const {
        if (unitCost.amount().value() != 0) {
            const Currency& baseCurrency =
                CommoditySettings::instance().currency();
            const UnitOfMeasure baseUnitOfMeasure =
                CommoditySettings::instance().unitOfMeasure();
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

    void EnergyCommodity::calculateSecondaryCostAmounts(
                                           const CommodityType& commodityType,
                                           Real totalQuantityValue,
                                           const Date& evaluationDate) const {
        secondaryCostAmounts_.clear();
        if (secondaryCosts_ != nullptr) {
            const Currency& baseCurrency =
                CommoditySettings::instance().currency();
            try {
                for (auto & i : *secondaryCosts_) {
                    if (ext::any_cast<CommodityUnitCost>(&i.second) != nullptr) {
                        Real value =
                            calculateUnitCost(
                                commodityType,
                                ext::any_cast<CommodityUnitCost>(i.second),
                                evaluationDate) * totalQuantityValue;
                        secondaryCostAmounts_[i.first] =
                            Money(baseCurrency, value);
                    } else if (ext::any_cast<Money>(&i.second) != nullptr) {
                        const Money& amount = ext::any_cast<Money>(i.second);
                        Real fxConversionFactor =
                            calculateFxConversionFactor(amount.currency(),
                                                        baseCurrency,
                                                        evaluationDate);
                        secondaryCostAmounts_[i.first] =
                            Money(baseCurrency,
                                  amount.value() * fxConversionFactor);
                    }
                }
            } catch (const std::exception& e) {
                QL_FAIL("error calculating secondary costs: " << e.what());
            }
        }
    }

}

