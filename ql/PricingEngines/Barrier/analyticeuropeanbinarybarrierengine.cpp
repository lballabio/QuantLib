
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

/*! \file analyticeuropeanbinarybarrierengine.cpp
    \brief European binary barrier option engine
*/

#include <ql/PricingEngines/Barrier/binarybarrierengines.hpp>

namespace QuantLib {

    void AnalyticEuropeanBinaryBarrierEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "AnalyticBinaryBarrierEngine::calculate() : "
                   "not an European Option");

        #if defined(HAVE_BOOST)
        Handle<PlainVanillaPayoff> plainPayoff = 
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(plainPayoff,
                   "AnalyticEuropeanBinaryBarrierEngine: non-plain payoff given");
        #else
        Handle<PlainVanillaPayoff> plainPayoff = arguments_.payoff;
        #endif

        Option::Type type = plainPayoff->optionType();
        double payoff = arguments_.cashPayoff;

        double u = arguments_.underlying;
        double k = arguments_.barrier;

        double vol = arguments_.volTS->blackVol(
            arguments_.exercise->lastDate(), k);

        DiscountFactor discount = arguments_.riskFreeTS->discount(
            arguments_.exercise->lastDate());
        Rate r = arguments_.riskFreeTS->zeroYield(
            arguments_.exercise->lastDate());
        Rate q = arguments_.dividendTS->zeroYield(
            arguments_.exercise->lastDate());

        double volSqrtT = QL_SQRT(arguments_.volTS->blackVariance(
            arguments_.exercise->lastDate(), k));

        double T = volSqrtT*volSqrtT/(vol*vol);

        CumulativeNormalDistribution f;

        double D1 = QL_LOG(u/k) / volSqrtT
            + (r - q) * T / volSqrtT
            + volSqrtT / 2.0;
        double D2 = D1 - volSqrtT;

        double ND2 = f(D2);

        double sign, beta, NID2;
        double inTheMoneyProbability;
        switch (type) {
          case Option::Call:
            sign = 1.0;
            beta = ND2;
            NID2 = f.derivative(D2);
            inTheMoneyProbability = ND2;
            break;
          case Option::Put:
            sign = -1.0;
            beta = ND2 - 1.0;
            NID2 = f.derivative(D2);
            inTheMoneyProbability = 1.0 - ND2;
            break;
          case Option::Straddle:
            sign = 0.0;
            beta = 2.0 * ND2 - 1.0;
            NID2 = 2.0 * f.derivative(D2);
            inTheMoneyProbability = 1.0;
            break;
          default:
            throw Error("AnalyticEuropeanBinaryBarrierEngine: "
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

