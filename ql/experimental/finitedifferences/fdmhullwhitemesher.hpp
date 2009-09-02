/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Klaus Spanderen

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

/*! \file fdmhullwhitemesher.hpp
    \brief One-dimensional grid mesher for the Hull-White short rate process
*/

#ifndef quantlib_fdm_hull_white_mesher_hpp
#define quantlib_fdm_hull_white_mesher_hpp

#include <ql/experimental/finitedifferences/fdm1dmesher.hpp>
#include <boost/shared_ptr.hpp>

namespace QuantLib {

    class HullWhiteProcess;

    class FdmHullWhiteMesher : public Fdm1dMesher {
      public:
        FdmHullWhiteMesher(
            Size size,
            const boost::shared_ptr<HullWhiteProcess>& process,
            Time maturity, Size tAvgSteps = 10, Real epsilon = 0.0001);
    };

}

#endif
