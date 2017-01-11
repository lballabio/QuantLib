/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis
 Copyright (C) 2017 Peter Caspers

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

#include <ql/pricingengines/cliquet/mcperformanceengine.hpp>

namespace QuantLib {

    PerformanceOptionPathPricer::PerformanceOptionPathPricer(
                                 Option::Type type, Real strike,
                                 Real localCap, Real localFloor,
                                 Real globalCap, Real globalFloor,
                                 Real accruedCoupon, Real lastFixing,
                                 const std::vector<DiscountFactor>& discounts)
        : strike_(strike), localCap_(localCap), localFloor_(localFloor),
          globalCap_(globalCap), globalFloor_(globalFloor),
          accruedCoupon_(accruedCoupon), lastFixing_(lastFixing),
          type_(type), discounts_(discounts) {}

    Real PerformanceOptionPathPricer::operator()(const Path& path) const {

        Size n = path.length();
        QL_REQUIRE(n==discounts_.size()+1, "discounts/options mismatch");
        PlainVanillaPayoff payoff(type_,strike_);

        Real sum = accruedCoupon_ == Null<Real>() ? 0.0 : accruedCoupon_;
        Real lastFixing = lastFixing_, currentFixing;
        for (Size i = 1 ; i < n; i++) {
            currentFixing = path[i];
            if (i > 1 || lastFixing != Null<Real>())
                sum += discounts_[i - 1] *
                    std::min(std::max(payoff(currentFixing / lastFixing), localFloor_), localCap_);
            lastFixing = currentFixing;
        }

        return std::min(std::max(sum, globalFloor_), globalCap_);
    }

}

