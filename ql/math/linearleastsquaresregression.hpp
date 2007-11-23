/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen

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

/*! \file linearleastsquaresregression.hpp
    \brief general linear least square regression
*/

#ifndef quantlib_linear_least_squares_regression_hpp
#define quantlib_linear_least_squares_regression_hpp

#include <ql/qldefines.hpp>
#include <ql/math/matrixutilities/svd.hpp>
#include <ql/math/array.hpp>
#include <boost/function.hpp>
#include <vector>

namespace QuantLib {

    //! general linear least squares regression
    /*! References:
       "Numerical Recipes in C", 2nd edition,
        Press, Teukolsky, Vetterling, Flannery,

        \test the correctness of the returned values is tested by
              checking their properties.
    */
    template <class ArgumentType = Real>
    class LinearLeastSquaresRegression {
      public:
        LinearLeastSquaresRegression(
            const std::vector<ArgumentType> & x,
            const std::vector<Real> &         y,
            const std::vector<boost::function1<Real, ArgumentType> > & v);

        const Array& a() const   { return a_;  }
        const Array& error() const { return err_;}

      private:
        Array a_;
        Array err_;
    };

    template <class ArgumentType> inline
    LinearLeastSquaresRegression<ArgumentType>::LinearLeastSquaresRegression(
        const std::vector<ArgumentType> & x,
        const std::vector<Real> &         y,
        const std::vector<boost::function1<Real, ArgumentType> > & v)
    : a_  (v.size(), 0.0),
      err_(v.size(), 0.0) {

        QL_REQUIRE(x.size() == y.size(),
                   "sample set need to be of the same size");
        QL_REQUIRE(x.size() >= v.size(), "sample set is too small");

        Size i;
        const Size n = x.size();
        const Size m = v.size();

        Matrix A(n, m);
        for (i=0; i<m; ++i)
            std::transform(x.begin(), x.end(), A.column_begin(i), v[i]);

        const SVD svd(A);
        const Matrix& V = svd.V();
        const Matrix& U = svd.U();
        const Array&  w = svd.singularValues();
        const Real threshold = n*QL_EPSILON;

        for (i=0; i<m; ++i) {
            if (w[i] > threshold) {
                const Real u = std::inner_product(U.column_begin(i),
                                                  U.column_end(i),
                                                  y.begin(), 0.0)/w[i];

                for (Size j=0; j<m; ++j) {
                    a_[j]  +=u*V[j][i];
                    err_[j]+=V[j][i]*V[j][i]/(w[i]*w[i]);
                }
            }
        }
        err_=Sqrt(err_);
    }
}

#endif
