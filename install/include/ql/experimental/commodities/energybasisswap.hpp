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

/*! \file energybasisswap.hpp
    \brief Energy basis swap
*/

#ifndef quantlib_energy_basis_swap_hpp
#define quantlib_energy_basis_swap_hpp

#include <ql/experimental/commodities/energyswap.hpp>
#include <ql/experimental/commodities/commodityindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! Energy basis swap
    class EnergyBasisSwap : public EnergySwap {
      public:
        EnergyBasisSwap(const Calendar& calendar,
                        ext::shared_ptr<CommodityIndex> spreadIndex,
                        ext::shared_ptr<CommodityIndex> payIndex,
                        ext::shared_ptr<CommodityIndex> receiveIndex,
                        bool spreadToPayLeg,
                        const Currency& payCurrency,
                        const Currency& receiveCurrency,
                        const PricingPeriods& pricingPeriods,
                        CommodityUnitCost basis,
                        const CommodityType& commodityType,
                        const ext::shared_ptr<SecondaryCosts>& secondaryCosts,
                        Handle<YieldTermStructure> payLegTermStructure,
                        Handle<YieldTermStructure> receiveLegTermStructure,
                        Handle<YieldTermStructure> discountTermStructure);

        const ext::shared_ptr<CommodityIndex>& payIndex() const {
            return payIndex_;
        }
        const ext::shared_ptr<CommodityIndex>& receiveIndex() const {
            return receiveIndex_;
        }
        const CommodityUnitCost& basis() const { return basis_; }

      protected:
        void performCalculations() const override;

        ext::shared_ptr<CommodityIndex> spreadIndex_;
        ext::shared_ptr<CommodityIndex> payIndex_;
        ext::shared_ptr<CommodityIndex> receiveIndex_;
        bool spreadToPayLeg_;
        CommodityUnitCost basis_;
        Handle<YieldTermStructure> payLegTermStructure_;
        Handle<YieldTermStructure> receiveLegTermStructure_;
        Handle<YieldTermStructure> discountTermStructure_;
    };

}

#endif
