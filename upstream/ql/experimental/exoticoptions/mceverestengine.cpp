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

#include <ql/experimental/exoticoptions/mceverestengine.hpp>

namespace QuantLib {

    EverestMultiPathPricer::EverestMultiPathPricer(Real notional,
                                                   Real guarantee,
                                                   DiscountFactor discount)
    : notional_(notional), guarantee_(guarantee), discount_(discount) {}

    Real EverestMultiPathPricer::operator()(const MultiPath& multiPath) const {

        Size n = multiPath.pathSize();
        QL_REQUIRE(n>0, "the path cannot be empty");

        Size numAssets = multiPath.assetNumber();
        QL_REQUIRE(numAssets>0, "there must be some paths");

        // We search the yield min
        Real minYield = multiPath[0].back() / multiPath[0].front() - 1.0;
        for (Size j=1; j<numAssets; ++j) {
            Rate yield = multiPath[j].back() / multiPath[j].front() - 1.0;
            minYield = std::min(minYield, yield);
        }
        return (1.0 + minYield + guarantee_) * notional_ * discount_;
    }

}

