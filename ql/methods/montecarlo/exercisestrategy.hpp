/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

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


#ifndef quantlib_exercise_strategy_hpp
#define quantlib_exercise_strategy_hpp

#include <ql/types.hpp>
#include <vector>
#include <memory>

namespace QuantLib {

    template <class State>
    class ExerciseStrategy {
      public:
        virtual ~ExerciseStrategy() = default;
        virtual std::vector<Time> exerciseTimes() const = 0;
        virtual std::vector<Time> relevantTimes() const = 0;
        virtual void reset() = 0;
        virtual bool exercise(const State& currentState) const = 0;
        virtual void nextStep(const State& currentState) = 0;
        virtual std::unique_ptr<ExerciseStrategy<State> > clone() const = 0;
    };

}


#endif
