
/*
 Copyright (C) 2003, 2004 Neil Firth
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003, 2004 StatPro Italia srl

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

#include <ql/PricingEngines/Barrier/analyticeuropeanbinarybarrierengine.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    void AnalyticEuropeanBinaryBarrierEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "AnalyticBinaryBarrierEngine::calculate() : "
                   "not an European Option");

        #if defined(HAVE_BOOST)
        Handle<CashOrNothingPayoff> payoff = 
            boost::dynamic_pointer_cast<CashOrNothingPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "AnalyticEuropeanBinaryBarrierEngine: wrong payoff given");
        #else
        Handle<CashOrNothingPayoff> payoff = arguments_.payoff;
        #endif

        Option::Type type = payoff->optionType();
        double cashPayoff = payoff->cashPayoff();

        double u = arguments_.blackScholesProcess->stateVariable->value();
        double k = payoff->strike();


        double vol = arguments_.blackScholesProcess->volTS->blackVol(
            arguments_.exercise->lastDate(), k);

        DiscountFactor discount = 
            arguments_.blackScholesProcess->riskFreeTS->discount(
                arguments_.exercise->lastDate());
        Rate r = arguments_.blackScholesProcess->riskFreeTS->zeroYield(
            arguments_.exercise->lastDate());
        Rate q = arguments_.blackScholesProcess->dividendTS->zeroYield(
            arguments_.exercise->lastDate());

        double volSqrtT = 
            QL_SQRT(arguments_.blackScholesProcess->volTS->blackVariance(
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
            QL_FAIL("AnalyticEuropeanBinaryBarrierEngine: "
                    "invalid option type");
        }

        results_.value = cashPayoff * discount * inTheMoneyProbability;

        results_.delta = sign * cashPayoff * discount * NID2/(u*volSqrtT);

        results_.gamma = - cashPayoff * discount * sign * NID2 *
            (1.0 + D2/volSqrtT) / (u*u*volSqrtT);

        if (type == Option::Straddle) {
            results_.theta = cashPayoff * discount * r;
        } else {
            double temp = (- QL_LOG(u/k) / volSqrtT 
                           + (r-q)*T/volSqrtT
                           - volSqrtT/2.0) / (2.0*T);
            results_.theta = - cashPayoff * discount * sign *
                ( temp * NID2 - r * beta);
        }

        if (type == Option::Straddle) {
            results_.rho = - cashPayoff * T * discount;
        } else {
            results_.rho = 
                cashPayoff * discount * sign * T* (NID2/volSqrtT-beta);
        }

        if (type == Option::Straddle) {
            results_.dividendRho = 0.0;
        } else {
            double temp = T/volSqrtT;
            results_.dividendRho = -cashPayoff * discount * sign * temp * NID2;
        }

        results_.vega = - sign * cashPayoff * discount * NID2 * D1 / vol;
    }

}

