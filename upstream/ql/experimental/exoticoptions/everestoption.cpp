/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/experimental/exoticoptions/everestoption.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    EverestOption::EverestOption(Real notional,
                                 Rate guarantee,
                                 const ext::shared_ptr<Exercise>& exercise)
    : MultiAssetOption(ext::shared_ptr<Payoff>(new NullPayoff), exercise),
      notional_(notional), guarantee_(guarantee) {}

    Rate EverestOption::yield() const {
        calculate();
        QL_REQUIRE(yield_ != Null<Rate>(), "yield not provided");
        return yield_;
    }

    void EverestOption::setupArguments(PricingEngine::arguments* args) const {
        MultiAssetOption::setupArguments(args);

        auto* arguments = dynamic_cast<EverestOption::arguments*>(args);
        QL_REQUIRE(arguments != nullptr, "wrong argument type");

        arguments->notional = notional_;
        arguments->guarantee= guarantee_;
    }

    void EverestOption::fetchResults(const PricingEngine::results* r) const {
        MultiAssetOption::fetchResults(r);
        const auto* results = dynamic_cast<const EverestOption::results*>(r);
        QL_ENSURE(results != nullptr, "no results returned from pricing engine");
        yield_ = results->yield;
    }


    EverestOption::arguments::arguments()
    : notional(Null<Real>()), guarantee(Null<Rate>()) {}

    void EverestOption::arguments::validate() const {
        MultiAssetOption::arguments::validate();
        QL_REQUIRE(notional != Null<Rate>(), "no notional given");
        QL_REQUIRE(notional != 0.0, "null notional given");
        QL_REQUIRE(guarantee != Null<Rate>(), "no guarantee given");
    }


    void EverestOption::results::reset() {
        MultiAssetOption::results::reset();
        yield = Null<Rate>();
    }

}

