/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen

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

/*! \file linearleastsquaresregression.hpp
    \brief general linear least square regression
*/

#ifndef quantlib_linear_least_squares_regression_hpp
#define quantlib_linear_least_squares_regression_hpp

#include <ql/Math/array.hpp>
#ifdef QL_PATCH_MSVC71
#pragma warning(disable:4224)
#endif
#include <boost/function.hpp>
#ifdef QL_PATCH_MSVC71
#pragma warning(default:4224)
#endif
#include <vector>

namespace QuantLib {

    //! general linear least squares regression
    /*! References:
       "Numerical Recipes in C", 2nd edition,
        Press, Teukolsky, Vetterling, Flannery,

        \test the correctness of the returned values is tested by
              checking their properties.
    */
    class LinearLeastSquaresRegression {
      public:
        LinearLeastSquaresRegression(
            const Array& x, const Array& y,
            const std::vector<boost::function1<Real, Real> >& v);

        const Array& a() const;
        const Array& err() const;

      private:
        Array a_;
        Array err_;
    };
}


#endif

