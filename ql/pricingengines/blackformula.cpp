/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 Chiara Fornarola

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
#include <ql/math/solvers1d/newtonsafe.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace {
    void checkParameters(QuantLib::Real strike,
                         QuantLib::Real forward,
                         QuantLib::Real displacement)
    {
        QL_REQUIRE(strike>=0.0,
                   "strike (" << strike << ") must be non-negative");
        QL_REQUIRE(forward>0.0,
                   "forward (" << forward << ") must be positive");
        QL_REQUIRE(displacement>=0.0,
                   "displacement (" << displacement << ") must be non-negative");
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

        if (stdDev==0.0)
            return std::max((forward-strike)*optionType, Real(0.0))*discount;

        forward = forward + displacement;
        strike = strike + displacement;

        // since displacement is non-negative strike==0 iff displacement==0
        // so returning forward*discount is OK
        if (strike==0.0)
            return (optionType==Option::Call ? forward*discount : 0.0);

        Real d1 = std::log(forward/strike)/stdDev + 0.5*stdDev;
        Real d2 = d1 - stdDev;
        CumulativeNormalDistribution phi;
        Real nd1 = phi(optionType*d1);
        Real nd2 = phi(optionType*d2);
        Real result = discount * optionType * (forward*nd1 - strike*nd2);
        QL_ENSURE(result>=0.0,
                  "negative value (" << result << ") for " <<
                  stdDev << " stdDev, " <<
                  optionType << " option, " <<
                  strike << " strike , " <<
                  forward << " forward");
        return result;
    }

    Real blackFormula(const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                      Real forward,
                      Real stdDev,
                      Real discount,
                      Real displacement) {
        return blackFormula(payoff->optionType(),
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
            Real moneynessDelta = optionType*(forward-strike);
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
                      const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                      Real forward,
                      Real blackPrice,
                      Real discount,
                      Real displacement) {
        return blackFormulaImpliedStdDevApproximation(payoff->optionType(),
            payoff->strike(), forward, blackPrice, discount, displacement);
    }


    class BlackImpliedStdDevHelper {
      public:
        BlackImpliedStdDevHelper(Option::Type optionType,
                                 Real strike,
                                 Real forward,
                                 Real undiscountedBlackPrice,
                                 Real displacement = 0.0)
        : halfOptionType_(0.5*optionType), signedStrike_(optionType*(strike+displacement)),
          signedForward_(optionType*(forward+displacement)),
          undiscountedBlackPrice_(undiscountedBlackPrice)
        {
            checkParameters(strike, forward, displacement);
            QL_REQUIRE(undiscountedBlackPrice>=0.0,
                       "undiscounted Black price (" <<
                       undiscountedBlackPrice << ") must be non-negative");
            signedMoneyness_ = optionType*std::log((forward+displacement)/(strike+displacement));
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
        QL_REQUIRE(blackPrice>=0.0,
                   "blackPrice (" << blackPrice << ") must be non-negative");
        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");

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
        Real minSdtDev = 0.0, maxStdDev = 3.0;
        Real stdDev = solver.solve(f, accuracy, guess, minSdtDev, maxStdDev);
        QL_ENSURE(stdDev>=0.0,
                  "stdDev (" << stdDev << ") must be non-negative");
        return stdDev;
    }

    Real blackFormulaImpliedStdDev(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
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

    Real blackFormulaCashItmProbability(Option::Type optionType,
                                        Real strike,
                                        Real forward,
                                        Real stdDev,
                                        Real displacement) {
        checkParameters(strike, forward, displacement);
        if (stdDev==0.0)
            return (forward*optionType > strike*optionType ? 1.0 : 0.0);

        forward = forward + displacement;
        strike = strike + displacement;
        if (strike==0.0)
            return (optionType==Option::Call ? 1.0 : 0.0);
        Real d2 = std::log(forward/strike)/stdDev - 0.5*stdDev;
        CumulativeNormalDistribution phi;
        return phi(optionType*d2);
    }


    Real blackFormulaCashItmProbability(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real displacement) {
        return blackFormulaCashItmProbability(payoff->optionType(),
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

        if (stdDev==0.0) {
            if (forward>strike)
                return discount * forward;
            else
                return 0.0;
        }

        Real d1 = std::log(forward/strike)/stdDev + .5*stdDev;
        return discount * forward *
            CumulativeNormalDistribution().derivative(d1);
    }

    Real blackFormulaStdDevDerivative(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real discount,
                        Real displacement) {
        return blackFormulaStdDevDerivative(payoff->strike(), forward,
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
        Real d = (forward-strike)*optionType, h = d/stdDev;
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
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real discount) {
        return bachelierBlackFormula(payoff->optionType(),
            payoff->strike(), forward, stdDev, discount);
    }

}
