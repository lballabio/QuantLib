/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

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

#include <ql/models/marketmodels/callability/marketmodelparametricexercise.hpp>
#include <ql/models/marketmodels/callability/parametricexerciseadapter.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <utility>

namespace QuantLib {

    ParametricExerciseAdapter::ParametricExerciseAdapter(
        const MarketModelParametricExercise& exercise, std::vector<std::vector<Real> > parameters)
    : exercise_(exercise), parameters_(std::move(parameters)),
      isExerciseTime_(exercise.isExerciseTime()), numberOfVariables_(exercise.numberOfVariables()) {
        std::vector<Time> evolutionTimes =
            exercise_->evolution().evolutionTimes();
        for (Size i=0; i<evolutionTimes.size(); ++i) {
            if (isExerciseTime_[i])
                exerciseTimes_.push_back(evolutionTimes[i]);
        }
    }

    std::vector<Time> ParametricExerciseAdapter::exerciseTimes() const {
        return exerciseTimes_;
    }

    std::vector<Time> ParametricExerciseAdapter::relevantTimes() const {
        return exercise_->evolution().evolutionTimes();
    }

    void ParametricExerciseAdapter::reset() {
        exercise_->reset();
        currentStep_ = currentExercise_ = 0;
    }

    void ParametricExerciseAdapter::nextStep(const CurveState& currentState) {
        exercise_->nextStep(currentState);
        if (isExerciseTime_[currentStep_])
            ++currentExercise_;
        ++currentStep_;
    }

    bool ParametricExerciseAdapter::exercise(const CurveState& currentState) const {
        variables_.resize(numberOfVariables_[currentExercise_-1]);
        exercise_->values(currentState, variables_);
        return exercise_->exercise(currentExercise_-1,
                                   parameters_[currentExercise_-1],
                                   variables_);
    }

    std::unique_ptr<ExerciseStrategy<CurveState>> ParametricExerciseAdapter::clone() const {
        return std::unique_ptr<ExerciseStrategy<CurveState>>(new ParametricExerciseAdapter(*this));
    }

}
