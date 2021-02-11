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

#ifndef quantlib_longstaff_schwartz_exercise_strategy_hpp
#define quantlib_longstaff_schwartz_exercise_strategy_hpp

#include <ql/methods/montecarlo/exercisestrategy.hpp>
#include <ql/models/marketmodels/callability/marketmodelbasissystem.hpp>
#include <ql/models/marketmodels/callability/exercisevalue.hpp>
#include <ql/utilities/clone.hpp>

namespace QuantLib {

    class MarketModelDiscounter;

    class LongstaffSchwartzExerciseStrategy
        : public ExerciseStrategy<CurveState> {
      public:
        LongstaffSchwartzExerciseStrategy(Clone<MarketModelBasisSystem> basisSystem,
                                          std::vector<std::vector<Real> > basisCoefficients,
                                          const EvolutionDescription& evolution,
                                          const std::vector<Size>& numeraires,
                                          Clone<MarketModelExerciseValue> exercise,
                                          Clone<MarketModelExerciseValue> control);
        std::vector<Time> exerciseTimes() const override;
        std::vector<Time> relevantTimes() const override;
        void reset() override;
        bool exercise(const CurveState& currentState) const override;
        void nextStep(const CurveState& currentState) override;
#if defined(QL_USE_STD_UNIQUE_PTR)
        std::unique_ptr<ExerciseStrategy<CurveState> > clone() const override;
#else
        std::auto_ptr<ExerciseStrategy<CurveState> > clone() const;
        #endif
      private:
        Clone<MarketModelBasisSystem> basisSystem_;
        std::vector<std::vector<Real> > basisCoefficients_;
        Clone<MarketModelExerciseValue> exercise_;
        Clone<MarketModelExerciseValue> control_;
        std::vector<Size> numeraires_;
        // work variable
        Size currentIndex_;
        Real principalInNumerairePortfolio_, newPrincipal_;
        std::vector<Time> exerciseTimes_, relevantTimes_;
        std::valarray<bool> isBasisTime_, isRebateTime_, isControlTime_;
        std::valarray<bool> isExerciseTime_;
        std::vector<MarketModelDiscounter> rebateDiscounters_;
        std::vector<MarketModelDiscounter> controlDiscounters_;
        mutable std::vector<std::vector<Real> > basisValues_;
        std::vector<Size> exerciseIndex_;
    };

}


#endif
