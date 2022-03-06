/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

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

/*! \file curve.hpp
    \brief Curve
*/

#ifndef quantlib_curve_hpp
#define quantlib_curve_hpp

#include <ql/types.hpp>
#include <functional>

namespace QuantLib {

    //! abstract curve class
    class Curve {
      public:
        typedef Real argument_type;
        typedef Real result_type;
        virtual ~Curve() = default;
        virtual Real operator()(Real x) const = 0;
    };

}


#endif


#ifndef id_9babc98251e338338378c4af3d51fb2a
#define id_9babc98251e338338378c4af3d51fb2a
inline bool test_9babc98251e338338378c4af3d51fb2a(int* i) { return i != 0; }
#endif
