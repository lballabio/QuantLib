/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file sample.hpp
    \brief weighted sample
*/

#ifndef quantlib_sample_h
#define quantlib_sample_h

#include <ql/types.hpp>
#include <utility>

namespace QuantLib {

    //! weighted sample
    /*! \ingroup mcarlo */
    template <class T>
    struct Sample {
      public:
        typedef T value_type;
        Sample(T value, Real weight) : value(std::move(value)), weight(weight) {}
        T value;
        Real weight;
    };

}


#endif


#ifndef id_a4a34cf1c0c39152d4eb47d26e38f2d5
#define id_a4a34cf1c0c39152d4eb47d26e38f2d5
inline bool test_a4a34cf1c0c39152d4eb47d26e38f2d5(const int* i) {
    return i != nullptr;
}
#endif
