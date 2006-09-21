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


#ifndef quantlib_longstaff_schwartz_exercise_strategy_hpp
#define quantlib_longstaff_schwartz_exercise_strategy_hpp

#include <ql/MonteCarlo/exercisestrategy.hpp>
#include <ql/MarketModels/lsbasisfunctions.hpp>
#include <ql/MarketModels/exercisevalue.hpp>
#include <ql/MarketModels/accountingengine.hpp>

namespace QuantLib {

    class LongstaffSchwartzExerciseStrategy
        : public ExerciseStrategy<CurveState> {
      public:
        LongstaffSchwartzExerciseStrategy(
                const boost::shared_ptr<MarketModelBasisSystem>& basisSystem,
                const std::vector<std::vector<Real> >& basisCoefficients,
                const EvolutionDescription& evolution,
                const boost::shared_ptr<MarketModelExerciseValue>& exercise,
                const boost::shared_ptr<MarketModelExerciseValue>& control);
        std::vector<Time> exerciseTimes() const;
        std::vector<Time> relevantTimes() const;
        void reset();
        bool exercise(const CurveState& currentState) const;
        void nextStep(const CurveState& currentState);
      private:
        boost::shared_ptr<MarketModelBasisSystem> basisSystem_;
        std::vector<std::vector<Real> > basisCoefficients_;
        boost::shared_ptr<MarketModelExerciseValue> exercise_;
        boost::shared_ptr<MarketModelExerciseValue> control_;
        std::vector<Size> numeraires_;
        // work variable
        Size currentIndex_;
        Real principalInNumerairePortfolio_, newPrincipal_;
        std::vector<Time> exerciseTimes_, relevantTimes_;
        std::vector<bool> isBasisTime_, isRebateTime_, isControlTime_;
        std::vector<bool> isExerciseTime_;
        std::vector<MarketModelDiscounter> rebateDiscounters_;
        std::vector<MarketModelDiscounter> controlDiscounters_;
        mutable std::vector<std::vector<Real> > basisValues_;
        std::vector<Size> exerciseIndex_;
    };

}


#endif
