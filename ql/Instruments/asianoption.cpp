
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

#include <ql/Instruments/asianoption.hpp>

namespace QuantLib {

    DiscreteAveragingAsianOption::DiscreteAveragingAsianOption(
        Average::Type averageType,
        double runningProduct,
        Size pastFixings,
        std::vector<Date> fixingDates,
        const boost::shared_ptr<BlackScholesStochasticProcess>& stochProc,
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise,
        const boost::shared_ptr<PricingEngine>& engine)
    : OneAssetStrikedOption(stochProc, payoff, exercise, engine), 
      averageType_(averageType), runningProduct_(runningProduct),
      pastFixings_(pastFixings), fixingDates_(fixingDates) {
        std::sort(fixingDates_.begin(), fixingDates_.end());
    }



    void DiscreteAveragingAsianOption::setupArguments(Arguments* args) const {

        OneAssetStrikedOption::setupArguments(args);

        DiscreteAveragingAsianOption::arguments* moreArgs =
            dynamic_cast<DiscreteAveragingAsianOption::arguments*>(args);
        QL_REQUIRE(moreArgs != 0, "wrong argument type");
        moreArgs->averageType = averageType_;
        moreArgs->runningProduct = runningProduct_;
        moreArgs->pastFixings = pastFixings_;
        moreArgs->fixingDates = fixingDates_;

    }

    void DiscreteAveragingAsianOption::arguments::validate() const {

        #if defined(QL_PATCH_MICROSOFT)
        OneAssetStrikedOption::arguments copy = *this;
        copy.validate();
        #else
        OneAssetStrikedOption::arguments::validate();
        #endif

        QL_REQUIRE(runningProduct >= 0.0,
                   "negative running product");

        // check fixingTimes_ here
    }

}

