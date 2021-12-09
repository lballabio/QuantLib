/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/experimental/exoticoptions/simplechooseroption.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    SimpleChooserOption::SimpleChooserOption(
                                  Date choosingDate,
                                  Real strike,
                                  const ext::shared_ptr<Exercise>& exercise)
    : OneAssetOption(ext::shared_ptr<Payoff>(
                                new PlainVanillaPayoff(Option::Call, strike)),
                     exercise),
      choosingDate_(choosingDate) {}

    void SimpleChooserOption::setupArguments(
                                    PricingEngine::arguments* args) const {
        OneAssetOption::setupArguments(args);
        auto* moreArgs = dynamic_cast<SimpleChooserOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");
        moreArgs->choosingDate=choosingDate_;
    }

    void SimpleChooserOption::arguments::validate() const {
        OneAssetOption::arguments::validate();
        QL_REQUIRE(choosingDate != Date(), " no choosing date given");
        QL_REQUIRE(choosingDate < exercise->lastDate(),
                   "choosing date later than or equal to maturity date");
    }

}
