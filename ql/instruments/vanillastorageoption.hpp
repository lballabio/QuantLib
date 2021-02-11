/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file vanillastorageoption.hpp
    \brief vanilla storage option class
*/

#ifndef quantlib_vanilla_storage_option_hpp
#define quantlib_vanilla_storage_option_hpp

#include <ql/event.hpp>
#include <ql/exercise.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/instruments/oneassetoption.hpp>

namespace QuantLib {

    //! base option class
    class VanillaStorageOption : public OneAssetOption {
      public:
          class arguments;
          VanillaStorageOption(const ext::shared_ptr<BermudanExercise>& ex,
                               Real capacity, Real load, Real changeRate)
        : OneAssetOption(ext::shared_ptr<Payoff>(new NullPayoff), ex),
          capacity_  (capacity),
          load_      (load),
          changeRate_(changeRate) {}

          bool isExpired() const override;
          void setupArguments(PricingEngine::arguments*) const override;

        private:
          const Real capacity_;
          const Real load_;
          const Real changeRate_;
    };

    class VanillaStorageOption::arguments
        : public virtual PricingEngine::arguments {
      public:
        arguments() = default;
        void validate() const override {
            QL_REQUIRE(payoff, "no payoff given");
            QL_REQUIRE(exercise, "no exercise given");

            QL_REQUIRE(capacity > 0.0 && changeRate > 0.0 && load >= 0.0,
                      "positive capacity, load and change rate required");
            QL_REQUIRE(load <= capacity && changeRate <= capacity,
                        "illegal values load of changeRate");
        }

        Real capacity;
        Real load;
        Real changeRate;
        ext::shared_ptr<NullPayoff> payoff;
        ext::shared_ptr<BermudanExercise> exercise;
    };

    inline void VanillaStorageOption::setupArguments(
                                PricingEngine::arguments* args) const {
        auto* arguments = dynamic_cast<VanillaStorageOption::arguments*>(args);
        QL_REQUIRE(arguments != nullptr, "wrong argument type");

        arguments->payoff
            = ext::dynamic_pointer_cast<NullPayoff>(payoff_);
        arguments->exercise
            = ext::dynamic_pointer_cast<BermudanExercise>(exercise_);
        arguments->capacity   = capacity_;
        arguments->load       = load_;
        arguments->changeRate = changeRate_;
    }

    inline bool VanillaStorageOption::isExpired() const {
        return detail::simple_event(exercise_->lastDate()).hasOccurred();
    }
}

#endif
