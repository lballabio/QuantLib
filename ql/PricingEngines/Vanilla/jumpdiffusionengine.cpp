
/*
 Copyright (C) 2004 Ferdinando Ametrano

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

#include <ql/PricingEngines/Vanilla/jumpdiffusionengine.hpp>
#include <ql/Processes/merton76process.hpp>
#include <ql/Math/poissondistribution.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Utilities/dataformatters.hpp>

namespace QuantLib {

    JumpDiffusionEngine::JumpDiffusionEngine(
        const boost::shared_ptr<VanillaOption::engine>& baseEngine,
        Real relativeAccuracy,
        Size maxIterations)
    : baseEngine_(baseEngine), relativeAccuracy_(relativeAccuracy),
      maxIterations_(maxIterations) {
        QL_REQUIRE(baseEngine_, "null base engine");
    }


    void JumpDiffusionEngine::calculate() const {

        boost::shared_ptr<Merton76Process> jdProcess =
            boost::dynamic_pointer_cast<Merton76Process>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(jdProcess, "not a jump diffusion process");

        Real jumpSquareVol = jdProcess->logJumpVolatility()->value()
            * jdProcess->logJumpVolatility()->value();
        Real muPlusHalfSquareVol = jdProcess->logMeanJump()->value()
            + 0.5*jumpSquareVol;
        // mean jump size
        Real k = std::exp(muPlusHalfSquareVol) - 1.0;
        Real lambda = (k+1.0) * jdProcess->jumpIntensity()->value();

        // dummy strike
        Real variance = jdProcess->blackVolatility()->blackVariance(
                                        arguments_.exercise->lastDate(), 1.0);
        DayCounter voldc = jdProcess->blackVolatility()->dayCounter();
        Date volRefDate = jdProcess->blackVolatility()->referenceDate();
        Time t = voldc.yearFraction(volRefDate,
                                    arguments_.exercise->lastDate());
        Rate riskFreeRate = -std::log(jdProcess->riskFreeRate()->discount(
                                          arguments_.exercise->lastDate()))/t;
        Date rateRefDate = jdProcess->riskFreeRate()->referenceDate();

        PoissonDistribution p(lambda*t);

        baseEngine_->reset();

        VanillaOption::arguments* baseArguments =
            dynamic_cast<VanillaOption::arguments*>(baseEngine_->arguments());

        baseArguments->payoff   = arguments_.payoff;
        baseArguments->exercise = arguments_.exercise;
        Handle<Quote> stateVariable(jdProcess->stateVariable());
        Handle<YieldTermStructure> dividendTS(jdProcess->dividendYield());
        Handle<YieldTermStructure> riskFreeTS(jdProcess->riskFreeRate());
        Handle<BlackVolTermStructure> volTS(jdProcess->blackVolatility());
        baseArguments->stochasticProcess =
            boost::shared_ptr<StochasticProcess>(
                            new BlackScholesProcess(stateVariable, dividendTS,
                                                    riskFreeTS, volTS));
        baseArguments->validate();

        const VanillaOption::results* baseResults =
            dynamic_cast<const VanillaOption::results*>(
            baseEngine_->results());

        results_.value       = 0.0;
        results_.delta       = 0.0;
        results_.gamma       = 0.0;
//        results_.theta       = 0.0;
//        results_.vega        = 0.0;
        results_.rho         = 0.0;
        results_.dividendRho = 0.0;

        Real r, v, weight, lastContribution = 1.0;
        Size i;
        // Haug arbitrary criterium is:
        //for (i=0; i<11; i++) {
        for (i=0;
             lastContribution>relativeAccuracy_ && i<maxIterations_;
             i++) {

            // constant vol/rate assumption. It should be relaxed
            v = std::sqrt((variance + i*jumpSquareVol)/t);
            r = riskFreeRate - jdProcess->jumpIntensity()->value()*k
                + i*muPlusHalfSquareVol/t;
            riskFreeTS.linkTo(boost::shared_ptr<YieldTermStructure>(
                                new FlatForward(rateRefDate, r, voldc)));
            volTS.linkTo(boost::shared_ptr<BlackVolTermStructure>(
                                new BlackConstantVol(rateRefDate, v, voldc)));

            baseArguments->validate();
            baseEngine_->calculate();


            weight = p(Size(i));
            results_.value       += weight * baseResults->value;
            results_.delta       += weight * baseResults->delta;
            results_.gamma       += weight * baseResults->gamma;
//            results_.theta       += weight * baseResults->theta;
//            results_.vega        += weight * baseResults->vega;
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
/*
            lastContribution = std::max<Real>(lastContribution,
                std::fabs(baseResults->theta /
               (std::fabs(results_.theta)>QL_EPSILON ? results_.theta : 1.0)));

            lastContribution = std::max<Real>(lastContribution,
                std::fabs(baseResults->vega /
               (std::fabs(results_.vega)>QL_EPSILON ? results_.vega : 1.0)));
*/
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

