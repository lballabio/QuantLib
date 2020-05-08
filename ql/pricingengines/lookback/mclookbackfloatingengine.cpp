/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Lew Wei Hao

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

#include "mclookbackfloatingengine.hpp"
#include <algorithm>

namespace QuantLib {

    LookbackFloatingPathPricer::LookbackFloatingPathPricer(
        Option::Type type,
        const DiscountFactor discount)
    : payoff_(type), discount_(discount) {}

    Real LookbackFloatingPathPricer::operator()(const Path& path) const {
        QL_REQUIRE(!path.empty(), "the path cannot be empty");

        Real terminalPrice = path.back();
        Real strike;
        switch (payoff_.optionType()) {
          case Option::Call:
            strike = *std::min_element(path.begin()+1, path.end());
            break;
          case Option::Put:
            strike = *std::max_element(path.begin()+1, path.end());
            break;
          default:
            QL_FAIL("unknown option type");
        }

        return payoff_(terminalPrice, strike) * discount_;
    }

}
