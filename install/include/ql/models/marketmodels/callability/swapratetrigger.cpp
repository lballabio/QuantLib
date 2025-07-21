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

#include <ql/models/marketmodels/callability/swapratetrigger.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <utility>

namespace QuantLib {

    SwapRateTrigger::SwapRateTrigger(const std::vector<Time>& rateTimes,
                                     std::vector<Rate> swapTriggers,
                                     const std::vector<Time>& exerciseTimes)
    : rateTimes_(rateTimes), swapTriggers_(std::move(swapTriggers)), exerciseTimes_(exerciseTimes),
      rateIndex_(exerciseTimes.size()) {

        checkIncreasingTimes(rateTimes);
        QL_REQUIRE(rateTimes.size()>1,
                   "Rate times must contain at least two values");

        checkIncreasingTimes(exerciseTimes);

        QL_REQUIRE(swapTriggers_.size()==exerciseTimes_.size(),
                   "swapTriggers/exerciseTimes mismatch");
        Size j = 0;
        for (Size i=0; i<exerciseTimes.size(); ++i) {
            while (j < rateTimes.size() && rateTimes[j] < exerciseTimes[i])
                ++j;
            rateIndex_[i] = j;
        }
    }

    std::vector<Time> SwapRateTrigger::exerciseTimes() const {
        return exerciseTimes_;
    }

    std::vector<Time> SwapRateTrigger::relevantTimes() const {
        return exerciseTimes_;
    }

    void SwapRateTrigger::reset() {
        currentIndex_=0;
    }

    bool SwapRateTrigger::exercise(const CurveState& currentState) const {
        Size rateIndex = rateIndex_[currentIndex_-1];
        Rate currentSwapRate =
            currentState.coterminalSwapRate(rateIndex);
        return swapTriggers_[currentIndex_-1]<currentSwapRate;
    }

    void SwapRateTrigger::nextStep(const CurveState&) {
        ++currentIndex_;
    }

    std::unique_ptr<ExerciseStrategy<CurveState>>
    SwapRateTrigger::clone() const {
        return std::unique_ptr<ExerciseStrategy<CurveState>>(new SwapRateTrigger(*this));
    }

}
