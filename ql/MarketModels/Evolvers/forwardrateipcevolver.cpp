/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

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

#include <ql/MarketModels/Evolvers/forwardrateipcevolver.hpp>

namespace QuantLib {

    ForwardRateIpcEvolver::ForwardRateIpcEvolver(
                           const boost::shared_ptr<PseudoRoot>& pseudoRoot,
                           const EvolutionDescription& evolution,
                           const BrownianGeneratorFactory& factory)
    : pseudoRoot_(pseudoRoot), evolution_(evolution),
      n_(pseudoRoot->numberOfRates()), F_(pseudoRoot->numberOfFactors()),
      curveState_(evolution.rateTimes()),
      forwards_(pseudoRoot->initialRates()),
      displacements_(pseudoRoot->displacements()),
      logForwards_(n_), initialLogForwards_(n_), drifts1_(n_),
      initialDrifts_(n_), g_(n_), brownians_(F_), correlatedBrownians_(n_),
      alive_(evolution.evolutionTimes().size())
    {
        QL_REQUIRE(evolution.isInTerminalMeasure(),
                   "terminal measure required for ipc ");

        const Array& initialForwards = pseudoRoot->initialRates();
        
        Size steps = evolution_.numberOfSteps();

        const Array& rateTimes = evolution_.rateTimes();
        const Array& evolutionTimes = evolution_.evolutionTimes();

        generator_ = factory.create(F_, steps);
        currentStep_ = 0;

        for (Size i=0; i<n_; ++i) {
            initialLogForwards_[i] = std::log(initialForwards[i] +
                                              displacements_[i]);
        }

        for (Size j=0; j<steps; ++j)
        {
            const Matrix& A = pseudoRoot->pseudoRoot(j);
            calculators_.push_back(DriftCalculator(A, 
                                                   displacements_,
                                                   evolution_.rateTaus(),
                                                   evolution_.numeraires()[j],
                                                   alive_[j]));
            C_.push_back(A*transpose(A));

            Array fixed(n_);
            for (Size k=0; k < n_; ++k) {
                Real variance = C_.back()[k][k];
                fixed[k] = -0.5*variance;
            }
            fixedDrifts_.push_back(fixed);
        }

        calculators_.front().compute(initialForwards, initialDrifts_);
        //calculators_.front().computeReduced(initialForwards, F_, initialDrifts_);
    }

    Real ForwardRateIpcEvolver::startNewPath() {
        currentStep_ = 0;
        std::copy(initialLogForwards_.begin(), initialLogForwards_.end(),
                  logForwards_.begin());
        return generator_->nextPath();
    }

    Real ForwardRateIpcEvolver::advanceStep()
    {
        const Array& rateTaus = evolution_.rateTaus();

        // we're going from T1 to T2:
        if (currentStep_ > 0) {
            calculators_[currentStep_].compute(forwards_, drifts1_);
            //calculators_[currentStep_].computeReduced(forwards_, F_, drifts1_);
        } else {
            std::copy(initialDrifts_.begin(), initialDrifts_.end(),
                      drifts1_.begin());
        }

        Real weight = generator_->nextStep(brownians_);
        const Matrix& A = pseudoRoot_->pseudoRoot(currentStep_);
        const Array& fixedDrift = fixedDrifts_[currentStep_];

        Integer alive = alive_[currentStep_];
        Real drifts2;
        for (Integer i=n_-1; i>=alive; --i) {
            drifts2 = 0.0;
            for (Size j=i+1; j<n_; ++j) {
                drifts2 -= g_[j]*C_[currentStep_][i][j];
            }
            logForwards_[i] += 0.5*(drifts1_[i]+drifts2) + fixedDrift[i];
            logForwards_[i] +=
                std::inner_product(A.row_begin(i), A.row_end(i),
                                   brownians_.begin(), 0.0);
            forwards_[i] = std::exp(logForwards_[i]) - displacements_[i];
            g_[i] = rateTaus[i]*(forwards_[i]+displacements_[i])/
                (1.0+rateTaus[i]*forwards_[i]);
        }

        // update curve state
        curveState_.setOnForwardRates(forwards_);

        ++currentStep_;

        return weight;
    }

    Size ForwardRateIpcEvolver::currentStep() const {
        return currentStep_;
    }

    const CurveState& ForwardRateIpcEvolver::currentState() const {
        return curveState_;
    }

}
