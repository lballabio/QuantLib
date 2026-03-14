/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/pricingengines/forward/mcforwardeuropeanhestonengine.hpp>

namespace QuantLib {

    ForwardEuropeanHestonPathPricer::ForwardEuropeanHestonPathPricer(
                                          Option::Type type,
                                          Real moneyness,
                                          Size resetIndex,
                                          DiscountFactor discount)
    : type_(type), moneyness_(moneyness), resetIndex_(resetIndex),
      discount_(discount)
       {
        QL_REQUIRE(moneyness>=0.0,
                   "moneyness less than zero not allowed");
    }

    Real ForwardEuropeanHestonPathPricer::operator()(const MultiPath& multiPath) const {
        const Path& path = multiPath[0];
        const Size n = multiPath.pathSize();
        QL_REQUIRE(n>0, "the path cannot be empty");

        const Real resetLevel = path[resetIndex_];
        const Real strike = resetLevel * moneyness_;
        const PlainVanillaPayoff payoff = PlainVanillaPayoff(type_, strike);

        return payoff(path.back()) * discount_;
    }

}