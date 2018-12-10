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

#ifndef quantlib_swap_rate_trigger_hpp
#define quantlib_swap_rate_trigger_hpp

#include <ql/methods/montecarlo/exercisestrategy.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <vector>

namespace QuantLib {

    class SwapRateTrigger : public ExerciseStrategy<CurveState> {
      public:
        SwapRateTrigger(const std::vector<Time>& rateTimes,
                        const std::vector<Rate>& swapTriggers,
                        const std::vector<Time>& exerciseTimes);
        std::vector<Time> exerciseTimes() const;
        std::vector<Time> relevantTimes() const;
        void reset();
        bool exercise(const CurveState& currentState) const;
        void nextStep(const CurveState& currentState);
        #if defined(QL_USE_STD_UNIQUE_PTR)
        std::unique_ptr<ExerciseStrategy<CurveState> > clone() const;
        #else
        std::auto_ptr<ExerciseStrategy<CurveState> > clone() const;
        #endif
      private:
        std::vector<Time> rateTimes_;
        std::vector<Rate> swapTriggers_;
        std::vector<Time> exerciseTimes_;
        Size currentIndex_;
        std::vector<Size> rateIndex_;
    };

}


#endif
