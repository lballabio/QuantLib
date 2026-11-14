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

#include <ql/math/distributions/studenttdistribution.hpp>
#include <ql/math/distributions/gammadistribution.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/beta.hpp>

#include <boost/math/distributions/students_t.hpp>

namespace QuantLib {

    Real StudentDistribution::operator()(Real x) const {
        static GammaFunction G;
        // the gamma functions and the power overflow for large n: work with logarithms
        const Real n = n_;
        const Real logNormalization =
            G.logValue(0.5 * (n + 1.0)) - G.logValue(0.5 * n) - 0.5 * std::log(M_PI * n);
        return std::exp(logNormalization - 0.5 * (n + 1.0) * std::log1p(x * x / n));
    }

    Real CumulativeStudentDistribution::operator()(Real x) const {
        Real xx = 1.0 * n_ / (x*x + n_);
        Real sig = (x > 0 ? 1.0 : - 1.0);

        return 0.5 + 0.5 * sig * ( incompleteBetaFunction (0.5 * n_, 0.5, 1.0)
                                   -incompleteBetaFunction (0.5 * n_, 0.5, xx));
    }

    Real InverseCumulativeStudent::operator()(Real y) const {
        if (y <= 0.0 || y >= 1.0) {
            if (close_enough(y, 1.0)) {
                return QL_MAX_REAL;
            } else if (std::fabs(y) < QL_EPSILON) {
                return QL_MIN_REAL;
            } else {
                QL_FAIL("InverseCumulativeStudent(" << y
                        << ") undefined: must be 0 < x < 1");
            }
        }

        if (y == 0.5)
            return 0.0;

        return boost::math::quantile(
            boost::math::students_t_distribution<Real>(
                static_cast<Real>(n_)), y);
    }

}
