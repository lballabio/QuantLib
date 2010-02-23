/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/experimental/exoticoptions/mchimalayaengine.hpp>
#include <ql/payoff.hpp>

namespace QuantLib {

    HimalayaMultiPathPricer::HimalayaMultiPathPricer(
                                      const boost::shared_ptr<Payoff>& payoff,
                                      DiscountFactor discount)
    : payoff_(payoff), discount_(discount) {}

    Real HimalayaMultiPathPricer::operator()(const MultiPath& multiPath)
                                                                      const {
        Size numAssets = multiPath.assetNumber();
        Size numNodes = multiPath.pathSize();
        QL_REQUIRE(numAssets > 0, "no asset given");

        std::vector<bool> remainingAssets(numAssets, true);
        Real averagePrice = 0.0;
        Size fixings = numNodes-1;
        for (Size i = 1; i < numNodes; i++) {
            Real bestPrice = 0.0;
            Real bestYield = QL_MIN_REAL;
            // dummy assignement to avoid compiler warning
            Size removeAsset = 0;
            for (Size j = 0; j < numAssets; j++) {
                if (remainingAssets[j]) {
                    Real price = multiPath[j][i];
                    Real yield = price/multiPath[j].front();
                    if (yield >= bestYield) {
                        bestPrice = price;
                        removeAsset = j;
                    }
                }
            }
            remainingAssets[removeAsset] = false;
            averagePrice += bestPrice;
        }
        averagePrice /= std::min(fixings, numAssets);

        Real payoff = (*payoff_)(averagePrice);
        return payoff * discount_;
    }

}

