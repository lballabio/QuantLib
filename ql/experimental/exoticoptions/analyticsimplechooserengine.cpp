/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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
#include <ql/experimental/exoticoptions/analyticsimplechooserengine.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <utility>

namespace QuantLib {

    AnalyticSimpleChooserEngine::AnalyticSimpleChooserEngine(
        std::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    void AnalyticSimpleChooserEngine::calculate() const {
        Date today = Settings::instance().evaluationDate();
        DayCounter rfdc  = process_->riskFreeRate()->dayCounter();
        DayCounter divdc = process_->dividendYield()->dayCounter();
        DayCounter voldc = process_->blackVolatility()->dayCounter();
        QL_REQUIRE(rfdc==divdc,
                   "Risk-free rate and dividend yield must"
                   "have the same day counter");
        QL_REQUIRE(rfdc==voldc,
                   "Risk-free rate and volatility must"
                   "have the same day counter");
        Real spot = process_->stateVariable()->value();
        std::shared_ptr<StrikedTypePayoff> payoff =
            std::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");
        Real strike = payoff->strike();
        Volatility volatility = process_->blackVolatility()->blackVol(
                                                arguments_.exercise->lastDate(),
                                                strike);
        Date maturity = arguments_.exercise->lastDate();
        Real timeToMaturity = rfdc.yearFraction(today,maturity);
        Real timeToChoosing = rfdc.yearFraction(today,arguments_.choosingDate);
        Rate dividendRate =
            process_->dividendYield()->zeroRate(maturity, divdc,
                                                Continuous, NoFrequency);
        Rate riskFreeRate =
            process_->riskFreeRate()->zeroRate(maturity, rfdc,
                                               Continuous, NoFrequency);

        QL_REQUIRE(spot > 0.0, "negative or null spot value");
        QL_REQUIRE(strike > 0.0, "negative or null strike value");
        QL_REQUIRE(volatility > 0.0,
                   "negative or null volatility");
        QL_REQUIRE(timeToChoosing > 0.0,
                   "choosing date earlier than or equal to evaluation date");

        Real d = (std::log(spot/strike)
            + ((riskFreeRate-dividendRate) + volatility*volatility*0.5)*timeToMaturity)
            /(volatility*std::sqrt(timeToMaturity));

        Real y = (std::log(spot/strike) + (riskFreeRate-dividendRate)*timeToMaturity
            + (volatility*volatility*timeToChoosing/2))
            /(volatility*std::sqrt(timeToChoosing));

        CumulativeNormalDistribution f;

        results_.value = spot*std::exp(-dividendRate*timeToMaturity)*f(d)
            - strike*std::exp(-riskFreeRate*timeToMaturity)
            *f(d-volatility*std::sqrt(timeToMaturity))
            -spot*std::exp(-dividendRate*timeToMaturity)*f(-y)
            +strike*std::exp(-riskFreeRate*timeToMaturity)
            *f(-y+volatility*std::sqrt(timeToChoosing));
     }

}
