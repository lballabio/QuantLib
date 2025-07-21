/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio

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

#include <ql/experimental/math/multidimintegrator.hpp>

namespace QuantLib {

    MultidimIntegral::MultidimIntegral(
            const std::vector<ext::shared_ptr<Integrator> >& integrators)
    : integrators_(integrators), 
      integrationLevelEntries_(maxDimensions_),
      varBuffer_(integrators.size(), 0.) {
        QL_REQUIRE(integrators.size() <= maxDimensions_, 
            "Too many dimensions in integration.");
        spawnFcts<maxDimensions_>();
    }

}
