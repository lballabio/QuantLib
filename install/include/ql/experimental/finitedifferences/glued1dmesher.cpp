/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Peter Caspers

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

/*! \file glued1dmesher.cpp
    \brief One-dimensional grid mesher combining two existing ones
*/

#include <ql/experimental/finitedifferences/glued1dmesher.hpp>
#include <ql/errors.hpp>
#include <ql/math/comparison.hpp>
#include <ql/utilities/null.hpp>

namespace QuantLib {

    Glued1dMesher::Glued1dMesher(
        const Fdm1dMesher& leftMesher,
        const Fdm1dMesher& rightMesher)
    : Fdm1dMesher(leftMesher.locations().size()+rightMesher.locations().size()
                  - ( close(leftMesher.locations().back(),
                      rightMesher.locations().front()) ? 1 :0) ),
      commonPoint_( close(leftMesher.locations().back(),
                    rightMesher.locations().front()) ) {

        QL_REQUIRE(leftMesher.locations().back()
                    <= rightMesher.locations().front(),
            "left meshers rightmost point (" <<
            leftMesher.locations().back() <<
            ") may not be greater than right meshers leftmost point (" <<
            rightMesher.locations().front() << ")");

        std::copy(leftMesher.locations().begin(),leftMesher.locations().end(),
                  locations_.begin());
        std::copy(rightMesher.locations().begin() + (commonPoint_ ? 1 : 0),
                  rightMesher.locations().end(),
                  locations_.begin()+leftMesher.locations().size());

        for (Size i=0; i < locations_.size()-1; ++i) {
            dplus_[i] = dminus_[i+1] = locations_[i+1] - locations_[i];
        }
        dplus_.back() = dminus_.front() = Null<Real>();
    }
}
