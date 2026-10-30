/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Warren Chou
 Copyright (C) 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/instruments/lookbackoption.hpp>

namespace QuantLib {

    ContinuousFloatingLookbackOption::ContinuousFloatingLookbackOption(
        Real minmax,
        const ext::shared_ptr<TypePayoff>& payoff,
        const ext::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff, exercise),
      minmax_(minmax) {}

    void ContinuousFloatingLookbackOption::setupArguments(
                                       PricingEngine::arguments* args) const {

        OneAssetOption::setupArguments(args);

        auto* moreArgs = dynamic_cast<ContinuousFloatingLookbackOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");
        moreArgs->minmax = minmax_;
    }

    void ContinuousFloatingLookbackOption::arguments::validate() const {

        OneAssetOption::arguments::validate();

        QL_REQUIRE(minmax != Null<Real>(), "null prior extremum");
        QL_REQUIRE(minmax >= 0.0, "nonnegative prior extremum required: "
                   << minmax << " not allowed");
    }


    ContinuousFixedLookbackOption::ContinuousFixedLookbackOption(
        Real minmax,
        const ext::shared_ptr<StrikedTypePayoff>& payoff,
        const ext::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff, exercise),
      minmax_(minmax) {}

    void ContinuousFixedLookbackOption::setupArguments(
                                       PricingEngine::arguments* args) const {

        OneAssetOption::setupArguments(args);

        auto* moreArgs = dynamic_cast<ContinuousFixedLookbackOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");
        moreArgs->minmax = minmax_;
    }

    void ContinuousFixedLookbackOption::arguments::validate() const {

        OneAssetOption::arguments::validate();

        QL_REQUIRE(minmax != Null<Real>(), "null prior extremum");
        QL_REQUIRE(minmax >= 0.0, "nonnegative prior extremum required: "
                   << minmax << " not allowed");
    }

    ContinuousPartialFloatingLookbackOption::ContinuousPartialFloatingLookbackOption(
        Real minmax,
        Real lambda,
        Date lookbackPeriodEnd,
        const ext::shared_ptr<TypePayoff>& payoff,
        const ext::shared_ptr<Exercise>& exercise)
    : ContinuousFloatingLookbackOption(minmax, payoff, exercise),
      lambda_(lambda),
      lookbackPeriodEnd_(lookbackPeriodEnd) {}

    void ContinuousPartialFloatingLookbackOption::setupArguments(
                                       PricingEngine::arguments* args) const {

        ContinuousFloatingLookbackOption::setupArguments(args);

        auto* moreArgs = dynamic_cast<ContinuousPartialFloatingLookbackOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");
        moreArgs->lambda = lambda_;
        moreArgs->lookbackPeriodEnd = lookbackPeriodEnd_;
    }

    void ContinuousPartialFloatingLookbackOption::arguments::validate() const {

        ContinuousFloatingLookbackOption::arguments::validate();

        ext::shared_ptr<EuropeanExercise> europeanExercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(exercise);
        QL_REQUIRE(lookbackPeriodEnd <= europeanExercise->lastDate(), 
            "lookback start date must be earlier than exercise date");
        
        ext::shared_ptr<FloatingTypePayoff> floatingTypePayoff =
            ext::dynamic_pointer_cast<FloatingTypePayoff>(payoff);
        
        if (floatingTypePayoff->optionType() == Option::Call) {
            QL_REQUIRE(lambda >= 1.0,
                       "lambda should be greater than or equal to 1 for calls");
        }
        if (floatingTypePayoff->optionType() == Option::Put) {
            QL_REQUIRE(lambda <= 1.0,
                       "lambda should be smaller than or equal to 1 for puts");
        }
    }

    ContinuousPartialFixedLookbackOption::ContinuousPartialFixedLookbackOption(
        Date lookbackPeriodStart,
        const ext::shared_ptr<StrikedTypePayoff>& payoff,
        const ext::shared_ptr<Exercise>& exercise)
    : ContinuousFixedLookbackOption(0, payoff, exercise),
      lookbackPeriodStart_(lookbackPeriodStart) {}

    void ContinuousPartialFixedLookbackOption::setupArguments(
                                       PricingEngine::arguments* args) const {

        ContinuousFixedLookbackOption::setupArguments(args);

        auto* moreArgs = dynamic_cast<ContinuousPartialFixedLookbackOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");
        moreArgs->lookbackPeriodStart = lookbackPeriodStart_;
    }

    void ContinuousPartialFixedLookbackOption::arguments::validate() const {

        ContinuousFixedLookbackOption::arguments::validate();

        ext::shared_ptr<EuropeanExercise> europeanExercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(exercise);
        QL_REQUIRE(lookbackPeriodStart <= europeanExercise->lastDate(), 
            "lookback start date must be earlier than exercise date");
    }
}

