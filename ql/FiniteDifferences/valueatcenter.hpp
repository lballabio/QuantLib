/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file valueatcenter.hpp
    \brief compute value, first, and second derivatives at grid center
*/

#ifndef quantlib_finite_difference_value_at_center_h
#define quantlib_finite_difference_value_at_center_h

#include <ql/Math/array.hpp>

namespace QuantLib {

    /*! mid-point value

        \todo replace with a more general (not "centered") function:
              valueAt(Real spot, const Array& a);
    */
    Real valueAtCenter(const Array& a);

    /*! mid-point first derivative

        \todo replace with a more general (not "centered") function:
              firstDerivativeAt(Real spot,
                                const Array& a,
                                const Array& grid);
    */
    Real firstDerivativeAtCenter(const Array& a,
                                 const Array& grid);

    /*! mid-point second derivative

        \todo replace with a more general (not "centered") function:
              secondDerivativeAt(Real spot,
                                 const Array& a,
                                 const Array& grid);
    */
    Real secondDerivativeAtCenter(const Array& a,
                                  const Array& grid);

}


#endif
