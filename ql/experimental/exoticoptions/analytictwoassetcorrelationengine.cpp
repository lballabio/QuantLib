/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/exercise.hpp>
#include <ql/experimental/exoticoptions/analytictwoassetcorrelationengine.hpp>
#include <ql/math/distributions/bivariatenormaldistribution.hpp>
#include <utility>

using std::log;

namespace QuantLib {

    AnalyticTwoAssetCorrelationEngine::AnalyticTwoAssetCorrelationEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> p1,
        ext::shared_ptr<GeneralizedBlackScholesProcess> p2,
        Handle<Quote> correlation)
    : p1_(std::move(p1)), p2_(std::move(p2)), correlation_(std::move(correlation)) {
        registerWith(p1_);
        registerWith(p2_);
        registerWith(correlation_);
    }

    void AnalyticTwoAssetCorrelationEngine::calculate() const {
        BivariateCumulativeNormalDistributionDr78 M(correlation_->value());

        const ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");
        QL_REQUIRE(payoff->strike()>0.0, "strike must be positive");
        ext::shared_ptr<Exercise> exercise = arguments_.exercise;
        Real strike = payoff->strike();//X1
        Real spot = p1_->x0();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");

        Volatility sigma1 =
            p1_->blackVolatility()->blackVol(p1_->time(exercise->lastDate()),
                                             payoff->strike());
        Volatility sigma2 =
            p2_->blackVolatility()->blackVol(p2_->time(exercise->lastDate()),
                                             payoff->strike());

        Time T = p2_->time(arguments_.exercise->lastDate());

        Real s1=p1_->x0();
        Real s2=p2_->x0();
        Rate q1= p1_->dividendYield()->zeroRate(T, Continuous, NoFrequency);
        Rate q2= p2_->dividendYield()->zeroRate(T, Continuous, NoFrequency);
        Rate r=p1_->riskFreeRate()->zeroRate(T, Continuous, NoFrequency);
        Rate b1=r-q1;
        Rate b2=r-q2;
        Real rho = correlation_->value();

        Real y1=(log(s1/strike)+(b1-(sigma1*sigma1)/2)*T)/(sigma1*std::sqrt(T));
        Real y2=(log(s2/arguments_.X2)+(b2-(sigma2*sigma2)/2)*T)/(sigma2*std::sqrt(T));

        switch (payoff->optionType()) {
          case Option::Call:
            results_.value=s2*std::exp((b2-r)*T)*M(y2+sigma2*std::sqrt(T),y1+rho*sigma2*std::sqrt(T))-arguments_.X2*std::exp(-r*T)*M(y2,y1);
            break;
          case Option::Put:
            results_.value=arguments_.X2*std::exp(-r*T)*M(-y2,-y1)-s2*std::exp((b2-r)*T)*M(-y2-sigma2*std::sqrt(T),-y1-rho*sigma2*std::sqrt(T));
            break;
          default:
            QL_FAIL("unknown option type");
        }
    }

}
