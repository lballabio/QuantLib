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

#include <ql/MarketModels/swapbasissystem.hpp>

namespace QuantLib {

    SwapBasisSystem::SwapBasisSystem(const std::vector<Time>& rateTimes,
                                     const std::vector<Time>& exerciseTimes)
    : rateTimes_(rateTimes), exerciseTimes_(exerciseTimes),
      rateIndex_(exerciseTimes.size()) {
        Size j = 0;
        for (Size i=0; i<exerciseTimes.size(); ++i) {
            while (j < rateTimes.size() && rateTimes[j] < exerciseTimes[i])
                ++j;
            rateIndex_[i] = j;
        }
        evolution_ = EvolutionDescription(rateTimes_, exerciseTimes_);
    }

    Size SwapBasisSystem::numberOfExercises() const {
        return exerciseTimes_.size();
    }

    std::vector<Size> SwapBasisSystem::numberOfFunctions() const {
        std::vector<Size> sizes(exerciseTimes_.size(), 3);
        if (rateIndex_[exerciseTimes_.size()-1] == rateTimes_.size()-2)
            sizes.back() = 2;
        return sizes;
    }

    const EvolutionDescription& SwapBasisSystem::evolution() const {
        return evolution_;
    }

    void SwapBasisSystem::nextStep(const CurveState&) {
        ++currentIndex_;
    }

    void SwapBasisSystem::reset() {
        currentIndex_ = 0;
    }

    std::vector<bool> SwapBasisSystem::isExerciseTime() const {
        return std::vector<bool>(exerciseTimes_.size(), true);
    }

    void SwapBasisSystem::values(const CurveState& currentState,
                                 std::vector<Real>& results) const {
        Size rateIndex = rateIndex_[currentIndex_-1];

        results.reserve(3);
        results.resize(2);
        results[0] = 1.0;
        results[1] = currentState.forwardRate(rateIndex);
        if (rateIndex < rateTimes_.size()-2)
            results.push_back(currentState.coterminalSwapRate(rateIndex+1));
    }

    std::auto_ptr<MarketModelBasisSystem> SwapBasisSystem::clone() const {
        return std::auto_ptr<MarketModelBasisSystem>(
                                                  new SwapBasisSystem(*this));
    }

}

