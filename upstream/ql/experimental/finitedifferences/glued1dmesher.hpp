/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Peter Caspers

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

/*! \file glued1dmesher.hpp
    \brief One-dimensional grid mesher combining two existing ones
*/

#ifndef quantlib_glued_1d_mesher_hpp
#define quantlib_glued_1d_mesher_hpp

#include <ql/methods/finitedifferences/meshers/fdm1dmesher.hpp>

namespace QuantLib {

    class Glued1dMesher : public Fdm1dMesher {
      public:
        Glued1dMesher(
            const Fdm1dMesher& leftMesher,
            const Fdm1dMesher& rightMesher);
      private:
          const bool commonPoint_;
    };
}

#endif
