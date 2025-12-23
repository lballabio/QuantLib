/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Klaus Spanderen

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

/*! \file modifiedbessel.cpp
    \brief modified Bessel functions of first and second kind
*/

#include <ql/math/modifiedbessel.hpp>
#include <ql/math/distributions/gammadistribution.hpp>

#include <cmath>

namespace QuantLib {

    namespace {

        template <class T>  struct I {};
        template <> struct I<Real> { Real value() { return 0.0;} };
        template <> struct I<std::complex<Real> > {
            std::complex<Real> value() { return std::complex<Real>(0.0,1.0);}
        };
        template <class T> struct Unweighted {
            T weightSmallX(const T& x) { return 1.0; }
            T weight1LargeX(const T& x) { return std::exp(x); }
            T weight2LargeX(const T& x) { return std::exp(-x); }
        };
        template <class T> struct ExponentiallyWeighted {
            T weightSmallX(const T& x) { return std::exp(-x); }
            T weight1LargeX(const T& x) { return 1.0; }
            T weight2LargeX(const T& x) { return std::exp(-2.0*x); }
        };

        template <class T, template <class> class W>
        T modifiedBesselFunction_i_impl(Real nu, const T& x) {
            if (std::abs(x) < 13.0) {
                const T alpha = std::pow(0.5*x, nu)
                               /GammaFunction().value(1.0+nu);
                const T Y = 0.25*x*x;
                Size k=1;
                T sum=alpha, B_k=alpha;

                while (std::abs(B_k*=Y/(k*(k+nu)))>std::abs(sum)*QL_EPSILON) {
                    sum += B_k;
                    QL_REQUIRE(++k < 1000, "max iterations exceeded");
                }
                return sum * W<T>().weightSmallX(x);
            }
            else {
                Real na_k=1.0, sign=1.0;
                T da_k=T(1.0);

                T s1=T(1.0), s2=T(1.0);
                for (Size k=1; k < 30; ++k) {
                    sign*=-1;
                    na_k *= (4.0 * nu * nu -
                             (2.0 * static_cast<Real>(k) - 1.0) *
                                 (2.0 * static_cast<Real>(k) - 1.0));
                    da_k *= (8.0 * k) * x;
                    const T a_k = na_k/da_k;

                    s2+=a_k;
                    s1+=sign*a_k;
                }

                const T i = I<T>().value();
                return 1.0 / std::sqrt(2 * M_PI * x) *
                    (W<T>().weight1LargeX(x) * s1 +
                     i * std::exp(i * nu * M_PI) * W<T>().weight2LargeX(x) * s2);
            }
        }

        template <class T, template <class> class W>
        T modifiedBesselFunction_k_impl(Real nu, const T& x) {
            return M_PI_2 * (modifiedBesselFunction_i_impl<T,W>(-nu, x) -
                             modifiedBesselFunction_i_impl<T,W>(nu, x)) /
                             std::sin(M_PI * nu);
        }
    }

    Real modifiedBesselFunction_i(Real nu, Real x) {
        QL_REQUIRE(x >= 0.0, "negative argument requires complex version of "
                             "modifiedBesselFunction");
        return modifiedBesselFunction_i_impl<Real, Unweighted>(nu, x);
    }

    std::complex<Real> modifiedBesselFunction_i(Real nu,
                                                const std::complex<Real> &z) {
        if (z.imag() == 0.0 && z.real() >= 0.0)
            return std::complex<Real>(modifiedBesselFunction_i(nu, z.real()));

        return modifiedBesselFunction_i_impl<
            std::complex<Real>, Unweighted>(nu, z);
    }

    Real modifiedBesselFunction_k(Real nu, Real x) {
        return modifiedBesselFunction_k_impl<Real, Unweighted>(nu, x);
    }

    std::complex<Real> modifiedBesselFunction_k(Real nu,
                                                const std::complex<Real> &z) {
        if (z.imag() == 0.0 && z.real() >= 0.0)
            return std::complex<Real>(modifiedBesselFunction_k(nu, z.real()));

        return modifiedBesselFunction_k_impl<
            std::complex<Real>, Unweighted>(nu, z);
    }

    Real modifiedBesselFunction_i_exponentiallyWeighted(Real nu, Real x) {
        QL_REQUIRE(x >= 0.0, "negative argument requires complex version of "
                             "modifiedBesselFunction");
        return modifiedBesselFunction_i_impl<Real, ExponentiallyWeighted>(
            nu, x);
    }

    std::complex<Real> modifiedBesselFunction_i_exponentiallyWeighted(
        Real nu, const std::complex<Real> &z) {

        if (z.imag() == 0.0 && z.real() >= 0.0)
            return std::complex<Real>(
                modifiedBesselFunction_i_exponentiallyWeighted(nu, z.real()));

        return modifiedBesselFunction_i_impl<
            std::complex<Real>, ExponentiallyWeighted>(nu, z);
    }

    Real modifiedBesselFunction_k_exponentiallyWeighted(Real nu, Real x) {
        return modifiedBesselFunction_k_impl<Real, ExponentiallyWeighted>(
            nu, x);
    }

    std::complex<Real> modifiedBesselFunction_k_exponentiallyWeighted(
        Real nu, const std::complex<Real> &z) {

        if (z.imag() == 0.0 && z.real() >= 0.0)
            return std::complex<Real>(
                modifiedBesselFunction_k_exponentiallyWeighted(nu, z.real()));

        return modifiedBesselFunction_k_impl<
            std::complex<Real>, ExponentiallyWeighted>(nu, z);
    }

}
