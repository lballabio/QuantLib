/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2015 Peter Caspers
 Copyright (C) 2019 Klaus Spanderen

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
#include <ql/math/functional.hpp>
#include <ql/errors.hpp>
#include <ql/termstructures/volatility/volatilitytype.hpp>

namespace QuantLib {

    Real unsafeSabrLogNormalVolatility(
                              Rate strike,
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
                              Real shift,
                              VolatilityType volatilityType) {
        if (volatilityType == VolatilityType::Normal) {
            return unsafeSabrNormalVolatility(strike + shift, forward + shift, expiryTime, alpha, beta, nu, rho);
        } else {
            return unsafeSabrLogNormalVolatility(strike + shift, forward + shift, expiryTime, alpha, beta, nu, rho);
        }
    }

    Real unsafeSabrNormalVolatility(
        Rate strike, Rate forward, Time expiryTime, Real alpha, Real beta, Real nu, Real rho) {
        const Real oneMinusBeta = 1.0 - beta;
        const Real minusBeta = -1.0 * beta;
        const Real A = std::pow(forward * strike, oneMinusBeta);
        const Real sqrtA = std::sqrt(A);
        Real logM;
        if (!close(forward, strike))
            logM = std::log(forward / strike);
        else {
            const Real epsilon = (forward - strike) / strike;
            logM = epsilon - .5 * epsilon * epsilon;
        }
        const Real z = (nu / alpha) * sqrtA * logM;
        const Real B = 1.0 - 2.0 * rho * z + z * z;
        const Real C = oneMinusBeta * oneMinusBeta * logM * logM;
        const Real D = logM * logM;
        const Real tmp = (std::sqrt(B) + z - rho) / (1.0 - rho);
        const Real xx = std::log(tmp);
        const Real E_1 = (1.0 + D / 24.0 + D * D / 1920.0);
        const Real E_2 = (1.0 + C / 24.0 + C * C / 1920.0);
        const Real E = E_1 / E_2;
        const Real d = 1.0 + expiryTime * (minusBeta * (2 - beta) * alpha * alpha / (24.0 * A) +
                                0.25 * rho * beta * nu * alpha / sqrtA +
                                (2.0 - 3.0 * rho * rho) * (nu * nu / 24.0));

        Real multiplier;
        // computations become precise enough if the square of z worth
        // slightly more than the precision machine (hence the m)
        static const Real m = 10;
        if (std::fabs(z * z) > QL_EPSILON * m)
            multiplier = z / xx;
        else {
            multiplier = 1.0 - 0.5 * rho * z - (3.0 * rho * rho - 2.0) * z * z / 12.0;
        }
        const Real F = alpha * std::pow(forward * strike, beta / 2.0);

       return F * E * multiplier * d;
    }

     Real unsafeSabrVolatility(Rate strike,
                              Rate forward,
                              Time expiryTime,
                              Real alpha,
                              Real beta,
                              Real nu,
                              Real rho,
                              VolatilityType volatilityType) {
        if (volatilityType == VolatilityType::Normal) {
            return unsafeSabrNormalVolatility(strike, forward, expiryTime, alpha, beta, nu, rho);
        } else {
            return unsafeSabrLogNormalVolatility(strike, forward, expiryTime, alpha, beta, nu, rho);
        }
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
                        Real rho,
                        VolatilityType volatilityType) {
        QL_REQUIRE(strike>0.0, "strike must be positive: "
                               << io::rate(strike) << " not allowed");
        QL_REQUIRE(forward>0.0, "at the money forward rate must be "
                   "positive: " << io::rate(forward) << " not allowed");
        QL_REQUIRE(expiryTime>=0.0, "expiry time must be non-negative: "
                                   << expiryTime << " not allowed");
        validateSabrParameters(alpha, beta, nu, rho);
        return unsafeSabrVolatility(strike, forward, expiryTime, alpha, beta, nu, rho,
                                             volatilityType);
    }

    Real shiftedSabrVolatility(Rate strike,
                               Rate forward,
                               Time expiryTime,
                               Real alpha,
                               Real beta,
                               Real nu,
                               Real rho,
                               Real shift,
                               VolatilityType volatilityType) {
        QL_REQUIRE(strike + shift > 0.0, "strike+shift must be positive: "
                   << io::rate(strike) << "+" << io::rate(shift) << " not allowed");
        QL_REQUIRE(forward + shift > 0.0, "at the money forward rate + shift must be "
                   "positive: " << io::rate(forward) << " " << io::rate(shift) << " not allowed");
        QL_REQUIRE(expiryTime>=0.0, "expiry time must be non-negative: "
                                   << expiryTime << " not allowed");
        validateSabrParameters(alpha, beta, nu, rho);
        return unsafeShiftedSabrVolatility(strike, forward, expiryTime,
                                             alpha, beta, nu, rho,shift, volatilityType);
    }

    namespace {
        struct SabrFlochKennedyVolatility {
            Real F, alpha, beta, nu, rho, t;

            Real y(Real k) const {
                return -1.0/(1.0-beta)*(std::pow(F,1-beta)-std::pow(k,1-beta));
            }

            Real Dint(Real k) const {
                return 1/nu*std::log( ( std::sqrt(1+2*rho*nu/alpha*y(k)
                    + square<Real>()(nu/alpha*y(k)) )
                    - rho - nu/alpha*y(k) ) / (1-rho) );
            }

            Real D(Real k) const {
                return std::sqrt(alpha*alpha+2*alpha*rho*nu*y(k)
                    + square<Real>()(nu*y(k)))*std::pow(k,beta);
            }

            Real omega0(Real k) const {
                return std::log(F/k)/Dint(k);
            }

            Real operator()(Real k) const {
                const Real m = F/k;
                if (m > 1.0025 || m < 0.9975) {
                    return omega0(k)*(1+0.25*rho*nu*alpha*
                       (std::pow(k,beta)-std::pow(F,beta))/(k-F)*t)
                       -omega0(k)/square<Real>()(Dint(k))*(std::log(
                           omega0(k)) + 0.5*std::log((F*k/(D(F)*D(k))) ))*t;
                }
                else {
                    return taylorExpansion(k);
                }
            }

            Real taylorExpansion(Real k) const {
                const Real F2 = F*F;
                const Real alpha2 = alpha*alpha;
                const Real rho2 = rho*rho;
                return
                    (alpha*std::pow(F,-3 + beta)*(alpha2*square<Real>()(-1 + beta)*std::pow(F,2*beta)*t + 6*alpha*beta*nu*std::pow(F,1 + beta)*rho*t +
                        F2*(24 + nu*nu*(2 - 3*rho2)*t)))/24.0 +
                     (3*alpha2*alpha*std::pow(-1 + beta,3)*std::pow(F,3*beta)*t +
                        3*alpha2*(-1 + beta)*(-1 + 5*beta)*nu*std::pow(F,1 + 2*beta)*rho*t + nu*F2*F*rho*(24 + nu*nu*(-4 + 3*rho2)*t) +
                        alpha*std::pow(F,2 + beta)*(24*(-1 + beta) + nu*nu*(2*(-1 + beta) + 3*(1 + beta)*rho2)*t))/(48.*F2*F2) * (k-F) +
                    (std::pow(F,-5 - beta)*(alpha2*alpha2*std::pow(-1 + beta,3)*(-209 + 119*beta)*std::pow(F,4*beta)*t + 30*alpha2*alpha*(-1 + beta)*(9 + beta*(-37 + 18*beta))*nu*std::pow(F,1 + 3*beta)*rho*t -
                        30*alpha*nu*std::pow(F,3 + beta)*rho*(24 + nu*nu*(-4*(1 + beta) + 3*(1 + 2*beta)*rho2)*t) +
                        10*alpha2*std::pow(F,2 + 2*beta)*(24*(-4 + beta)*(-1 + beta) + nu*nu*(2*(-1 + beta)*(-7 + 4*beta) + 3*(-4 + beta*(-7 + 5*beta))*rho2)*t) +
                        nu*nu*F2*F2*(480 - 720*rho2 + nu*nu*(-64 + 75*rho2*(4 - 3*rho2))*t)))/(2880*alpha) * (k-F)*(k-F);
            }
        };
    }

    Real sabrFlochKennedyVolatility(Rate strike,
                                Rate forward,
                                Time expiryTime,
                                Real alpha,
                                Real beta,
                                Real nu,
                                Real rho) {
        const SabrFlochKennedyVolatility v =
            {forward, alpha, beta, nu, rho, expiryTime};

        return v(strike);
    }

}
