/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Allen Kuo

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

/*! \file bernsteinpolynomial.hpp
    \brief Bernstein polynomials
*/

#ifndef quantlib_bernstein_polynomial_hpp
#define quantlib_bernstein_polynomial_hpp

#include <ql/types.hpp>

namespace QuantLib {

    //! class of Bernstein polynomials
    /*! see definition:

        Weisstein, Eric W. "Bernstein Polynomial." From MathWorld--A
        Wolfram Web Resource.
        <http://mathworld.wolfram.com/BernsteinPolynomial.html>

        The Bernstein polynomials \f$  B_{i,n}(x) \f$ are defined as

        \f[
        B_{i,n}(x) \equiv \left( \begin{array}{c} n \\ i \end{array} \right)
        x^i (1-x)^{n-i}
        \f]
    */
    class BernsteinPolynomial {
      public:
        static Real get(Natural i, Natural n, Real x) ;
    };

}


#endif

