/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Instruments/oneassetstrikedoption.hpp>
#include <ql/Instruments/payoffs.hpp>

namespace QuantLib {

    OneAssetStrikedOption::OneAssetStrikedOption(
        const boost::shared_ptr<StochasticProcess>& process,
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise,
        const boost::shared_ptr<PricingEngine>& engine)
    : OneAssetOption(process, payoff, exercise, engine) {}


    Real OneAssetStrikedOption::strikeSensitivity() const {
        calculate();
        QL_REQUIRE(strikeSensitivity_ != Null<Real>(),
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

    void OneAssetStrikedOption::setupExpired() const {
        OneAssetOption::setupExpired();
        strikeSensitivity_ = 0.0;
    }

    void OneAssetStrikedOption::fetchResults(const Results* r) const {
        OneAssetOption::fetchResults(r);
        const MoreGreeks* results = dynamic_cast<const MoreGreeks*>(r);
        QL_ENSURE(results != 0,
                  "no more-greeks returned from pricing engine");
        strikeSensitivity_ = results->strikeSensitivity;
    }

}

