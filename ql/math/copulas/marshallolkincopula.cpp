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

#include <ql/math/copulas/marshallolkincopula.hpp>
#include <ql/errors.hpp>
#include <algorithm>

namespace QuantLib {

    MarshallOlkinCopula::MarshallOlkinCopula(Real a1,Real a2)
    : a1_(1.0-a1), a2_(1.0-a2)
    {
        QL_REQUIRE(a1 >= 0.0,
                   "1st parameter (" << a1 << ") must be non-negative");
        QL_REQUIRE(a2 >= 0.0,
                   "2nd parameter (" << a2 << ") must be non-negative");
    }
    
    Real MarshallOlkinCopula::operator()(Real x, Real y) const 
    {
        QL_REQUIRE(x >= 0.0 && x <=1.0 ,
                   "1st argument (" << x << ") must be in [0,1]");
        QL_REQUIRE(y >= 0.0 && y <=1.0 ,
                   "2nd argument (" << y << ") must be in [0,1]");
        return std::min(  y*std::pow(x, a1_)  ,  x*std::pow(y, a2_)  );
    }

}
