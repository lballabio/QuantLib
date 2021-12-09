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
#include <ql/settings.hpp>
#include <utility>

namespace QuantLib {

    DiscreteAveragingAsianOption::DiscreteAveragingAsianOption(
        Average::Type averageType,
        Real runningAccumulator,
        Size pastFixings,
        std::vector<Date> fixingDates,
        const ext::shared_ptr<StrikedTypePayoff>& payoff,
        const ext::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff, exercise), averageType_(averageType),
      runningAccumulator_(runningAccumulator), pastFixings_(pastFixings),
      fixingDates_(std::move(fixingDates)), allPastFixingsProvided_(false),
      allPastFixings_(std::vector<Real>()) {
        std::sort(fixingDates_.begin(), fixingDates_.end());

        // Add a hard override to the runningAccumulator if pastFixings is 0
        // (ie. the option is unseasoned)
        if (pastFixings_ == 0) {
            if (averageType == Average::Geometric) {
                runningAccumulator_ = 1.0;
            } else if (averageType == Average::Arithmetic) {
                runningAccumulator_ = 0.0;
            } else {
                QL_FAIL("Unrecognised average type, must be Average::Arithmetic or Average::Geometric");
            }
        }
    }

    DiscreteAveragingAsianOption::DiscreteAveragingAsianOption(
        Average::Type averageType,
        std::vector<Date> fixingDates,
        const ext::shared_ptr<StrikedTypePayoff>& payoff,
        const ext::shared_ptr<Exercise>& exercise,
        std::vector<Real> allPastFixings)
    : OneAssetOption(payoff, exercise), averageType_(averageType), runningAccumulator_(0.0),
      pastFixings_(0), fixingDates_(std::move(std::move(fixingDates))),
      allPastFixingsProvided_(true), allPastFixings_(std::move(allPastFixings)) {}

    void DiscreteAveragingAsianOption::setupArguments(
                                       PricingEngine::arguments* args) const {

        Real runningAccumulator = runningAccumulator_;
        Size pastFixings = pastFixings_;
        std::vector<Date> fixingDates = fixingDates_;

        // If the option was initialised with a list of fixings, before pricing we
        // compare the evaluation date to the fixing dates, and set up the pastFixings,
        // fixingDates, and runningAccumulator accordingly
        if (allPastFixingsProvided_) {
            std::vector<Date> futureFixingDates = std::vector<Date>();
            Date today = Settings::instance().evaluationDate();

            pastFixings = 0;
            for (auto fixingDate : fixingDates_) {
                if (fixingDate < today) {
                    pastFixings += 1;
                } else {
                    futureFixingDates.push_back(fixingDate);
                }
            }
            fixingDates = futureFixingDates;

            if (pastFixings > allPastFixings_.size())
                QL_FAIL("Not enough past fixings have been provided for the required historical fixing dates");

            if (averageType_ == Average::Geometric) {
                runningAccumulator = 1.0;
                for (Size i=0; i<pastFixings; i++)
                    runningAccumulator *= allPastFixings_[i];

            } else if (averageType_ == Average::Arithmetic) {
                runningAccumulator = 0.0;
                for (Size i=0; i<pastFixings; i++)
                    runningAccumulator += allPastFixings_[i];

            } else {
                QL_FAIL("Unrecognised average type, must be Average::Arithmetic or Average::Geometric");
            }

        }

        OneAssetOption::setupArguments(args);

        auto* moreArgs = dynamic_cast<DiscreteAveragingAsianOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");
        moreArgs->averageType = averageType_;
        moreArgs->runningAccumulator = runningAccumulator;
        moreArgs->pastFixings = pastFixings;
        moreArgs->fixingDates = fixingDates;
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
        const ext::shared_ptr<StrikedTypePayoff>& payoff,
        const ext::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff, exercise),
      averageType_(averageType) {}

    void ContinuousAveragingAsianOption::setupArguments(
                                       PricingEngine::arguments* args) const {

        OneAssetOption::setupArguments(args);

        auto* moreArgs = dynamic_cast<ContinuousAveragingAsianOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");
        moreArgs->averageType = averageType_;
    }

    void ContinuousAveragingAsianOption::arguments::validate() const {

        OneAssetOption::arguments::validate();

        QL_REQUIRE(Integer(averageType) != -1, "unspecified average type");
    }

}

