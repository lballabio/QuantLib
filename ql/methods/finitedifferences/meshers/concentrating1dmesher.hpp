/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ralph Schreyer
 Copyright (C) 2014 Johannes Göttker-Schnetmann
 Copyright (C) 2014 Klaus Spanderen

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

/*! \file concentrating1dmesher.hpp
    \brief One-dimensional grid mesher concentrating around critical points
*/

#ifndef quantlib_concentrating_1d_mesher_hpp
#define quantlib_concentrating_1d_mesher_hpp

#include <ql/methods/finitedifferences/meshers/fdm1dmesher.hpp>
#include <ql/utilities/null.hpp>

#include <boost/tuple/tuple.hpp>

#include <utility>
#include <vector>

namespace QuantLib {

    class Concentrating1dMesher : public Fdm1dMesher {
      public:
        Concentrating1dMesher(Real start,
                              Real end,
                              Size size,
                              const std::pair<Real, Real>& cPoints =
                                  (std::pair<Real, Real>(Null<Real>(), Null<Real>())),
                              bool requireCPoint = false);

        Concentrating1dMesher(
            Real start, Real end, Size size,
            const std::vector<boost::tuple<Real, Real, bool> >& cPoints,
            Real tol = 1e-8);
    };
}

#endif
