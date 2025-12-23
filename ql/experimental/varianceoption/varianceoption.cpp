/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 StatPro Italia srl

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

#include <ql/event.hpp>
#include <ql/experimental/varianceoption/varianceoption.hpp>
#include <utility>

namespace QuantLib {

    VarianceOption::VarianceOption(ext::shared_ptr<Payoff> payoff,
                                   Real notional,
                                   const Date& startDate,
                                   const Date& maturityDate)
    : payoff_(std::move(payoff)), notional_(notional), startDate_(startDate),
      maturityDate_(maturityDate) {}

    void VarianceOption::setupArguments(PricingEngine::arguments* args) const {
        auto* arguments = dynamic_cast<VarianceOption::arguments*>(args);
        QL_REQUIRE(arguments != nullptr, "wrong argument type");

        arguments->payoff = payoff_;
        arguments->notional = notional_;
        arguments->startDate = startDate_;
        arguments->maturityDate = maturityDate_;
    }

    void VarianceOption::arguments::validate() const {
        QL_REQUIRE(payoff, "no strike given");
        QL_REQUIRE(notional != Null<Real>(), "no notional given");
        QL_REQUIRE(notional > 0.0, "negative or null notional given");
        QL_REQUIRE(startDate != Date(), "null start date given");
        QL_REQUIRE(maturityDate != Date(), "null maturity date given");
    }

    bool VarianceOption::isExpired() const {
        return detail::simple_event(maturityDate_).hasOccurred();
    }

}
