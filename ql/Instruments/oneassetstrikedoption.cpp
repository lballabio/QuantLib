
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/Instruments/oneassetstrikedoption.hpp>
#include <ql/Instruments/payoffs.hpp>

namespace QuantLib {

    OneAssetStrikedOption::OneAssetStrikedOption(
        const boost::shared_ptr<BlackScholesStochasticProcess>& stochProc,
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise,
        const boost::shared_ptr<PricingEngine>& engine)
    : OneAssetOption(stochProc, payoff, exercise, engine) {}


    double OneAssetStrikedOption::strikeSensitivity() const {
        calculate();
        QL_REQUIRE(strikeSensitivity_ != Null<double>(),
                   "strike sensitivity not provided");
        return strikeSensitivity_;
    }

    void OneAssetStrikedOption::setupArguments(Arguments* args) const {

        OneAssetOption::setupArguments(args);

        OneAssetStrikedOption::arguments* moreArgs =
            dynamic_cast<OneAssetStrikedOption::arguments*>(args);
        QL_REQUIRE(moreArgs != 0,
                   "wrong argument type");
        moreArgs->payoff = payoff_;
    }

    void OneAssetStrikedOption::performCalculations() const {
        OneAssetOption::performCalculations();
        const MoreGreeks* results =
            dynamic_cast<const MoreGreeks*>(engine_->results());
        QL_ENSURE(results != 0,
                  "no more-greeks returned from pricing engine");
        /* no check on null values - just copy.
           this allows:
           a) to decide in derived options what to do when null
           results are returned (throw? numerical calculation?)
           b) to implement slim engines which only calculate the
           value---of course care must be taken not to call
           the greeks methods when using these.
        */
        strikeSensitivity_ = results->strikeSensitivity;
    }

}

