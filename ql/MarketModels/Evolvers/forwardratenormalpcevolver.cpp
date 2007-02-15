/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Chiara Fornarola

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

#include <ql/MarketModels/Evolvers/forwardratenormalpcevolver.hpp>
#include <ql/MarketModels/duffsdeviceinnerproduct.hpp>

namespace QuantLib {

    ForwardRateNormalPcEvolver::ForwardRateNormalPcEvolver(
                           const boost::shared_ptr<MarketModel>& marketModel,
                           const BrownianGeneratorFactory& factory,
                           const std::vector<Size>& numeraires,
                           Size initialStep)
    : marketModel_(marketModel),
      numeraires_(numeraires),
      initialStep_(initialStep),
      n_(marketModel->numberOfRates()), F_(marketModel_->numberOfFactors()),
      curveState_(marketModel->evolution().rateTimes()),
      forwards_(marketModel->initialRates()),
      initialForwards_(marketModel->initialRates()),
      drifts1_(n_), drifts2_(n_),
      initialDrifts_(n_), brownians_(F_), correlatedBrownians_(n_),
      alive_(marketModel->evolution().firstAliveRate())
    {
        checkCompatibility(marketModel->evolution(), numeraires);

        Size steps = marketModel->evolution().numberOfSteps();

        generator_ = factory.create(F_, steps-initialStep_);

        currentStep_ = initialStep_;

        calculators_.reserve(steps);
        for (Size j=0; j<steps; ++j) {
            const Matrix& A = marketModel_->pseudoRoot(j);
            calculators_.push_back(
                LMMNormalDriftCalculator(A,
                                         marketModel->evolution().rateTaus(),
                                         numeraires[j],
                                         alive_[j]));
            for (Size k=0; k<n_; ++k) {
                Real variance =
                    std::inner_product(A.row_begin(k), A.row_end(k),
                                       A.row_begin(k), 0.0);
            }
        }

        setForwards(marketModel_->initialRates());
    }

    const std::vector<Size>& ForwardRateNormalPcEvolver::numeraires() const {
        return numeraires_;
    }

    void ForwardRateNormalPcEvolver::setForwards(const std::vector<Real>& forwards)
    {
        QL_REQUIRE(forwards.size()==n_,
                   "mismatch between forwards and rateTimes");
        for (Size i=0; i<n_; ++i) 
        calculators_[initialStep_].compute(forwards, initialDrifts_);
    }

    void ForwardRateNormalPcEvolver::setInitialState(const CurveState& cs) {
        setForwards(cs.forwardRates());
    }

    Real ForwardRateNormalPcEvolver::startNewPath() {
        currentStep_ = initialStep_;
        std::copy(initialForwards_.begin(), initialForwards_.end(),
                  forwards_.begin());
        return generator_->nextPath();
    }

    Real ForwardRateNormalPcEvolver::advanceStep()
    {
        // we're going from T1 to T2

        // a) compute drifts D1 at T1;
        if (currentStep_ > initialStep_) {
            calculators_[currentStep_].compute(forwards_, drifts1_);
        } else {
            std::copy(initialDrifts_.begin(), initialDrifts_.end(),
                      drifts1_.begin());
        }

        // b) evolve forwards up to T2 using D1;
        Real weight = generator_->nextStep(brownians_);
        const Matrix& A = marketModel_->pseudoRoot(currentStep_);

        Size i, alive = alive_[currentStep_];
        for (i=alive; i<n_; ++i) {
            forwards_[i] += drifts1_[i] ;
            forwards_[i] +=
                std::inner_product(A.row_begin(i), A.row_end(i),
                                   brownians_.begin(), 0.0);
        }

        // c) recompute drifts D2 using the predicted forwards;
        calculators_[currentStep_].compute(forwards_, drifts2_);

        // d) correct forwards using both drifts
        for (i=alive; i<n_; ++i) {
            forwards_[i] += (drifts2_[i]-drifts1_[i])/2.0;
        }

        // e) update curve state
        curveState_.setOnForwardRates(forwards_);

        ++currentStep_;

        return weight;
    }

    Size ForwardRateNormalPcEvolver::currentStep() const {
        return currentStep_;
    }

    const CurveState& ForwardRateNormalPcEvolver::currentState() const {
        return curveState_;
    }

}
