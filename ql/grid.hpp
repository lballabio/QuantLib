/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file grid.hpp
    \brief Grid constructors
*/

#ifndef quantlib_grid_hpp
#define quantlib_grid_hpp

#include <ql/math/array.hpp>

namespace QuantLib {

    Disposable<Array> CenteredGrid(Real center, Real dx, Size steps);
    Disposable<Array> BoundedGrid(Real xMin, Real xMax, Size steps);
    Disposable<Array> BoundedLogGrid(Real xMin, Real xMax, Size steps);

    // inline definitions

    inline Disposable<Array> CenteredGrid(Real center, Real dx,
                                          Size steps) {
        Array result(steps+1);
        for (Size i=0; i<steps+1; i++)
            result[i] = center + (i - steps/2.0)*dx;
        return result;
    }

    inline Disposable<Array> BoundedGrid(Real xMin, Real xMax,
                                         Size steps) {
        Array result(steps+1);
        Real x=xMin, dx=(xMax-xMin)/steps;
        for (Size i=0; i<steps+1; i++, x+=dx)
            result[i] = x;
        return result;
    }

    inline Disposable<Array> BoundedLogGrid(Real xMin, Real xMax,
                                            Size steps) {
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


#ifndef id_f533ed6ce5c0e9aa06c41e6e6cdaf7e6
#define id_f533ed6ce5c0e9aa06c41e6e6cdaf7e6
inline bool test_f533ed6ce5c0e9aa06c41e6e6cdaf7e6(const int* i) {
    return i != nullptr;
}
#endif
