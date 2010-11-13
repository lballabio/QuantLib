/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen
 Copyright (C) 2005 Gary Kennedy

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

/*! \file gaussianquadratures.hpp
    \brief Integral of a 1-dimensional function using the Gauss quadratures
*/

#ifndef quantlib_gaussian_quadratures_hpp
#define quantlib_gaussian_quadratures_hpp

#include <ql/math/array.hpp>
#include <ql/math/integrals/gaussianorthogonalpolynomial.hpp>

namespace QuantLib {
    class GaussianOrthogonalPolynomial;

    //! Integral of a 1-dimensional function using the Gauss quadratures method
    /*! References:
        Gauss quadratures and orthogonal polynomials

        G.H. Gloub and J.H. Welsch: Calculation of Gauss quadrature rule.
        Math. Comput. 23 (1986), 221-230

        "Numerical Recipes in C", 2nd edition,
        Press, Teukolsky, Vetterling, Flannery,

        \test the correctness of the result is tested by checking it
              against known good values.
    */
    class GaussianQuadrature {
      public:
        GaussianQuadrature(Size n,
                           const GaussianOrthogonalPolynomial& p);

        template <class F>
        Real operator()(const F& f) const {
            Real sum = 0.0;
            for (Integer i = order()-1; i >= 0; --i) {
                sum += w_[i] * f(x_[i]);
            }
            return sum;
        }

        Size order() const { return x_.size(); }
        const Array& weights() { return w_; }
        const Array& x()       { return x_; }
        
      private:
        Array x_, w_;
    };


    //! generalized Gauss-Laguerre integration
    /*! This class performs a 1-dimensional Gauss-Laguerre integration.
        \f[
        \int_{0}^{\inf} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x;s)=x^s \exp{-x}
        \f]
        and \f[ s > -1 \f]
    */
    class GaussLaguerreIntegration : public GaussianQuadrature {
      public:
        GaussLaguerreIntegration(Size n, Real s = 0.0)
        : GaussianQuadrature(n, GaussLaguerrePolynomial(s)) {}
    };

    //! generalized Gauss-Hermite integration
    /*! This class performs a 1-dimensional Gauss-Hermite integration.
        \f[
        \int_{-\inf}^{\inf} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x;\mu)=|x|^{2\mu} \exp{-x*x}
        \f]
        and \f[ \mu > -0.5 \f]
    */
    class GaussHermiteIntegration : public GaussianQuadrature {
      public:
        GaussHermiteIntegration(Size n, Real mu = 0.0)
        : GaussianQuadrature(n, GaussHermitePolynomial(mu)) {}
    };

    //! Gauss-Jacobi integration
    /*! This class performs a 1-dimensional Gauss-Jacobi integration.
        \f[
        \int_{-1}^{1} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x;\alpha,\beta)=(1-x)^\alpha (1+x)^\beta
        \f]
    */
    class GaussJacobiIntegration : public GaussianQuadrature {
      public:
        GaussJacobiIntegration(Size n, Real alpha, Real beta)
        : GaussianQuadrature(n, GaussJacobiPolynomial(alpha, beta)) {}
    };

    //! Gauss-Hyperbolic integration
    /*! This class performs a 1-dimensional Gauss-Hyperbolic integration.
        \f[
        \int_{-\inf}^{\inf} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x)=1/cosh(x)
        \f]
    */
    class GaussHyperbolicIntegration : public GaussianQuadrature {
      public:
        GaussHyperbolicIntegration(Size n)
        : GaussianQuadrature(n, GaussHyperbolicPolynomial()) {}
    };

    //! Gauss-Legendre integration
    /*! This class performs a 1-dimensional Gauss-Legendre integration.
        \f[
        \int_{-1}^{1} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x)=1
        \f]
    */
    class GaussLegendreIntegration : public GaussianQuadrature {
      public:
        GaussLegendreIntegration(Size n)
        : GaussianQuadrature(n, GaussJacobiPolynomial(0.0, 0.0)) {}
    };

    //! Gauss-Chebyshev integration
    /*! This class performs a 1-dimensional Gauss-Chebyshev integration.
        \f[
        \int_{-1}^{1} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x)=(1-x^2)^{-1/2}
        \f]
    */
    class GaussChebyshevIntegration : public GaussianQuadrature {
      public:
        GaussChebyshevIntegration(Size n)
        : GaussianQuadrature(n, GaussJacobiPolynomial(-0.5, -0.5)) {}
    };

    //! Gauss-Chebyshev integration (second kind)
    /*! This class performs a 1-dimensional Gauss-Chebyshev integration.
        \f[
        \int_{-1}^{1} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x)=(1-x^2)^{1/2}
        \f]
    */
    class GaussChebyshev2ndIntegration : public GaussianQuadrature {
      public:
        GaussChebyshev2ndIntegration(Size n)
      : GaussianQuadrature(n, GaussJacobiPolynomial(0.5, 0.5)) {}
    };

    //! Gauss-Gegenbauer integration
    /*! This class performs a 1-dimensional Gauss-Gegenbauer integration.
        \f[
        \int_{-1}^{1} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x)=(1-x^2)^{\lambda-1/2}
        \f]
    */
    class GaussGegenbauerIntegration : public GaussianQuadrature {
      public:
        GaussGegenbauerIntegration(Size n, Real lambda)
        : GaussianQuadrature(n, GaussJacobiPolynomial(lambda-0.5, lambda-0.5))
        {}
    };


    //! tabulated Gauss-Legendre quadratures
    class TabulatedGaussLegendre {
      public:
        TabulatedGaussLegendre(Size n = 20) { order(n); }
        template <class F>
        Real operator() (const F& f) const {
            QL_ASSERT(w_!=0, "Null weights" );
            QL_ASSERT(x_!=0, "Null abscissas");
            Size startIdx;
            Real val;

            const Size isOrderOdd = order_ & 1;

            if (isOrderOdd) {
              QL_ASSERT((n_>0), "assume at least 1 point in quadrature");
              val = w_[0]*f(x_[0]);
              startIdx=1;
            } else {
              val = 0.0;
              startIdx=0;
            }

            for (Size i=startIdx; i<n_; ++i) {
                val += w_[i]*f( x_[i]);
                val += w_[i]*f(-x_[i]);
            }
            return val;
        }

        void order(Size);
        Size order() const { return order_; }

      private:
        Size order_;

        const Real* w_;
        const Real* x_;
        Size  n_;

        static const Real w6[3];
        static const Real x6[3];
        static const Size n6;

        static const Real w7[4];
        static const Real x7[4];
        static const Size n7;

        static const Real w12[6];
        static const Real x12[6];
        static const Size n12;

        static const Real w20[10];
        static const Real x20[10];
        static const Size n20;
    };

}

#endif
