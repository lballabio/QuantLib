
/*
 Copyright (C) 2005 Joseph Wang

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

#include <ql/PricingEngines/Vanilla/fdeuropeanengine.hpp>
#include <ql/Pricers/fdeuropean.hpp>

namespace QuantLib {

    void FDEuropeanEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        const boost::shared_ptr<BlackScholesProcess>& process =
            arguments_.blackScholesProcess;

        Real spot = process->stateVariable()->value();
        Date exerciseDate = arguments_.exercise->lastDate();

        DayCounter rfdc  = process->riskFreeRate()->dayCounter();
        Time t = rfdc.yearFraction(process->riskFreeRate()->referenceDate(),
                                   arguments_.exercise->lastDate());

        FdEuropean option(payoff->optionType(),
                          spot,
                          payoff->strike(),
                          process->dividendYield()->zeroRate(t, Continuous),
                          process->riskFreeRate()->zeroRate(t, Continuous),
                          t,
                          process->blackVolatility()->blackVol(
                                              exerciseDate, payoff->strike()),
                          timeSteps_, gridPoints_);

        results_.value = option.value();
        results_.delta = option.delta();
        results_.deltaForward = Null<Real>();
        results_.elasticity = Null<Real>();
        results_.gamma = option.gamma();

        results_.dividendRho = option.dividendRho();

        results_.vega = option.vega();
        try {
            results_.theta = option.theta();
        } catch (Error&) {
            results_.theta = Null<Real>();
        }
        results_.thetaPerDay = Null<Real>();

        results_.strikeSensitivity  = Null<Real>();
        results_.itmCashProbability = Null<Real>();
    }

}

