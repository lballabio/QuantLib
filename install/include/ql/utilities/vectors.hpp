/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file vectors.hpp
    \brief Utilities for vector manipulation
*/

#ifndef quantlib_utilities_vectors_hpp
#define quantlib_utilities_vectors_hpp

#include <ql/types.hpp>
#include <vector>

namespace QuantLib::detail {

        template <class T, class U>
        T get(const std::vector<T>& v,
              Size i,
              U defaultValue) {
            if (v.empty()) {
                return static_cast<T>(defaultValue);
            } else if (i < v.size()) {
                return v[i];
            } else {
                return v.back();
            }
        }

    }


#endif
