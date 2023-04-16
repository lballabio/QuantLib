/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2011 Klaus Spanderen

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

/*! \file fdmsimpleprocess1dmesher.hpp
    \brief One-dimensional grid mesher
*/

#ifndef quantlib_fdm_simple_process_1d_mesher_hpp
#define quantlib_fdm_simple_process_1d_mesher_hpp

#include <ql/methods/finitedifferences/meshers/fdm1dmesher.hpp>
#include <ql/utilities/null.hpp>
#include <memory>

namespace QuantLib {

    class StochasticProcess1D;

    class FdmSimpleProcess1dMesher : public Fdm1dMesher {
      public:
          FdmSimpleProcess1dMesher(
            Size size,
            const std::shared_ptr<StochasticProcess1D>& process,
            Time maturity, Size tAvgSteps = 10, Real epsilon = 0.0001,
            Real mandatoryPoint = Null<Real>());
    };

}

#endif
