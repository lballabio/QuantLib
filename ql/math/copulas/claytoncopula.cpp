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

#include <ql/math/copulas/claytoncopula.hpp>
#include <ql/errors.hpp>
#include <algorithm>
#include <cmath>

namespace QuantLib {

    ClaytonCopula::ClaytonCopula(Real theta): theta_(theta)
    {
        QL_REQUIRE(theta >= -1.0,
                   "theta (" << theta << ") must be greater or equal to -1");
        QL_REQUIRE(theta != 0.0,
                   "theta (" << theta << ") must be different from 0");
    }
    
    Real ClaytonCopula::operator()(Real x, Real y) const 
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

        using std::pow;
        const Real base = pow(x, -theta_) + pow(y, -theta_) - 1.0;
        return pow(std::max(base, Real(0.0)), -1.0 / theta_);
    }

}
