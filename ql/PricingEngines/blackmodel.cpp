/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano

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


#include <ql/PricingEngines/blackmodel.hpp>
#include <ql/Solvers1D/newtonsafe.hpp>

namespace QuantLib {

    Real blackFormula(Option::Type optionType,
                      Real strike,
                      Real forward,
                      Real stdDev) {
        QL_REQUIRE(strike>=0.0,
                   "strike (" << strike << ") must be non-negative");
        QL_REQUIRE(forward>0.0,
                   "forward (" << forward << ") must be positive");
        QL_REQUIRE(stdDev>=0.0,
                   "stdDev (" << stdDev << ") must be non-negative");
        if (stdDev==0.0)
            return std::max((forward-strike)*optionType, Real(0.0));
        if (strike==0.0)
            return (optionType==Option::Call ? forward : 0.0);
        Real d1 = std::log(forward/strike)/stdDev + 0.5*stdDev;
        Real d2 = d1 - stdDev;
        CumulativeNormalDistribution phi;
        Real result = optionType *
            (forward*phi(optionType*d1) - strike*phi(optionType*d2));
        QL_ENSURE(result>=0.0,
                  "negative value (" << result << ") for a " << stdDev <<
                  " stdDev " << optionType << " option struck at " <<
                  strike << " on a " << forward << " forward");
        return result;
    }

    class BlackImpliedVolHelper {
      public:
        BlackImpliedVolHelper(Option::Type optionType,
                              Real strike,
                              Real forward,
                              Real blackPrice)
        : halfOptionType_(0.5*optionType), signedStrike_(optionType*strike),
          signedForward_(optionType*forward), blackPrice_(blackPrice)
        {
            QL_REQUIRE(strike>=0.0,
                    "strike (" << strike << ") must be non-negative");
            QL_REQUIRE(forward>0.0,
                    "forward (" << forward << ") must be positive");
            QL_REQUIRE(blackPrice>=0.0,
                    "blackPrice (" << blackPrice << ") must be non-negative");
            signedMoneyness_ = optionType*std::log(forward/strike);
        }
        Real operator()(Real stdDev) const {
            #ifdef QL_DEBUG
            QL_REQUIRE(stdDev>=0.0,
                    "stdDev (" << stdDev << ") must be non-negative");
            #endif
            if (stdDev==0.0)
                return std::max(signedForward_-signedStrike_, Real(0.0))
                                                               - blackPrice_;
            Real temp = halfOptionType_*stdDev;
            Real d = signedMoneyness_/stdDev;
            Real signedD1 = d + temp;
            Real signedD2 = d - temp;
            Real result = signedForward_ * N_(signedD1)
                - signedStrike_ * N_(signedD2);
            // numerical inaccuracies can yield a negative answer
            return std::max(Real(0.0), result) - blackPrice_;
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
        Real signedStrike_, signedForward_, blackPrice_, signedMoneyness_;
        CumulativeNormalDistribution N_;
    };

    Real blackImpliedStdDevApproximation(Option::Type optionType,
                                         Real strike,
                                         Real forward,
                                         Real blackPrice) {
        QL_REQUIRE(strike>=0.0,
                "strike (" << strike << ") must be non-negative");
        QL_REQUIRE(forward>0.0,
                "forward (" << forward << ") must be positive");
        QL_REQUIRE(blackPrice>=0.0,
                "blackPrice (" << blackPrice << ") must be non-negative");
        Real stdDev;
        if (strike==forward)
            // Brenner-Subrahmanyan (1988) and Feinstein (1988) ATM approx.
            stdDev = blackPrice*std::sqrt(2.0 * M_PI)/forward;
        else {
            // Corrado and Miller extended moneyness approximation
            Real moneynessDelta = optionType*(forward-strike);
            Real moneynessDelta_2 = moneynessDelta/2.0;
            Real temp = blackPrice - moneynessDelta_2;
            Real moneynessDelta_PI = moneynessDelta*moneynessDelta/M_PI;
            Real temp2 = temp*temp-moneynessDelta_PI;
            if (temp2<0.0) // approximation breaks down
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

    Real blackImpliedStdDev(Option::Type optionType,
                            Real strike,
                            Real forward,
                            Real blackPrice,
                            Real guess,
                            Real accuracy) {
        QL_REQUIRE(strike>=0.0,
                "strike (" << strike << ") must be non-negative");
        QL_REQUIRE(forward>0.0,
                "forward (" << forward << ") must be positive");
        QL_REQUIRE(blackPrice>=0.0,
                "blackPrice (" << blackPrice << ") must be non-negative");
        if (guess==Null<Real>())
            guess = blackImpliedStdDevApproximation(optionType, strike,
                                                    forward, blackPrice);
        else
            QL_REQUIRE(guess>=0.0,
                       "stdDev guess (" << guess << ") must be non-negative");
        BlackImpliedVolHelper f(optionType, strike, forward, blackPrice);
        //Brent solver;
        //Newton solver;
        NewtonSafe solver;
        solver.setMaxEvaluations(100);
        Real minSdtDev = 0.0, maxStdDev = 3.0;
        Real stdDev = solver.solve(f, accuracy, guess, minSdtDev, maxStdDev);
        QL_ENSURE(stdDev>=0.0,
                  "stdDev (" << stdDev << ") must be non-negative");
        return stdDev;
    }
}
