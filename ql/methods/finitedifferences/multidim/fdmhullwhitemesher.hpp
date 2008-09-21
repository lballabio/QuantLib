/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file fdmhestonvariancemesher.hpp
    \brief One-dimensional grid mesher for the variance in the heston problem
*/

#ifndef quantlib_fdm_hull_white_mesher_hpp
#define quantlib_fdm_hull_white_mesher_hpp

#include <ql/processes/hullwhiteprocess.hpp>
#include <ql/methods/finitedifferences/multidim/fdm1dmesher.hpp>

namespace QuantLib {

    class FdmHullWhiteMesher : public Fdm1dMesher {
      public:
        FdmHullWhiteMesher(
            Size size,
            const boost::shared_ptr<HestonProcess> & process,
            Time maturity);

        Real volaEstimate() const { return volaEstimate_; }

      private:
        Real volaEstimate_;
    };
}

#endif
