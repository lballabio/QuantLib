
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
#include <ql/Math/poissondistribution.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>

namespace QuantLib {

    JumpDiffusionEngine::JumpDiffusionEngine(
        const boost::shared_ptr<VanillaEngine>& baseEngine,
        double relativeAccuracy,
        Size maxIterations)
    : baseEngine_(baseEngine), relativeAccuracy_(relativeAccuracy),
      maxIterations_(maxIterations) {
        QL_REQUIRE(baseEngine_,
                   "JumpDiffusionEngine: null base engine");
    }


    void JumpDiffusionEngine::calculate() const {

        boost::shared_ptr<Merton76StochasticProcess> jdProcess =
            boost::dynamic_pointer_cast<Merton76StochasticProcess>(
                                              arguments_.blackScholesProcess);
        QL_REQUIRE(jdProcess, "not a jump diffusion process");

        double jumpSquareVol = jdProcess->logJumpVolatility->value()
            * jdProcess->logJumpVolatility->value();
        double muPlusHalfSquareVol = jdProcess->logJumpMean->value()
            + 0.5*jumpSquareVol;
        // mean jump size
        double k = QL_EXP(muPlusHalfSquareVol) - 1.0;
        double lambda = (k+1.0) * jdProcess->jumpIntensity->value();

        // dummy strike
        double variance = jdProcess->volTS->blackVariance(
            arguments_.exercise->lastDate(), 1.0);
        DayCounter dc = jdProcess->volTS->dayCounter();
        Date volRefDate = jdProcess->volTS->referenceDate();
        Time t = dc.yearFraction(volRefDate,
            arguments_.exercise->lastDate());
        Rate riskFreeRate = -QL_LOG(jdProcess->riskFreeTS->discount(
            arguments_.exercise->lastDate()))/t;
        Date rateRefDate = jdProcess->riskFreeTS->referenceDate();

        PoissonDistribution p(lambda*t);

        baseEngine_->reset();

        VanillaOption::arguments* baseArguments =
            dynamic_cast<VanillaOption::arguments*>(baseEngine_->arguments());

        baseArguments->payoff   = arguments_.payoff;
        baseArguments->exercise = arguments_.exercise;
        baseArguments->blackScholesProcess = 
            boost::shared_ptr<BlackScholesStochasticProcess>(
                  new BlackScholesStochasticProcess(jdProcess->stateVariable, 
                                                    jdProcess->dividendTS,
                                                    jdProcess->riskFreeTS, 
                                                    jdProcess->volTS));

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

        double r, v, weight, lastContribution = 1.0;
        Size i;
        // Haug arbitrary criterium is:
        //for (i=0; i<11; i++) {
        for (i=0;
             lastContribution>relativeAccuracy_ && i<maxIterations_;
             i++) {

            // constant vol/rate assumption. It should be relaxed
            v = QL_SQRT((variance + i*jumpSquareVol)/t);
            r = riskFreeRate - jdProcess->jumpIntensity->value()*k
                + i*muPlusHalfSquareVol/t;
            baseArguments->blackScholesProcess->riskFreeTS =
                RelinkableHandle<TermStructure>(
                    boost::shared_ptr<TermStructure>(new
                        FlatForward(rateRefDate, rateRefDate, r, dc)));
            baseArguments->blackScholesProcess->volTS =
                RelinkableHandle<BlackVolTermStructure>(
                    boost::shared_ptr<BlackVolTermStructure>(new
                        BlackConstantVol(rateRefDate, v, dc)));


            baseArguments->validate();
            baseEngine_->calculate();


            weight = p(i);
            results_.value       += weight * baseResults->value;
            results_.delta       += weight * baseResults->delta;
            results_.gamma       += weight * baseResults->gamma;
//            results_.theta       += weight * baseResults->theta;
//            results_.vega        += weight * baseResults->vega;
            results_.rho         += weight * baseResults->rho;
            results_.dividendRho += weight * baseResults->dividendRho;

            lastContribution = QL_FABS(baseResults->value /
                (QL_FABS(results_.value)>QL_EPSILON ? results_.value : 1.0));
            
            lastContribution = QL_MAX(lastContribution,
                QL_FABS(baseResults->delta /
                (QL_FABS(results_.delta)>QL_EPSILON ? results_.delta : 1.0)));

            lastContribution = QL_MAX(lastContribution,
                QL_FABS(baseResults->gamma /
                (QL_FABS(results_.gamma)>QL_EPSILON ? results_.gamma : 1.0)));
/*
            lastContribution = QL_MAX(lastContribution,
                QL_FABS(baseResults->theta /
                (QL_FABS(results_.theta)>QL_EPSILON ? results_.theta : 1.0)));

            lastContribution = QL_MAX(lastContribution,
                QL_FABS(baseResults->vega /
                (QL_FABS(results_.vega)>QL_EPSILON ? results_.vega : 1.0)));
*/
            lastContribution = QL_MAX(lastContribution,
                QL_FABS(baseResults->rho /
                (QL_FABS(results_.rho)>QL_EPSILON ? results_.rho : 1.0)));

            lastContribution = QL_MAX(lastContribution,
                QL_FABS(baseResults->dividendRho /
                (QL_FABS(results_.dividendRho)>QL_EPSILON ?
                                          results_.dividendRho : 1.0)));

            lastContribution *= weight;
        }
        QL_ENSURE(i<maxIterations_,
            "JumpDiffusionEngine::calculate : "
            + IntegerFormatter::toString(i) +
            " iterations have been not enough to reach the required "
            + DoubleFormatter::toExponential(relativeAccuracy_) +
            " accuracy. The "
            + IntegerFormatter::toOrdinal(i) +
            " addendum was "
            + DoubleFormatter::toExponential(lastContribution) +
            " while the running sum was "
            + DoubleFormatter::toExponential(results_.value));


//        std::cout << std::endl << DoubleFormatter::toString(lastContribution * results_.value,6) << ", i=" << i;
    }

}

