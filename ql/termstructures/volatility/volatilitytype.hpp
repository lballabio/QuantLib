/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

/*! \file volatilitytype.hpp
    \brief volatility types
*/

#ifndef quantlib_volatility_type_hpp
#define quantlib_volatility_type_hpp

#include <ql/qldefines.hpp>
#include <ostream>

namespace QuantLib {

    enum VolatilityType { ShiftedLognormal, Normal };

    inline std::ostream& operator<<(std::ostream& out,
                                    const VolatilityType& t) {
        switch(t) {
          case Normal:
            return out << "Normal";
          case ShiftedLognormal:
            return out << "ShiftedLognormal";
          default:
            return out << "Unknown volatility type (" << t << ")";
        }
    };

}


#endif
