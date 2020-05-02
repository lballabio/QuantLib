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

#include "mclookbackpartialfixedengine.hpp"

namespace QuantLib {

    LookbackPartialFixedPathPricer::LookbackPartialFixedPathPricer(
        Time lookbackStart,
        Option::Type type,
        Real strike,
        const std::vector<DiscountFactor>& discounts)
        : lookbackStart_(lookbackStart), payoff_(type, strike), discounts_(discounts) {
        QL_REQUIRE(strike>=0.0,
                   "strike less than zero not allowed");
    }

    Real LookbackPartialFixedPathPricer::operator()(const Path& path) const {
        Size n = path.length();
        QL_REQUIRE(n>1, "the path cannot be empty");
        Real underlying;
        TimeGrid timeGrid = path.timeGrid();
        Size lookbackStart = timeGrid.closestIndex(lookbackStart_);
        Size i;
        Real winnerUnderlying;

        switch (payoff_.optionType()) {
            case Option::Put:
                winnerUnderlying = INT_MAX;

                for (i = lookbackStart; i < n-1; i++) {
                    underlying = path[i + 1];
                    if (underlying < winnerUnderlying){
                        winnerUnderlying = underlying;
                    }
                }
                break;
            case Option::Call:
                winnerUnderlying = 0;

                for (i = lookbackStart; i < n-1; i++) {
                    underlying = path[i + 1];
                    if (underlying > winnerUnderlying){
                        winnerUnderlying = underlying;
                    }
                }
                break;
            default:
                QL_FAIL("unknown option type");
        }

        return payoff_(winnerUnderlying) * discounts_.back();
    }

}
