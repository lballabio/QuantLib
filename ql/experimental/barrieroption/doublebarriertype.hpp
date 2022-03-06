/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Thema Consulting SA

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

/*! \file doublebarriertype.hpp
    \brief Double Barrier type
*/

#ifndef quantlib_double_barrier_type_hpp
#define quantlib_double_barrier_type_hpp

#include <ql/qldefines.hpp>
#include <ostream>

namespace QuantLib {

    //! Placeholder for enumerated barrier types
    struct DoubleBarrier {
        enum Type {
           KnockIn,
           KnockOut,
           KIKO,     //! lower barrier KI, upper KO
           KOKI      //! lower barrier KO, upper KI
        };
    };

    std::ostream& operator<<(std::ostream& out,
                             DoubleBarrier::Type type);

}

#endif


#ifndef id_4a1a122fa92b26281df3c297fe45a621
#define id_4a1a122fa92b26281df3c297fe45a621
inline bool test_4a1a122fa92b26281df3c297fe45a621(int* i) { return i != 0; }
#endif
