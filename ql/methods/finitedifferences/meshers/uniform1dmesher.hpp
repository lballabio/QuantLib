/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

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

/*! \file uniform1dmesher.hpp
    \brief One-dimensional simple uniform grid mesher
*/

#ifndef quantlib_fdm_uniform_1d_mesher_hpp
#define quantlib_fdm_uniform_1d_mesher_hpp

#include <ql/methods/finitedifferences/meshers/fdm1dmesher.hpp>
#include <ql/errors.hpp>
#include <ql/utilities/null.hpp>

namespace QuantLib {

    class Uniform1dMesher : public Fdm1dMesher {
      public:
        Uniform1dMesher(Real start, Real end, Size size)
        : Fdm1dMesher(size) {
            QL_REQUIRE(end > start, "end must be larger than start");

            const Real dx = (end-start)/(size-1);

            for (Size i=0; i < size-1; ++i) {
                locations_[i] = start + i*dx;
                dplus_[i] = dminus_[i+1] = dx;
            }

            locations_.back() = end;
            dplus_.back() = dminus_.front() = Null<Real>();
        }
    };
}

#endif
