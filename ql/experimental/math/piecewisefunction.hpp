/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

/*! \file piecewisefunction.hpp
    \brief utility macro for piecewise functions
*/

#ifndef quantlib_piecewise_function_hpp
#define quantlib_piecewise_function_hpp

#include <ql/qldefines.hpp>
#include <algorithm>

/*! This defines a piecewise constant function which is RCLL and takes
    the values Y[0], Y[1], ... Y[n] on the intervals
    (-\infty, X[0]), [ X[1], X[2] ), ... , [ X[n-1], \infty)
    Normally Y.size() should be X.size() + 1. If more values for Y are
    given, they are ignored. If less values are given the last given
    value is kept the same for the remaining intervals.
    If X.size() is 0 a constant function taking the value Y[0] is
    evaluated.

    \warning If Y.size() is 0, an invalid access occurs. This
             condition is not checked for performance reasons.
*/

#define QL_PIECEWISE_FUNCTION(X, Y, x)                                         \
    Y[std::min<std::size_t>(                                                   \
        std::upper_bound(X.begin(), X.end(), x) - X.begin(), Y.size() - 1)]

#endif
