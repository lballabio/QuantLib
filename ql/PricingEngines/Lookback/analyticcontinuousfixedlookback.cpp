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

#include <ql/PricingEngines/Lookback/analyticcontinuousfixedlookback.hpp>
#include <ql/Processes/blackscholesprocess.hpp>

namespace QuantLib {

    void AnalyticContinuousFixedLookbackEngine::calculate() const {

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "Non-plain payoff given");
        QL_REQUIRE(payoff->strike()>0.0,
                   "Strike must be positive");

        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");

        Real strike = payoff->strike();

        switch (payoff->optionType()) {
          case Option::Call:
            if (strike <= minmax())
                results_.value = A(1) + C(1);
            else
                results_.value = B(1);
            break;
          case Option::Put:
            if (strike >= minmax())
                results_.value = A(-1) + C(-1);
            else
                results_.value = B(-1);
            break;
          default:
            QL_FAIL("Unknown type");
        }
    }


    Real AnalyticContinuousFixedLookbackEngine::underlying() const {
        return arguments_.stochasticProcess->initialValues()[0];
    }

    Real AnalyticContinuousFixedLookbackEngine::strike() const {
        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "Non-plain payoff given");
        return payoff->strike();
    }

    Time AnalyticContinuousFixedLookbackEngine::residualTime() const {
        return arguments_.stochasticProcess->time(
                                             arguments_.exercise->lastDate());
    }

    Volatility AnalyticContinuousFixedLookbackEngine::volatility() const {
        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");
        return process->blackVolatility()->blackVol(residualTime(), strike());
    }

    Real AnalyticContinuousFixedLookbackEngine::stdDeviation() const {
        return volatility() * std::sqrt(residualTime());
    }

    Rate AnalyticContinuousFixedLookbackEngine::riskFreeRate() const {
        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");
        return process->riskFreeRate()->zeroRate(residualTime(), Continuous,
                                                 NoFrequency);
    }

    DiscountFactor AnalyticContinuousFixedLookbackEngine::riskFreeDiscount()
                              const {
        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");
        return process->riskFreeRate()->discount(residualTime());
    }

    Rate AnalyticContinuousFixedLookbackEngine::dividendYield() const {
        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");
        return process->dividendYield()->zeroRate(residualTime(),
                                                  Continuous, NoFrequency);
    }

    DiscountFactor AnalyticContinuousFixedLookbackEngine::dividendDiscount()
                              const {
        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");
        return process->dividendYield()->discount(residualTime());
    }

    Real AnalyticContinuousFixedLookbackEngine::minmax() const {
        return arguments_.minmax;
    }

    Real AnalyticContinuousFixedLookbackEngine::A(Real eta) const {
        Volatility vol = volatility();
        Real lambda = 2.0*(riskFreeRate() - dividendYield())/(vol*vol);
        Real SS = underlying()/minmax();
        Real d1 =
            std::log(SS)/stdDeviation() + 0.5*(lambda+1.0)*stdDeviation();
        Real N1 = f_(eta*d1);
        Real N2 = f_(eta*(d1-stdDeviation()));
        Real N3 = f_(eta*(d1-lambda*stdDeviation()));
        Real N4 = f_(eta*d1);
        Real powSS = std::pow(SS, -lambda);
        return eta*(underlying() * dividendDiscount() * N1 -
                    minmax() * riskFreeDiscount() * N2 -
                    underlying() * riskFreeDiscount() *
                    (powSS * N3 - dividendDiscount()* N4/riskFreeDiscount())/
            lambda);
    }

    Real AnalyticContinuousFixedLookbackEngine::B(Real eta) const {
        Volatility vol = volatility();
        Real lambda = 2.0*(riskFreeRate() - dividendYield())/(vol*vol);
        Real SS = underlying()/strike();
        Real d1 =
            std::log(SS)/stdDeviation() + 0.5*(lambda+1.0)*stdDeviation();
        Real N1 = f_(eta*d1);
        Real N2 = f_(eta*(d1-stdDeviation()));
        Real N3 = f_(eta*(d1-lambda*stdDeviation()));
        Real N4 = f_(eta*d1);
        Real powSS = std::pow(SS, -lambda);
        return eta*(underlying() * dividendDiscount() * N1 -
                    strike() * riskFreeDiscount() * N2 -
                    underlying() * riskFreeDiscount() *
                    (powSS * N3 - dividendDiscount()* N4/riskFreeDiscount())/
            lambda);
    }

    Real AnalyticContinuousFixedLookbackEngine::C(Real eta) const {
        return eta*(riskFreeDiscount()*(minmax() - strike()));
    }

}

