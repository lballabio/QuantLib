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

namespace QuantLib {

    LookbackFloatingPathPricer::LookbackFloatingPathPricer(
        Option::Type type,
        const std::vector<DiscountFactor>& discounts)
        : payoff_(type), discounts_(discounts) {
    }

    Real LookbackFloatingPathPricer::operator()(const Path& path) const {
        Size n = path.length();
        QL_REQUIRE(n>1, "the path cannot be empty");
        Real underlying;
        Real terminalPrice = path.back();
        Size i;
        Real winnerStrike;

        switch (payoff_.optionType()) {
            case Option::Call:
                winnerStrike = INT_MAX;

                for (i = 0; i < n - 1; i++) {
                    underlying = path[i + 1];
                    if (underlying < winnerStrike){
                        winnerStrike = underlying;
                    }
                }
                break;
            case Option::Put:
                winnerStrike = 0;

                for (i = 0; i < n - 1; i++) {
                    underlying = path[i + 1];
                    if (underlying > winnerStrike){
                        winnerStrike = underlying;
                    }
                }
                break;
            default:
                QL_FAIL("unknown option type");
        }

        return payoff_(terminalPrice, winnerStrike) * discounts_.back();
    }

}
