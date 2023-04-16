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

#include <ql/experimental/commodities/energyswap.hpp>
#include <ql/settings.hpp>
#include <utility>

namespace QuantLib {

    EnergySwap::EnergySwap(Calendar calendar,
                           Currency payCurrency,
                           Currency receiveCurrency,
                           PricingPeriods pricingPeriods,
                           const CommodityType& commodityType,
                           const std::shared_ptr<SecondaryCosts>& secondaryCosts)
    : EnergyCommodity(commodityType, secondaryCosts), calendar_(std::move(calendar)),
      payCurrency_(std::move(payCurrency)), receiveCurrency_(std::move(receiveCurrency)),
      pricingPeriods_(std::move(pricingPeriods)) {}

    const CommodityType& EnergySwap::commodityType() const {
        QL_REQUIRE(!pricingPeriods_.empty(), "no pricing periods");
        return pricingPeriods_[0]->quantity().commodityType();
    }

    Quantity EnergySwap::quantity() const {
        Real totalQuantityAmount = 0;
        for (const auto& pricingPeriod : pricingPeriods_) {
            totalQuantityAmount += pricingPeriod->quantity().amount();
        }
        return Quantity(pricingPeriods_[0]->quantity().commodityType(),
                        pricingPeriods_[0]->quantity().unitOfMeasure(),
                        totalQuantityAmount);
    }

    bool EnergySwap::isExpired() const {
        return pricingPeriods_.empty()
            || detail::simple_event(pricingPeriods_.back()->paymentDate())
               .hasOccurred();
    }

}

