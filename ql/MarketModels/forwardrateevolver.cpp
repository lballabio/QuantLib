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
                           const boost::shared_ptr<PseudoRoot> pseudoRoot,
                           const EvolutionDescription& evolution,
                           const BrownianGeneratorFactory& factory,
                           const Array& initialForwards,
                           const Array& displacements)
    : pseudoRoot_(pseudoRoot), evolution_(evolution),
      initialForwards_(initialForwards), displacements_(displacements),
      curveState_(evolution.rateTimes()),
      forwards_(initialForwards), logForwards_(initialForwards.size()),
      initialLogForwards_(initialForwards.size()),
      drifts_(initialForwards.size()), initialDrifts_(initialForwards.size()),
      alive_(evolution.evolutionTimes().size()) {
        
        Size factors = pseudoRoot_->numberOfFactors();
        Size steps = evolution_.numberOfSteps();
        Size n = initialForwards.size();

        generator_ = factory.create(factors, steps);
        currentStep_ = 0;

        for (Size i=0; i<n; ++i) {
            initialLogForwards_[i] = std::log(initialForwards[i] +
                                              displacements[i]);
        }

        Time lastTime = 0.0;
        for (Size j=0; j<steps; ++j) {
            Size alive = 0;
            while (evolution_.rateTimes()[alive] <= lastTime)
                ++alive;

            calculators_.push_back(DriftCalculator(pseudoRoot_->pseudoRoot(j),
                                                   displacements,
                                                   evolution_.taus(),
                                                   evolution_.numeraires()[j],
                                                   alive));
            alive_[j] = alive;
            lastTime = evolution_.evolutionTimes()[j];
        }

        calculators_.front().compute(initialForwards_, initialDrifts_);

    }

    Real ForwardRateEvolver::startNewPath() {
        currentStep_ = 0;
        std::copy(initialLogForwards_.begin(), initialLogForwards_.end(),
                  logForwards_.begin());
        return 1.0;
    }

    Real ForwardRateEvolver::advanceStep() {
        
        return 1.0;
    }


    const CurveState& ForwardRateEvolver::currentState() const {
        return curveState_;
    }

}
