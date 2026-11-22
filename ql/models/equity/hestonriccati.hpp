/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file hestonriccati.hpp
    \brief Stable characteristic-function step for the Heston model
*/

#ifndef quantlib_heston_riccati_hpp
#define quantlib_heston_riccati_hpp

#include <ql/math/expm1.hpp>
#include <utility>

namespace QuantLib {
    namespace detail {

        // Return the new D and the increment to C in exp(C + v*D).
        inline std::pair<std::complex<Real>, std::complex<Real>>
        hestonRiccatiStep(const std::complex<Real>& z, Time t,
                         Real kappa, Real theta, Real sigma, Real rho,
                         const std::complex<Real>& D = 0.0) {
            const Real sigma2 = sigma*sigma;
            const std::complex<Real> w = z*std::complex<Real>(z.real(), z.imag()+1.0);
            if (w == std::complex<Real>() && D == std::complex<Real>())
                return {};

            const std::complex<Real> k =
                kappa + rho*sigma*std::complex<Real>(z.imag(), -z.real());
            const std::complex<Real> d = std::sqrt(k*k + sigma2*w);

            // Rationalize k-d when the subtraction would lose precision, as in
            // the Andersen-Lake form in AnalyticHestonEngine::lnChF.
            const bool rationalizeDifference = k.real()*d.real() + k.imag()*d.imag() > 0.0;
            std::complex<Real> r = k-d;
            if (rationalizeDifference)
                r = -sigma2*w/(k+d);

            const std::complex<Real> exponent = -d*t;
            std::complex<Real> decay, decayMinusOne;
            if (std::norm(exponent) < 1.0) {
                decayMinusOne = expm1(exponent);
                decay = 1.0 + decayMinusOne;
            } else {
                decay = std::exp(exponent);
                decayMinusOne = decay - 1.0;
            }

            std::complex<Real> y = -0.5*t;
            if (d.real() != 0.0 || d.imag() != 0.0)
                y = decayMinusOne/(2.0*d);

            // The old (1-gt*exp(-d*t))/(1-gt) ratio is 1+q. Form q directly,
            // including the D carried in from the next segment.
            const std::complex<Real> scaledD = sigma2*D;
            const std::complex<Real> q = (scaledD-r)*y;
            std::complex<Real> denominator = 1.0+q;
            if (q.real() < -0.5 && std::norm(denominator) < 0.25) {
                // Near zero, retain the decaying term and rationalize k+d instead.
                std::complex<Real> rootSum = k+d;
                if (!rationalizeDifference && r != std::complex<Real>())
                    rootSum = -sigma2*w/r;
                const std::complex<Real> correction = (rootSum-scaledD)*y;
                denominator = decay - correction;
                return { (D*decay - (r/sigma2)*correction)/denominator,
                         kappa*theta/sigma2*(r*t - 2.0*std::log(denominator)) };
            }

            return { (D*(decay+r*y) + w*y)/denominator,
                     kappa*theta/sigma2*(r*t - 2.0*log1p(q)) };
        }

    }
}

#endif
