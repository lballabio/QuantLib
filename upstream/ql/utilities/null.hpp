/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004 StatPro Italia srl
 Copyright (C) 2010 Kakhkhor Abdijalilov

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

/*! \file null.hpp
    \brief null values
*/

#ifndef quantlib_null_hpp
#define quantlib_null_hpp

#include <ql/types.hpp>
#include <type_traits>
#include <limits>

namespace QuantLib {

    #ifdef QL_NULL_AS_FUNCTIONS

    //! template function providing a null value for a given type.
    template <typename T>
    constexpr T Null() {
        if constexpr (std::is_floating_point_v<T>) {
            // a specific, unlikely value that should fit into any Real
            return (std::numeric_limits<float>::max)();
        } else if constexpr (std::is_integral_v<T>) {
            // this should fit into any Integer
            return (std::numeric_limits<int>::max)();
        } else {
            return T();
        }
    }

    #else

    //! template class providing a null value for a given type.
    template <class Type>
    class Null;

    // default implementation for built-in types
    template <typename T>
    class Null {
      public:
        constexpr Null() = default;
        constexpr operator T() const {
            if constexpr (std::is_floating_point_v<T>) {
                // a specific, unlikely value that should fit into any Real
                return (std::numeric_limits<float>::max)();
            } else if constexpr (std::is_integral_v<T>) {
                // this should fit into any Integer
                return (std::numeric_limits<int>::max)();
            } else {
                return T();
            }
        }
    };

    #endif

}


#endif
