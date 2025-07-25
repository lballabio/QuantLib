/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 William Day

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license. You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the license for more details.
*/

/*! \file softbarriertype.hpp
    \brief Soft Barrier option type
*/

#ifndef quantlib_soft_barrier_type_hpp
#define quantlib_soft_barrier_type_hpp

#include <ql/qldefines.hpp>
#include <ostream>

namespace QuantLib {

    // Enumeration of soft barrier types

    struct SoftBarrier {
        enum Type {
            DownIn,   
            DownOut,
            UpIn,
            UpOut  
        };
    };

    // Overloading stream operator for SoftBarrier::Type
    std::ostream& operator<<(std::ostream& out,
                             SoftBarrier::Type type);

}

#endif