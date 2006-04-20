/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Warren Chou

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

#include <ql/Instruments/lookbackoption.hpp>

namespace QuantLib {

    ContinuousFloatingLookbackOption::ContinuousFloatingLookbackOption(
        Real minmax,
        const boost::shared_ptr<StochasticProcess>& process,
        const boost::shared_ptr<TypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise,
        const boost::shared_ptr<PricingEngine>& engine)
    : OneAssetOption(process, payoff, exercise, engine),
      minmax_(minmax) {}

    void ContinuousFloatingLookbackOption::setupArguments(Arguments* args)
								 const {

        OneAssetOption::setupArguments(args);

        ContinuousFloatingLookbackOption::arguments* moreArgs =
            dynamic_cast<ContinuousFloatingLookbackOption::arguments*>(args);
        QL_REQUIRE(moreArgs != 0, "wrong argument type");
        moreArgs->payoff = payoff_;
        moreArgs->minmax = minmax_;
    }

    void ContinuousFloatingLookbackOption::arguments::validate() const {

        #if defined(QL_PATCH_MSVC6)
        OneAssetOption::arguments copy = *this;
        copy.validate();
        #else
        OneAssetOption::arguments::validate();
        #endif

        QL_REQUIRE(minmax != Null<Real>(), "null prior extremum");
        QL_REQUIRE(minmax >= 0.0, "nonnegative prior extremum required: "
	           << minmax << " not allowed");
    }


    ContinuousFixedLookbackOption::ContinuousFixedLookbackOption(
        Real minmax,
        const boost::shared_ptr<StochasticProcess>& process,
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise,
        const boost::shared_ptr<PricingEngine>& engine)
    : OneAssetStrikedOption(process, payoff, exercise, engine),
      minmax_(minmax) {}

    void ContinuousFixedLookbackOption::setupArguments(Arguments* args) const {

        OneAssetStrikedOption::setupArguments(args);

        ContinuousFixedLookbackOption::arguments* moreArgs =
            dynamic_cast<ContinuousFixedLookbackOption::arguments*>(args);
        QL_REQUIRE(moreArgs != 0, "wrong argument type");
        moreArgs->minmax = minmax_;
    }

    void ContinuousFixedLookbackOption::arguments::validate() const {

        #if defined(QL_PATCH_MSVC6)
        OneAssetStrikedOption::arguments copy = *this;
        copy.validate();
        #else
        OneAssetStrikedOption::arguments::validate();
        #endif

        QL_REQUIRE(minmax != Null<Real>(), "null prior extremum");
        QL_REQUIRE(minmax >= 0.0, "nonnegative prior extremum required: "
	           << minmax << " not allowed");
    }

}

