/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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
#include <ql/math/distributions/poissondistribution.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/jumpdiffusionengine.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

namespace QuantLib {

    JumpDiffusionEngine::JumpDiffusionEngine(ext::shared_ptr<Merton76Process> process,
                                             Real relativeAccuracy,
                                             Size maxIterations)
    : process_(std::move(process)), relativeAccuracy_(relativeAccuracy),
      maxIterations_(maxIterations) {
        registerWith(process_);
    }


    void JumpDiffusionEngine::calculate() const {

        Real jumpSquareVol = process_->logJumpVolatility()->value()
            * process_->logJumpVolatility()->value();
        Real muPlusHalfSquareVol = process_->logMeanJump()->value()
            + 0.5*jumpSquareVol;
        // mean jump size
        Real k = std::exp(muPlusHalfSquareVol) - 1.0;
        Real lambda = (k+1.0) * process_->jumpIntensity()->value();

        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        Real variance =
            process_->blackVolatility()->blackVariance(
                                              arguments_.exercise->lastDate(),
                                              payoff->strike());

        DayCounter voldc = process_->blackVolatility()->dayCounter();
        Calendar volcal = process_->blackVolatility()->calendar();
        Date volRefDate = process_->blackVolatility()->referenceDate();
        Time t = voldc.yearFraction(volRefDate,
                                    arguments_.exercise->lastDate());
        Rate riskFreeRate = -std::log(process_->riskFreeRate()->discount(
                                          arguments_.exercise->lastDate()))/t;
        Date rateRefDate = process_->riskFreeRate()->referenceDate();

        PoissonDistribution p(lambda*t);

        Handle<Quote> stateVariable = process_->stateVariable();
        Handle<YieldTermStructure> dividendTS = process_->dividendYield();
        RelinkableHandle<YieldTermStructure> riskFreeTS(
                                                   *process_->riskFreeRate());
        RelinkableHandle<BlackVolTermStructure> volTS(
                                                *process_->blackVolatility());

        ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess(
                 new GeneralizedBlackScholesProcess(stateVariable, dividendTS,
                                                    riskFreeTS, volTS));

        AnalyticEuropeanEngine baseEngine(bsProcess);

        auto* baseArguments = dynamic_cast<VanillaOption::arguments*>(baseEngine.getArguments());

        baseArguments->payoff   = arguments_.payoff;
        baseArguments->exercise = arguments_.exercise;

        baseArguments->validate();

        const auto* baseResults =
            dynamic_cast<const VanillaOption::results*>(baseEngine.getResults());

        results_.value       = 0.0;
        results_.delta       = 0.0;
        results_.gamma       = 0.0;
        results_.theta       = 0.0;
        results_.vega        = 0.0;
        results_.rho         = 0.0;
        results_.dividendRho = 0.0;

        Real r, v, weight, lastContribution = 1.0;
        Size i;
        Real theta_correction;
        // Haug arbitrary criterium is:
        //for (i=0; i<11; i++) {
        for (i=0;  (lastContribution>relativeAccuracy_ && i<maxIterations_) 
                 || i < ql_cast<Size>(lambda*t); i++) {

            // constant vol/rate assumption. It should be relaxed
            v = std::sqrt((variance + i*jumpSquareVol)/t);
            r = riskFreeRate - process_->jumpIntensity()->value()*k
                + i*muPlusHalfSquareVol/t;
            riskFreeTS.linkTo(ext::shared_ptr<YieldTermStructure>(new
                FlatForward(rateRefDate, r, voldc)));
            volTS.linkTo(ext::shared_ptr<BlackVolTermStructure>(new
                BlackConstantVol(rateRefDate, volcal, v, voldc)));

            baseArguments->validate();
            baseEngine.calculate();

            weight = p(Size(i));
            results_.value       += weight * baseResults->value;
            results_.delta       += weight * baseResults->delta;
            results_.gamma       += weight * baseResults->gamma;
            results_.vega        += weight * (std::sqrt(variance/t)/v)*
                                                           baseResults->vega;
            // theta modified
            theta_correction = baseResults->vega*((i*jumpSquareVol)/
                                                  (2.0*v*t*t)) +
                baseResults->rho*i*muPlusHalfSquareVol/(t*t);
            results_.theta += weight *(baseResults->theta + theta_correction +
                                  lambda*baseResults->value);
            if(i != 0){
                 results_.theta -= (p(Size(i-1))*lambda* baseResults->value);
            }
            //end theta calculation
            results_.rho         += weight * baseResults->rho;
            results_.dividendRho += weight * baseResults->dividendRho;

            lastContribution = std::fabs(baseResults->value /
                (std::fabs(results_.value)>QL_EPSILON ? results_.value : 1.0));

            lastContribution = std::max<Real>(lastContribution,
                std::fabs(baseResults->delta /
               (std::fabs(results_.delta)>QL_EPSILON ? results_.delta : 1.0)));

            lastContribution = std::max<Real>(lastContribution,
                std::fabs(baseResults->gamma /
               (std::fabs(results_.gamma)>QL_EPSILON ? results_.gamma : 1.0)));

            lastContribution = std::max<Real>(lastContribution,
                std::fabs(baseResults->theta /
               (std::fabs(results_.theta)>QL_EPSILON ? results_.theta : 1.0)));

            lastContribution = std::max<Real>(lastContribution,
                std::fabs(baseResults->vega /
               (std::fabs(results_.vega)>QL_EPSILON ? results_.vega : 1.0)));

            lastContribution = std::max<Real>(lastContribution,
                std::fabs(baseResults->rho /
               (std::fabs(results_.rho)>QL_EPSILON ? results_.rho : 1.0)));

            lastContribution = std::max<Real>(lastContribution,
                std::fabs(baseResults->dividendRho /
               (std::fabs(results_.dividendRho)>QL_EPSILON ?
                                          results_.dividendRho : 1.0)));

            lastContribution *= weight;
        }
        QL_ENSURE(i<maxIterations_,
                  i << " iterations have been not enough to reach "
                  << "the required " << relativeAccuracy_
                  << " accuracy. The " << io::ordinal(i)
                  << " addendum was " << lastContribution
                  << " while the running sum was " << results_.value);
    }

}

