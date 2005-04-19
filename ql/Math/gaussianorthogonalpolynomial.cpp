/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

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

/*! \file gaussianquadratures.hpp
    \brief Integral of a 1-dimensional function using the Gauss quadratures
*/

#include <ql/Math/gaussianorthogonalpolynomial.hpp>
#include <ql/Math/gammadistribution.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    GaussLaguerrePolynomial::GaussLaguerrePolynomial(Real s)
    : s_(s) {
        QL_REQUIRE(s > -1.0, "s must be bigger than -1");
    }

    Real GaussLaguerrePolynomial::mu_0() const {
        return std::exp(GammaFunction().logValue(s_+1));
    }

    Real GaussLaguerrePolynomial::alpha(Size i) const {
        return 2*i+1+s_;
    }

    Real GaussLaguerrePolynomial::beta(Size i) const {
        return i*(i+s_);
    }

    Real GaussLaguerrePolynomial::w(Real x) const {
        return std::pow(x, s_)*std::exp(-x);
    }


    GaussHermitePolynomial::GaussHermitePolynomial(Real mu)
    : mu_(mu) {
        QL_REQUIRE(mu > -0.5, "mu must be bigger than -0.5");
    }

    Real GaussHermitePolynomial::mu_0() const {
        return std::exp(GammaFunction().logValue(mu_+0.5));
    }

    Real GaussHermitePolynomial::alpha(Size i) const {
        return 0.0;
    }

    Real GaussHermitePolynomial::beta(Size i) const {
        return (i%2)? i/2.0 + mu_ : i/2.0;
    }

    Real GaussHermitePolynomial::w(Real x) const {
        return std::pow(std::fabs(x), 2*mu_)*std::exp(-x*x);
    }

    GaussJacobiPolynomial::GaussJacobiPolynomial(Real alpha, Real beta)
    : alpha_(alpha), beta_ (beta) {
        QL_REQUIRE(alpha_+beta_ > -2.0,"alpha+beta must be bigger than -2");
        QL_REQUIRE(alpha_       > -1.0,"alpha must be bigger than -1");
        QL_REQUIRE(beta_        > -1.0,"beta  must be bigger than -1");
    }

    Real GaussJacobiPolynomial::mu_0() const {
        return std::pow(2.0, alpha_+beta_+1)
            * std::exp( GammaFunction().logValue(alpha_+1)
                        +GammaFunction().logValue(beta_ +1)
                        -GammaFunction().logValue(alpha_+beta_+2));
    }

    Real GaussJacobiPolynomial::alpha(Size i) const {
        Real num = beta_*beta_ - alpha_*alpha_;
        Real denom = (2.0*i+alpha_+beta_)*(2.0*i+alpha_+beta_+2);

        if (!denom) {
            if (num) {
                QL_FAIL("can't compute b_k for jacobi integration\n");
            }
            else {
                // l'Hospital
                num  = 2*beta_;
                denom= 2*(2.0*i+alpha_+beta_+1);

                QL_ASSERT(denom, "can't compute a_k for jacobi integration\n");
            }
        }

        return num / denom;
    }

    Real GaussJacobiPolynomial::beta(Size i) const {
        Real num = 4.0*i*(i+alpha_)*(i+beta_)*(i+alpha_+beta_);
        Real denom = (2.0*i+alpha_+beta_)*(2.0*i+alpha_+beta_)
                   * ((2.0*i+alpha_+beta_)*(2.0*i+alpha_+beta_)-1);

        if (!denom) {
            if (num) {
                QL_FAIL("can't compute b_k for jacobi integration\n");
            } else {
                // l'Hospital
                num  = 4.0*i*(i+beta_)* (2.0*i+2*alpha_+beta_);
                denom= 2.0*(2.0*i+alpha_+beta_);
                denom*=denom-1;
                QL_ASSERT(denom, "can't compute b_k for jacobi integration\n");
            }
        }
        return num / denom;
    }

    Real GaussJacobiPolynomial::w(Real x) const {
        return std::pow(1-x, alpha_)*std::pow(1+x, beta_);
    }


    Real GaussHyperbolicPolynomial::mu_0() const {
        return M_PI;
    }

    Real GaussHyperbolicPolynomial::alpha(Size i) const {
        return 0.0;
    }

    Real GaussHyperbolicPolynomial::beta(Size i) const {
        return i ? M_PI_2*M_PI_2*i*i : M_PI;
    }

    Real GaussHyperbolicPolynomial::w(Real x) const {
        return 1/std::cosh(x);
    }

}

