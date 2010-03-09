/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andrea Odetti

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

#include <ql/experimental/mcbasket/mcpathbasketengine.hpp>

namespace QuantLib {

    EuropeanPathMultiPathPricer::EuropeanPathMultiPathPricer(
                                       boost::shared_ptr<PathPayoff> & payoff,
                                       const std::vector<Size> & timePositions,
                                       const std::vector<Handle<YieldTermStructure> > & forwardTermStructures,
                                       const Array & discounts)
    :  payoff_(payoff), timePositions_(timePositions), forwardTermStructures_(forwardTermStructures), discounts_(discounts) {}

    Real EuropeanPathMultiPathPricer::operator()(const MultiPath& multiPath)
                                                                       const {

        Size n = multiPath.pathSize();
        QL_REQUIRE(n > 0, "the path cannot be empty");

        Size numberOfAssets = multiPath.assetNumber();
        QL_REQUIRE(numberOfAssets > 0, "there must be some paths");

        const Size numberOfTimes = timePositions_.size();

        Matrix path(numberOfAssets, numberOfTimes, Null<Real>());

        for (Size i = 0; i < numberOfTimes; ++i) {
            const Size pos = timePositions_[i];
            for (Size j = 0; j < numberOfAssets; ++j)
                path[j][i] = multiPath[j][pos];
        }

        Array values(numberOfTimes, 0.0);

        // ignored
        Array exercises;
        std::vector<Array> states;

        payoff_->value(path, forwardTermStructures_, values, exercises, states);

        // in this engine we ignore early exercise

        Real discountedPayoff = DotProduct(values, discounts_);

        return discountedPayoff;
    }

}

