
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file asianoption.cpp
    \brief Asian option (average rate) on a single asset
*/

#include <ql/Instruments/asianoption.hpp>

namespace QuantLib {

    DiscreteAveragingAsianOption::DiscreteAveragingAsianOption(
                         Average::Type averageType,
                         Option::Type type,
                         const RelinkableHandle<Quote>& underlying,
                         double strike,
                         double runningAverage,
                         double pastWeight,
                         std::vector<Time> fixingTimes,
                         const RelinkableHandle<TermStructure>& dividendTS,
                         const RelinkableHandle<TermStructure>& riskFreeTS,
                         const Exercise& exercise,
                         const RelinkableHandle<BlackVolTermStructure>& volTS,
                         const Handle<PricingEngine>& engine,
                         const std::string& isinCode,
                         const std::string& description)
    : OneAssetStrikedOption(type, underlying, strike, dividendTS, riskFreeTS,
      exercise, volTS, engine, isinCode, description),
      averageType_(averageType), 
      runningAverage_(runningAverage), pastWeight_(pastWeight),
      fixingTimes_(fixingTimes) {}



    void DiscreteAveragingAsianOption::setupArguments(Arguments* args) const {

        DiscreteAveragingAsianOption::arguments* moreArgs =
            dynamic_cast<DiscreteAveragingAsianOption::arguments*>(args);
        QL_REQUIRE(moreArgs != 0,
                   "DiscreteAveragingAsianOption::setupArguments : "
                   "wrong argument type");
        moreArgs->averageType = averageType_;
        moreArgs->runningAverage = runningAverage_;
        moreArgs->pastWeight = pastWeight_;
        moreArgs->fixingTimes = fixingTimes_;

        OneAssetStrikedOption::arguments* arguments =
            dynamic_cast<OneAssetStrikedOption::arguments*>(args);
        QL_REQUIRE(arguments != 0,
                   "DiscreteAveragingAsianOption::setupArguments : "
                   "wrong argument type");
        OneAssetStrikedOption::setupArguments(arguments);

    }

    void DiscreteAveragingAsianOption::performCalculations() const {
        // enforce in this class any check on engine/payoff coupling
        OneAssetStrikedOption::performCalculations();
    }

    void DiscreteAveragingAsianOption::arguments::validate() const {

        #if defined(QL_PATCH_MICROSOFT)
        OneAssetStrikedOption::arguments copy = *this;
        copy.validate();
        #else
        OneAssetStrikedOption::arguments::validate();
        #endif

        QL_REQUIRE(runningAverage >= 0.0,
                   "DiscreteAveragingAsianOption::arguments::validate() : "
                   "negative running average");
        QL_REQUIRE(pastWeight >= 0.0,
                   "DiscreteAveragingAsianOption::arguments::validate() : "
                   "negative weight to past observations");
        QL_REQUIRE(pastWeight <= 1.0,
                   "DiscreteAveragingAsianOption::arguments::validate() : "
                   "past observations' weight > 1");


        // check fixingTimes_ here
    }

}

