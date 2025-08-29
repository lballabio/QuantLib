/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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

#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmbermudanstepcondition.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <utility>

namespace QuantLib {

    FdmBermudanStepCondition::FdmBermudanStepCondition(
        const std::vector<Date>& exerciseDates,
        const Date& referenceDate,
        const DayCounter& dayCounter,
        ext::shared_ptr<FdmMesher> mesher,
        ext::shared_ptr<FdmInnerValueCalculator> calculator)
    : mesher_(std::move(mesher)), calculator_(std::move(calculator)) {

        exerciseTimes_.reserve(exerciseDates.size());
        for (auto exerciseDate : exerciseDates) {
            exerciseTimes_.push_back(dayCounter.yearFraction(referenceDate, exerciseDate));
        }
    }

    const std::vector<Time>& FdmBermudanStepCondition::exerciseTimes() const {
        return exerciseTimes_;
    }
    
    void FdmBermudanStepCondition::applyTo(Array& a, Time t) const {
        if (std::find(exerciseTimes_.begin(), exerciseTimes_.end(), t) 
              != exerciseTimes_.end()) {
            
            QL_REQUIRE(mesher_->layout()->size() == a.size(),
                       "inconsistent array dimensions");

            const Size dims = mesher_->layout()->dim().size();
            Array locations(dims);

            for (const auto& iter : *mesher_->layout()) {
                for (Size i=0; i < dims; ++i)
                    locations[i] = mesher_->location(iter, i);

                const Real innerValue = calculator_->innerValue(iter, t);
                if (innerValue > a[iter.index()]) {
                    a[iter.index()] = innerValue;
                }
            }            
        }
    }
}
