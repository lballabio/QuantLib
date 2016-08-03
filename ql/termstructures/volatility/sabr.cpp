/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2015 Peter Caspers

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

#include <ql/termstructures/volatility/sabr.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/math/comparison.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    Real unsafeSabrVolatility(Rate strike,
                              Rate forward,
                              Time expiryTime,
                              Real alpha,
                              Real beta,
                              Real nu,
                              Real rho) {
        const Real oneMinusBeta = 1.0-beta;
        const Real A = std::pow(forward*strike, oneMinusBeta);
        const Real sqrtA= std::sqrt(A);
        Real logM;
        if (!close(forward, strike))
            logM = std::log(forward/strike);
        else {
            const Real epsilon = (forward-strike)/strike;
            logM = epsilon - .5 * epsilon * epsilon ;
        }
        const Real z = (nu/alpha)*sqrtA*logM;
        const Real B = 1.0-2.0*rho*z+z*z;
        const Real C = oneMinusBeta*oneMinusBeta*logM*logM;
        const Real tmp = (std::sqrt(B)+z-rho)/(1.0-rho);
        const Real xx = std::log(tmp);
        const Real D = sqrtA*(1.0+C/24.0+C*C/1920.0);
        const Real d = 1.0 + expiryTime *
            (oneMinusBeta*oneMinusBeta*alpha*alpha/(24.0*A)
                                + 0.25*rho*beta*nu*alpha/sqrtA
                                    +(2.0-3.0*rho*rho)*(nu*nu/24.0));

        Real multiplier;
        // computations become precise enough if the square of z worth
        // slightly more than the precision machine (hence the m)
        static const Real m = 10;
        if (std::fabs(z*z)>QL_EPSILON * m)
            multiplier = z/xx;
        else {
            multiplier = 1.0 - 0.5*rho*z - (3.0*rho*rho-2.0)*z*z/12.0;
        }
        return (alpha/D)*multiplier*d;
    }

    Real unsafeShiftedSabrVolatility(Rate strike,
                              Rate forward,
                              Time expiryTime,
                              Real alpha,
                              Real beta,
                              Real nu,
                              Real rho,
                              Real shift) {

        return unsafeSabrVolatility(strike+shift,forward+shift,expiryTime,
                                    alpha,beta,nu,rho);

    }

    void validateSabrParameters(Real alpha,
                                Real beta,
                                Real nu,
                                Real rho) {
        QL_REQUIRE(alpha>0.0, "alpha must be positive: "
                              << alpha << " not allowed");
        QL_REQUIRE(beta>=0.0 && beta<=1.0, "beta must be in (0.0, 1.0): "
                                         << beta << " not allowed");
        QL_REQUIRE(nu>=0.0, "nu must be non negative: "
                            << nu << " not allowed");
        QL_REQUIRE(rho*rho<1.0, "rho square must be less than one: "
                                << rho << " not allowed");
    }

    Real sabrVolatility(Rate strike,
                        Rate forward,
                        Time expiryTime,
                        Real alpha,
                        Real beta,
                        Real nu,
                        Real rho) {
        QL_REQUIRE(strike>0.0, "strike must be positive: "
                               << io::rate(strike) << " not allowed");
        QL_REQUIRE(forward>0.0, "at the money forward rate must be "
                   "positive: " << io::rate(forward) << " not allowed");
        QL_REQUIRE(expiryTime>=0.0, "expiry time must be non-negative: "
                                   << expiryTime << " not allowed");
        validateSabrParameters(alpha, beta, nu, rho);
        return unsafeSabrVolatility(strike, forward, expiryTime,
                                    alpha, beta, nu, rho);
    }

    Real shiftedSabrVolatility(Rate strike,
                                 Rate forward,
                                 Time expiryTime,
                                 Real alpha,
                                 Real beta,
                                 Real nu,
                                 Real rho,
                                 Real shift) {
        QL_REQUIRE(strike + shift > 0.0, "strike+shift must be positive: "
                   << io::rate(strike) << "+" << io::rate(shift) << " not allowed");
        QL_REQUIRE(forward + shift > 0.0, "at the money forward rate + shift must be "
                   "positive: " << io::rate(forward) << " " << io::rate(shift) << " not allowed");
        QL_REQUIRE(expiryTime>=0.0, "expiry time must be non-negative: "
                                   << expiryTime << " not allowed");
        validateSabrParameters(alpha, beta, nu, rho);
        return unsafeShiftedSabrVolatility(strike, forward, expiryTime,
                                             alpha, beta, nu, rho,shift);
    }

}
