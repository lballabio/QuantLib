/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

/*! \file gaussianorthogonalpolynomial.hpp
    \brief orthogonal polynomials for gaussian quadratures
*/

#ifndef quantlib_gaussian_orthogonal_polynomial_hpp
#define quantlib_gaussian_orthogonal_polynomial_hpp

#include <ql/types.hpp>

namespace QuantLib {

    //! orthogonal polynomial for Gaussian quadratures
    /*! References:
        Gauss quadratures and orthogonal polynomials

        G.H. Gloub and J.H. Welsch: Calculation of Gauss quadrature rule.
        Math. Comput. 23 (1986), 221-230

        "Numerical Recipes in C", 2nd edition,
        Press, Teukolsky, Vetterling, Flannery,

        The polynomials are defined by the three-term recurrence relation
        \f[
        P_{k+1}(x)=(x-\alpha_k) P_k(x) - \beta_k P_{k-1}(x)
        \f]
        and
        \f[
        \mu_0 = \int{w(x)dx}
        \f]
    */
    class GaussianOrthogonalPolynomial {
      public:
        virtual ~GaussianOrthogonalPolynomial() = default;
        virtual Real mu_0()        const = 0;
        virtual Real alpha(Size i) const = 0;
        virtual Real beta(Size i)  const = 0;
        virtual Real w(Real x)     const = 0;

        Real value(Size i, Real x) const;
        Real weightedValue(Size i, Real x) const;
    };

    //! Gauss-Laguerre polynomial
    class GaussLaguerrePolynomial : public GaussianOrthogonalPolynomial {
      public:
        explicit GaussLaguerrePolynomial(Real s = 0.0);

        Real mu_0() const override;
        Real alpha(Size i) const override;
        Real beta(Size i) const override;
        Real w(Real x) const override;

      private:
        const Real s_;
    };

    //! Gauss-Hermite polynomial
    class GaussHermitePolynomial : public GaussianOrthogonalPolynomial {
      public:
        explicit GaussHermitePolynomial(Real mu = 0.0);

        Real mu_0() const override;
        Real alpha(Size i) const override;
        Real beta(Size i) const override;
        Real w(Real x) const override;

      private:
        const Real mu_;
    };

    //! Gauss-Jacobi polynomial
    class GaussJacobiPolynomial : public GaussianOrthogonalPolynomial {
      public:
        explicit GaussJacobiPolynomial(Real alpha, Real beta);

        Real mu_0() const override;
        Real alpha(Size i) const override;
        Real beta(Size i) const override;
        Real w(Real x) const override;

      private:
        const Real alpha_;
        const Real beta_;
    };

    //! Gauss-Legendre polynomial
    class GaussLegendrePolynomial : public GaussJacobiPolynomial {
      public:
        GaussLegendrePolynomial();
    };

    //! Gauss-Chebyshev polynomial
    class GaussChebyshevPolynomial : public GaussJacobiPolynomial {
      public:
        GaussChebyshevPolynomial();
    };

    //! Gauss-Chebyshev polynomial (second kind)
    class GaussChebyshev2ndPolynomial : public GaussJacobiPolynomial {
      public:
        GaussChebyshev2ndPolynomial();
    };

    //! Gauss-Gegenbauer polynomial
    class GaussGegenbauerPolynomial : public GaussJacobiPolynomial {
      public:
        explicit GaussGegenbauerPolynomial(Real lambda);
    };

    //! Gauss hyperbolic polynomial
    class GaussHyperbolicPolynomial : public GaussianOrthogonalPolynomial {
      public:
        Real mu_0() const override;
        Real alpha(Size i) const override;
        Real beta(Size i) const override;
        Real w(Real x) const override;
    };

}

#endif
