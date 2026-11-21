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

    FrankCopula::FrankCopula(Real theta): theta_(theta)
    {
        QL_REQUIRE(theta != 0.0,
                   "theta (" << theta << ") must be different from 0");
    }
    
    Real FrankCopula::operator()(Real x, Real y) const 
    {
        QL_REQUIRE(x >= 0.0 && x <=1.0 ,
                   "1st argument (" << x << ") must be in [0,1]");
        QL_REQUIRE(y >= 0.0 && y <=1.0 ,
                   "2nd argument (" << y << ") must be in [0,1]");
        if (x == 0.0 || y == 0.0)
            return 0.0;
        if (x == 1.0)
            return y;
        if (y == 1.0)
            return x;

        using std::exp;
        using std::log;
        using std::expm1;
        using std::log1p;

        if (theta_ > 1.0) {
            const Real a = x < y ? x : y;
            const Real b = x < y ? y : x;
            const Real u = -theta_ * a, v = -theta_ * b, t = -theta_;
            const Real ax = -expm1(u), bx = -expm1(v), total = -expm1(t);
            const Real p = ax * bx / total;
            if (p < 0.5) {
                const Real arg = -p;
                return -log1p(arg) / theta_;
            }
            // Factor exp(-theta*a) out of
            // exp(-theta*a) - exp(-theta) + exp(-theta*b)*(1-exp(-theta*a)).
            const Real rem = -theta_ * (1.0 - a);
            const Real sep = -theta_ * (b - a);
            const Real bracket = -expm1(rem) + exp(sep) * ax;
            const Real logTotal = log(total);
            return a + (logTotal - log(bracket)) / theta_;
        }
        if (theta_ < -1.0) {
            const Real a = x < y ? x : y;
            const Real b = x < y ? y : x;
            const Real k = -theta_;
            const Real tx = -k * a, ty = -k * b, tk = -k;
            const Real ax = -expm1(tx), bx = -expm1(ty), total = -expm1(tk);
            const Real logAx = log(ax), logBx = log(bx), logTotal = log(total);
            // Compute a+b-1 before multiplying by k. Near a+b=1 the two
            // differences are exact in binary floating point.
            const Real sumMinusOne = b >= 0.5
                ? Real(a - (1.0 - b)) : Real((a - 0.5) + (b - 0.5));
            const Real z = k * sumMinusOne + logAx + logBx - logTotal;
            if (z > 0.0) {
                const Real arg = -z, correction = exp(arg);
                return (z + log1p(correction)) / k;
            }
            const Real correction = exp(z);
            return log1p(correction) / k;
        }

        // Preserve the existing weak-dependence interior calculation.
        const Real tx = -theta_ * x, ty = -theta_ * y, t = -theta_;
        const Real base = 1.0 + (exp(tx) - 1.0) * (exp(ty) - 1.0) / (exp(t) - 1.0);
        return -1.0 / theta_ * log(base);
    }

}
