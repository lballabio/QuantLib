/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2012 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2013 Gary Kennedy
 Copyright (C) 2015 Peter Caspers
 Copyright (C) 2017 Klaus Spanderen
 Copyright (C) 2019 Wojciech Ślusarski
 Copyright (C) 2020 Marcin Rybacki

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

#include <ql/pricingengines/blackformula.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/solvers1d/newtonsafe.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <boost/math/special_functions/atanh.hpp>
#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif

#include <boost/math/special_functions/sign.hpp>

namespace {
    void checkParameters(QuantLib::Real strike,
                         QuantLib::Real forward,
                         QuantLib::Real displacement)
    {
        QL_REQUIRE(displacement >= 0.0, "displacement ("
                                            << displacement
                                            << ") must be non-negative");
        QL_REQUIRE(strike + displacement >= 0.0,
                   "strike + displacement (" << strike << " + " << displacement
                                             << ") must be non-negative");
        QL_REQUIRE(forward + displacement > 0.0, "forward + displacement ("
                                                     << forward << " + "
                                                     << displacement
                                                     << ") must be positive");
    }
}

namespace QuantLib {

    Real blackFormula(Option::Type optionType,
                      Real strike,
                      Real forward,
                      Real stdDev,
                      Real discount,
                      Real displacement)
    {
        checkParameters(strike, forward, displacement);
        QL_REQUIRE(stdDev>=0.0,
                   "stdDev (" << stdDev << ") must be non-negative");
        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");

        Integer sign = Integer(optionType);

        if (stdDev == 0.0)
            return std::max((forward-strike) * sign, Real(0.0)) * discount;

        forward = forward + displacement;
        strike = strike + displacement;

        // since displacement is non-negative strike==0 iff displacement==0
        // so returning forward*discount is OK
        if (strike==0.0)
            return (optionType==Option::Call ? forward*discount : 0.0);

        Real d1 = std::log(forward/strike)/stdDev + 0.5*stdDev;
        Real d2 = d1 - stdDev;
        CumulativeNormalDistribution phi;
        Real nd1 = phi(sign * d1);
        Real nd2 = phi(sign * d2);
        Real result = discount * sign * (forward*nd1 - strike*nd2);
        QL_ENSURE(result>=0.0,
                  "negative value (" << result << ") for " <<
                  stdDev << " stdDev, " <<
                  optionType << " option, " <<
                  strike << " strike , " <<
                  forward << " forward");
        return result;
    }

    Real blackFormula(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                      Real forward,
                      Real stdDev,
                      Real discount,
                      Real displacement) {
        return blackFormula(payoff->optionType(),
            payoff->strike(), forward, stdDev, discount, displacement);
    }

    Real blackFormulaForwardDerivative(Option::Type optionType,
                                       Real strike,
                                       Real forward,
                                       Real stdDev,
                                       Real discount,
                                       Real displacement)
    {
        checkParameters(strike, forward, displacement);
        QL_REQUIRE(stdDev>=0.0,
                   "stdDev (" << stdDev << ") must be non-negative");
        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");

        Integer sign = Integer(optionType);

        if (stdDev == 0.0)
            return sign * std::max(1.0 * boost::math::sign((forward - strike) * sign), 0.0) * discount;

        forward = forward + displacement;
        strike = strike + displacement;

        if (strike == 0.0)
            return (optionType == Option::Call ? discount : 0.0);

        Real d1 = std::log(forward/strike)/stdDev + 0.5*stdDev;
        CumulativeNormalDistribution phi;
        return sign * phi(sign * d1) * discount;
    }

    Real blackFormulaForwardDerivative(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                                       Real forward,
                                       Real stdDev,
                                       Real discount,
                                       Real displacement) 
    {
        return blackFormulaForwardDerivative(payoff->optionType(),
            payoff->strike(), forward, stdDev, discount, displacement);
    }

    Real blackFormulaImpliedStdDevApproximation(Option::Type optionType,
                                                Real strike,
                                                Real forward,
                                                Real blackPrice,
                                                Real discount,
                                                Real displacement)
    {
        checkParameters(strike, forward, displacement);
        QL_REQUIRE(blackPrice>=0.0,
                   "blackPrice (" << blackPrice << ") must be non-negative");
        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");

        Real stdDev;
        forward = forward + displacement;
        strike = strike + displacement;
        if (strike==forward)
            // Brenner-Subrahmanyan (1988) and Feinstein (1988) ATM approx.
            stdDev = blackPrice/discount*std::sqrt(2.0 * M_PI)/forward;
        else {
            // Corrado and Miller extended moneyness approximation
            Real moneynessDelta = Integer(optionType) * (forward-strike);
            Real moneynessDelta_2 = moneynessDelta/2.0;
            Real temp = blackPrice/discount - moneynessDelta_2;
            Real moneynessDelta_PI = moneynessDelta*moneynessDelta/M_PI;
            Real temp2 = temp*temp-moneynessDelta_PI;
            if (temp2<0.0) // approximation breaks down, 2 alternatives:
                // 1. zero it
                temp2=0.0;
                // 2. Manaster-Koehler (1982) efficient Newton-Raphson seed
                //return std::fabs(std::log(forward/strike))*std::sqrt(2.0);
            temp2 = std::sqrt(temp2);
            temp += temp2;
            temp *= std::sqrt(2.0 * M_PI);
            stdDev = temp/(forward+strike);
        }
        QL_ENSURE(stdDev>=0.0,
                  "stdDev (" << stdDev << ") must be non-negative");
        return stdDev;
    }

    Real blackFormulaImpliedStdDevApproximation(
                      const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                      Real forward,
                      Real blackPrice,
                      Real discount,
                      Real displacement) {
        return blackFormulaImpliedStdDevApproximation(payoff->optionType(),
            payoff->strike(), forward, blackPrice, discount, displacement);
    }

    Real blackFormulaImpliedStdDevChambers(Option::Type optionType,
                                                Real strike,
                                                Real forward,
                                                Real blackPrice,
                                                Real blackAtmPrice,
                                                Real discount,
                                                Real displacement) {
        checkParameters(strike, forward, displacement);
        QL_REQUIRE(blackPrice >= 0.0,
                   "blackPrice (" << blackPrice << ") must be non-negative");
        QL_REQUIRE(blackAtmPrice >= 0.0, "blackAtmPrice ("
                                             << blackAtmPrice
                                             << ") must be non-negative");
        QL_REQUIRE(discount > 0.0, "discount (" << discount
                                                << ") must be positive");

        Real stdDev;

        forward = forward + displacement;
        strike = strike + displacement;
        blackPrice /= discount;
        blackAtmPrice /= discount;

        Real s0 = M_SQRT2 * M_SQRTPI * blackAtmPrice /
                  forward; // Brenner-Subrahmanyam formula
        Real priceAtmVol =
            blackFormula(optionType, strike, forward, s0, 1.0, 0.0);
        Real dc = blackPrice - priceAtmVol;

        if (close(dc, 0.0)) {
            stdDev = s0;
        } else {
            Real d1 =
                blackFormulaStdDevDerivative(strike, forward, s0, 1.0, 0.0);
            Real d2 = blackFormulaStdDevSecondDerivative(strike, forward, s0,
                                                         1.0, 0.0);
            Real ds = 0.0;
            Real tmp = d1 * d1 + 2.0 * d2 * dc;
            if (std::fabs(d2) > 1E-10 && tmp >= 0.0)
                ds = (-d1 + std::sqrt(tmp)) / d2; // second order approximation
            else
                if(std::fabs(d1) > 1E-10)
                    ds = dc / d1; // first order approximation
            stdDev = s0 + ds;
        }

        QL_ENSURE(stdDev >= 0.0, "stdDev (" << stdDev
                                            << ") must be non-negative");
        return stdDev;
    }

    Real blackFormulaImpliedStdDevChambers(
        const ext::shared_ptr<PlainVanillaPayoff> &payoff,
        Real forward,
        Real blackPrice,
        Real blackAtmPrice,
        Real discount,
        Real displacement) {
        return blackFormulaImpliedStdDevChambers(
            payoff->optionType(), payoff->strike(), forward, blackPrice,
            blackAtmPrice, discount, displacement);
    }

    namespace {
        Real Af(Real x) {
            return 0.5*(1.0+boost::math::sign(x)
                *std::sqrt(1.0-std::exp(-M_2_PI*x*x)));
        }
    }

    Real blackFormulaImpliedStdDevApproximationRS(
        Option::Type type, Real K, Real F,
        Real marketValue, Real df, Real displacement) {

        checkParameters(K, F, displacement);
        QL_REQUIRE(marketValue >= 0.0,
                   "blackPrice (" << marketValue << ") must be non-negative");
        QL_REQUIRE(df > 0.0, "discount (" << df << ") must be positive");

        F = F + displacement;
        K = K + displacement;

        const Real ey = F/K;
        const Real ey2 = ey*ey;
        const Real y = std::log(ey);
        const Real alpha = marketValue/(K*df);
        const Real R = 2*alpha + ((type == Option::Call) ? -ey+1.0 : ey-1.0);
        const Real R2 = R*R;

        const Real a = std::exp((1.0-M_2_PI)*y);
        const Real A = square<Real>()(a - 1.0/a);
        const Real b = std::exp(M_2_PI*y);
        const Real B = 4.0*(b + 1/b)
            - 2*K/F*(a + 1.0/a)*(ey2 + 1 - R2);
        const Real C = (R2-square<Real>()(ey-1))*(square<Real>()(ey+1)-R2)/ey2;

        const Real beta = 2*C/(B+std::sqrt(B*B+4*A*C));
        const Real gamma = -M_PI_2*std::log(beta);

        if (y >= 0.0) {
            const Real M0 = K*df*(
                (type == Option::Call) ? ey*Af(std::sqrt(2*y)) - 0.5
                                       : 0.5-ey*Af(-std::sqrt(2*y)));

            if (marketValue <= M0)
                return std::sqrt(gamma+y)-std::sqrt(gamma-y);
            else
                return std::sqrt(gamma+y)+std::sqrt(gamma-y);
        }
        else {
            const Real M0 = K*df*(
                (type == Option::Call) ? 0.5*ey - Af(-std::sqrt(-2*y))
                                       : Af(std::sqrt(-2*y)) - 0.5*ey);

            if (marketValue <= M0)
                return std::sqrt(gamma-y)-std::sqrt(gamma+y);
            else
                return std::sqrt(gamma+y)+std::sqrt(gamma-y);
        }
    }

    Real blackFormulaImpliedStdDevApproximationRS(
        const ext::shared_ptr<PlainVanillaPayoff> &payoff,
        Real F, Real marketValue,
        Real df, Real displacement) {

        return blackFormulaImpliedStdDevApproximationRS(
            payoff->optionType(), payoff->strike(),
            F, marketValue, df, displacement);
    }

    class BlackImpliedStdDevHelper {
      public:
        BlackImpliedStdDevHelper(Option::Type optionType,
                                 Real strike,
                                 Real forward,
                                 Real undiscountedBlackPrice,
                                 Real displacement = 0.0)
        : halfOptionType_(0.5 * Integer(optionType)),
          signedStrike_(Integer(optionType) * (strike+displacement)),
          signedForward_(Integer(optionType) * (forward+displacement)),
          undiscountedBlackPrice_(undiscountedBlackPrice)
        {
            checkParameters(strike, forward, displacement);
            QL_REQUIRE(undiscountedBlackPrice>=0.0,
                       "undiscounted Black price (" <<
                       undiscountedBlackPrice << ") must be non-negative");
            signedMoneyness_ = Integer(optionType) * std::log((forward+displacement)/(strike+displacement));
        }

        Real operator()(Real stdDev) const {
            #if defined(QL_EXTRA_SAFETY_CHECKS)
            QL_REQUIRE(stdDev>=0.0,
                       "stdDev (" << stdDev << ") must be non-negative");
            #endif
            if (stdDev==0.0)
                return std::max(signedForward_-signedStrike_, Real(0.0))
                                                   - undiscountedBlackPrice_;
            Real temp = halfOptionType_*stdDev;
            Real d = signedMoneyness_/stdDev;
            Real signedD1 = d + temp;
            Real signedD2 = d - temp;
            Real result = signedForward_ * N_(signedD1)
                - signedStrike_ * N_(signedD2);
            // numerical inaccuracies can yield a negative answer
            return std::max(Real(0.0), result) - undiscountedBlackPrice_;
        }
        Real derivative(Real stdDev) const {
            #if defined(QL_EXTRA_SAFETY_CHECKS)
            QL_REQUIRE(stdDev>=0.0,
                       "stdDev (" << stdDev << ") must be non-negative");
            #endif
            Real signedD1 = signedMoneyness_/stdDev + halfOptionType_*stdDev;
            return signedForward_*N_.derivative(signedD1);
        }
      private:
        Real halfOptionType_;
        Real signedStrike_, signedForward_;
        Real undiscountedBlackPrice_, signedMoneyness_;
        CumulativeNormalDistribution N_;
    };


    Real blackFormulaImpliedStdDev(Option::Type optionType,
                                   Real strike,
                                   Real forward,
                                   Real blackPrice,
                                   Real discount,
                                   Real displacement,
                                   Real guess,
                                   Real accuracy,
                                   Natural maxIterations)
    {
        checkParameters(strike, forward, displacement);

        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");

        QL_REQUIRE(blackPrice>=0.0,
                   "option price (" << blackPrice << ") must be non-negative");
        // check the price of the "other" option implied by put-call paity
        Real otherOptionPrice = blackPrice - Integer(optionType) * (forward-strike)*discount;
        QL_REQUIRE(otherOptionPrice>=0.0,
                   "negative " << Option::Type(-1*optionType) <<
                   " price (" << otherOptionPrice <<
                   ") implied by put-call parity. No solution exists for " <<
                   optionType << " strike " << strike <<
                   ", forward " << forward <<
                   ", price " << blackPrice <<
                   ", deflator " << discount);

        // solve for the out-of-the-money option which has
        // greater vega/price ratio, i.e.
        // it is numerically more robust for implied vol calculations
        if (optionType==Option::Put && strike>forward) {
            optionType = Option::Call;
            blackPrice = otherOptionPrice;
        }
        if (optionType==Option::Call && strike<forward) {
            optionType = Option::Put;
            blackPrice = otherOptionPrice;
        }

        strike = strike + displacement;
        forward = forward + displacement;

        if (guess==Null<Real>())
            guess = blackFormulaImpliedStdDevApproximation(
                optionType, strike, forward, blackPrice, discount, displacement);
        else
            QL_REQUIRE(guess>=0.0,
                       "stdDev guess (" << guess << ") must be non-negative");
        BlackImpliedStdDevHelper f(optionType, strike, forward,
                                   blackPrice/discount);
        NewtonSafe solver;
        solver.setMaxEvaluations(maxIterations);
        Real minSdtDev = 0.0, maxStdDev = 24.0; // 24 = 300% * sqrt(60)
        Real stdDev = solver.solve(f, accuracy, guess, minSdtDev, maxStdDev);
        QL_ENSURE(stdDev>=0.0,
                  "stdDev (" << stdDev << ") must be non-negative");
        return stdDev;
    }

    Real blackFormulaImpliedStdDev(
                        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real blackPrice,
                        Real discount,
                        Real displacement,
                        Real guess,
                        Real accuracy,
                        Natural maxIterations) {
        return blackFormulaImpliedStdDev(payoff->optionType(), payoff->strike(),
            forward, blackPrice, discount, displacement, guess, accuracy, maxIterations);
    }


    namespace {
        Real Np(Real x, Real v) {
            return CumulativeNormalDistribution()(x/v + 0.5*v);
        }
        Real Nm(Real x, Real v) {
            return std::exp(-x)*CumulativeNormalDistribution()(x/v - 0.5*v);
        }
        Real phi(Real x, Real v) {
            const Real ax = 2*std::fabs(x);
            const Real v2 = v*v;
            return (v2-ax)/(v2+ax);
        }
        Real F(Real v, Real x, Real cs, Real w) {
            return cs+Nm(x,v)+w*Np(x,v);
        }
        Real G(Real v, Real x, Real cs, Real w) {
            const Real q = F(v,x,cs,w)/(1+w);

            // Acklam's inverse w/o Halley's refinement step
            // does not provide enough accuracy. But both together are
            // slower than the boost replacement.
            const Real k = MaddockInverseCumulativeNormal()(q);

            return k + std::sqrt(k*k + 2*std::fabs(x));
        }
    }

    Real blackFormulaImpliedStdDevLiRS(
        Option::Type optionType,
        Real strike,
        Real forward,
        Real blackPrice,
        Real discount,
        Real displacement,
        Real guess,
        Real w,
        Real accuracy,
        Natural maxIterations) {

        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");

        QL_REQUIRE(blackPrice>=0.0,
                   "option price (" << blackPrice << ") must be non-negative");

        strike = strike + displacement;
        forward = forward + displacement;

        if (guess == Null<Real>()) {
            guess = blackFormulaImpliedStdDevApproximationRS(
                optionType, strike, forward,
                blackPrice, discount, displacement);
        }
        else {
            QL_REQUIRE(guess>=0.0,
                "stdDev guess (" << guess << ") must be non-negative");
        }

        Real x = std::log(forward/strike);
        Real cs = (optionType == Option::Call)
            ? blackPrice / (forward*discount)
            : (blackPrice/ (forward*discount) + 1.0 - strike/forward);

        QL_REQUIRE(cs >= 0.0, "normalized call price (" << cs
                   << ") must be positive");

        if (x > 0) {
            // use in-out duality
            cs = forward/strike*cs + 1.0 - forward/strike;
            QL_REQUIRE(cs >= 0.0, "negative option price from in-out duality");
            x = -x;
        }

        Size nIter = 0;
        Real dv, vk, vkp1 = guess;

        do {
            vk = vkp1;
            const Real alphaK = (1+w)/(1+phi(x,vk));
            vkp1 = alphaK*G(vk,x,cs,w) + (1-alphaK)*vk;
            dv = std::fabs(vkp1 - vk);
        } while (dv > accuracy && ++nIter < maxIterations);

        QL_REQUIRE(dv <= accuracy, "max iterations exceeded");
        QL_REQUIRE(vk >= 0.0, "stdDev (" << vk << ") must be non-negative");

        return vk;
    }

    Real blackFormulaImpliedStdDevLiRS(
        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
        Real forward,
        Real blackPrice,
        Real discount,
        Real displacement,
        Real guess,
        Real omega,
        Real accuracy,
        Natural maxIterations) {

        return blackFormulaImpliedStdDevLiRS(
            payoff->optionType(), payoff->strike(),
            forward, blackPrice, discount, displacement,
            guess, omega, accuracy, maxIterations);
    }


    Real blackFormulaCashItmProbability(Option::Type optionType,
                                        Real strike,
                                        Real forward,
                                        Real stdDev,
                                        Real displacement) {
        checkParameters(strike, forward, displacement);

        Integer sign = Integer(optionType);

        if (stdDev==0.0)
            return (forward * sign > strike * sign ? 1.0 : 0.0);

        forward = forward + displacement;
        strike = strike + displacement;
        if (strike==0.0)
            return (optionType == Option::Call ? 1.0 : 0.0);
        Real d2 = std::log(forward/strike)/stdDev - 0.5*stdDev;
        CumulativeNormalDistribution phi;
        return phi(sign * d2);
    }

    Real blackFormulaCashItmProbability(
                        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real displacement) {
        return blackFormulaCashItmProbability(payoff->optionType(),
            payoff->strike(), forward, stdDev , displacement);
    }

    Real blackFormulaAssetItmProbability(
                        Option::Type optionType,
                        Real strike,
                        Real forward,
                        Real stdDev,
                        Real displacement) {
        checkParameters(strike, forward, displacement);

        Integer sign = Integer(optionType);

        if (stdDev==0.0)
            return (forward * sign < strike * sign ? 1.0 : 0.0);

        forward = forward + displacement;
        strike = strike + displacement;
        if (strike == 0.0)
            return (optionType == Option::Call ? 1.0 : 0.0);
        Real d1 = std::log(forward/strike)/stdDev + 0.5*stdDev;
        CumulativeNormalDistribution phi;
        return phi(sign * d1);
    }

    Real blackFormulaAssetItmProbability(
                        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real displacement) {
        return blackFormulaAssetItmProbability(payoff->optionType(),
            payoff->strike(), forward, stdDev , displacement);
    }

    Real blackFormulaVolDerivative(Rate strike,
                                      Rate forward,
                                      Real stdDev,
                                      Real expiry,
                                      Real discount,
                                      Real displacement)
    {
        return  blackFormulaStdDevDerivative(strike,
                                     forward,
                                     stdDev,
                                     discount,
                                     displacement)*std::sqrt(expiry);
    }

    Real blackFormulaStdDevDerivative(Rate strike,
                                      Rate forward,
                                      Real stdDev,
                                      Real discount,
                                      Real displacement)
    {
        checkParameters(strike, forward, displacement);
        QL_REQUIRE(stdDev>=0.0,
                   "stdDev (" << stdDev << ") must be non-negative");
        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");

        forward = forward + displacement;
        strike = strike + displacement;

        if (stdDev==0.0 || strike==0.0)
            return 0.0;

        Real d1 = std::log(forward/strike)/stdDev + .5*stdDev;
        return discount * forward *
            CumulativeNormalDistribution().derivative(d1);
    }

    Real blackFormulaStdDevDerivative(
                        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real discount,
                        Real displacement) {
        return blackFormulaStdDevDerivative(payoff->strike(), forward,
                                     stdDev, discount, displacement);
    }

    Real blackFormulaStdDevSecondDerivative(Rate strike,
                                            Rate forward,
                                            Real stdDev,
                                            Real discount,
                                            Real displacement)
    {
        checkParameters(strike, forward, displacement);
        QL_REQUIRE(stdDev>=0.0,
                   "stdDev (" << stdDev << ") must be non-negative");
        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");

        forward = forward + displacement;
        strike = strike + displacement;

        if (stdDev==0.0 || strike==0.0)
            return 0.0;

        Real d1 = std::log(forward/strike)/stdDev + .5*stdDev;
        Real d1p = -std::log(forward/strike)/(stdDev*stdDev) + .5;
        return discount * forward *
            NormalDistribution().derivative(d1) * d1p;
    }

    Real blackFormulaStdDevSecondDerivative(
                        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real discount,
                        Real displacement) {
        return blackFormulaStdDevSecondDerivative(payoff->strike(), forward,
                                     stdDev, discount, displacement);
    }

    Real bachelierBlackFormula(Option::Type optionType,
                               Real strike,
                               Real forward,
                               Real stdDev,
                               Real discount)
    {
        QL_REQUIRE(stdDev>=0.0,
                   "stdDev (" << stdDev << ") must be non-negative");
        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");
        Real d = (forward-strike) * Integer(optionType), h = d / stdDev;
        if (stdDev==0.0)
            return discount*std::max(d, 0.0);
        CumulativeNormalDistribution phi;
        Real result = discount*(stdDev*phi.derivative(h) + d*phi(h));
        QL_ENSURE(result>=0.0,
                  "negative value (" << result << ") for " <<
                  stdDev << " stdDev, " <<
                  optionType << " option, " <<
                  strike << " strike , " <<
                  forward << " forward");
        return result;
    }

    Real bachelierBlackFormula(
                        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real discount) {
        return bachelierBlackFormula(payoff->optionType(),
            payoff->strike(), forward, stdDev, discount);
    }

    Real bachelierBlackFormulaForwardDerivative(
        Option::Type optionType, Real strike, Real forward, Real stdDev, Real discount)
    {
        QL_REQUIRE(stdDev>=0.0,
                   "stdDev (" << stdDev << ") must be non-negative");
        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");
        Integer sign = Integer(optionType);
        if (stdDev == 0.0)
            return sign * std::max(1.0 * boost::math::sign((forward - strike) * sign), 0.0) * discount;
        Real d = (forward - strike) * sign, h = d / stdDev;
        CumulativeNormalDistribution phi;
        return sign * phi(h) * discount;
    }

    Real bachelierBlackFormulaForwardDerivative(
        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
        Real forward,
        Real stdDev,
        Real discount)
    {
        return bachelierBlackFormulaForwardDerivative(payoff->optionType(),
            payoff->strike(), forward, stdDev, discount);
    }

    static Real h(Real eta) {

        const static Real  A0          = 3.994961687345134e-1;
        const static Real  A1          = 2.100960795068497e+1;
        const static Real  A2          = 4.980340217855084e+1;
        const static Real  A3          = 5.988761102690991e+2;
        const static Real  A4          = 1.848489695437094e+3;
        const static Real  A5          = 6.106322407867059e+3;
        const static Real  A6          = 2.493415285349361e+4;
        const static Real  A7          = 1.266458051348246e+4;

        const static Real  B0          = 1.000000000000000e+0;
        const static Real  B1          = 4.990534153589422e+1;
        const static Real  B2          = 3.093573936743112e+1;
        const static Real  B3          = 1.495105008310999e+3;
        const static Real  B4          = 1.323614537899738e+3;
        const static Real  B5          = 1.598919697679745e+4;
        const static Real  B6          = 2.392008891720782e+4;
        const static Real  B7          = 3.608817108375034e+3;
        const static Real  B8          = -2.067719486400926e+2;
        const static Real  B9          = 1.174240599306013e+1;

        QL_REQUIRE(eta>=0.0,
                       "eta (" << eta << ") must be non-negative");

        const Real num = A0 + eta * (A1 + eta * (A2 + eta * (A3 + eta * (A4 + eta
                    * (A5 + eta * (A6 + eta * A7))))));

        const Real den = B0 + eta * (B1 + eta * (B2 + eta * (B3 + eta * (B4 + eta
                    * (B5 + eta * (B6 + eta * (B7 + eta * (B8 + eta * B9))))))));

        return std::sqrt(eta) * (num / den);

    }

    Real bachelierBlackFormulaImpliedVol(Option::Type optionType,
                                   Real strike,
                                   Real forward,
                                   Real tte,
                                   Real bachelierPrice,
                                   Real discount) {

        const static Real SQRT_QL_EPSILON = std::sqrt(QL_EPSILON);

        QL_REQUIRE(tte>0.0,
                   "tte (" << tte << ") must be positive");

        Real forwardPremium = bachelierPrice/discount;

        Real straddlePremium;
        if (optionType==Option::Call){
            straddlePremium = 2.0 * forwardPremium - (forward - strike);
        } else {
            straddlePremium = 2.0 * forwardPremium + (forward - strike);
        }

        Real nu = (forward - strike) / straddlePremium;
        QL_REQUIRE(nu<1.0 || close_enough(nu,1.0),
                   "nu (" << nu << ") must be <= 1.0");
        QL_REQUIRE(nu>-1.0 || close_enough(nu,-1.0),
                     "nu (" << nu << ") must be >= -1.0");

        nu = std::max(-1.0 + QL_EPSILON, std::min(nu,1.0 - QL_EPSILON));

        // nu / arctanh(nu) -> 1 as nu -> 0
        Real eta = (std::fabs(nu) < SQRT_QL_EPSILON) ? 1.0 : nu / boost::math::atanh(nu);

        Real heta = h(eta);

        Real impliedBpvol = std::sqrt(M_PI / (2 * tte)) * straddlePremium * heta;

        return impliedBpvol;
    }


        Real bachelierBlackFormulaStdDevDerivative(Rate strike,
                                      Rate forward,
                                      Real stdDev,
                                      Real discount)
    {
        QL_REQUIRE(stdDev>=0.0,
                   "stdDev (" << stdDev << ") must be non-negative");
        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");

        if (stdDev==0.0)
            return 0.0;

        Real d1 = (forward - strike)/stdDev;
        return discount *
            CumulativeNormalDistribution().derivative(d1);
    }

    Real bachelierBlackFormulaStdDevDerivative(
                        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real discount) {
        return bachelierBlackFormulaStdDevDerivative(payoff->strike(), forward,
                                     stdDev, discount);
    }

    Real bachelierBlackFormulaAssetItmProbability(
                        Option::Type optionType,
                        Real strike,
                        Real forward,
                        Real stdDev) {
        QL_REQUIRE(stdDev>=0.0,
                   "stdDev (" << stdDev << ") must be non-negative");
        Real d = (forward - strike) * Integer(optionType), h = d / stdDev;
        if (stdDev==0.0)
            return std::max(d, 0.0);
        CumulativeNormalDistribution phi;
        Real result = phi(h);
        return result;
    }

    Real bachelierBlackFormulaAssetItmProbability(
                        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev) {
        return bachelierBlackFormulaAssetItmProbability(payoff->optionType(),
            payoff->strike(), forward, stdDev);
    }
}
