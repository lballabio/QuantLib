/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2005, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
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

#include <ql/PricingEngines/blackformula.hpp>
#include <ql/Solvers1D/newtonsafe.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    Real blackFormula(Option::Type optionType,
                      Real strike,
                      Real forward,
                      Real stdDev,
                      Real discount) {
        QL_REQUIRE(strike>=0.0,
                   "strike (" << strike << ") must be non-negative");
        QL_REQUIRE(forward>0.0,
                   "forward (" << forward << ") must be positive");
        QL_REQUIRE(stdDev>=0.0,
                   "stdDev (" << stdDev << ") must be non-negative");
        QL_REQUIRE(discount>0.0, "positive discount required: " <<
                   discount << " not allowed");
        if (stdDev==0.0)
            return std::max((forward-strike)*optionType, Real(0.0))*discount;
        if (strike==0.0)
            return (optionType==Option::Call ? forward*discount : 0.0);
        Real d1 = std::log(forward/strike)/stdDev + 0.5*stdDev;
        Real d2 = d1 - stdDev;
        CumulativeNormalDistribution phi;
        Real result = discount * optionType *
            (forward*phi(optionType*d1) - strike*phi(optionType*d2));
        QL_ENSURE(result>=0.0,
                  "negative value (" << result << ") for a " << stdDev <<
                  " stdDev " << optionType << " option struck at " <<
                  strike << " on a " << forward << " forward");
        return result;
    }

    Real blackFormula(const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                      Real forward,
                      Real stdDev,
                      Real discount) {
        return blackFormula(payoff->optionType(),
            payoff->strike(), forward, stdDev, discount);
    }


    Real blackImpliedStdDevApproximation(Option::Type optionType,
                                         Real strike,
                                         Real forward,
                                         Real blackPrice,
                                         Real discount) {
        QL_REQUIRE(strike>=0.0,
                "strike (" << strike << ") must be non-negative");
        QL_REQUIRE(forward>0.0,
                "forward (" << forward << ") must be positive");
        QL_REQUIRE(blackPrice>=0.0,
                "blackPrice (" << blackPrice << ") must be non-negative");
        QL_REQUIRE(discount>0.0, "positive discount required: " <<
                   discount << " not allowed");
        Real stdDev;
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

    Real blackImpliedStdDevApproximation(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real blackPrice,
                        Real) {
        return blackImpliedStdDevApproximation(payoff->optionType(),
            payoff->strike(), forward, blackPrice);
    }


    class BlackImpliedStdDevHelper {
      public:
        BlackImpliedStdDevHelper(Option::Type optionType,
                                 Real strike,
                                 Real forward,
                                 Real undiscountedBlackPrice)
        : halfOptionType_(0.5*optionType), signedStrike_(optionType*strike),
          signedForward_(optionType*forward),
          undiscountedBlackPrice_(undiscountedBlackPrice)
        {
            QL_REQUIRE(strike>=0.0,
                       "strike (" << strike << ") must be non-negative");
            QL_REQUIRE(forward>0.0,
                       "forward (" << forward << ") must be positive");
            QL_REQUIRE(undiscountedBlackPrice>=0.0,
                       "undiscounted Black price (" <<
                       undiscountedBlackPrice << ") must be non-negative");
            signedMoneyness_ = optionType*std::log(forward/strike);
        }
        Real operator()(Real stdDev) const {
            #ifdef QL_DEBUG
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
            #ifdef QL_DEBUG
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

    Real blackImpliedStdDev(Option::Type optionType,
                            Real strike,
                            Real forward,
                            Real blackPrice,
                            Real discount,
                            Real guess,
                            Real accuracy) {
        QL_REQUIRE(strike>=0.0,
                "strike (" << strike << ") must be non-negative");
        QL_REQUIRE(forward>0.0,
                "forward (" << forward << ") must be positive");
        QL_REQUIRE(blackPrice>=0.0,
                "blackPrice (" << blackPrice << ") must be non-negative");
        QL_REQUIRE(discount>0.0, "positive discount required: " <<
                   discount << " not allowed");
        if (guess==Null<Real>())
            guess = blackImpliedStdDevApproximation(
                optionType, strike, forward, blackPrice, discount);
        else
            QL_REQUIRE(guess>=0.0,
                       "stdDev guess (" << guess << ") must be non-negative");
        BlackImpliedStdDevHelper f(optionType, strike, forward,
                                   blackPrice/discount);
        //Brent solver;
        NewtonSafe solver;
        solver.setMaxEvaluations(100);
        Real minSdtDev = 0.0, maxStdDev = 3.0;
        Real stdDev = solver.solve(f, accuracy, guess, minSdtDev, maxStdDev);
        QL_ENSURE(stdDev>=0.0,
                  "stdDev (" << stdDev << ") must be non-negative");
        return stdDev;
    }

    Real blackImpliedStdDev(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real blackPrice,
                        Real discount,
                        Real guess,
                        Real accuracy) {
        return blackImpliedStdDev(payoff->optionType(), payoff->strike(),
            forward, blackPrice, discount, guess, accuracy);
    }


    Real blackCashItmProbability(Option::Type optionType,
                                 Real strike,
                                 Real forward,
                                 Real stdDev) {
        if (stdDev==0.0)
            return (forward*optionType > strike*optionType ? 1.0 : 0.0);
        if (strike==0.0)
            return (optionType==Option::Call ? 1.0 : 0.0);
        Real d1 = std::log(forward/strike)/stdDev + 0.5*stdDev;
        Real d2 = d1 - stdDev;
        CumulativeNormalDistribution phi;
        return phi(optionType*d2);
    }

    Real blackCashItmProbability(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev) {
        return blackCashItmProbability(payoff->optionType(),
            payoff->strike(), forward, stdDev);
    }

    Real blackStdDevDerivative(Rate strike, Rate forward, Real stdDev,
                               Real discount) {
        QL_REQUIRE(strike>=0.0,
                   "strike (" << strike << ") must be non-negative");
        QL_REQUIRE(forward>0.0,
                   "forward (" << forward << ") must be positive");
        QL_REQUIRE(stdDev>=0.0,
                   "stdDev (" << stdDev << ") must be non-negative");
        QL_REQUIRE(discount>0.0, "positive discount required: " <<
                   discount << " not allowed");
        Real d1 = std::log(forward/strike)/stdDev + .5*stdDev;
        return discount * forward *
            CumulativeNormalDistribution().derivative(d1);
    }

    Real blackStdDevDerivative(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real discount) {
        return blackStdDevDerivative(payoff->strike(), forward,
                                     stdDev, discount);
    }

    /* Bachelier model */
    Real bachelierBlackFormula(Option::Type optionType,
                               Real strike,
                               Real forward,
                               Real stdDev,
                               Real discount) {
        QL_REQUIRE(stdDev>=0.0,
                   "stdDev (" << stdDev << ") must be non-negative");
        QL_REQUIRE(discount>0.0, "positive discount required: " <<
                   discount << " not allowed");
        Real d = (forward-strike)*optionType, h = d/stdDev;
        if (stdDev==0.0)
            return discount*std::max(d, 0.0);
        CumulativeNormalDistribution phi;
        Real result = discount*stdDev*phi.derivative(h) + d*phi(h);
        QL_ENSURE(result>=0.0,
                  "negative value (" << result << ") for a " << stdDev <<
                  " stdDev " << optionType << " option struck at " <<
                  strike << " on a " << forward << " forward "
                  "(Bachelier model)");
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
