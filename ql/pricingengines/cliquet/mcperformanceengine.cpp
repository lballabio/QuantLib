/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/pricingengines/cliquet/mcperformanceengine.hpp>
#include <utility>

namespace QuantLib {

    PerformanceOptionPathPricer::PerformanceOptionPathPricer(Option::Type type,
                                                             Real strike,
                                                             std::vector<DiscountFactor> discounts)
    : strike_(strike), type_(type), discounts_(std::move(discounts)) {}

    Real PerformanceOptionPathPricer::operator()(const Path& path) const {

        Size n = path.length();
        QL_REQUIRE(n==discounts_.size()+1, "discounts/options mismatch");
        PlainVanillaPayoff payoff(type_,strike_);

        Real sum = 0.0;
        for (Size i = 2 ; i < n; i++) {
            sum += discounts_[i-1] * payoff(path[i]/path[i-1]);
        }

        return sum;
    }

}

