/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi

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

#include <ql/models/marketmodels/evolvers/lognormalfwdrateipc.hpp>
#include <ql/models/marketmodels/marketmodel.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/browniangenerator.hpp>
#include <ql/models/marketmodels/driftcomputation/lmmdriftcalculator.hpp>

namespace QuantLib {

    LogNormalFwdRateIpc::LogNormalFwdRateIpc(
                           const boost::shared_ptr<MarketModel>& marketModel,
                           const BrownianGeneratorFactory& factory,
                           const std::vector<Size>& numeraires,
                           Size initialStep)
    : marketModel_(marketModel),
      numeraires_(numeraires),
      initialStep_(initialStep),
      numberOfRates_(marketModel->numberOfRates()),
      numberOfFactors_(marketModel->numberOfFactors()),
      curveState_(marketModel->evolution().rateTimes()),
      forwards_(marketModel->initialRates()),
      displacements_(marketModel->displacements()),
      logForwards_(numberOfRates_), initialLogForwards_(numberOfRates_),
      drifts1_(numberOfRates_), initialDrifts_(numberOfRates_),
      g_(numberOfRates_), brownians_(numberOfFactors_),
      correlatedBrownians_(numberOfRates_),
      rateTaus_(marketModel->evolution().rateTaus()),
      alive_(marketModel->evolution().firstAliveRate())
    {
        checkCompatibility(marketModel->evolution(), numeraires);
        QL_REQUIRE(isInTerminalMeasure(marketModel->evolution(), numeraires),
                   "terminal measure required for ipc ");

        Size steps = marketModel->evolution().numberOfSteps();

        generator_ = factory.create(numberOfFactors_, steps-initialStep_);

        currentStep_ = initialStep_;

        calculators_.reserve(steps);
        fixedDrifts_.reserve(steps);
        for (Size j=0; j<steps; ++j) {
            const Matrix& A = marketModel->pseudoRoot(j);
            calculators_.push_back(LMMDriftCalculator(A,
                                                   displacements_,
                                                   marketModel->evolution().rateTaus(),
                                                   numeraires[j],
                                                   alive_[j]));
            const Matrix& C = marketModel->covariance(j);
            std::vector<Real> fixed(numberOfRates_);
            for (Size k=0; k<numberOfRates_; ++k) {
                Real variance = C[k][k];
                fixed[k] = -0.5*variance;
            }
            fixedDrifts_.push_back(fixed);
        }

        setForwards(marketModel_->initialRates());
    }

    const std::vector<Size>& LogNormalFwdRateIpc::numeraires() const {
        return numeraires_;
    }

    void LogNormalFwdRateIpc::setForwards(const std::vector<Real>& forwards)
    {
        QL_REQUIRE(forwards.size()==numberOfRates_,
                   "mismatch between forwards and rateTimes");
        for (Size i=0; i<numberOfRates_; ++i)
            initialLogForwards_[i] = std::log(forwards[i] +
                                              displacements_[i]);
        calculators_[initialStep_].compute(forwards, initialDrifts_);
    }

    void LogNormalFwdRateIpc::setInitialState(const CurveState& cs) {
        setForwards(cs.forwardRates());
    }

    Real LogNormalFwdRateIpc::startNewPath() {
        currentStep_ = initialStep_;
        std::copy(initialLogForwards_.begin(), initialLogForwards_.end(),
                  logForwards_.begin());
        return generator_->nextPath();
    }

    Real LogNormalFwdRateIpc::advanceStep()
    {
        // we're going from T1 to T2:

        // a) compute drifts D1 at T1;
        if (currentStep_ > initialStep_) {
            calculators_[currentStep_].computePlain(forwards_, drifts1_);
        } else {
            std::copy(initialDrifts_.begin(), initialDrifts_.end(),
                      drifts1_.begin());
        }

        Real weight = generator_->nextStep(brownians_);
        const Matrix& A = marketModel_->pseudoRoot(currentStep_);
        const Matrix& C = marketModel_->covariance(currentStep_);
        const std::vector<Real>& fixedDrift = fixedDrifts_[currentStep_];

        Integer alive = alive_[currentStep_];
        Real drifts2;
        for (Integer i=numberOfRates_-1; i>=alive; --i) {
            drifts2 = 0.0;
            for (Size j=i+1; j<numberOfRates_; ++j) {
                drifts2 -= g_[j]*C[i][j];
            }
            logForwards_[i] += 0.5*(drifts1_[i]+drifts2) + fixedDrift[i];
            logForwards_[i] +=
                std::inner_product(A.row_begin(i), A.row_end(i),
                                   brownians_.begin(), 0.0);
            forwards_[i] = std::exp(logForwards_[i]) - displacements_[i];
            g_[i] = rateTaus_[i]*(forwards_[i]+displacements_[i])/
                (1.0+rateTaus_[i]*forwards_[i]);
        }

        // update curve state
        curveState_.setOnForwardRates(forwards_);

        ++currentStep_;

        return weight;
    }

    Size LogNormalFwdRateIpc::currentStep() const {
        return currentStep_;
    }

    const CurveState& LogNormalFwdRateIpc::currentState() const {
        return curveState_;
    }

}
