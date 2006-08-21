/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2005, 2006 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/MarketModels/PseudoRoots/abcd.hpp>

namespace QuantLib {
    
    Abcd::Abcd(Real a, Real b, Real c, Real d, Real T, Real S)
    : a_(a), b_(b), c_(c), d_(d), S_(S), T_(T) {
        QL_REQUIRE(a+d>=0, "a+d must be non negative");
        QL_REQUIRE(d>=0, "d must be non negative");
        QL_REQUIRE(c>=0, "c must be non negative");
    }

}
