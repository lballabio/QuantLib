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

#include <ql/auto_ptr.hpp>
#include <ql/models/marketmodels/callability/triggeredswapexercise.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <utility>

namespace QuantLib {

    TriggeredSwapExercise::TriggeredSwapExercise(const std::vector<Time>& rateTimes,
                                                 const std::vector<Time>& exerciseTimes,
                                                 std::vector<Rate> strikes)
    : rateTimes_(rateTimes), exerciseTimes_(exerciseTimes), strikes_(std::move(strikes)),
      currentStep_(0), rateIndex_(exerciseTimes.size()), evolution_(rateTimes, exerciseTimes) {
        Size j = 0;
        for (Size i=0; i<exerciseTimes.size(); ++i) {
            while (j < rateTimes.size() && rateTimes[j] < exerciseTimes[i])
                ++j;
            rateIndex_[i] = j;
        }
    }

    Size TriggeredSwapExercise::numberOfExercises() const {
        return exerciseTimes_.size();
    }

    const EvolutionDescription& TriggeredSwapExercise::evolution() const {
        return evolution_;
    }

    void TriggeredSwapExercise::nextStep(const CurveState&) {
        ++currentStep_;
    }

    void TriggeredSwapExercise::reset() {
        currentStep_ = 0;
    }

    std::valarray<bool> TriggeredSwapExercise::isExerciseTime() const {
        return std::valarray<bool>(true,numberOfExercises());
    }

    void TriggeredSwapExercise::values(const CurveState& state,
                                       std::vector<Real>& results) const {
        Size swapIndex = rateIndex_[currentStep_-1];
        results.resize(1);
        results[0] = state.coterminalSwapRate(swapIndex);
    }

    std::vector<Size> TriggeredSwapExercise::numberOfVariables() const {
        return std::vector<Size>(numberOfExercises(), 1);
    }

    std::vector<Size> TriggeredSwapExercise::numberOfParameters() const {
        return std::vector<Size>(numberOfExercises(), 1);
    }

    bool TriggeredSwapExercise::exercise(
                                   Size,
                                   const std::vector<Real>& parameters,
                                   const std::vector<Real>& variables) const {
        return variables[0] >= parameters[0];
    }

    void TriggeredSwapExercise::guess(Size exerciseIndex,
                                      std::vector<Real>& parameters) const {
        parameters.resize(1);
        parameters[0] = strikes_.at(exerciseIndex);
    }

    QL_UNIQUE_OR_AUTO_PTR<MarketModelParametricExercise>
    TriggeredSwapExercise::clone() const {
        return QL_UNIQUE_OR_AUTO_PTR<MarketModelParametricExercise>(
                                            new TriggeredSwapExercise(*this));
    }

}

