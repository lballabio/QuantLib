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

#include <ql/models/marketmodels/callability/nothingexercisevalue.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    NothingExerciseValue::NothingExerciseValue(
                                          const std::vector<Time>& rateTimes)
    : numberOfExercises_(rateTimes.empty() ? 0 : rateTimes.size()-1),
      rateTimes_(rateTimes),
      currentIndex_(0) {

        checkIncreasingTimes(rateTimes);
        QL_REQUIRE(numberOfExercises_>0,
                   "Rate times must contain at least two values");
        cf_.amount = 0.0;
        std::vector<Time> evolutionTimes(rateTimes_);
        evolutionTimes.pop_back();
        evolution_= EvolutionDescription(rateTimes_, evolutionTimes);
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
        return std::valarray<bool>(true, numberOfExercises_); // opposite way round from vector constructor
    }

    MarketModelMultiProduct::CashFlow
    NothingExerciseValue::value(const CurveState&) const {
         return cf_;
    }

    std::auto_ptr<MarketModelExerciseValue>
    NothingExerciseValue::clone() const {
        return std::auto_ptr<MarketModelExerciseValue>(
                                             new NothingExerciseValue(*this));
    }

}
