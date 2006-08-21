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

#include <ql/MarketModels/Evolvers/forwardratepcevolver.hpp>

namespace QuantLib {

    ForwardRatePcEvolver::ForwardRatePcEvolver(
                           const boost::shared_ptr<PseudoRoot>& pseudoRoot,
                           const EvolutionDescription& evolution,
                           const BrownianGeneratorFactory& factory)
    : pseudoRoot_(pseudoRoot), evolution_(evolution),
      n_(pseudoRoot->numberOfRates()), F_(pseudoRoot_->numberOfFactors()),
      curveState_(evolution.rateTimes()),
      forwards_(pseudoRoot->initialRates()),
      displacements_(pseudoRoot->displacements()),
      logForwards_(n_), initialLogForwards_(n_), drifts1_(n_), drifts2_(n_),
      initialDrifts_(n_), brownians_(F_), correlatedBrownians_(n_),
      alive_(evolution_.firstAliveRate())
    {
        const std::vector<Rate>& initialForwards = pseudoRoot_->initialRates();
        
        Size steps = evolution_.numberOfSteps();

        const std::vector<Time>& rateTimes = evolution_.rateTimes();
        const std::vector<Time>& evolutionTimes = evolution_.evolutionTimes();

        generator_ = factory.create(F_, steps);
        currentStep_ = 0;

        for (Size i=0; i<n_; ++i) {
            initialLogForwards_[i] = std::log(initialForwards[i] +
                                              displacements_[i]);
        }

        for (Size j=0; j<steps; ++j)
        {
            const Matrix& A = pseudoRoot_->pseudoRoot(j);
            calculators_.push_back(DriftCalculator(A, 
                                                   displacements_,
                                                   evolution_.rateTaus(),
                                                   evolution_.numeraires()[j],
                                                   alive_[j]));
            std::vector<Real> fixed(n_);
            for (Size k=0; k < n_; ++k) {
                Real variance =
                    std::inner_product(A.row_begin(k), A.row_end(k),
                                       A.row_begin(k), 0.0);
                fixed[k] = -0.5*variance;
            }
            fixedDrifts_.push_back(fixed);
        }

        calculators_.front().compute(initialForwards, initialDrifts_);
        //calculators_.front().computeReduced(initialForwards, F_, initialDrifts_);
    }

	ForwardRatePcEvolver::~ForwardRatePcEvolver() {}

    Real ForwardRatePcEvolver::startNewPath() {
        currentStep_ = 0;
        std::copy(initialLogForwards_.begin(), initialLogForwards_.end(),
                  logForwards_.begin());
        return generator_->nextPath();
    }

    Real ForwardRatePcEvolver::advanceStep()
    {
        // we're going from T1 to T2

        // a) compute drifts D1 at T1;
        if (currentStep_ > 0) {
            calculators_[currentStep_].compute(forwards_, drifts1_);
            //calculators_[currentStep_].computeReduced(forwards_, F_, drifts1_);
        } else {
            std::copy(initialDrifts_.begin(), initialDrifts_.end(),
                      drifts1_.begin());
        }

        // b) evolve forwards up to T2 using D1;
        Real weight = generator_->nextStep(brownians_);
        const Matrix& A = pseudoRoot_->pseudoRoot(currentStep_);
        const std::vector<Real>& fixedDrift = fixedDrifts_[currentStep_];

        Size alive = alive_[currentStep_];
        for (Size i=alive; i<n_; i++) {
            logForwards_[i] += drifts1_[i] + fixedDrift[i];
            logForwards_[i] +=
                std::inner_product(A.row_begin(i), A.row_end(i),
                                   brownians_.begin(), 0.0);
            forwards_[i] = std::exp(logForwards_[i]) - displacements_[i];
        }

        // c) recompute drifts D2 using the predicted forwards;
        calculators_[currentStep_].compute(forwards_, drifts2_);
        //calculators_[currentStep_].computeReduced(forwards_, F_, drifts2_);
        
        // d) correct forwards using both drifts
        for (Size i=alive; i<n_; ++i) {
            logForwards_[i] += (drifts2_[i]-drifts1_[i])/2.0;
            forwards_[i] = std::exp(logForwards_[i]) - displacements_[i];
        }

        // e) update curve state
        curveState_.setOnForwardRates(forwards_);

        ++currentStep_;

        return weight;
    }

}
