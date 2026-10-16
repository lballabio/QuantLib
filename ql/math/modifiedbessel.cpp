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

        // psi(m) for positive integer m: psi(1) = -gamma and
        // psi(m) = -gamma + sum_{j=1}^{m-1} 1/j. The integer-order series
        // below only ever needs the digamma function at positive integers,
        // so the general function is not required.
        Real digammaInteger(Size m) {
            // Euler-Mascheroni constant
            static const Real gamma = 0.57721566490153286060651209008240243;

            Real s = -gamma;
            for (Size j=1; j < m; ++j)
                s += 1.0/static_cast<Real>(j);
            return s;
        }

        // K_n for integer n, Abramowitz & Stegun 9.6.11. The
        // I_(-nu) - I_nu route cannot be used here: sin(nu*pi) vanishes at
        // integer order, so the quotient is 0/0 at nu = 0 and catastrophically
        // ill-conditioned at nu = 1, 2, ...
        template <class T, template <class> class W>
        T modifiedBesselFunction_k_integer_impl(Size n, const T& x) {
            const T half = 0.5*x;
            const T y = half*half;

            T sum1 = T(0.0);
            if (n > 0) {
                // (1/2) (x/2)^-n sum_{k=0}^{n-1} ((n-k-1)!/k!) (-x^2/4)^k
                Real coeff = 1.0;
                for (Size j=1; j < n; ++j)
                    coeff *= static_cast<Real>(j);   // (n-1)!
                T minusY = T(1.0);
                Real kFactorial = 1.0;
                for (Size k=0; k < n; ++k) {
                    if (k > 0) {
                        kFactorial *= static_cast<Real>(k);
                        coeff /= static_cast<Real>(n-k);
                        minusY *= -y;
                    }
                    sum1 += (coeff/kFactorial) * minusY;
                }
                sum1 *= 0.5 * std::pow(half, -static_cast<Real>(n));
            }

            const Real sign = (n % 2 == 0) ? -1.0 : 1.0;
            const T sum2 = sign * std::log(half) *
                modifiedBesselFunction_i_impl<T, Unweighted>(
                    static_cast<Real>(n), x);

            T sum3 = T(0.0), yPower = T(1.0);
            Real kFactorial = 1.0, nkFactorial = 1.0;
            for (Size j=1; j <= n; ++j)
                nkFactorial *= static_cast<Real>(j);      // n!
            for (Size k=0; k < 1000; ++k) {
                if (k > 0) {
                    kFactorial *= static_cast<Real>(k);
                    nkFactorial *= static_cast<Real>(n+k);
                    yPower *= y;
                }
                const T term = (digammaInteger(k+1) + digammaInteger(n+k+1))
                               * yPower / (kFactorial*nkFactorial);
                sum3 += term;
                if (k > 2 && std::abs(term) <= std::abs(sum3)*QL_EPSILON)
                    break;
            }
            sum3 *= -sign * 0.5 * std::pow(half, static_cast<Real>(n));

            return (sum1 + sum2 + sum3) * W<T>().weightSmallX(x);
        }

        template <class T, template <class> class W>
        T modifiedBesselFunction_k_impl(Real nu, const T& x) {
            if (std::abs(x) >= 13.0) {
                // K cannot be recovered from the difference of the two I
                // series out here: that asymptotic expansion depends on nu
                // only through nu*nu, so I_(-nu) and I_nu are identical and
                // the difference is exactly zero rather than merely
                // ill-conditioned. Sum the K expansion directly instead; it
                // shares the coefficients of the I expansion but without the
                // alternating sign, and having no sin(nu*pi) denominator it
                // is also unaffected by integer order. Note this only bites
                // for real arguments: for std::complex the i*exp(i*nu*pi)
                // term above is non-zero, so the two I values differ there
                // and the subtraction remains usable.
                Real na_k=1.0;
                T da_k=T(1.0), s=T(1.0);

                for (Size k=1; k < 30; ++k) {
                    na_k *= (4.0 * nu * nu -
                             (2.0 * static_cast<Real>(k) - 1.0) *
                                 (2.0 * static_cast<Real>(k) - 1.0));
                    da_k *= (8.0 * k) * x;
                    s += na_k/da_k;
                }

                return std::sqrt(M_PI / (2.0 * x)) *
                       W<T>().weight2LargeX(x) * s;
            }

            // At integer order sin(nu*pi) vanishes together with the
            // numerator: the singularity is removable, but evaluating the
            // quotient directly gives 0/0 at nu = 0 and a result wrong by
            // many orders of magnitude, with an arbitrary sign, at
            // nu = 1, 2, ... Use the integer-order series instead. The
            // threshold is deliberately tight: away from it the quotient
            // below is the more accurate of the two.
            const Real nearest = std::floor(std::abs(nu) + 0.5);
            if (std::abs(std::abs(nu) - nearest) < 1e-9) {
                // K is even in nu, so K_(-n) = K_n
                return modifiedBesselFunction_k_integer_impl<T, W>(
                    static_cast<Size>(nearest), x);
            }

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
