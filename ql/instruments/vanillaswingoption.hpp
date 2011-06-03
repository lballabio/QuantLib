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

#include <ql/event.hpp>
#include <ql/exercise.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/instruments/oneassetoption.hpp>

namespace QuantLib {

    //! Swing exercise
    /*! A Swing option can only be exercised at a set of fixed date times
    */
    class SwingExercise : public BermudanExercise {
      public:
        SwingExercise(const std::vector<Date>& dates,
                      const std::vector<Size>& seconds = std::vector<Size>())
        : BermudanExercise(dates),
          seconds_(seconds.empty() ? std::vector<Size>(dates.size(), 0u)
                                   : seconds)
        {
            QL_REQUIRE(dates_.size() == seconds_.size(),
                       "dates and seconds must have the same size");
            for (Size i=0; i < dates_.size(); ++i) {
                QL_REQUIRE(seconds_[i] < 24*3600,
                           "a date can not have more than 24*3600 seconds");
                if (i > 0) {
                    QL_REQUIRE(dates_[i-1] < dates_[i]
                               || (dates_[i-1] == dates_[i]
                                   && seconds_[i-1] < seconds_[i]),
                               "date times must be sorted");
                }
            }
        }

        const std::vector<Size>& seconds() const { return seconds_; }

      private:
        std::vector<Size> seconds_;
    };


    //! base option class
    class VanillaSwingOption : public OneAssetOption {
      public:
          class arguments;
          VanillaSwingOption(const boost::shared_ptr<StrikedTypePayoff>& payoff,
                             const boost::shared_ptr<SwingExercise>& ex,
                             Size minExerciseRights, Size maxExerciseRights)
        : OneAssetOption(payoff, ex),
          minExerciseRights_(minExerciseRights),
          maxExerciseRights_(maxExerciseRights) {}

        bool isExpired() const;
        void setupArguments(PricingEngine::arguments*) const;
        
      private:
        const Size minExerciseRights_, maxExerciseRights_;
    };

    class VanillaSwingOption::arguments 
        : public virtual PricingEngine::arguments {
      public:
        arguments() {}
        void validate() const {
            QL_REQUIRE(payoff, "no payoff given");
            QL_REQUIRE(exercise, "no exercise given");

            QL_REQUIRE(minExerciseRights <= maxExerciseRights,
                       "minExerciseRights <= maxExerciseRights")
            QL_REQUIRE(exercise->dates().size() >= maxExerciseRights,
                       "number of exercise rights exceeds "
                       "number of exercise dates");
        }

        Size minExerciseRights, maxExerciseRights;
        boost::shared_ptr<StrikedTypePayoff> payoff;
        boost::shared_ptr<SwingExercise> exercise;
    };

    inline void VanillaSwingOption::setupArguments(
                                PricingEngine::arguments* args) const {
        VanillaSwingOption::arguments* arguments =
            dynamic_cast<VanillaSwingOption::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");
        
        arguments->payoff 
            = boost::dynamic_pointer_cast<StrikedTypePayoff>(payoff_);
        arguments->exercise 
            = boost::dynamic_pointer_cast<SwingExercise>(exercise_);
        arguments->minExerciseRights = minExerciseRights_;
        arguments->maxExerciseRights = maxExerciseRights_;
    }
    
    inline bool VanillaSwingOption::isExpired() const {
        return detail::simple_event(exercise_->lastDate()).hasOccurred();
    }
}

#endif
