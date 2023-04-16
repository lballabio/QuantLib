/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andrea Odetti

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

#include <ql/experimental/mcbasket/pathmultiassetoption.hpp>
#include <ql/processes/stochasticprocessarray.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/event.hpp>

namespace QuantLib {

    PathMultiAssetOption::PathMultiAssetOption(
                            const std::shared_ptr<PricingEngine>& engine) {
        if (engine != nullptr)
            setPricingEngine(engine);
    }

    bool PathMultiAssetOption::isExpired() const {
        return detail::simple_event(fixingDates().back()).hasOccurred();
    }

    void PathMultiAssetOption::setupExpired() const {
        NPV_ = 0.0;
    }

    void PathMultiAssetOption::setupArguments(PricingEngine::arguments* args)
                                                                       const {
        auto* arguments = dynamic_cast<PathMultiAssetOption::arguments*>(args);

        QL_REQUIRE(arguments != nullptr, "wrong argument type");

        arguments->payoff            = pathPayoff();
        arguments->fixingDates       = fixingDates();
    }

    void PathMultiAssetOption::arguments::validate() const {
        QL_REQUIRE(payoff,                 "no payoff given");
        QL_REQUIRE(!fixingDates.empty(), "no dates given");
    }
}

