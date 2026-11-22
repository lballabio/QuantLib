/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Marek Glowacki

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

#include <ql/math/copulas/frankcopula.hpp>
#include <ql/errors.hpp>
#include <cmath>

namespace QuantLib {

    namespace {
        using std::exp;
        using std::expm1;
        using std::log;
        using std::log1p;

        // Both ratios have removable singularities at zero.  These expansions
        // avoid division by a rounded-to-zero argument (including subnormals).
        Real expm1OverX(Real z) {
            if (z > -1e-4 && z < 1e-4)
                return 1.0 + z * (0.5 + z * (1.0/6.0 + z *
                       (1.0/24.0 + z / 120.0)));
            return expm1(z) / z;
        }

        Real log1pOverX(Real z) {
            if (z > -1e-4 && z < 1e-4)
                return 1.0 + z * (-0.5 + z * (1.0/3.0 + z *
                       (-0.25 + z / 5.0)));
            return log1p(z) / z;
        }
    }

    FrankCopula::FrankCopula(Real theta): theta_(theta)
    {
        QL_REQUIRE(theta != 0.0,
                   "theta (" << theta << ") must be different from 0");
    }

    Real FrankCopula::operator()(Real x, Real y) const
    {
        QL_REQUIRE(x >= 0.0 && x <= 1.0,
                   "1st argument (" << x << ") must be in [0,1]");
        QL_REQUIRE(y >= 0.0 && y <= 1.0,
                   "2nd argument (" << y << ") must be in [0,1]");
        if (x == 0.0 || y == 0.0)
            return 0.0;
        if (x == 1.0)
            return y;
        if (y == 1.0)
            return x;

        const Real a = x < y ? x : y;
        const Real b = x < y ? y : x;

        if (theta_ > 1.0) {
            const Real u = -theta_ * a, v = -theta_ * b, t = -theta_;
            const Real ax = -expm1(u), bx = -expm1(v);
            const Real total = -expm1(t);
            const Real p = ax * bx / total;
            if (p < 0.5) {
                // Extract the smaller coordinate before multiplying: the
                // original numerator may underflow while the CDF does not.
                return a * (expm1OverX(u) * (bx / total) * log1pOverX(-p));
            }
            // 1-p = exp(-theta*a) * bracket / (1-exp(-theta)).
            const Real rem = -theta_ * (1.0 - a);
            const Real sep = -theta_ * (b - a);
            const Real bracket = -expm1(rem) + exp(sep) * ax;
            return a + (log(total) - log(bracket)) / theta_;
        }

        if (theta_ < -1.0) {
            const Real k = -theta_;
            const Real ka = k * a;
            if (ka <= 1.0) {
                // (exp(k*b)-1)/(exp(k)-1), without ever forming exp(k).
                const Real remaining = -k * (1.0 - b);
                const Real negKb = -k * b, negK = -k;
                const Real ratioB = exp(remaining) * (-expm1(negKb)) / (-expm1(negK));
                const Real q = expm1(ka) * ratioB;
                return a * (expm1OverX(ka) * ratioB * log1pOverX(q));
            }

            const Real negKa = -ka, negKb = -k * b, negK = -k;
            const Real ax = -expm1(negKa), bx = -expm1(negKb);
            const Real total = -expm1(negK);
            // Avoid rounding a+b to 1 before subtracting 1: this matters
            // when k is large and the lower Frechet bound is near zero.
            const Real sumMinusOne = b >= 0.5
                ? Real(a - (1.0 - b)) : Real((a - 0.5) + (b - 0.5));
            const Real z = k * sumMinusOne + log(ax) + log(bx)
                           - log(total);
            if (z > 0.0) {
                const Real negZ = -z, correction = exp(negZ);
                return (z + log1p(correction)) / k;
            }
            const Real correction = exp(z);
            return log1p(correction) / k;
        }

        // expm1(-theta*t) = -theta*t*expm1OverX(-theta*t).
        // Factor out a*b and cancel theta analytically; never form
        // log1p(something tiny)/theta or a prematurely underflowing a*b.
        const Real u = -theta_ * a, v = -theta_ * b, t = -theta_;
        const Real ratio = expm1OverX(u) * (expm1OverX(v) / expm1OverX(t));
        const Real z = u * (b * ratio);
        return a * (b * ratio * log1pOverX(z));
    }
}
