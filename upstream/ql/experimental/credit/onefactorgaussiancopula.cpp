/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

#include <ql/experimental/credit/onefactorgaussiancopula.hpp>

namespace QuantLib {

    //-----------------------------------------------------------------------
    Real OneFactorGaussianCopula::testCumulativeY (Real y) const {
    //-----------------------------------------------------------------------
        Real c = correlation_->value();

        if (c == 0)
            return CumulativeNormalDistribution()(y);

        if (c == 1)
            return CumulativeNormalDistribution()(y);

        NormalDistribution dz;
        NormalDistribution dm;

        Real minimum = -10;
        Real maximum = +10;
        int steps = 200;

        Real delta = (maximum - minimum) / steps;
        Real cumulated = 0;
        if (c < 0.5) {
            // outer integral -> 1 for c -> 0
            // inner integral -> CumulativeNormal()(y) for c-> 0
            for (Real m = minimum; m < maximum; m += delta)
                for (Real z = minimum; z < (y - std::sqrt(c) * m) / std::sqrt (1. - c);
                     z += delta)
                    cumulated += dm (m) * dz (z);
        }
        else {
            // outer integral -> 1 for c -> 1
            // inner integral -> CumulativeNormal()(y) for c-> 1
            for (Real z = minimum; z < maximum; z += delta)
                for (Real m = minimum; m < (y - std::sqrt(1.0 - c) * z) / std::sqrt(c);
                     m += delta)
                    cumulated += dm (m) * dz (z);
        }
        cumulated *= (delta * delta);

        return cumulated;
    }

}

