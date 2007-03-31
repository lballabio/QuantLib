/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006 StatPro Italia srl

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

#include <ql/Volatilities/sabr.hpp>
#include <ql/Utilities/dataformatters.hpp>
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
        const Real logM = std::log(forward/strike);
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
        const Real multiplier = (xx!=0.0 ? z/xx : 1.0);
        return (alpha/D)*multiplier*d;
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
        QL_REQUIRE(forward>0.0, "forward must be positive: "
                                << io::rate(forward) << " not allowed");
        QL_REQUIRE(expiryTime>=0.0, "expiry time must be non-negative: "
                                   << expiryTime << " not allowed");
        validateSabrParameters(alpha, beta, nu, rho);
        return unsafeSabrVolatility(strike, forward, expiryTime,
                                    alpha, beta, nu, rho);
    }

}
