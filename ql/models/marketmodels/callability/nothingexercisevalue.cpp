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

#include <ql/errors.hpp>
#include <ql/models/marketmodels/callability/nothingexercisevalue.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <utility>

namespace QuantLib {

    NothingExerciseValue::NothingExerciseValue(const std::vector<Time>& rateTimes,
                                               std::valarray<bool> isExerciseTime)
    : rateTimes_(rateTimes), isExerciseTime_(std::move(isExerciseTime)) {

        checkIncreasingTimes(rateTimes);
        QL_REQUIRE(rateTimes.size() >= 2,
                   "Rate times must contain at least two values");
        cf_.amount = 0.0;
        std::vector<Time> evolutionTimes(rateTimes_);
        evolutionTimes.pop_back();
        evolution_= EvolutionDescription(rateTimes_, evolutionTimes);
        if(isExerciseTime_.size()==0) {
            isExerciseTime_ = std::valarray<bool>(true,rateTimes.empty() ? 0 : rateTimes.size()-1);
        }
        else {
            QL_REQUIRE(isExerciseTime_.size() ==
                           (rateTimes.empty() ? 0 : rateTimes.size() - 1),
                       "isExerciseTime ("
                           << isExerciseTime_.size() << ") must "
                           << "have same size as rateTimes minus 1 ("
                           << (rateTimes.empty() ? 0 : rateTimes.size() - 1)
                           << ")");
        }
        numberOfExercises_ = 0;
        for(Size i=0;i<isExerciseTime_.size();i++)
            if(isExerciseTime_[i])
                ++numberOfExercises_;
    }

    Size NothingExerciseValue::numberOfExercises() const {
        return numberOfExercises_;
    }

    const EvolutionDescription& NothingExerciseValue::evolution() const {
        return evolution_;
    }

    std::vector<Time> NothingExerciseValue::possibleCashFlowTimes() const {
        return rateTimes_;
    }

    void NothingExerciseValue::reset() {
        currentIndex_=0;
    }

    void NothingExerciseValue::nextStep(const CurveState&) {
        cf_.timeIndex = currentIndex_;
        ++currentIndex_;
    }


    std::valarray<bool> NothingExerciseValue::isExerciseTime() const {
        return isExerciseTime_;
    }

    MarketModelMultiProduct::CashFlow
    NothingExerciseValue::value(const CurveState&) const {
         return cf_;
    }

    std::unique_ptr<MarketModelExerciseValue>
    NothingExerciseValue::clone() const {
        return std::unique_ptr<MarketModelExerciseValue>(new NothingExerciseValue(*this));
    }

}
