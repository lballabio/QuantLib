/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 Ferdinando Ametrano

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

#include <ql/pricingengines/basket/mceuropeanbasketengine.hpp>
#include <utility>

namespace QuantLib {

    EuropeanMultiPathPricer::EuropeanMultiPathPricer(ext::shared_ptr<BasketPayoff> payoff,
                                                     DiscountFactor discount)
    : payoff_(std::move(payoff)), discount_(discount) {}

    Real EuropeanMultiPathPricer::operator()(const MultiPath& multiPath)
                                                                      const {
        Size n = multiPath.pathSize();
        QL_REQUIRE(n>0, "the path cannot be empty");

        Size numAssets = multiPath.assetNumber();
        QL_REQUIRE(numAssets>0, "there must be some paths");

        Size j;
        // calculate the final price of each asset
        Array finalPrice(numAssets, 0.0);
        for (j = 0; j < numAssets; j++)
            finalPrice[j] = multiPath[j].back();
        return (*payoff_)(finalPrice) * discount_;
    }

}

