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
 <http://quantlib.org/license.shtml>.

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

    
   
    namespace detail {

        template <bool>
        struct FloatingPointNull;

        // null value for floating-point types
        template <>
        struct FloatingPointNull<true> {
            constexpr static float nullValue() {
                // a specific values that should fit into any Real
                return (std::numeric_limits<float>::max)();
            }
        };

        // null value for integer types
        template <>
        struct FloatingPointNull<false> {
            constexpr static int nullValue() {
                // a specific values that should fit into any Integer
                return (std::numeric_limits<int>::max)();
            }
        };

    }

    //! template function providing a null value for a given type.
    template <typename T>
    T Null() {
        return T(detail::FloatingPointNull<std::is_floating_point<T>::value>::nullValue());
    }

}


#endif
