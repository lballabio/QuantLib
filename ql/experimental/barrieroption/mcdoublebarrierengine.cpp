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

#include <ql/experimental/barrieroption/mcdoublebarrierengine.hpp>
#include <utility>

namespace QuantLib {

    DoubleBarrierPathPricer::DoubleBarrierPathPricer(DoubleBarrier::Type barrierType,
                                                     Real barrierLow,
                                                     Real barrierHigh,
                                                     Real rebate,
                                                     Option::Type type,
                                                     Real strike,
                                                     std::vector<DiscountFactor> discounts)
    : barrierType_(barrierType), barrierLow_(barrierLow), barrierHigh_(barrierHigh),
      rebate_(rebate), payoff_(type, strike), discounts_(std::move(discounts)) {
        QL_REQUIRE(strike>=0.0,
                   "strike less than zero not allowed");
        QL_REQUIRE(barrierLow>0.0,
                   "low barrier less/equal zero not allowed");
        QL_REQUIRE(barrierHigh>0.0,
                   "high barrier less/equal zero not allowed");
    }

    Real DoubleBarrierPathPricer::operator()(const Path& path) const {
        static Size null = Null<Size>();
        Size n = path.length();
        QL_REQUIRE(n>1, "the path cannot be empty");

        bool isOptionActive = false;
        Size knockNode = null;
        Real terminal_price = path.back();
        Real new_asset_price;
        Size i;

        switch (barrierType_) {
            case DoubleBarrier::KnockOut:
                isOptionActive = true;
                for (i = 0; i < n-1; i++) {
                    new_asset_price = path[i + 1];

                    if (new_asset_price >= barrierHigh_ || new_asset_price <= barrierLow_){
                        isOptionActive = false;
                        if (knockNode == null)
                            knockNode = i+1;
                        break;
                    }
                }
                break;
            case DoubleBarrier::KnockIn:
                isOptionActive = false;
                for (i = 0; i < n-1; i++) {
                    new_asset_price = path[i + 1];

                    if (new_asset_price >= barrierHigh_ || new_asset_price <= barrierLow_){
                        isOptionActive = true;
                        if (knockNode == null)
                            knockNode = i+1;
                        break;
                    }
                }
                break;
            default:
                QL_FAIL("unknown barrier type");
        }

        if (isOptionActive) {
            return payoff_(terminal_price) * discounts_.back();
        } else {
            switch (barrierType_) {
                case DoubleBarrier::KnockOut:
                    return rebate_*discounts_[knockNode];
                case DoubleBarrier::KnockIn:
                    return rebate_*discounts_.back();
                default:
                    QL_FAIL("unknown barrier type");
            }
        }
    }

}
