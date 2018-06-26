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

#include <ql/experimental/exoticoptions/compoundoption.hpp>

namespace QuantLib {

    CompoundOption::CompoundOption(
                   const ext::shared_ptr<StrikedTypePayoff>& motherPayoff,
                   const ext::shared_ptr<Exercise>& motherExercise,
                   const ext::shared_ptr<StrikedTypePayoff>& daughterPayoff,
                   const ext::shared_ptr<Exercise>& daughterExercise)
    : OneAssetOption(motherPayoff, motherExercise),
      daughterPayoff_(daughterPayoff), daughterExercise_(daughterExercise) {}

    void CompoundOption::setupArguments(PricingEngine::arguments* args) const {
        OneAssetOption::setupArguments(args);

        CompoundOption::arguments* moreArgs =
            dynamic_cast<CompoundOption::arguments*>(args);
        QL_REQUIRE(moreArgs != 0, "wrong argument type");
        moreArgs->daughterPayoff = daughterPayoff_;
        moreArgs->daughterExercise = daughterExercise_;
    }

    void CompoundOption::arguments::validate() const {
        OneAssetOption::arguments::validate();
        QL_REQUIRE(daughterPayoff,
                   "no payoff given for underlying option");
        QL_REQUIRE(daughterExercise,
                   "no exercise given for underlying option");
        QL_REQUIRE(exercise->lastDate() <= daughterExercise->lastDate(),
                   "maturity of compound option exceeds "
                   "maturity of underlying option");
    }

}
