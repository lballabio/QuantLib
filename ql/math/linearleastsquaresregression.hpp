/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Dirk Eddelbuettel
 Copyright (C) 2006, 2009 Klaus Spanderen
 Copyright (C) 2010 Kakhkhor Abdijalilov

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
#include <ql/math/functional.hpp>
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

        const Array& coefficients()   const { return a_; }
        const Array& residuals()      const { return residuals_; }

        //! standard parameter errors as given by Excel, R etc.
        const Array& standardErrors() const { return standardErrors_; }
        //! modeling uncertainty as definied in Numerical Recipes
        const Array& error()          const { return err_;}

#ifndef QL_DISABLE_DEPRECATED
        const Array& a() const     { return a_;  }
#endif

      private:
        Array a_, err_, residuals_, standardErrors_;
    };

    //! linear regression y_i = a_0 + a_1*x_0 +..+a_n*x_{n-1} + eps
    class LinearRegression {
      public:
        //! one dimensional linear regression
        LinearRegression(const std::vector<Real>& x,
                         const std::vector<Real>& y);

        //! multi dimensional linear regression
        LinearRegression(const std::vector<std::vector<Real> >& x,
                         const std::vector<Real>& y);

        //! returns paramters {a_0, a_1, ..., a_n}
        const Array& coefficients()   const { return reg_.coefficients(); }

        const Array& residuals()      const { return reg_.residuals(); }
        const Array& standardErrors() const { return reg_.standardErrors(); }

      private:
        LinearLeastSquaresRegression<std::vector<Real> > reg_;
    };

    template <class ArgumentType> inline
    LinearLeastSquaresRegression<ArgumentType>::LinearLeastSquaresRegression(
        const std::vector<ArgumentType> & x,
        const std::vector<Real> &         y,
        const std::vector<boost::function1<Real, ArgumentType> > & v)
    : a_             (v.size(), 0.0),
      err_           (v.size(), 0.0),
      residuals_     (x.size()),
      standardErrors_(v.size()) {

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

        for (i=0; i<svd.rank(); ++i) {
            const Real u = std::inner_product(U.column_begin(i),
                                              U.column_end(i),
                                              y.begin(), 0.0)/w[i];

            for (Size j=0; j<m; ++j) {
                a_[j]+=u*V[j][i];
                err_[j]+=V[j][i]*V[j][i]/(w[i]*w[i]);
            }
        }

        err_      = Sqrt(err_);
        residuals_= A*a_-Array(y.begin(), y.end());

        const Real chiSq
            = std::inner_product(residuals_.begin(), residuals_.end(),
                                 residuals_.begin(), 0.0);
        std::transform(err_.begin(), err_.end(), standardErrors_.begin(),
                       std::bind1st(std::multiplies<Real>(),
                                    std::sqrt(chiSq/(n-2))));
    }

    namespace details {
        class LinearFct : public std::unary_function<Real, std::vector<Real> >{
          public:
            LinearFct(Size i) : i_(i) {}

            inline Real operator()(const std::vector<Real>& x) const {
                return x[i_];
            }

          private:
            const Size i_;
        };

        inline std::vector<boost::function1<Real, std::vector<Real> > >
        linearFcts(Size dims) {
            std::vector<boost::function1<Real, std::vector<Real> > > retVal;
            retVal.push_back(constant<std::vector<Real>, Real>(1.0));

            for (Size i=0; i < dims; ++i) {
                retVal.push_back(LinearFct(i));
            }

            return retVal;
        }

        inline std::vector<std::vector<Real> > argumentWrapper(
            const std::vector<Real>& x) {
            std::vector<std::vector<Real> > retVal;
            for (std::vector<Real>::const_iterator iter = x.begin();
                 iter != x.end(); ++iter) {
                retVal.push_back(std::vector<Real>(1, *iter));
            }

            return retVal;
        }
    }

    inline LinearRegression::LinearRegression(
        const std::vector<std::vector<Real> >& x,
        const std::vector<Real>& y)
    : reg_(x, y, details::linearFcts(x[0].size())) {
#ifdef QL_EXTRA_SAFETY_CHECKS
        for (Size i=1; i < x.size(); ++i) {
            QL_REQUIRE(x[i-1].size() == x[i].size(),
                        "inconsistent sample size");
        }
#endif        
    }

    inline LinearRegression::LinearRegression(
        const std::vector<Real>& x,
        const std::vector<Real>& y)
    : reg_(details::argumentWrapper(x), y, details::linearFcts(1)) { }
}
#endif
