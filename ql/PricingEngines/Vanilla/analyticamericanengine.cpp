
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2003 Neil Firth

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

/*! \file analyticamericanengine.cpp
    \brief Digital payoff American option engine using analytic formulas
*/

#include <ql/PricingEngines/Vanilla/analyticamericanengine.hpp>
#include <ql/PricingEngines/americanpayoffathit.hpp>
#include <ql/PricingEngines/americanpayoffatexpiry.hpp>

namespace QuantLib {

    void AnalyticAmericanEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::American,
                   "AnalyticAmericanEngine::calculate() : "
                   "not an American Option");

        #if defined(HAVE_BOOST)
        Handle<AmericanExercise> ex = 
            boost::dynamic_pointer_cast<AmericanExercise>(arguments_.exercise);
        QL_REQUIRE(ex, "AnalyticAmericanEngine: non-American exercise given");
        #else
        Handle<AmericanExercise> ex = arguments_.exercise;
        #endif

        QL_REQUIRE(ex->dates()[0]<=
            arguments_.blackScholesProcess->volTS->referenceDate(),
                   "AnalyticAmericanEngine::calculate() : "
                   "American option with window exercise not handled yet");


        #if defined(HAVE_BOOST)
        Handle<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "AnalyticEuropeanEngine: non-striked payoff given");
        #else
        Handle<StrikedTypePayoff> payoff = arguments_.payoff;
        #endif

        double spot = arguments_.blackScholesProcess->stateVariable->value();
        double variance = arguments_.blackScholesProcess->volTS->blackVariance(
            ex->lastDate(), payoff->strike());
        Rate dividendDiscount = arguments_.blackScholesProcess->dividendTS->discount(
            ex->lastDate());
        Rate riskFreeDiscount = arguments_.blackScholesProcess->riskFreeTS->discount(
            ex->lastDate());

        if(ex->payoffAtExpiry()) {
            AmericanPayoffAtExpiry pricer(spot, riskFreeDiscount,
                dividendDiscount, variance, payoff);

            results_.value = pricer.value();
        } else {
            AmericanPayoffAtHit pricer(spot, riskFreeDiscount,
                dividendDiscount, variance, payoff);

            results_.value = pricer.value();
            results_.delta = pricer.delta();
            results_.gamma = pricer.gamma();

            Time t = arguments_.blackScholesProcess->riskFreeTS->dayCounter().yearFraction(
                arguments_.blackScholesProcess->riskFreeTS->referenceDate(),
                arguments_.exercise->lastDate());
            results_.rho = pricer.rho(t);
        }

    }

}

