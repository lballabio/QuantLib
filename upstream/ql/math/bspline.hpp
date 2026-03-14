/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Allen Kuo

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

/*! \file bspline.hpp
    \brief B-spline basis functions
*/

#ifndef quantlib_bspline_hpp
#define quantlib_bspline_hpp

#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    //! B-spline basis functions
    /*! Follows treatment and notation from:

        Weisstein, Eric W. "B-Spline." From MathWorld--A Wolfram Web
        Resource.  <http://mathworld.wolfram.com/B-Spline.html>

        \f$ (p+1) \f$-th order B-spline (or p degree polynomial) basis
        functions \f$ N_{i,p}(x), i = 0,1,2 \ldots n \f$, with \f$ n+1 \f$
        control points, or equivalently, an associated knot vector
        of size \f$ p+n+2 \f$ defined at the increasingly sorted points
        \f$ (x_0, x_1 \ldots x_{n+p+1}) \f$. A linear B-spline has
        \f$ p=1 \f$, quadratic B-spline has \f$ p=2 \f$, a cubic
        B-spline has \f$ p=3 \f$, etc.

        The B-spline basis functions are defined recursively
        as follows:

        \f[
        \begin{array}{rcl}
        N_{i,0}(x) &=& 1   \textrm{\ if\ } x_{i} \leq x < x_{i+1} \\
                   &=& 0   \textrm{\ otherwise} \\
        N_{i,p}(x) &=& N_{i,p-1}(x) \frac{(x - x_{i})}{ (x_{i+p-1} - x_{i})} +
                       N_{i+1,p-1}(x) \frac{(x_{i+p} - x)}{(x_{i+p} - x_{i+1})}
        \end{array}
        \f]
    */
    class BSpline {
      public:
        BSpline(Natural p,
                Natural n,
                const std::vector<Real>& knots);

        Real operator()(Natural i, Real x) const;

      private:
        // recursive definition of N, the B-spline basis function
        Real N(Natural i, Natural p, Real x) const;
        // e.g. p_=2 is a quadratic B-spline, p_=3 is a cubic B-Spline, etc.
        Natural p_;
        // n_ + 1 =  "control points" = max number of basis functions
        Natural n_;
        std::vector<Real> knots_;
    };

}


#endif
