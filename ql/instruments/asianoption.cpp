/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/instruments/asianoption.hpp>
#include <ql/time/date.hpp>

namespace QuantLib {

    DiscreteAveragingAsianOption::DiscreteAveragingAsianOption(
        Average::Type averageType,
        Real runningAccumulator,
        Size pastFixings,
        const std::vector<Date>& fixingDates,
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff, exercise),
      averageType_(averageType), runningAccumulator_(runningAccumulator),
      pastFixings_(pastFixings), fixingDates_(fixingDates) {
        std::sort(fixingDates_.begin(), fixingDates_.end());
    }

    void DiscreteAveragingAsianOption::setupArguments(
                                       PricingEngine::arguments* args) const {

        OneAssetOption::setupArguments(args);

        DiscreteAveragingAsianOption::arguments* moreArgs =
            dynamic_cast<DiscreteAveragingAsianOption::arguments*>(args);
        QL_REQUIRE(moreArgs != 0, "wrong argument type");
        moreArgs->averageType = averageType_;
        moreArgs->runningAccumulator = runningAccumulator_;
        moreArgs->pastFixings = pastFixings_;
        moreArgs->fixingDates = fixingDates_;
    }

    void DiscreteAveragingAsianOption::arguments::validate() const {

        OneAssetOption::arguments::validate();

        QL_REQUIRE(Integer(averageType) != -1, "unspecified average type");
        QL_REQUIRE(pastFixings != Null<Size>(), "null past-fixing number");
        QL_REQUIRE(runningAccumulator != Null<Real>(), "null running product");
        switch (averageType) {
            case Average::Arithmetic:
                QL_REQUIRE(runningAccumulator >= 0.0,
                           "non negative running sum required: "
                           << runningAccumulator << " not allowed");
                break;
            case Average::Geometric:
                QL_REQUIRE(runningAccumulator > 0.0,
                           "positive running product required: "
                           << runningAccumulator << " not allowed");
                break;
            default:
                QL_FAIL("invalid average type");
        }

        // check fixingTimes_ here
    }




    ContinuousAveragingAsianOption::ContinuousAveragingAsianOption(
        Average::Type averageType,
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff, exercise),
      averageType_(averageType) {}

    void ContinuousAveragingAsianOption::setupArguments(
                                       PricingEngine::arguments* args) const {

        OneAssetOption::setupArguments(args);

        ContinuousAveragingAsianOption::arguments* moreArgs =
            dynamic_cast<ContinuousAveragingAsianOption::arguments*>(args);
        QL_REQUIRE(moreArgs != 0, "wrong argument type");
        moreArgs->averageType = averageType_;
    }

    void ContinuousAveragingAsianOption::arguments::validate() const {

        OneAssetOption::arguments::validate();

        QL_REQUIRE(Integer(averageType) != -1, "unspecified average type");
    }

}

