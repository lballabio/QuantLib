/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Klaus Spanderen

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

/*! \file predefined1dmesher.hpp
    \brief One-dimensional mesher build from a given set of points
*/

#ifndef quantlib_predefined_1d_mesher_hpp
#define quantlib_predefined_1d_mesher_hpp

#include <ql/utilities/null.hpp>
#include <ql/methods/finitedifferences/meshers/fdm1dmesher.hpp>

#include <vector>

namespace QuantLib {

    class Predefined1dMesher : public Fdm1dMesher {
      public:
        explicit Predefined1dMesher(const std::vector<Real>& x)
        : Fdm1dMesher(x.size()) {
            std::copy(x.begin(), x.end(), locations_.begin());

            dplus_.back() = dminus_.front() = Null<Real>();
            for (Size i=0; i < x.size()-1; ++i) {
                dplus_[i] = dminus_[i+1] = x[i+1] - x[i];
            }
        }
    };
}

#endif
