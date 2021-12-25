/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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

/*! \file vanillaswingoption.hpp
    \brief vanilla swing option class
*/

#ifndef quantlib_vanilla_swing_option_hpp
#define quantlib_vanilla_swing_option_hpp

#include <ql/exercise.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/instruments/oneassetoption.hpp>

namespace QuantLib {

    //! Swing exercise
    /*! A Swing option can only be exercised at a set of fixed date times
    */
    class SwingExercise : public BermudanExercise {
      public:
        explicit SwingExercise(const std::vector<Date>& dates,
                               const std::vector<Size>& seconds = std::vector<Size>());
        SwingExercise(const Date& from, const Date& to, Size stepSizeSecs);

        const std::vector<Size>& seconds() const;

        std::vector<Time> exerciseTimes(const DayCounter& dc,
                                        const Date& refDate) const;

      private:
        const std::vector<Size> seconds_;
    };

    class VanillaForwardPayoff : public StrikedTypePayoff {
      public:
        VanillaForwardPayoff(Option::Type type, Real strike)
          : StrikedTypePayoff(type, strike) {}

        std::string name() const override { return "ForwardTypePayoff"; }
        Real operator()(Real price) const override;
        void accept(AcyclicVisitor&) override;
    };

    //! base option class
    class VanillaSwingOption : public OneAssetOption {
      public:
          class arguments;
          VanillaSwingOption(const ext::shared_ptr<Payoff>& payoff,
                             const ext::shared_ptr<SwingExercise>& ex,
                             Size minExerciseRights, Size maxExerciseRights)
        : OneAssetOption(payoff, ex),
          minExerciseRights_(minExerciseRights),
          maxExerciseRights_(maxExerciseRights) {}

          bool isExpired() const override;
          void setupArguments(PricingEngine::arguments*) const override;

        private:
          const Size minExerciseRights_, maxExerciseRights_;
    };

    class VanillaSwingOption::arguments 
        : public virtual PricingEngine::arguments {
      public:
        arguments() = default;
        void validate() const override;

        Size minExerciseRights, maxExerciseRights;
        ext::shared_ptr<StrikedTypePayoff> payoff;
        ext::shared_ptr<SwingExercise> exercise;
    };
}

#endif
