
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2002, 2003 Sad Rejeb
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file analyticeuropeanbinaryengine.cpp
    \brief European binary option engine
*/

#include <ql/PricingEngines/binaryengines.hpp>

namespace QuantLib {

    #if !defined(QL_PATCH_SOLARIS)
    const CumulativeNormalDistribution AnalyticEuropeanBinaryEngine::f_;
     #endif

    void AnalyticEuropeanBinaryEngine::calculate() const {

        QL_REQUIRE(arguments_.exerciseType == Exercise::European,
                   "AnalyticBinaryEngine::calculate() : "
                   "not an European Option");

        Option::Type type = 
            Handle<PlainVanillaPayoff>(arguments_.payoff)->optionType();
        double payoff = arguments_.cashPayoff;

        double u = arguments_.underlying;
        double k = arguments_.barrier;

        Time T = arguments_.maturity;

        double vol = arguments_.volTS->blackVol(T, k);

        DiscountFactor discount =
            arguments_.riskFreeTS->discount(T);
        Rate r = arguments_.riskFreeTS->zeroYield(T);
        Rate q = arguments_.dividendTS->zeroYield(T);

        double volSqrtT = vol * QL_SQRT(T);

        double D1 = QL_LOG(u/k) / volSqrtT
            + (r - q) * T / volSqrtT
            + volSqrtT / 2.0;
        double D2 = D1 - volSqrtT;

        double ND2 = f_(D2);

        double sign, beta, NID2;
        double inTheMoneyProbability;
        switch (type) {
          case Option::Call:
            sign = 1.0;
            beta = ND2;
            NID2 = f_.derivative(D2);
            inTheMoneyProbability = ND2;
            break;
          case Option::Put:
            sign = -1.0;
            beta = ND2 - 1.0;
            NID2 = f_.derivative(D2);
            inTheMoneyProbability = 1.0 - ND2;
            break;
          case Option::Straddle:
            sign = 0.0;
            beta = 2.0 * ND2 - 1.0;
            NID2 = 2.0 * f_.derivative(D2);
            inTheMoneyProbability = 1.0;
            break;
          default:
            throw Error("AnalyticEuropeanBinaryEngine: "
                        "invalid option type");
        }

        results_.value = payoff * discount * inTheMoneyProbability;

        results_.delta = sign * payoff * discount * NID2/(u*volSqrtT);

        results_.gamma = - payoff * discount * sign * NID2 *
            (1.0 + D2/volSqrtT) / (u*u*volSqrtT);

        if (type == Option::Straddle) {
            results_.theta = payoff * discount * r;
        } else {
            double temp = (- QL_LOG(u/k) / volSqrtT 
                           + (r-q)*T/volSqrtT
                           - volSqrtT/2.0) / (2.0*T);
            results_.theta = - payoff * discount * sign *
                ( temp * NID2 - r * beta);
        }

        if (type == Option::Straddle) {
            results_.rho = - payoff * T * discount;
        } else {
            double temp = T/volSqrtT;
            results_.rho = payoff * discount * sign * (temp*NID2-T*beta);
        }

        if (type == Option::Straddle) {
            results_.dividendRho = 0.0;
        } else {
            double temp = T/volSqrtT;
            results_.dividendRho = -payoff * discount * sign * temp * NID2;
        }

        results_.vega = - sign * payoff * discount * NID2 * D1 / vol;
    }

}

