/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano

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

#include <ql/pricingengines/blackscholescalculator.hpp>

namespace QuantLib {

    BlackScholesCalculator::BlackScholesCalculator(
                        const ext::shared_ptr<StrikedTypePayoff>& payoff,
                        Real spot,
                        DiscountFactor growth,
                        Real stdDev,
                        DiscountFactor discount)
    : BlackCalculator(payoff, spot*growth/discount, stdDev, discount),
      spot_(spot), growth_(growth)
    {
        QL_REQUIRE(spot_>0.0,
                   "spot (" << spot_ << ") must be positive");
        QL_REQUIRE(growth_>0.0,
                   "growth (" << growth_ << ") must be positive");
    }

    BlackScholesCalculator::BlackScholesCalculator(Option::Type type,
                                                   Real strike,
                                                   Real spot,
                                                   DiscountFactor growth,
                                                   Real stdDev,
                                                   DiscountFactor discount)
    : BlackCalculator(type, strike, spot*growth/discount, stdDev, discount),
      spot_(spot), growth_(growth)
    {
        QL_REQUIRE(spot_>0.0,
                   "spot (" << spot_ << ") must be positive");
        QL_REQUIRE(growth_>0.0,
                   "growth (" << growth_ << ") must be positive");
    }

}
