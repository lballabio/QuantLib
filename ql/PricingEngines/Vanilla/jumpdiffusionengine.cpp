
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

/*! \file jumpdiffusion.cpp
    \brief Jump diffusion (Merton 1976) engine
*/

#include <ql/PricingEngines/Vanilla/jumpdiffusionengine.hpp>
#include <ql/Math/poissondistribution.hpp>

namespace QuantLib {

    JumpDiffusionEngine::JumpDiffusionEngine(
        const Handle<VanillaEngine>& baseEngine)
    : baseEngine_(baseEngine) {
        QL_REQUIRE(!IsNull(baseEngine_),
                   "JumpDiffusionEngine::JumpDiffusionEngine: null base engine");
    }


    void JumpDiffusionEngine::calculate() const {

        Handle<Merton76StochasticProcess> jdProcess;
        #if defined(HAVE_BOOST)
        jdProcess = boost::dynamic_pointer_cast<Merton76StochasticProcess>(
            arguments_.blackScholesProcess);
        #else
        try {
            jdProcess = arguments_.blackScholesProcess;
        } catch (...) {}
        #endif
        QL_REQUIRE(!IsNull(jdProcess),"not a jump diffusion process");
           
        double jumpSquareVol =
            jdProcess->jumpVolatility*jdProcess->jumpVolatility;
        double muPlusHalfSquareVol = jdProcess->meanLogJump + 0.5*jumpSquareVol;
        // mean jump size
        double k = QL_EXP(muPlusHalfSquareVol) - 1.0;
        double lambda = (k+1.0) * jdProcess->jumpIntensity;
            
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
//        *baseArguments = arguments_;
        baseArguments->payoff   = arguments_.payoff;
        baseArguments->exercise = arguments_.exercise;
        baseArguments->blackScholesProcess->dividendTS =
            arguments_.blackScholesProcess->dividendTS;
        baseArguments->blackScholesProcess->stateVariable =
            arguments_.blackScholesProcess->stateVariable;


        const VanillaOption::results* baseResults =
            dynamic_cast<const VanillaOption::results*>(
            baseEngine_->results());

        double r, v, weight, weightSum = 0.0;
        for (Size i=0; i<11; i++) {

            // constant vol/rate assumption. It should be relaxed
            v = QL_SQRT((variance + i*jumpSquareVol)/t);
            r = riskFreeRate - jdProcess->jumpIntensity*k
                + i*muPlusHalfSquareVol/t;
            baseArguments->blackScholesProcess->riskFreeTS =
                RelinkableHandle<TermStructure>(
                    Handle<TermStructure>(new
                        FlatForward(rateRefDate, rateRefDate, r, dc)));
            baseArguments->blackScholesProcess->volTS =
                RelinkableHandle<BlackVolTermStructure>(
                    Handle<BlackVolTermStructure>(new
                        BlackConstantVol(rateRefDate, v, dc)));

            baseArguments->validate();

            baseEngine_->calculate();


            weight = p(i);
            results_.value       += weight * baseResults->value;
            results_.delta       += weight * baseResults->delta;
            results_.gamma       += weight * baseResults->gamma;
            results_.theta       += weight * baseResults->theta;
            results_.vega        += weight * baseResults->vega;
            results_.rho         += weight * baseResults->rho;
            results_.dividendRho += weight * baseResults->dividendRho;
        }

        

    
    }

}

