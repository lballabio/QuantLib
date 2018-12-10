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


#ifndef quantlib_multistep_exercise_adapter_hpp
#define quantlib_multistep_exercise_adapter_hpp

#include <ql/models/marketmodels/products/multiproductmultistep.hpp>
#include <ql/models/marketmodels/callability/exercisevalue.hpp>
#include <ql/utilities/clone.hpp>

namespace QuantLib {

    class ExerciseAdapter : public MultiProductMultiStep {
      public:
        ExerciseAdapter(const Clone<MarketModelExerciseValue>& exercise,
                        Size numberOfProducts = 1);
        //! \name MarketModelMultiProduct interface
        //@{
        std::vector<Time> possibleCashFlowTimes() const;
        Size numberOfProducts() const;
        const EvolutionDescription& evolution() const;
        Size maxNumberOfCashFlowsPerProductPerStep() const;
        void reset();
        bool nextTimeStep(const CurveState&,
                          std::vector<Size>&,
                          std::vector<std::vector<CashFlow> >&);
        #if defined(QL_USE_STD_UNIQUE_PTR)
        std::unique_ptr<MarketModelMultiProduct> clone() const;
        #else
        std::auto_ptr<MarketModelMultiProduct> clone() const;
        #endif
        //@}
        //! \name inspectors
        //@{
        const MarketModelExerciseValue& exerciseValue() const;
        //@}
      private:
        Clone<MarketModelExerciseValue> exercise_;
        Size numberOfProducts_;
        std::valarray<bool> isExerciseTime_;
        Size currentIndex_;
    };

    // inline definition

    inline std::vector<Time>
    ExerciseAdapter::possibleCashFlowTimes() const {
        return exercise_->possibleCashFlowTimes();
    }

    inline Size ExerciseAdapter::numberOfProducts() const {
        return numberOfProducts_;
    }

    inline const EvolutionDescription& ExerciseAdapter::evolution() const {
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

    inline const MarketModelExerciseValue&
    ExerciseAdapter::exerciseValue() const {
        return *exercise_;
    }

}

#endif
