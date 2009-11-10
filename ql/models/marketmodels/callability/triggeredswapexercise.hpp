/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

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

#ifndef quantlib_triggered_swap_exercise_hpp
#define quantlib_triggered_swap_exercise_hpp

#include <ql/models/marketmodels/callability/marketmodelparametricexercise.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>

namespace QuantLib {

    class TriggeredSwapExercise : public MarketModelParametricExercise {
      public:
        TriggeredSwapExercise(const std::vector<Time>& rateTimes,
                              const std::vector<Time>& exerciseTimes,
                              const std::vector<Rate>& strikes);

        // NodeDataProvider interface
        Size numberOfExercises() const;
        const EvolutionDescription& evolution() const;
        void nextStep(const CurveState&);
        void reset();
        std::valarray<bool> isExerciseTime() const;
        void values(const CurveState&,
                    std::vector<Real>& results) const;

        // ParametricExercise interface
        std::vector<Size> numberOfVariables() const;
        std::vector<Size> numberOfParameters() const;
        bool exercise(Size exerciseNumber,
                      const std::vector<Real>& parameters,
                      const std::vector<Real>& variables) const;
        void guess(Size exerciseNumber,
                   std::vector<Real>& parameters) const;

        std::auto_ptr<MarketModelParametricExercise> clone() const;

      private:
        std::vector<Time> rateTimes_, exerciseTimes_;
        std::vector<Rate> strikes_;
        Size currentStep_;
        std::vector<Size> rateIndex_;
        EvolutionDescription evolution_;
    };

}

#endif
