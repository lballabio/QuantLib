/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

 Copyright (C) 2024
 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license. You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the license for more details.
*/

#include <ql/math/distributions/generalizedhyperbolicdistribution.hpp>
#include <ql/math/constants.hpp>
#include <boost/math/special_functions/bessel.hpp>
#include <cmath>

namespace QuantLib {

    class GeneralizedHyperbolicDistribution {
      public:
        GeneralizedHyperbolicDistribution(Real lambda, Real alpha, Real beta, Real delta, Real mu)
        : lambda_(lambda), alpha_(alpha), beta_(beta), delta_(delta), mu_(mu) {
            QL_REQUIRE(alpha > std::abs(beta), "alpha must be greater than |beta|");
        }

        Real operator()(Real x) const;

      private:
        Real lambda_, alpha_, beta_, delta_, mu_;
    };

    Real GeneralizedHyperbolicDistribution::operator()(Real x) const {
        Real z = (x - mu_) / delta_;
        Real gamma = std::sqrt(alpha_ * alpha_ - beta_ * beta_);
        Real scale = std::pow(gamma / alpha_, lambda_) / (std::sqrt(2 * M_PI) * delta_ * boost::math::cyl_bessel_k(lambda_, delta_ * gamma));
        Real exponent = -(alpha_ * std::sqrt(delta_ * delta_ + z * z) + beta_ * z);
        return scale * std::exp(exponent) * boost::math::cyl_bessel_k(lambda_ - 0.5, alpha_ * std::sqrt(delta_ * delta_ + z * z));
    }
}

#endif
