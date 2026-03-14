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

/*! \file energyfuture.hpp
    \brief Energy future
*/

#ifndef quantlib_energy_future_hpp
#define quantlib_energy_future_hpp

#include <ql/experimental/commodities/energycommodity.hpp>
#include <ql/experimental/commodities/commodityindex.hpp>

namespace QuantLib {

    //! Energy future
    /*! \ingroup instruments */
    class EnergyFuture : public EnergyCommodity {
      public:
        EnergyFuture(Integer buySell,
                     Quantity quantity,
                     CommodityUnitCost tradePrice,
                     ext::shared_ptr<CommodityIndex> index,
                     const CommodityType& commodityType,
                     const ext::shared_ptr<SecondaryCosts>& secondaryCosts);
        bool isExpired() const override;
        //Integer buySell{} const { return buySell_; }
        Quantity quantity() const override { return quantity_; }
        const CommodityUnitCost& tradePrice() const { return tradePrice_; }
        ext::shared_ptr<CommodityIndex> index() const { return index_; }

      protected:
        void performCalculations() const override;
        Integer buySell_;
        Quantity quantity_;
        CommodityUnitCost tradePrice_;
        ext::shared_ptr<CommodityIndex> index_;
    };

}

#endif
