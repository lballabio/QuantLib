/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio

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

#include <ql/experimental/math/multidimquadrature.hpp>

#ifndef QL_PATCH_SOLARIS

namespace QuantLib {

    GaussianQuadMultidimIntegrator::GaussianQuadMultidimIntegrator(
        Size dimension, Size quadOrder, Real mu) 
        : integral_(quadOrder, mu),
          integralV_(quadOrder, mu),
          integrationEntries_(maxDimensions_),
          integrationEntriesVR_(maxDimensions_),
          dimension_(dimension),
          varBuffer_(dimension_, 0.)
    {
        spawnFcts<maxDimensions_>();
    }

}

#endif
