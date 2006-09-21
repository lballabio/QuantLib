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


#ifndef quantlib_bermudan_swaption_exercise_value_hpp
#define quantlib_bermudan_swaption_exercise_value_hpp

#include <ql/MarketModels/exercisevalue.hpp>
#include <ql/option.hpp>

namespace QuantLib {

    class NothingExerciseValue : public MarketModelExerciseValue {
      public:
        //! \todo use Payoff
        NothingExerciseValue(const std::vector<Time>& rateTimes);
        virtual Size numberOfExercises() const = 0;
        // including any time at which state should be updated
        virtual EvolutionDescription evolution() const = 0;
        virtual std::vector<Time> possibleCashFlowTimes() const = 0;
        virtual void nextStep(const CurveState&) = 0;
        virtual void reset() = 0;
        // whether or not evolution times are exercise times
        virtual std::vector<bool> isExerciseTime() const = 0;
        virtual MarketModelMultiProduct::CashFlow value(
                                               const CurveState&) const = 0;
      private:
          Size numberOfExercises_;
          std::vector<Time> rateTimes_;
          // evolving
          Size currentIndex_;
          MarketModelMultiProduct::CashFlow cf_;
    };

}

#endif
