/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/auto_ptr.hpp>
#include <ql/models/marketmodels/callability/lsstrategy.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/discounter.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <utility>

namespace QuantLib {

    typedef MarketModelMultiProduct::CashFlow CashFlow;

    LongstaffSchwartzExerciseStrategy::LongstaffSchwartzExerciseStrategy(
        Clone<MarketModelBasisSystem> basisSystem,
        std::vector<std::vector<Real> > basisCoefficients,
        const EvolutionDescription& evolution,
        const std::vector<Size>& numeraires,
        Clone<MarketModelExerciseValue> exercise,
        Clone<MarketModelExerciseValue> control)
    : basisSystem_(std::move(basisSystem)), basisCoefficients_(std::move(basisCoefficients)),
      exercise_(std::move(exercise)), control_(std::move(control)), numeraires_(numeraires) {

        checkCompatibility(evolution, numeraires);
        relevantTimes_ = evolution.evolutionTimes();

        isBasisTime_.resize(relevantTimes_.size());
        isBasisTime_ = isInSubset(relevantTimes_,
                                  basisSystem_->evolution().evolutionTimes());
        isRebateTime_.resize(relevantTimes_.size());
        isRebateTime_ = isInSubset(relevantTimes_,
                                   exercise_->evolution().evolutionTimes());
        isControlTime_.resize(relevantTimes_.size());
        isControlTime_ = isInSubset(relevantTimes_,
                                    control_->evolution().evolutionTimes());

        exerciseIndex_ = std::vector<Size>(relevantTimes_.size());
        isExerciseTime_.resize(relevantTimes_.size(), false);
        std::valarray<bool> v = exercise_->isExerciseTime();
        Size exercises = 0, idx = 0;
        Size i;
        for (i=0; i<relevantTimes_.size(); ++i) {
            exerciseIndex_[i] = exercises;
            if (isRebateTime_[i]) {
                isExerciseTime_[i] = v[idx++];
                if (isExerciseTime_[i]) {
                    exerciseTimes_.push_back(relevantTimes_[i]);
                    ++exercises;
                }
            }
        }

        const std::vector<Time>& rateTimes = evolution.rateTimes();
        std::vector<Time> rebateTimes = exercise_->possibleCashFlowTimes();
        rebateDiscounters_.reserve(rebateTimes.size());
        for (i=0; i<rebateTimes.size(); ++i)
            rebateDiscounters_.emplace_back(rebateTimes[i], rateTimes);

        std::vector<Time> controlTimes = control_->possibleCashFlowTimes();
        controlDiscounters_.reserve(controlTimes.size());
        for (i=0; i<controlTimes.size(); ++i)
            controlDiscounters_.emplace_back(controlTimes[i], rateTimes);

        std::vector<Size> basisSizes = basisSystem_->numberOfFunctions();
        basisValues_.resize(basisSystem_->numberOfExercises());
        for (i=0; i<basisValues_.size(); ++i)
            basisValues_[i].resize(basisSizes[i]);
    }

    std::vector<Time>
    LongstaffSchwartzExerciseStrategy::exerciseTimes() const {
        return exerciseTimes_;
    }

    std::vector<Time>
    LongstaffSchwartzExerciseStrategy::relevantTimes() const {
        return relevantTimes_;
    }

    void LongstaffSchwartzExerciseStrategy::reset() {
        exercise_->reset();
        control_->reset();
        basisSystem_->reset();
        currentIndex_ = 0;
        principalInNumerairePortfolio_ = newPrincipal_ = 1.0;
    }

    bool LongstaffSchwartzExerciseStrategy::exercise(
                                      const CurveState& currentState) const {

        Size exerciseIndex = exerciseIndex_[currentIndex_-1];

        CashFlow exerciseCF = exercise_->value(currentState);
        Real exerciseValue = exerciseCF.amount *
            rebateDiscounters_[exerciseCF.timeIndex]
                .numeraireBonds(currentState,
                                numeraires_[currentIndex_-1]) /
            principalInNumerairePortfolio_;

        CashFlow controlCF = control_->value(currentState);
        Real controlValue = controlCF.amount *
            controlDiscounters_[controlCF.timeIndex]
                .numeraireBonds(currentState,
                                numeraires_[currentIndex_-1]) /
            principalInNumerairePortfolio_;

        basisSystem_->values(currentState,
                             basisValues_[exerciseIndex]);

        const std::vector<Real>& alphas = basisCoefficients_[exerciseIndex];
        Real continuationValue =
            std::inner_product(alphas.begin(), alphas.end(),
                               basisValues_[exerciseIndex].begin(),
                               controlValue);

        return exerciseValue >= continuationValue;
    }

    void LongstaffSchwartzExerciseStrategy::nextStep(
                                            const CurveState& currentState) {
        principalInNumerairePortfolio_ = newPrincipal_;

        if (isRebateTime_[currentIndex_])
            exercise_->nextStep(currentState);
        if (isControlTime_[currentIndex_])
            control_->nextStep(currentState);
        if (isBasisTime_[currentIndex_])
            basisSystem_->nextStep(currentState);

        if (currentIndex_ < numeraires_.size()-1) {
            Size numeraire = numeraires_[currentIndex_];
            Size nextNumeraire = numeraires_[currentIndex_+1];
            newPrincipal_ *=
                currentState.discountRatio(numeraire, nextNumeraire);
        }

        ++currentIndex_;
    }

    QL_UNIQUE_OR_AUTO_PTR<ExerciseStrategy<CurveState> >
    LongstaffSchwartzExerciseStrategy::clone() const {
        return QL_UNIQUE_OR_AUTO_PTR<ExerciseStrategy<CurveState> >(
                                new LongstaffSchwartzExerciseStrategy(*this));
    }

}

