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

namespace QuantLib {

    EnergySwap::EnergySwap(
                      const Calendar& calendar,
                      const Currency& payCurrency,
                      const Currency& receiveCurrency,
                      const PricingPeriods& pricingPeriods,
                      const CommodityType& commodityType,
                      const boost::shared_ptr<SecondaryCosts>& secondaryCosts)
    : EnergyCommodity(commodityType, secondaryCosts),
      calendar_(calendar), payCurrency_(payCurrency),
      receiveCurrency_(receiveCurrency), pricingPeriods_(pricingPeriods) {}

    const CommodityType& EnergySwap::commodityType() const {
        QL_REQUIRE(pricingPeriods_.size() > 0, "no pricing periods");
        return pricingPeriods_[0]->quantity().commodityType();
    }

    Quantity EnergySwap::quantity() const {
        Real totalQuantityAmount = 0;
        for (PricingPeriods::const_iterator pi = pricingPeriods_.begin();
             pi != pricingPeriods_.end(); pi++) {
            totalQuantityAmount += (*pi)->quantity().amount();
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

