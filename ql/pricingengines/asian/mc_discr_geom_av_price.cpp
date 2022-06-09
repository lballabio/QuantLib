/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004 Ferdinando Ametrano

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

#include <ql/pricingengines/asian/mc_discr_geom_av_price.hpp>

namespace QuantLib {

    GeometricAPOPathPricer::GeometricAPOPathPricer(
                                         Option::Type type,
                                         Real strike, DiscountFactor discount,
                                         Real runningProduct, Size pastFixings)
    : payoff_(type, strike), discount_(discount),
      runningProduct_(runningProduct), pastFixings_(pastFixings) {
        QL_REQUIRE(strike>=0.0, "negative strike given");
    }

    Real GeometricAPOPathPricer::operator()(const Path& path) const {
        Size n = path.length() - 1;
        QL_REQUIRE(n>0, "the path cannot be empty");

        Real averagePrice;
        Real product = runningProduct_;
        Size fixings = n+pastFixings_;
        if (path.timeGrid().mandatoryTimes()[0]==0.0) {
            fixings += 1;
            product *= path.front();
        }
        // care must be taken not to overflow product
        constexpr double maxValue = QL_MAX_REAL;
        averagePrice = 1.0;
        for (Size i=1; i<n+1; i++) {
            Real price = path[i];
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
