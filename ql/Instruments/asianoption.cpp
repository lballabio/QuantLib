
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Instruments/asianoption.hpp>

namespace QuantLib {

    DiscreteAveragingAsianOption::DiscreteAveragingAsianOption(
        Average::Type averageType,
        Real runningAccumulator,
        Size pastFixings,
        std::vector<Date> fixingDates,
        const boost::shared_ptr<StochasticProcess>& process,
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise,
        const boost::shared_ptr<PricingEngine>& engine)
    : OneAssetStrikedOption(process, payoff, exercise, engine),
      averageType_(averageType), runningAccumulator_(runningAccumulator),
      pastFixings_(pastFixings), fixingDates_(fixingDates) {
        std::sort(fixingDates_.begin(), fixingDates_.end());
    }

    void DiscreteAveragingAsianOption::setupArguments(Arguments* args) const {

        OneAssetStrikedOption::setupArguments(args);

        DiscreteAveragingAsianOption::arguments* moreArgs =
            dynamic_cast<DiscreteAveragingAsianOption::arguments*>(args);
        QL_REQUIRE(moreArgs != 0, "wrong argument type");
        moreArgs->averageType = averageType_;
        moreArgs->runningAccumulator = runningAccumulator_;
        moreArgs->pastFixings = pastFixings_;
        moreArgs->fixingDates = fixingDates_;
    }

    void DiscreteAveragingAsianOption::arguments::validate() const {

        #if defined(QL_PATCH_MSVC6)
        OneAssetStrikedOption::arguments copy = *this;
        copy.validate();
        #else
        OneAssetStrikedOption::arguments::validate();
        #endif

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
        const boost::shared_ptr<StochasticProcess>& process,
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise,
        const boost::shared_ptr<PricingEngine>& engine)
    : OneAssetStrikedOption(process, payoff, exercise, engine),
      averageType_(averageType) {}

    void ContinuousAveragingAsianOption::setupArguments(Arguments* args)
                                                                     const {
        OneAssetStrikedOption::setupArguments(args);

        ContinuousAveragingAsianOption::arguments* moreArgs =
            dynamic_cast<ContinuousAveragingAsianOption::arguments*>(args);
        QL_REQUIRE(moreArgs != 0, "wrong argument type");
        moreArgs->averageType = averageType_;
    }

    void ContinuousAveragingAsianOption::arguments::validate() const {

        #if defined(QL_PATCH_MSVC6)
        OneAssetStrikedOption::arguments copy = *this;
        copy.validate();
        #else
        OneAssetStrikedOption::arguments::validate();
        #endif

        QL_REQUIRE(Integer(averageType) != -1, "unspecified average type");
    }

}

