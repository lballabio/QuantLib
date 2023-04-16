/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Dimitri Reiswich

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

/*! \file compoundoption.hpp
    \brief Compound option on a single asset
*/

#ifndef quantlib_compound_option_hpp
#define quantlib_compound_option_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    //! %Compound option on a single asset.
    /*! \ingroup instruments */
    class CompoundOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        // Mother is the compound Option.
        // Daughter is the option which plays the role of the underlying.
        CompoundOption(const std::shared_ptr<StrikedTypePayoff>& motherPayoff,
                       const std::shared_ptr<Exercise>& motherExercise,
                       std::shared_ptr<StrikedTypePayoff> daughterPayoff,
                       std::shared_ptr<Exercise> daughterExercise);
        void setupArguments(PricingEngine::arguments*) const override;

      private:
        std::shared_ptr<StrikedTypePayoff> daughterPayoff_;
        std::shared_ptr<Exercise> daughterExercise_;
    };

    class CompoundOption::arguments : public OneAssetOption::arguments {
      public:
        std::shared_ptr<StrikedTypePayoff> daughterPayoff;
        std::shared_ptr<Exercise> daughterExercise;
        void validate() const override;
    };

    //! %Compound-option %engine base class
    class CompoundOption::engine
        : public GenericEngine<CompoundOption::arguments,
                               CompoundOption::results> {};

}

#endif
