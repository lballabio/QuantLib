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


#ifndef quantlib_multistep_exercise_adapter_hpp
#define quantlib_multistep_exercise_adapter_hpp

#include <ql/MarketModels/Products/multiproductmultistep.hpp>
#include <ql/MarketModels/exercisevalue.hpp>

namespace QuantLib {

    class ExerciseAdapter : public MultiProductMultiStep {
      public:
        ExerciseAdapter(
            const boost::shared_ptr<MarketModelExerciseValue>& exercise,
            Size numberOfProducts = 1);
        //! \name MarketModelMultiProduct interface
        //@{
        std::vector<Time> possibleCashFlowTimes() const;
        Size numberOfProducts() const;
        EvolutionDescription suggestedEvolution() const;
        Size maxNumberOfCashFlowsPerProductPerStep() const;
        void reset(); 
        bool nextTimeStep(const CurveState&, 
                          std::vector<Size>&,
                          std::vector<std::vector<CashFlow> >&);
        //@}
      private:
        boost::shared_ptr<MarketModelExerciseValue> exercise_;
        Size numberOfProducts_;
        std::vector<bool> isExerciseTime_;
        Size currentIndex_;
    };

    // inline 

    inline std::vector<Time>
    ExerciseAdapter::possibleCashFlowTimes() const {
        return exercise_->possibleCashFlowTimes();
    }

    inline Size ExerciseAdapter::numberOfProducts() const {
        return numberOfProducts_;
    }

    inline EvolutionDescription ExerciseAdapter::suggestedEvolution() const {
        return exercise_->evolution();
    }

    inline Size
    ExerciseAdapter::maxNumberOfCashFlowsPerProductPerStep() const {
        return 1;
    }

    inline void ExerciseAdapter::reset() {
        exercise_->reset();
        currentIndex_ = 0;
    }

}

#endif
