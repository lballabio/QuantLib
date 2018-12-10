/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/experimental/exoticoptions/himalayaoption.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    HimalayaOption::HimalayaOption(const std::vector<Date>& fixingDates,
                                   Real strike)
    : MultiAssetOption(ext::shared_ptr<Payoff>(
                                new PlainVanillaPayoff(Option::Call, strike)),
                       ext::shared_ptr<Exercise>(
                                   new EuropeanExercise(fixingDates.back()))),
      fixingDates_(fixingDates) {}

    void HimalayaOption::setupArguments(PricingEngine::arguments* args) const {
        MultiAssetOption::setupArguments(args);

        HimalayaOption::arguments* arguments =
            dynamic_cast<HimalayaOption::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->fixingDates = fixingDates_;
    }

    HimalayaOption::arguments::arguments() {}

    void HimalayaOption::arguments::validate() const {
        MultiAssetOption::arguments::validate();
        QL_REQUIRE(!fixingDates.empty(), "no fixing dates given");
    }

}

