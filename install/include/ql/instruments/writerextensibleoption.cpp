/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/event.hpp>
#include <ql/instruments/writerextensibleoption.hpp>
#include <utility>

namespace QuantLib {

    WriterExtensibleOption::WriterExtensibleOption(
        const ext::shared_ptr<PlainVanillaPayoff>& payoff1,
        const ext::shared_ptr<Exercise>& exercise1,
        const ext::shared_ptr<PlainVanillaPayoff>& payoff2,
        ext::shared_ptr<Exercise> exercise2)
    : OneAssetOption(payoff1, exercise1), payoff2_(payoff2), exercise2_(std::move(exercise2)) {}

    void WriterExtensibleOption::setupArguments(
                                       PricingEngine::arguments* args) const {
        OneAssetOption::setupArguments(args);

        auto* otherArguments = dynamic_cast<WriterExtensibleOption::arguments*>(args);
        QL_REQUIRE(otherArguments != nullptr, "wrong arguments type");

        otherArguments->payoff2 = payoff2_;
        otherArguments->exercise2 = exercise2_;
    }

    bool WriterExtensibleOption::isExpired() const {
        return detail::simple_event(exercise2_->lastDate()).hasOccurred();
    }

    void WriterExtensibleOption::arguments::validate() const {
        OneAssetOption::arguments::validate();
        QL_REQUIRE(payoff2, "no second payoff given");
        QL_REQUIRE(exercise2, "no second exercise given");
        QL_REQUIRE(exercise2->lastDate() > exercise->lastDate(),
                   "second exercise date is not later than the first");
    }

}
