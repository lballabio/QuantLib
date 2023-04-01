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

#include <ql/experimental/exoticoptions/mcpagodaengine.hpp>

namespace QuantLib {

    PagodaMultiPathPricer::PagodaMultiPathPricer(Real roof, Real fraction,
                                                 DiscountFactor discount)
    : discount_(discount), roof_(roof), fraction_(fraction) {}

    Real PagodaMultiPathPricer::operator()(const MultiPath& multiPath) const {

        Size numAssets = multiPath.assetNumber();
        Size numSteps = multiPath.pathSize();

        Real averagePerformance = 0.0;
        for (Size i = 1; i < numSteps; i++) {
            for (Size j = 0; j < numAssets; j++) {
                averagePerformance +=
                    multiPath[j].front() *
                    (multiPath[j][i]/multiPath[j][i-1] - 1.0);
            }
        }
        averagePerformance /= Real(numAssets);

        return discount_ * fraction_
            * std::max<Real>(0.0, std::min(roof_, averagePerformance));
    }

}

