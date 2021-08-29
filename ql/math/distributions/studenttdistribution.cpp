/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

#include <ql/math/distributions/studenttdistribution.hpp>
#include <ql/math/distributions/gammadistribution.hpp>
#include <ql/math/beta.hpp>

namespace QuantLib {

    Real StudentDistribution::operator()(Real x) const {
        Real g1 = std::exp (GammaFunction::logValue(0.5 * (n_ + 1)));
        Real g2 = std::exp (GammaFunction::logValue(0.5 * n_));

        Real power = std::pow (1. + x*x / n_, 0.5 * (n_ + 1));

        return g1 / (g2 * power * std::sqrt (M_PI * n_));
    }

    Real CumulativeStudentDistribution::operator()(Real x) const {
        Real xx = 1.0 * n_ / (x*x + n_);
        Real sig = (x > 0 ? 1.0 : - 1.0);

        return 0.5 + 0.5 * sig * ( incompleteBetaFunction (0.5 * n_, 0.5, 1.0)
                                   -incompleteBetaFunction (0.5 * n_, 0.5, xx));
    }

    Real InverseCumulativeStudent::operator()(Real y) const {
        QL_REQUIRE (y >= 0 && y <= 1, "argument out of range [0, 1]");

        Real x = 0;
        Size count = 0;

        // do a few newton steps to find x
        do {
            x -= (f_(x) - y) / d_(x);
            count++;
        }
        while (std::fabs(f_(x) - y) > accuracy_ && count < maxIterations_);

        QL_REQUIRE (count < maxIterations_,
                    "maximum number of iterations " << maxIterations_
                    << " reached in InverseCumulativeStudent, "
                    << "y=" << y << ", x=" << x);

        return x;
    }

}

