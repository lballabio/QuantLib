/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Dirk Eddelbuettel
 Copyright (C) 2006, 2009, 2010 Klaus Spanderen
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

#include <ql/math/generallinearleastsquares.hpp>
#include <ql/functional.hpp>

namespace QuantLib {

    namespace details {

        template <class Container>
        class LinearFct {
          public:
            /*! \deprecated Use `auto` or `decltype` instead.
                            Deprecated in version 1.29.
            */
            QL_DEPRECATED
            typedef Container argument_type;

            /*! \deprecated Use `auto` or `decltype` instead.
                            Deprecated in version 1.29.
            */
            QL_DEPRECATED
            typedef Real result_type;
            explicit LinearFct(Size i) : i_(i) {}

            inline Real operator()(const Container& x) const {
                return x[i_];
            }

          private:
            const Size i_;
       };

        // 1d implementation (arithmetic types)
        template <class xContainer, bool>
        class LinearFcts {
          public:
            typedef typename xContainer::value_type ArgumentType;
            LinearFcts (const xContainer &x, Real intercept) {
                if (intercept != 0.0)
                    v.push_back([=](ArgumentType x){ return intercept; });
                v.push_back([](ArgumentType x){ return x; });
            }

            const std::vector< ext::function<Real(ArgumentType)> > & fcts() {
                return v;
            }

          private:
            std::vector< ext::function<Real(ArgumentType)> > v;
        };

        // multi-dimensional implementation (container types)
        template <class xContainer>
        class LinearFcts<xContainer, false>  {
          public:
            typedef typename xContainer::value_type ArgumentType;
            LinearFcts (const xContainer &x, Real intercept) {
                if (intercept != 0.0)
                    v.push_back([=](ArgumentType x){ return intercept; });
                Size m = x.begin()->size();
                for (Size i = 0; i < m; ++i)
                    v.push_back(LinearFct<ArgumentType>(i));
            }

            const std::vector< ext::function<Real(ArgumentType)> > & fcts() {
               return v;
            }
          private:
            std::vector< ext::function<Real(ArgumentType)> > v;
        };
    }

    class LinearRegression : public GeneralLinearLeastSquares {
    public:
        //! linear regression y_i = a_0 + a_1*x_0 +..+a_n*x_{n-1} + eps
        template <class xContainer, class yContainer>
        LinearRegression(const xContainer& x, 
                         const yContainer& y, Real intercept = 1.0);

        template <class xContainer, class yContainer, class vContainer>
        LinearRegression(const xContainer& x, 
                         const yContainer& y, const vContainer &v);
    };


    template <class xContainer, class yContainer> inline
        LinearRegression::LinearRegression(const xContainer& x, 
                                           const yContainer& y, Real intercept) 
    : GeneralLinearLeastSquares(x, y,
          details::LinearFcts<xContainer, 
              boost::is_arithmetic<typename xContainer::value_type>::value>
                                                        (x, intercept).fcts()) {
    }

    template <class xContainer, class yContainer, class vContainer> inline
        LinearRegression::LinearRegression(const xContainer& x, 
                                           const yContainer& y, 
                                           const vContainer &v) 
    : GeneralLinearLeastSquares(x, y, v) {
    }

    // general linear least squares regression
    // this interface is support for backward compatibility only
    // please use GeneralLinearLeastSquares directly
    template <class ArgumentType = Real>
    class LinearLeastSquaresRegression : public GeneralLinearLeastSquares {
      public:
        LinearLeastSquaresRegression(
            const std::vector<ArgumentType> & x,
            const std::vector<Real> &         y,
            const std::vector<ext::function<Real(ArgumentType)> > & v)
        : GeneralLinearLeastSquares(x, y, v) {
        }
    };
}
#endif
