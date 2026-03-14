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

#include <ql/pricingengines/asian/mc_discr_arith_av_price_heston.hpp>
#include <utility>

namespace QuantLib {

    ArithmeticAPOHestonPathPricer::ArithmeticAPOHestonPathPricer(Option::Type type,
                                                                 Real strike,
                                                                 DiscountFactor discount,
                                                                 std::vector<Size> fixingIndices,
                                                                 Real runningSum,
                                                                 Size pastFixings)
    : payoff_(type, strike), discount_(discount), fixingIndices_(std::move(fixingIndices)),
      runningSum_(runningSum), pastFixings_(pastFixings) {
        QL_REQUIRE(strike>=0.0,
            "strike less than zero not allowed");
    }

    Real ArithmeticAPOHestonPathPricer::operator()(const MultiPath& multiPath) const  {
        const Path& path = multiPath[0];
        const Size n = multiPath.pathSize();
        QL_REQUIRE(n>0, "the path cannot be empty");

        Real sum = runningSum_;
        Size fixings = pastFixings_ + fixingIndices_.size();

        for (unsigned long fixingIndice : fixingIndices_) {
            sum += path[fixingIndice];
        }

        Real averagePrice = sum/fixings;
        return discount_ * payoff_(averagePrice);
    }

}
