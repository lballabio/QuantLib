/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/instruments/margrabeoption.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    MargrabeOption::MargrabeOption(Integer Q1,
                                   Integer Q2,
                                   const ext::shared_ptr<Exercise>& exercise)
    : MultiAssetOption(ext::shared_ptr<Payoff>(new NullPayoff), exercise),
      Q1_(Q1),
      Q2_(Q2) {}

    Real MargrabeOption::delta1() const {
        calculate();
        QL_REQUIRE(delta1_ != Null<Real>(), "delta1 not provided");
        return delta1_;
    }

    Real MargrabeOption::delta2() const {
        calculate();
        QL_REQUIRE(delta2_ != Null<Real>(), "delta2 not provided");
        return delta2_;
    }

    Real MargrabeOption::gamma1() const {
        calculate();
        QL_REQUIRE(gamma1_ != Null<Real>(), "gamma1 not provided");
        return gamma1_;
    }

    Real MargrabeOption::gamma2() const {
        calculate();
        QL_REQUIRE(gamma2_ != Null<Real>(), "gamma2 not provided");
        return gamma2_;
    }

    void MargrabeOption::setupArguments(PricingEngine::arguments* args) const {
        MultiAssetOption::setupArguments(args);

        auto* moreArgs = dynamic_cast<MargrabeOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");

        moreArgs->Q1 = Q1_;
        moreArgs->Q2 = Q2_;
    }

    void MargrabeOption::arguments::validate() const {
        MultiAssetOption::arguments::validate();

        QL_REQUIRE(Q1 != Null<Integer>(), "unspecified quantity for asset 1");
        QL_REQUIRE(Q2 != Null<Integer>(), "unspecified quantity for asset 2");
        QL_REQUIRE(Q1 > 0, "quantity of asset 1 must be positive");
        QL_REQUIRE(Q2 > 0, "quantity of asset 2 must be positive");
    }

    void MargrabeOption::fetchResults(const PricingEngine::results* r) const {
        MultiAssetOption::fetchResults(r);
        const auto* results = dynamic_cast<const MargrabeOption::results*>(r);
        QL_REQUIRE(results != nullptr, "wrong result type");
        delta1_          = results->delta1;
        delta2_          = results->delta2;
        gamma1_          = results->gamma1;
        gamma2_          = results->gamma2;
    }

}
