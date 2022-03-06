/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Klaus Spanderen

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

/*! \file fdmcev1dmesher.hpp
    \brief One-dimensional mesher for the CEV model
*/

#ifndef quantlib_fdm_cev_1d_mesher_hpp
#define quantlib_fdm_cev_1d_mesher_hpp

#include <ql/utilities/null.hpp>
#include <ql/methods/finitedifferences/meshers/fdm1dmesher.hpp>

namespace QuantLib {

    class FdmCEV1dMesher : public Fdm1dMesher {
      public:
        FdmCEV1dMesher(
            Size size,
            Real f0, Real alpha, Real beta,
            Time maturity,
            Real eps = 0.0001,
            Real scaleFactor = 1.5,
            const std::pair<Real, Real>& cPoint
                = (std::pair<Real, Real>(Null<Real>(), Null<Real>())));
    };
}

#endif


#ifndef id_7095acd22c97747a1eacb576da419801
#define id_7095acd22c97747a1eacb576da419801
inline bool test_7095acd22c97747a1eacb576da419801(const int* i) {
    return i != nullptr;
}
#endif
