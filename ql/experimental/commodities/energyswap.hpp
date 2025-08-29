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

/*! \file energyswap.hpp
    \brief Energy swap
*/

#ifndef quantlib_energy_swap_hpp
#define quantlib_energy_swap_hpp

#include <ql/experimental/commodities/energycommodity.hpp>
#include <ql/experimental/commodities/pricingperiod.hpp>
#include <ql/experimental/commodities/commoditycashflow.hpp>
#include <ql/time/calendar.hpp>

namespace QuantLib {

    class EnergySwap : public EnergyCommodity {
      public:
        EnergySwap(Calendar calendar,
                   Currency payCurrency,
                   Currency receiveCurrency,
                   PricingPeriods pricingPeriods,
                   const CommodityType& commodityType,
                   const ext::shared_ptr<SecondaryCosts>& secondaryCosts);

        bool isExpired() const override;
        const Calendar& calendar() const { return calendar_; }
        const Currency& payCurrency() const { return payCurrency_; }
        const Currency& receiveCurrency() const { return receiveCurrency_; }
        const PricingPeriods& pricingPeriods() const { return pricingPeriods_; }
        const EnergyDailyPositions& dailyPositions() const {
            return dailyPositions_;
        }
        const CommodityCashFlows& paymentCashFlows() const {
            return paymentCashFlows_;
        }

        const CommodityType& commodityType() const;
        Quantity quantity() const override;

      protected:
        Calendar calendar_;
        Currency payCurrency_;
        Currency receiveCurrency_;
        PricingPeriods pricingPeriods_;
        mutable EnergyDailyPositions dailyPositions_;
        mutable CommodityCashFlows paymentCashFlows_;
    };

}

#endif
