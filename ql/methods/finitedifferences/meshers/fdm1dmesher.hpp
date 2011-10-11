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

/*! \file fdm1dmesher.hpp
    \brief One-dimensional simple FDM mesher object working on an index
*/

#ifndef quantlib_fdm_1d_mesher_hpp
#define quantlib_fdm_1d_mesher_hpp

#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    class Fdm1dMesher {
      public:
        Fdm1dMesher(Size size)
        : locations_(size), dplus_(size), dminus_(size) {}

        Size size() const { return locations_.size(); }
        Real dplus(Size index) const {return dplus_[index];}
        Real dminus(Size index) const {return dminus_[index];}
        Real location(Size index) const {return locations_[index];}
        const std::vector<Real>& locations() const {return locations_;}

      protected:
        std::vector<Real> locations_;
        std::vector<Real> dplus_, dminus_;
    };
}

#endif
