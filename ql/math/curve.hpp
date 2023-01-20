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

    /*! \deprecated To be removed as unused.
                    Copy it in your codebase if you need it.
                    Deprecated in version 1.26.
    */
    class QL_DEPRECATED Curve {
      public:
        typedef Real argument_type;
        typedef Real result_type;
        virtual ~Curve() = default;
        virtual Real operator()(Real x) const = 0;
    };

}


#endif
