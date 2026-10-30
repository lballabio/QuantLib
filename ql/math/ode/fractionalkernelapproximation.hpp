/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Colin Alberts

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

/*! \file fractionalkernelapproximation.hpp
    \brief sum-of-exponentials approximation of the fractional power-law kernel
*/

#ifndef quantlib_fractional_kernel_approximation_hpp
#define quantlib_fractional_kernel_approximation_hpp

#include <ql/errors.hpp>
#include <ql/math/array.hpp>
#include <ql/mathconstants.hpp>
#include <ql/types.hpp>
#include <cmath>

namespace QuantLib {

    namespace detail {

        // (exp(y) - 1) / y, continuous at the origin
        inline Real expm1OverX(Real y) {
            if (std::fabs(y) < 1e-8)
                return 1.0 + 0.5 * y;
            return std::expm1(y) / y;
        }

        /*  sin(pi b) / (pi b), the reflection-formula rewriting of
            \f$ 1 / ((1 - \alpha)\Gamma(\alpha)\Gamma(1 - \alpha)) \f$ for
            \f$ b = 1 - \alpha \f$.  \f$ \Gamma(1 - \alpha) \f$ overflows as
            \f$ \alpha \to 1 \f$ whereas this form tends to one.
        */
        inline Real fractionalKernelNorm(Real b) {
            const Real y{M_PI * b};

            if (std::fabs(y) < 1e-8)
                return 1.0 - y * y / 6.0;
            return std::sin(y) / y;
        }
    }

    //! Sum-of-exponentials approximation of the fractional kernel
    /*! Approximates the power-law kernel
        \f[
            K(t) = \frac{t^{\alpha - 1}}{\Gamma(\alpha)},
            \qquad \alpha \in (0, 1],
        \f]
        by \f$ K^n(t) = \sum_{i=1}^n c_i e^{-x_i t} \f$. Following Abi
        Jaber, K is completely monotone with Bernstein representation
        \f[
            K(t) = \int_0^\infty e^{-xt} \mu(dx),
            \qquad \mu(dx) = \frac{x^{-\alpha}}{\Gamma(\alpha)\Gamma(1-\alpha)} dx ,
        \f]
        and a partition \f$ 0 = \eta_0 < \eta_1 < \ldots < \eta_n \f$ gives
        non-negative \f$ c_i, x_i \f$ — hence a \f$ K^n \f$ that is itself
        completely monotone — by collapsing each interval's mass onto its
        barycentre,
        \f[
            c_i = \int_{\eta_{i-1}}^{\eta_i} \mu(dx),
            \qquad
            x_i = \frac{1}{c_i}\int_{\eta_{i-1}}^{\eta_i} x\, \mu(dx).
        \f]
        Taking \f$ \eta_0 = 0 \f$ recovers the non-fractional limit exactly:
        as \f$ \alpha \to 1 \f$ the first interval absorbs all the mass, so
        \f$ c_1 \to 1 \f$, \f$ x_1 \to 0 \f$ and \f$ K^n \to K \equiv 1 \f$,
        with no special-casing.

        References:

        E. Abi Jaber, Lifting the Heston model, Quantitative Finance
        19(12), 1995-2013 (2019).

        E. Abi Jaber and O. El Euch, Multifactor approximation of rough
        volatility models, SIAM Journal on Financial Mathematics 10(2),
        309-349 (2019).

        \test the weights and rates are tested for non-negativity, for
              convergence of \f$ K^n \f$ to \f$ K \f$ as \f$ n \f$ grows,
              and for the exact \f$ \alpha = 1 \f$ collapse to a single
              node.
    */
    class FractionalKernelApproximation {
      public:
        FractionalKernelApproximation(Real alpha, Size n, Time tMax);

        //! Weights \f$ c_i \f$
        const Array& weights() const { return c_; }
        //! Mean-reversion rates \f$ x_i \f$
        const Array& rates() const { return x_; }

        Size size() const { return c_.size(); }

        //! \f$ K^n(t) = \sum_i c_i e^{-x_i t} \f$
        Real operator()(Time t) const {
            Real value{0.0};
            for (Size i{0}; i < c_.size(); ++i)
                value += c_[i] * std::exp(-x_[i] * t);

            return value;
        }

      private:
        void build(Real alpha, Size n, Real etaLo, Real etaHi);

        Array c_, x_;
    };


    /*! Both endpoints have to widen with n: refining n never subdivides the
        leading interval \f$ [0, \eta_{lo}] \f$, so holding it fixed stalls
        the error at a few percent, and \f$ \eta_{hi} \f$ is what represents
        the singularity at the origin.  The exponents are empirical: the
        relative \f$ L^1 \f$ kernel error, measured for
        \f$ H \in \{0.05, 0.1, 0.3\} \f$ and \f$ n \le 160 \f$, falls off
        like \f$ n^{-1.55} \f$.
    */
    inline FractionalKernelApproximation::FractionalKernelApproximation(
        Real alpha, Size n, Time tMax) {

        QL_REQUIRE(tMax > 0.0, "tMax (" << tMax << ") must be positive");
        QL_REQUIRE(n > 0, "at least one factor required");

        build(alpha, n, 1.0 / (std::sqrt(Real(n)) * tMax),
              4.0 * std::pow(Real(n), 2.5) / tMax);
    }

    inline void FractionalKernelApproximation::build(
        Real alpha, Size n, Real etaLo, Real etaHi) {

        QL_REQUIRE(alpha > 0.0 && alpha <= 1.0,
                   "fractional order alpha (" << alpha << ") must be in (0, 1]");
        QL_REQUIRE(n > 0, "at least one factor required");
        QL_REQUIRE(etaLo > 0.0 && etaHi >= etaLo,
                   "partition endpoints (" << etaLo << ", " << etaHi
                   << ") must satisfy 0 < etaLo <= etaHi");

        const Real b{1.0 - alpha};
        const Real norm{detail::fractionalKernelNorm(b)};

        c_ = Array(n);
        x_ = Array(n);

        // A one-factor partition has to put its single node at the fast end,
        // so that the whole mass still sits on [0, eta_1]
        const Real eta1{(n == 1) ? etaHi : etaLo};

        // The first interval is [0, eta_1], whose mass tends to the total
        // mass as alpha -> 1; this is what recovers the constant kernel
        c_[0] = std::pow(eta1, b) * norm;
        x_[0] = b / (1.0 + b) * eta1;

        if (n == 1)
            return;

        const Real dL{std::log(etaHi / etaLo) / (n - 1)};
        const Real r{std::exp(dL)};

        // Both weight and rate go through expm1, so that the differences
        // eta_i^b - eta_{i-1}^b, which suffer catastrophic cancellation as
        // alpha -> 1, are never formed explicitly
        const Real weightFactor{std::expm1(b * dL) * norm};
        const Real rateFactor{detail::expm1OverX((1.0 + b) * dL)
            / detail::expm1OverX(b * dL)};

        Real eta{eta1};
        for (Size i{1}; i < n; ++i) {
            c_[i] = std::pow(eta, b) * weightFactor;
            x_[i] = eta * rateFactor;

            eta *= r;
        }
    }
}

#endif
