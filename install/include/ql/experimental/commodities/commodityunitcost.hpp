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

/*! \file commodityunitcost.hpp
    \brief Commodity unit cost
*/

#ifndef quantlib_commodity_unit_cost_hpp
#define quantlib_commodity_unit_cost_hpp

#include <ql/experimental/commodities/unitofmeasure.hpp>
#include <ql/money.hpp>
#include <utility>

namespace QuantLib {

    class CommodityUnitCost {
      public:
        CommodityUnitCost() = default;
        CommodityUnitCost(Money amount, UnitOfMeasure unitOfMeasure)
        : amount_(std::move(amount)), unitOfMeasure_(std::move(unitOfMeasure)) {}

        const Money& amount() const { return amount_; }
        const UnitOfMeasure& unitOfMeasure() const { return unitOfMeasure_; }

      private:
        Money amount_;
        UnitOfMeasure unitOfMeasure_;
    };

    std::ostream& operator<<(std::ostream& out,
                             const CommodityUnitCost& unitCost);

}

#endif
