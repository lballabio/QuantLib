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

#include <ql/MarketModels/forwardrateevolver.hpp>

namespace QuantLib {

    ForwardRateEvolver::ForwardRateEvolver(
                           const boost::shared_ptr<PseudoRoot>& pseudoRoot,
                           const EvolutionDescription& evolution,
                           const BrownianGeneratorFactory& factory)
    : pseudoRoot_(pseudoRoot), evolution_(evolution),
      n_(pseudoRoot->numberOfRates()), F_(pseudoRoot_->numberOfFactors()),
      curveState_(evolution.rateTimes()),
      forwards_(pseudoRoot->initialRates()),
      logForwards_(n_), initialLogForwards_(n_), drifts1_(n_), drifts2_(n_),
      initialDrifts_(n_), brownians_(F_), correlatedBrownians_(n_),
      alive_(evolution.evolutionTimes().size()) {

        const Array& initialForwards = pseudoRoot_->initialRates();
        const Array& displacements = pseudoRoot_->displacements();
        
        Size factors = pseudoRoot_->numberOfFactors();
        Size steps = evolution_.numberOfSteps();

        const Array& rateTimes = evolution_.rateTimes();
        const Array& evolutionTimes = evolution_.evolutionTimes();

        generator_ = factory.create(factors, steps);
        currentStep_ = 0;

        for (Size i=0; i<n_; ++i) {
            initialLogForwards_[i] = std::log(initialForwards[i] +
                                              displacements[i]);
        }

        Time lastTime = 0.0;
        for (Size j=0; j<steps; ++j) {
            Size alive = 0;
            while (rateTimes[alive] <= lastTime)
                ++alive;

            calculators_.push_back(DriftCalculator(pseudoRoot_->pseudoRoot(j),
                                                   displacements,
                                                   evolution_.taus(),
                                                   evolution_.numeraires()[j],
                                                   alive));
            alive_[j] = alive;
            lastTime = evolutionTimes[j];
        }

        calculators_.front().compute(initialForwards, initialDrifts_);
    }

	ForwardRateEvolver::~ForwardRateEvolver() {}

    Real ForwardRateEvolver::startNewPath() {
        currentStep_ = 0;
        std::copy(initialLogForwards_.begin(), initialLogForwards_.end(),
                  logForwards_.begin());
        return generator_->nextPath();
    }

    Real ForwardRateEvolver::advanceStep() {
        const Array& displacements = pseudoRoot_->displacements();

        // we're going from T1 to T2:
        if (currentStep_ == 0) {
            std::copy(initialDrifts_.begin(), initialDrifts_.end(),
                      drifts1_.begin());
        } else {
            // a) compute drifts D1 at T1;
            calculators_[currentStep_].compute(forwards_, drifts1_);
        }

        // b) evolve forwards up to T2 using D1;
        Real weight = generator_->nextStep(brownians_);
        const Matrix& A = pseudoRoot_->pseudoRoot(currentStep_);

        Size alive = alive_[currentStep_];
        for (Size i=alive; i<n_; i++) {
            logForwards_[i] += drifts1_[i];
            logForwards_[i] +=
                std::inner_product(A.row_begin(i), A.row_end(i),
                                   brownians_.begin(), 0.0);
            forwards_[i] = std::exp(logForwards_[i]) - displacements[i];
        }

        // c) recompute drifts D2 using the predicted forwards;
        calculators_[currentStep_].compute(forwards_, drifts2_);
        
        // d) correct forwards using both drifts
        for (Size i=alive; i<n_; ++i) {
            logForwards_[i] += (drifts2_[i]-drifts1_[i])/2.0;
            forwards_[i] = std::exp(logForwards_[i]) - displacements[i];
        }

        // e) update curve state
        curveState_.setOnForwardRates(forwards_);

        return weight;
    }

    Size ForwardRateEvolver::currentStep() const {
        return currentStep_;
    }

    const CurveState& ForwardRateEvolver::currentState() const {
        return curveState_;
    }

}
