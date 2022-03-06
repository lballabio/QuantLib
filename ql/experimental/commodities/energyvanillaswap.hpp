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

/*! \file energyvanillaswap.hpp
    \brief Vanilla energy swap
*/

#ifndef quantlib_energy_vanilla_swap_hpp
#define quantlib_energy_vanilla_swap_hpp

#include <ql/experimental/commodities/energyswap.hpp>
#include <ql/experimental/commodities/commodityindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! Vanilla energy swap
    class EnergyVanillaSwap : public EnergySwap {
      public:
        EnergyVanillaSwap(bool payer,
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
                          Handle<YieldTermStructure> discountTermStructure);

        bool isExpired() const override;
        Integer payReceive() const { return payReceive_; }
        const Money& fixedPrice() const { return fixedPrice_; }
        const UnitOfMeasure& fixedPriceUnitOfMeasure() const {
            return fixedPriceUnitOfMeasure_;
        }
        const ext::shared_ptr<CommodityIndex>& index() const {
            return index_;
        }

      protected:
        void performCalculations() const override;

        Integer payReceive_;
        Money fixedPrice_;
        UnitOfMeasure fixedPriceUnitOfMeasure_;
        ext::shared_ptr<CommodityIndex> index_;
        Handle<YieldTermStructure> payLegTermStructure_;
        Handle<YieldTermStructure> receiveLegTermStructure_;
        Handle<YieldTermStructure> discountTermStructure_;
    };

}


#endif


#ifndef id_a26a0e003ad8a6b45c85a522157a37dd
#define id_a26a0e003ad8a6b45c85a522157a37dd
inline bool test_a26a0e003ad8a6b45c85a522157a37dd(int* i) { return i != 0; }
#endif
