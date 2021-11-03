/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Jack Gillett
 
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

#include <ql/pricingengines/asian/mc_discr_geom_av_price_heston.hpp>
#include <utility>

namespace QuantLib {

    GeometricAPOHestonPathPricer::GeometricAPOHestonPathPricer(Option::Type type,
                                                               Real strike,
                                                               DiscountFactor discount,
                                                               std::vector<Size> fixingIndices,
                                                               Real runningProduct,
                                                               Size pastFixings)
    : payoff_(type, strike), discount_(discount), fixingIndices_(std::move(fixingIndices)),
      runningProduct_(runningProduct), pastFixings_(pastFixings) {
        QL_REQUIRE(strike>=0.0,
            "strike less than zero not allowed");
    }

    Real GeometricAPOHestonPathPricer::operator()(const MultiPath& multiPath) const  {
        const Path& path = multiPath[0];
        const Size n = multiPath.pathSize();
        QL_REQUIRE(n>0, "the path cannot be empty");

        Real averagePrice = 1.0;
        Real product = runningProduct_;
        Size fixings = pastFixings_ + fixingIndices_.size();

        // care must be taken not to overflow product
        constexpr Real maxValue = QL_MAX_REAL;
        for (unsigned long fixingIndice : fixingIndices_) {
            Real price = path[fixingIndice];
            if (product < maxValue/price) {
                product *= price;
            } else {
                averagePrice *= std::pow(product, 1.0/fixings);
                product = price;
            }
        }

        averagePrice *= std::pow(product, 1.0/fixings);
        return discount_ * payoff_(averagePrice);
    }

}
