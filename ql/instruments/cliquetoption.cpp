/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2007 StatPro Italia srl

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

#include <ql/exercise.hpp>
#include <ql/instruments/cliquetoption.hpp>
#include <utility>

namespace QuantLib {

    CliquetOption::CliquetOption(const ext::shared_ptr<PercentageStrikePayoff>& payoff,
                                 const ext::shared_ptr<EuropeanExercise>& maturity,
                                 std::vector<Date> resetDates)
    : OneAssetOption(payoff, maturity), resetDates_(std::move(resetDates)) {}

    void CliquetOption::setupArguments(PricingEngine::arguments* args) const {
        OneAssetOption::setupArguments(args);
        // set accrued coupon, last fixing, caps, floors
        auto* moreArgs = dynamic_cast<CliquetOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong engine type");
        moreArgs->resetDates = resetDates_;
    }

    void CliquetOption::arguments::validate() const {
        OneAssetOption::arguments::validate();

        ext::shared_ptr<PercentageStrikePayoff> moneyness =
            ext::dynamic_pointer_cast<PercentageStrikePayoff>(payoff);
        QL_REQUIRE(moneyness,
                   "wrong payoff type");
        QL_REQUIRE(moneyness->strike() > 0.0,
                   "negative or zero moneyness given");
        QL_REQUIRE(accruedCoupon == Null<Real>() || accruedCoupon >= 0.0,
                   "negative accrued coupon");
        QL_REQUIRE(localCap == Null<Real>() || localCap >= 0.0,
                   "negative local cap");
        QL_REQUIRE(localFloor == Null<Real>() || localFloor >= 0.0,
                   "negative local floor");
        QL_REQUIRE(globalCap == Null<Real>() || globalCap >= 0.0,
                   "negative global cap");
        QL_REQUIRE(globalFloor == Null<Real>() || globalFloor >= 0.0,
                   "negative global floor");
        QL_REQUIRE(!resetDates.empty(),
                   "no reset dates given");
        for (Size i=0; i<resetDates.size(); ++i) {
            QL_REQUIRE(exercise->lastDate() > resetDates[i],
                       "reset date greater or equal to maturity");
            QL_REQUIRE(i == 0 || resetDates[i] > resetDates[i-1],
                       "unsorted reset dates");
        }
    }

}
