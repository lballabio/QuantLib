/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file surface.hpp
    \brief Surface
*/

#ifndef quantlib_surface_hpp
#define quantlib_surface_hpp

#include <ql/types.hpp>
#include <functional>

namespace QuantLib {
    //! %curve abstract class

    class Curve : public std::unary_function<Real, Real> {
      public:
        virtual Real operator()(Real x) const = 0;
    };

    class TestCurve : public Curve {
    public:
        Real operator()(Real x) const { return sin(x); }
    };

}


#endif
