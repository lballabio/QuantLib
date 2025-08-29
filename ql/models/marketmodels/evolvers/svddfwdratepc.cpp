/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/marketmodels/evolvers/svddfwdratepc.hpp>
#include <ql/models/marketmodels/marketmodel.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/browniangenerator.hpp>
#include <ql/models/marketmodels/driftcomputation/lmmdriftcalculator.hpp>
#include <ql/models/marketmodels/evolvers/marketmodelvolprocess.hpp>

namespace QuantLib {

    SVDDFwdRatePc::SVDDFwdRatePc(const ext::shared_ptr<MarketModel>& marketModel,
                           const BrownianGeneratorFactory& factory,
                           const ext::shared_ptr<MarketModelVolProcess>& volProcess,
                           Size firstVolatilityFactor, 
                           Size volatilityFactorStep,
                           const std::vector<Size>& numeraires,
                           Size initialStep )
    : marketModel_(marketModel),
      volProcess_(volProcess),
      firstVolatilityFactor_(firstVolatilityFactor),
      volFactorsPerStep_(volProcess->variatesPerStep()),
      numeraires_(numeraires),
      initialStep_(initialStep),
      isVolVariate_(false,volProcess->variatesPerStep()+marketModel_->numberOfFactors()),
      numberOfRates_(marketModel->numberOfRates()),
      numberOfFactors_(marketModel_->numberOfFactors()),
      curveState_(marketModel->evolution().rateTimes()),
      forwards_(marketModel->initialRates()),
      displacements_(marketModel->displacements()),
      logForwards_(numberOfRates_), initialLogForwards_(numberOfRates_),
      drifts1_(numberOfRates_), drifts2_(numberOfRates_),
      initialDrifts_(numberOfRates_), allBrownians_(volProcess->variatesPerStep()+marketModel_->numberOfFactors()), 
      brownians_(numberOfFactors_),
      volBrownians_(volProcess->variatesPerStep()), 
      correlatedBrownians_(numberOfRates_),
      alive_(marketModel->evolution().firstAliveRate())
    {
        QL_REQUIRE(initialStep ==0, "initial step zero only supported currently. ");
        checkCompatibility(marketModel->evolution(), numeraires);

        Size steps = marketModel->evolution().numberOfSteps();

        generator_ = factory.create(numberOfFactors_+volFactorsPerStep_, steps-initialStep_);

        currentStep_ = initialStep_;

        calculators_.reserve(steps);
        fixedDrifts_.reserve(steps);
        for (Size j=0; j<steps; ++j) 
        {
            const Matrix& A = marketModel_->pseudoRoot(j);
            calculators_.emplace_back(A, displacements_, marketModel->evolution().rateTaus(),
                                      numeraires[j], alive_[j]);
            std::vector<Real> fixed(numberOfRates_);
            for (Size k=0; k<numberOfRates_; ++k) 
            {
                Real variance =
                    std::inner_product(A.row_begin(k), A.row_end(k),
                                       A.row_begin(k), Real(0.0));
                fixed[k] = -0.5*variance;
            }
            fixedDrifts_.push_back(fixed);
        }

        setForwards(marketModel_->initialRates());

        Size variatesPerStep = numberOfFactors_+volFactorsPerStep_;

        firstVolatilityFactor_ = std::min(firstVolatilityFactor_,variatesPerStep - volFactorsPerStep_);

        Size volIncrement = (variatesPerStep - firstVolatilityFactor_)/volFactorsPerStep_;
        
        for (Size i=0; i < volFactorsPerStep_; ++i)
            isVolVariate_[firstVolatilityFactor_+i*volIncrement] = true;
    }

    const std::vector<Size>& SVDDFwdRatePc::numeraires() const {
        return numeraires_;
    }

    void SVDDFwdRatePc::setForwards(const std::vector<Real>& forwards)
    {
        QL_REQUIRE(forwards.size()==numberOfRates_,
                   "mismatch between forwards and rateTimes");
        for (Size i=0; i<numberOfRates_; ++i)
             initialLogForwards_[i] = std::log(forwards[i] +
                                               displacements_[i]);
        calculators_[initialStep_].compute(forwards, initialDrifts_);
    }

    void SVDDFwdRatePc::setInitialState(const CurveState& cs) 
    {
        setForwards(cs.forwardRates());
    }

    Real SVDDFwdRatePc::startNewPath() 
    {
        currentStep_ = initialStep_;
        std::copy(initialLogForwards_.begin(), initialLogForwards_.end(),
                  logForwards_.begin());
        volProcess_->nextPath();
        return  generator_->nextPath();
    }

    Real SVDDFwdRatePc::advanceStep()
    {
        // we're going from T1 to T2

        // a) compute drifts D1 at T1;
        if (currentStep_ > initialStep_) 
        {
            calculators_[currentStep_].compute(forwards_, drifts1_);
        } 
        else 
        {
            std::copy(initialDrifts_.begin(), initialDrifts_.end(),
                      drifts1_.begin());
        }

        // b) evolve forwards up to T2 using D1;
        Real weight = generator_->nextStep(allBrownians_);

        // divide Brownians between vol process and forward process

        for (Size i=0, j=0, k=0; i < allBrownians_.size(); ++i)
            if ( isVolVariate_[i])
            {
                volBrownians_[j] = allBrownians_[i];
                ++j;
            }
            else
            {
                brownians_[k] = allBrownians_[i];
                ++k;
            }


        // get sd for step

        Real weight2 = volProcess_->nextstep(volBrownians_);
        Real sdMultiplier = volProcess_->stepSd();
        Real varianceMultiplier = sdMultiplier*sdMultiplier;

        const Matrix& A = marketModel_->pseudoRoot(currentStep_);
        const std::vector<Real>& fixedDrift = fixedDrifts_[currentStep_];

        Size alive = alive_[currentStep_];
        for (Size i=alive; i<numberOfRates_; ++i) {
            logForwards_[i] += varianceMultiplier*(drifts1_[i] + fixedDrift[i]);
            logForwards_[i] += sdMultiplier*
                std::inner_product(A.row_begin(i), A.row_end(i),
                                   brownians_.begin(), Real(0.0));
            forwards_[i] = std::exp(logForwards_[i]) - displacements_[i];
        }

        // c) recompute drifts D2 using the predicted forwards;
        calculators_[currentStep_].compute(forwards_, drifts2_);

        // d) correct forwards using both drifts
        for (Size i=alive; i<numberOfRates_; ++i) {
            logForwards_[i] += varianceMultiplier*(drifts2_[i]-drifts1_[i])/2.0;
            forwards_[i] = std::exp(logForwards_[i]) - displacements_[i];
        }

        // e) update curve state
        curveState_.setOnForwardRates(forwards_);

        ++currentStep_;

        return weight*weight2;
    }

    Size SVDDFwdRatePc::currentStep() const {
        return currentStep_;
    }

    const CurveState& SVDDFwdRatePc::currentState() const {
        return curveState_;
    }

}
