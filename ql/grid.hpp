/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file grid.hpp
    \brief Grid constructors
*/

#ifndef quantlib_grid_hpp
#define quantlib_grid_hpp

#include <ql/math/array.hpp>

namespace QuantLib {

    /*! \deprecated Part of the old FD framework; copy this function
                    in your codebase if needed.
                    Deprecated in version 1.37.
    */
    [[deprecated("Part of the old FD framework; copy this function in your codebase if needed")]]
    Array CenteredGrid(Real center, Real dx, Size steps);

    /*! \deprecated Part of the old FD framework; copy this function
                    in your codebase if needed.
                    Deprecated in version 1.37.
    */
    [[deprecated("Part of the old FD framework; copy this function in your codebase if needed")]]
    Array BoundedGrid(Real xMin, Real xMax, Size steps);

    /*! \deprecated Part of the old FD framework; copy this function
                    in your codebase if needed.
                    Deprecated in version 1.37.
    */
    [[deprecated("Part of the old FD framework; copy this function in your codebase if needed")]]
    Array BoundedLogGrid(Real xMin, Real xMax, Size steps);


    // inline definitions

    inline Array CenteredGrid(Real center, Real dx, Size steps) {
        Array result(steps+1);
        for (Size i=0; i<steps+1; i++)
            result[i] = center + (i - steps/2.0)*dx;
        return result;
    }

    inline Array BoundedGrid(Real xMin, Real xMax, Size steps) {
        Array result(steps+1);
        Real x=xMin, dx=(xMax-xMin)/steps;
        for (Size i=0; i<steps+1; i++, x+=dx)
            result[i] = x;
        return result;
    }

    inline Array BoundedLogGrid(Real xMin, Real xMax, Size steps) {
        Array result(steps+1);
        Real gridLogSpacing = (std::log(xMax) - std::log(xMin)) / 
            (steps);
        Real edx = std::exp(gridLogSpacing);
        result[0] = xMin;
        for (Size j=1; j < steps+1; j++) {
            result[j] = result[j-1]*edx;
        }
        return result;
    }
}


#endif
