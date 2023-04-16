/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006, 2007 Mark Joshi

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

#include <ql/models/marketmodels/evolvers/lognormalcmswapratepc.hpp>
#include <ql/models/marketmodels/marketmodel.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/browniangenerator.hpp>
#include <ql/models/marketmodels/driftcomputation/cmsmmdriftcalculator.hpp>

namespace QuantLib {

    LogNormalCmSwapRatePc::LogNormalCmSwapRatePc(
                           const Size spanningForwards,
                           const std::shared_ptr<MarketModel>& marketModel,
                           const BrownianGeneratorFactory& factory,
                           const std::vector<Size>& numeraires,
                           Size initialStep)
    : spanningForwards_(spanningForwards),
      marketModel_(marketModel),
      numeraires_(numeraires),
      initialStep_(initialStep),
      numberOfRates_(marketModel->numberOfRates()),
      numberOfFactors_(marketModel_->numberOfFactors()),
      curveState_(marketModel->evolution().rateTimes(), spanningForwards),
      swapRates_(marketModel->initialRates()),
      displacements_(marketModel->displacements()),
      logSwapRates_(numberOfRates_), initialLogSwapRates_(numberOfRates_),
      drifts1_(numberOfRates_), drifts2_(numberOfRates_),
      initialDrifts_(numberOfRates_), brownians_(numberOfFactors_),
      correlatedBrownians_(numberOfRates_),
      alive_(marketModel->evolution().firstAliveRate())
    {
        checkCompatibility(marketModel->evolution(), numeraires);

        Size steps = marketModel->evolution().numberOfSteps();

        generator_ = factory.create(numberOfFactors_, steps-initialStep_);

        currentStep_ = initialStep_;

        calculators_.reserve(steps);
        fixedDrifts_.reserve(steps);
        for (Size j=0; j<steps; ++j) {
            const Matrix& A = marketModel_->pseudoRoot(j);
            calculators_.emplace_back(A, displacements_, marketModel->evolution().rateTaus(),
                                      numeraires[j], alive_[j], spanningForwards);
            std::vector<Real> fixed(numberOfRates_);
            for (Size k=0; k<numberOfRates_; ++k) {
                Real variance =
                    std::inner_product(A.row_begin(k), A.row_end(k),
                                       A.row_begin(k), Real(0.0));
                fixed[k] = -0.5*variance;
            }
            fixedDrifts_.push_back(fixed);
        }

        setCMSwapRates(marketModel_->initialRates());
    }

    const std::vector<Size>& LogNormalCmSwapRatePc::numeraires() const {
        return numeraires_;
    }

    void LogNormalCmSwapRatePc::setCMSwapRates(const std::vector<Real>& swapRates)
    {
        QL_REQUIRE(swapRates.size()==numberOfRates_,
                   "mismatch between swapRates and rateTimes");
        for (Size i=0; i<numberOfRates_; ++i)
            initialLogSwapRates_[i] = std::log(swapRates[i] +
                                               displacements_[i]);
        curveState_.setOnCMSwapRates(swapRates);
        calculators_[initialStep_].compute(curveState_, initialDrifts_);
    }

    void LogNormalCmSwapRatePc::setInitialState(const CurveState& cs) {
        const auto* cotcs = dynamic_cast<const CMSwapCurveState*>(&cs);
        const std::vector<Real>& swapRates = cotcs->cmSwapRates(spanningForwards_);
        setCMSwapRates(swapRates);
    }

    Real LogNormalCmSwapRatePc::startNewPath() {
        currentStep_ = initialStep_;
        std::copy(initialLogSwapRates_.begin(), initialLogSwapRates_.end(),
                  logSwapRates_.begin());
        return generator_->nextPath();
    }

    Real LogNormalCmSwapRatePc::advanceStep()
    {
        // we're going from T1 to T2

        // a) compute drifts D1 at T1;
        if (currentStep_ > initialStep_)
            calculators_[currentStep_].compute(curveState_, drifts1_);
        else
            std::copy(initialDrifts_.begin(), initialDrifts_.end(),
                      drifts1_.begin());

        // b) evolve forwards up to T2 using D1;
        Real weight = generator_->nextStep(brownians_);
        const Matrix& A = marketModel_->pseudoRoot(currentStep_);
        const std::vector<Real>& fixedDrift = fixedDrifts_[currentStep_];

        Size i, alive = alive_[currentStep_];
        for (i=alive; i<numberOfRates_; ++i) {
            logSwapRates_[i] += drifts1_[i] + fixedDrift[i];
            logSwapRates_[i] +=
                std::inner_product(A.row_begin(i), A.row_end(i),
                                   brownians_.begin(), Real(0.0));
            swapRates_[i] = std::exp(logSwapRates_[i]) - displacements_[i];
        }

        // intermediate curve state update
        curveState_.setOnCMSwapRates(swapRates_);

        // c) recompute drifts D2 using the predicted forwards;
        calculators_[currentStep_].compute(curveState_, drifts2_);

        // d) correct forwards using both drifts
        for (i=alive; i<numberOfRates_; ++i) {
            logSwapRates_[i] += (drifts2_[i]-drifts1_[i])/2.0;
            swapRates_[i] = std::exp(logSwapRates_[i]) - displacements_[i];
        }

        // e) update curve state
        //curveState_.setOnCMSwapRates(swapRates_, alive);
        curveState_.setOnCMSwapRates(swapRates_);

        ++currentStep_;

        return weight;
    }

    Size LogNormalCmSwapRatePc::currentStep() const {
        return currentStep_;
    }

    const CurveState& LogNormalCmSwapRatePc::currentState() const {
        return curveState_;
    }

}
