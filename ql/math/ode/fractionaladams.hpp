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

/*! \file fractionaladams.hpp
    \brief fractional Adams-Bashforth-Moulton predictor-corrector scheme
*/

#ifndef quantlib_fractional_adams_hpp
#define quantlib_fractional_adams_hpp

#include <ql/errors.hpp>
#include <ql/math/distributions/gammadistribution.hpp>
#include <ql/types.hpp>
#include <cmath>
#include <functional>
#include <vector>

namespace QuantLib {

    //! fractional Adams-Bashforth-Moulton predictor-corrector scheme
    /*! Solves the fractional-order initial value problem
        \f[
            D^{\alpha} y(t) = f(t, y(t)),\qquad y(0) = y_0,
            \qquad \alpha \in (0, 1],
        \f]
        where \f$ D^{\alpha} \f$ denotes the Caputo fractional derivative.
        For \f$ \alpha = 1 \f$ the scheme reduces to the classical one-step
        Adams-Bashforth-Moulton method.  The memory of the fractional
        kernel makes the cost \f$ O(N^2) \f$ in the number of time steps.

        References:
        K. Diethelm, N.J. Ford, A.D. Freed, A Predictor-Corrector Approach
        for the Numerical Solution of Fractional Differential Equations,
        Nonlinear Dynamics 29, 3-22 (2002).
    */

    template <class T = Real>
    class FractionalAdams {
      public:
        typedef std::function<T(Real, const T&)> OdeFct;

        explicit FractionalAdams(Real alpha) : alpha_(alpha) {
            QL_REQUIRE(alpha > 0.0 && alpha <= 1.0,
                       "fractional order alpha (" << alpha << ") must be in (0, 1]");
        }

        //! solution values on the uniform grid \f$ \{0, \Delta t, \ldots, t_{max}\} \f$
        std::vector<T> solve(const OdeFct& ode, const T& y0, Real tMax, Size steps) const {
            QL_REQUIRE(tMax > 0.0, "tMax (" << tMax << ") must be positive");
            QL_REQUIRE(steps > 0, "at least one step required");

            const Real dt(tMax / steps);
            const Real dtAlpha{std::pow(dt, alpha_)};
            const Real gamma1{GammaFunction().value(alpha_ + 1.0)};
            const Real gamma2{GammaFunction().value(alpha_ + 2.0)};

            // powers k ^ alpha and k ^ (alpha + 1) for the quadrature weights
            std::vector<Real> alphaPowers(steps + 2), alphaPlusOnePowers(steps + 2);
            for (Size k{0}; k <= steps + 1; ++k) {
                alphaPowers[k] = std::pow(static_cast<Real>(k), alpha_);
                alphaPlusOnePowers[k] = alphaPowers[k] * k;
            }

            // predictor weights b_k = (k + 1) ^ alpha - k ^ alpha and corrector
            // weights c_k = (k + 2) ^ (alpha + 1) + k ^ (alpha + 1) - 2(k + 1)^(alpha + 1),
            // both depending only on the distance k = n - j to the current step
            std::vector<Real> predictorWeights(steps), correctorWeights(steps);
            for (Size k{0}; k < steps; ++k) {
                predictorWeights[k] = alphaPowers[k + 1] - alphaPowers[k];
                correctorWeights[k] = alphaPlusOnePowers[k + 2] + alphaPlusOnePowers[k] -
                                       2.0 * alphaPlusOnePowers[k + 1];
            }

            std::vector<T> y(steps + 1), f(steps + 1);
            y[0] = y0;
            f[0] = ode(0.0, y0);

            for (Size n{0}; n < steps; ++n) {
                const Real tNext = (n + 1) * dt;

                T predictorSum = predictorWeights[n] * f[0];
                for (Size j{1}; j <= n; ++j)
                    predictorSum += predictorWeights[n - j] * f[j];
                const T yP = y0 + dtAlpha / gamma1 * predictorSum;

                // corrector weight of f_0: n ^ (alpha + 1) - (n - alpha)(n + 1) ^ alpha
                const Real a0 = alphaPlusOnePowers[n] - (n - alpha_) * alphaPowers[n + 1];

                T correctorSum = a0 * f[0];
                for (Size j{1}; j <= n; ++j)
                    correctorSum += correctorWeights[n - j] * f[j];
                correctorSum += ode(tNext, yP);

                y[n + 1] = y0 + dtAlpha / gamma2 * correctorSum;
                f[n + 1] = ode(tNext, y[n + 1]);
            }

            return y;
        }

      private:
        const Real alpha_;
    };


    //! product-trapezoidal Riemann-Liouville fractional integral
    /*! Approximates
        \f[
            I^{\alpha} y(t_N) = \frac{1}{\Gamma(\alpha)}
                \int_0^{t_N} (t_N - s)^{\alpha - 1} y(s)\,ds,
            \qquad \alpha \ge 0,
        \f]
        given values of \f$ y \f$ on a uniform grid, by integrating the
        piecewise linear interpolant of \f$ y \f$ against the kernel.
        For \f$ \alpha = 1 \f$ this is the trapezoidal rule;
        \f$ \alpha \to 0 \f$ recovers the identity.
    */
    template <class T = Real>
    T riemannLiouvilleIntegral(const std::vector<T>& y, Real alpha, Real dt) {
        QL_REQUIRE(alpha >= 0.0, "integration order alpha (" << alpha << ") must be non-negative");
        QL_REQUIRE(y.size() >= 2, "at least two grid values required");
        QL_REQUIRE(dt > 0.0, "grid spacing dt (" << dt << ") must be positive");

        const Size n{y.size() - 1};

        // powers m ^ (alpha + 1) for m = 0, ..., n + 1, reused across the shifted
        // (k - 1, k, k + 1) terms in the quadrature weights below
        std::vector<Real> powAlphaPlus1(n + 2);
        for (Size m{0}; m <= n + 1; ++m)
            powAlphaPlus1[m] = std::pow(static_cast<Real>(m), alpha + 1.0);

        // weight of y_0: (n - 1) ^ (alpha + 1) - (n - 1 - alpha) n ^ alpha
        T sum = (powAlphaPlus1[n - 1] - (n - 1 - alpha) * std::pow(Real(n), alpha)) * y[0];

        for (Size j{1}; j < n; ++j) {
            const Size k{n - j};
            sum += (powAlphaPlus1[k + 1] + powAlphaPlus1[k - 1] - 2.0 * powAlphaPlus1[k]) * y[j];
        }
        sum += y[n];

        return std::pow(dt, alpha) / GammaFunction().value(alpha + 2.0) * sum;
    }
}

#endif
