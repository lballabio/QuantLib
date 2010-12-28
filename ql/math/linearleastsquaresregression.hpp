/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Dirk Eddelbuettel
 Copyright (C) 2006, 2009 Klaus Spanderen
 Copyright (C) 2010 Kakhkhor Abdijalilov
 Copyright (C) 2010 Slava Mazur

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
#include <boost/type_traits.hpp>
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
        template <class xContainer, class yContainer, class vContainer>
        LinearLeastSquaresRegression(const xContainer & x, 
                                     const yContainer &y, const vContainer & v);

        template<class xIterator, class yIterator, class vIterator>
        LinearLeastSquaresRegression(
            xIterator xBegin, xIterator xEnd,
            yIterator yBegin, yIterator yEnd,
            vIterator vBegin, vIterator vEnd);

        const Array& coefficients()   const { return a_; }
        const Array& residuals()      const { return residuals_; }

        //! standard parameter errors as given by Excel, R etc.
        const Array& standardErrors() const { return standardErrors_; }
        //! modeling uncertainty as definied in Numerical Recipes
        const Array& error()          const { return err_;}

        template <class OtherLinearLeastSquaresRegression>
        void swap (OtherLinearLeastSquaresRegression &other);

#ifndef QL_DISABLE_DEPRECATED
        const Array& a() const     { return a_;  }
#endif
        Size size() const { return residuals_.size(); }

        Size dim() const { return a_.size(); }

    protected:
        Array a_, err_, residuals_, standardErrors_;

        LinearLeastSquaresRegression(Size n, Size m) 
        : a_(m, 0), err_(m, 0), residuals_(n, 0), standardErrors_(m, 0) {
        }

        template <class xIterator, class yIterator, class vIterator>
        void calculate(
            xIterator xBegin, xIterator xEnd,
            yIterator yBegin, yIterator yEnd,
            vIterator vBegin, vIterator vEnd);
    };

    class LinearRegression : public LinearLeastSquaresRegression<Real> {
    public:
        //! linear regression y_i = a_0 + a_1*x_0 +..+a_n*x_{n-1} + eps
        template <class xContainer, class yContainer>
        LinearRegression(const xContainer& x, 
                         const yContainer& y, Real intercept = 1.0);

        template <class xContainer, class yContainer, class vContainer>
        LinearRegression(const xContainer& x, 
                         const yContainer& y, const vContainer &v);
    };

    namespace details {
        template <class Container>
        class LinearFct : public std::unary_function<Real, Container > {
        public:
            LinearFct(Size i) : i_(i) {}

            inline Real operator()(const Container& x) const {
                return x[i_];
            }

        private:
            const Size i_;
        };

        // 1d implementation (arithmetic types)
        template <class xContainer, class yContainer, bool>
        class LinearFcts  
        : public LinearLeastSquaresRegression<typename xContainer::value_type> {
        public:
            typedef typename xContainer::value_type ArgumentType;
            typedef LinearLeastSquaresRegression<ArgumentType> Super;
            LinearFcts (const xContainer &x, 
                        const yContainer &y, Real intercept) : 
            Super(y.size(), (intercept ? 2 : 1)) {
                if (intercept)
                    v.push_back(constant<ArgumentType, Real>(intercept));
                v.push_back(identity<ArgumentType>());
                calculate(x.begin(), x.end(), 
                          y.begin(), y.end(), v.begin(), v.end());
            }

            std::vector< boost::function1<Real, ArgumentType> > v;
        };

        // multi-dimensional implementation (container types)
        template <class xContainer, class yContainer>
        class LinearFcts<xContainer, yContainer, false> 
            : public LinearLeastSquaresRegression<
                                            typename xContainer::value_type> {
        public:
            typedef typename xContainer::value_type ArgumentType;
            typedef LinearLeastSquaresRegression<ArgumentType> Super;
            LinearFcts (const xContainer &x, const yContainer &y,Real intercept) : 
            Super(y.size(),(intercept ? x.begin()->size()+1 : x.begin()->size())) {
                if (intercept)
                    v.push_back(constant<ArgumentType, Real>(intercept));
                Size m = x.begin()->size();
                for (Size i = 0; i < m; ++i)
                    v.push_back(LinearFct<ArgumentType>(i));
                calculate(x.begin(), x.end(), 
                          y.begin(), y.end(), v.begin(), v.end());
            }

            std::vector< boost::function1<Real, ArgumentType> > v;
        };
    }

    template <class ArgumentType>
    template <class xIterator, class yIterator, class vIterator>
    void LinearLeastSquaresRegression<ArgumentType>::calculate(
        xIterator xBegin, xIterator xEnd,
        yIterator yBegin, yIterator yEnd,
        vIterator vBegin, vIterator vEnd) {

            const Size n = residuals_.size();
            const Size m = err_.size();

            QL_REQUIRE( n == Size(std::distance(yBegin, yEnd)),
                "sample set need to be of the same size");
            QL_REQUIRE(n >= m, "sample set is too small");

            Size i;

            Matrix A(n, m);
            for (i=0; i<m; ++i)
                std::transform(xBegin, xEnd, A.column_begin(i), *vBegin++);

            const SVD svd(A);
            const Matrix& V = svd.V();
            const Matrix& U = svd.U();
            const Array& w = svd.singularValues();
            const Real threshold = n*QL_EPSILON;

            for (i=0; i<m; ++i) {
                if (w[i] > threshold) {
                    const Real u = std::inner_product(U.column_begin(i),
                        U.column_end(i),
                        yBegin, 0.0)/w[i];

                    for (Size j=0; j<m; ++j) {
                        a_[j]  +=u*V[j][i];
                        err_[j]+=V[j][i]*V[j][i]/(w[i]*w[i]);
                    }
                }
            }
            err_      = Sqrt(err_);
            Array tmp = A*a_;
            std::transform(tmp.begin(), tmp.end(), 
                           yBegin, residuals_.begin(), std::minus<Real>());

            const Real chiSq
                = std::inner_product(residuals_.begin(), residuals_.end(),
                residuals_.begin(), 0.0);
            std::transform(err_.begin(), err_.end(), standardErrors_.begin(),
                std::bind1st(std::multiplies<Real>(),
                std::sqrt(chiSq/(n-2))));
    }

    template <class ArgumentType>
    template <class xContainer, class yContainer, class vContainer> inline
        LinearLeastSquaresRegression<ArgumentType>::LinearLeastSquaresRegression(
            const xContainer & x, 
            const yContainer &y, const vContainer & v) 
    : a_(v.size(), 0.0), 
      err_(v.size(), 0.0), 
      residuals_(y.size()), 
      standardErrors_(v.size()) {
        calculate(x.begin(), x.end(), y.begin(), y.end(), v.begin(), v.end());
    }

    template <class ArgumentType>
    template<class xIterator, class yIterator, class vIterator> inline
        LinearLeastSquaresRegression<ArgumentType>::LinearLeastSquaresRegression(
        xIterator xBegin, xIterator xEnd,
        yIterator yBegin, yIterator yEnd,
        vIterator vBegin, vIterator vEnd) : 
    a_(std::distance(vBegin, vEnd), 0.0),
        err_(a_.size(), 0.0),
        residuals_(std::distance(yBegin, yEnd)),
        standardErrors_(a_.size()) {
            calculate(xBegin, xEnd, yBegin, yEnd, vBegin, vEnd);
    }

    template <class ArgumentType>
    template <class OtherLinearLeastSquaresRegression> inline
        void LinearLeastSquaresRegression<ArgumentType>::swap
                                (OtherLinearLeastSquaresRegression &other) {
            LinearLeastSquaresRegression<ArgumentType> &tmp 
                = (LinearLeastSquaresRegression<ArgumentType> &)(other);
            a_.swap(tmp.a_);
            err_.swap(tmp.err_);
            residuals_.swap(tmp.residuals_);
            standardErrors_.swap(tmp.standardErrors_);
    }

    template <class xContainer, class yContainer> inline
        LinearRegression::LinearRegression(const xContainer& x, 
                                           const yContainer& y, Real intercept) 
    : LinearLeastSquaresRegression<Real> (1, 1) {
        typedef typename xContainer::value_type ArgumentType;
        details::LinearFcts<xContainer, yContainer, 
                            boost::is_arithmetic<ArgumentType>::value> 
                                                          lfs(x, y, intercept);
                                                          
        lfs.swap(static_cast<LinearLeastSquaresRegression<Real>&>(*this));
    }

    template <class xContainer, class yContainer, class vContainer> inline
        LinearRegression::LinearRegression(const xContainer& x, 
                                           const yContainer& y, 
                                           const vContainer &v) 
    : LinearLeastSquaresRegression<Real> (x, y, v) {
    }
}
#endif
