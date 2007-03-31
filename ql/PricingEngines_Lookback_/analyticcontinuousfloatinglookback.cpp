/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Warren Chou

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

#include <ql/PricingEngines/Lookback/analyticcontinuousfloatinglookback.hpp>
#include <ql/Processes/blackscholesprocess.hpp>

namespace QuantLib {

    void AnalyticContinuousFloatingLookbackEngine::calculate() const {

        boost::shared_ptr<FloatingTypePayoff> payoff =
            boost::dynamic_pointer_cast<FloatingTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "Non-floating payoff given");

        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");

        switch (payoff->optionType()) {
          case Option::Call:
            results_.value = A(1);
            break;
          case Option::Put:
            results_.value = A(-1);
            break;
          default:
            QL_FAIL("Unknown type");
        }
    }

    Real AnalyticContinuousFloatingLookbackEngine::underlying() const {
        return arguments_.stochasticProcess->initialValues()[0];
    }

    Time AnalyticContinuousFloatingLookbackEngine::residualTime() const {
        return arguments_.stochasticProcess->time(
                                             arguments_.exercise->lastDate());
    }

    Volatility AnalyticContinuousFloatingLookbackEngine::volatility() const {
        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");
        return process->blackVolatility()->blackVol(residualTime(), minmax());
    }

    Real AnalyticContinuousFloatingLookbackEngine::stdDeviation() const {
        return volatility() * std::sqrt(residualTime());
    }

    Rate AnalyticContinuousFloatingLookbackEngine::riskFreeRate() const {
        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");
        return process->riskFreeRate()->zeroRate(residualTime(), Continuous,
                                                 NoFrequency);
    }

    DiscountFactor AnalyticContinuousFloatingLookbackEngine::riskFreeDiscount()
                                 const {
        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");
        return process->riskFreeRate()->discount(residualTime());
    }

    Rate AnalyticContinuousFloatingLookbackEngine::dividendYield() const {
        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");
        return process->dividendYield()->zeroRate(residualTime(),
                                                  Continuous, NoFrequency);
    }

    DiscountFactor AnalyticContinuousFloatingLookbackEngine::dividendDiscount()
                                 const {
        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");
        return process->dividendYield()->discount(residualTime());
    }

    Real AnalyticContinuousFloatingLookbackEngine::minmax() const {
        return arguments_.minmax;
    }

    Real AnalyticContinuousFloatingLookbackEngine::A(Real eta) const {
        Volatility vol = volatility();
        Real lambda = 2.0*(riskFreeRate() - dividendYield())/(vol*vol);
        Real SS = underlying()/minmax();
        Real d1 =
            std::log(SS)/stdDeviation() + 0.5*(lambda+1.0)*stdDeviation();
        Real N1 = f_(eta*d1);
        Real N2 = f_(eta*(d1-stdDeviation()));
        Real N3 = f_(eta*(-d1+lambda*stdDeviation()));
        Real N4 = f_(eta*-d1);
        Real powSS = std::pow(SS, -lambda);
        return eta*((underlying() * dividendDiscount() * N1 -
                    minmax() * riskFreeDiscount() * N2) +
                    (underlying() * riskFreeDiscount() *
                    (powSS * N3 - dividendDiscount()* N4/riskFreeDiscount())/
            lambda));
    }

}

