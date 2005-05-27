/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/PricingEngines/Basket/mcbasketengine.hpp>

namespace QuantLib {

    EuropeanMultiPathPricer::EuropeanMultiPathPricer(
                                      BasketOption::BasketType basketType,
                                      Option::Type type,
                                      Real strike,
                                      Array underlying,
                                      DiscountFactor discount)
    : basketType_(basketType), underlying_(underlying),
      payoff_(type, strike), discount_(discount) {

        // check underlying is not zero
        for (Size i = 0; i < underlying.size(); i++) {
            QL_REQUIRE(underlying[i]>0.0,
                       "underlying less/equal zero not allowed");
        }
        QL_REQUIRE(strike>=0.0,
                   "strike less than zero not allowed");
    }

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

        // this should be a basket payoff
        Real basketPrice = finalPrice[0];
        switch (basketType_) {
          case BasketOption::Max:
            for (j = 1; j < numAssets; j++) {
                if (finalPrice[j] > basketPrice) {
                    basketPrice = finalPrice[j];
                }
            }
          break;
          case BasketOption::Min:
            for (j = 1; j < numAssets; j++) {
                if (finalPrice[j] < basketPrice) {
                    basketPrice = finalPrice[j];
                }
            }
          break;
        }

        // return the payoff
        return payoff_(basketPrice) * discount_;
    }

}

