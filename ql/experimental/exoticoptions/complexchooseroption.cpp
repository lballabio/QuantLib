/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/experimental/exoticoptions/complexchooseroption.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    ComplexChooserOption::ComplexChooserOption(
        Date choosingDate,
        Real strikeCall,
        Real strikePut,
        const ext::shared_ptr<Exercise>& exerciseCall,
        const ext::shared_ptr<Exercise>& exercisePut)
    : OneAssetOption(ext::make_shared<PlainVanillaPayoff>(Option::Call,
                                                            strikeCall),
                     exerciseCall),
      choosingDate_(choosingDate),
      strikeCall_(strikeCall),
      strikePut_(strikePut),
      exerciseCall_(exerciseCall),
      exercisePut_(exercisePut) {}

    void ComplexChooserOption::setupArguments(
                                       PricingEngine::arguments* args) const {
        OneAssetOption::setupArguments(args);
        auto* moreArgs = dynamic_cast<ComplexChooserOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");
        moreArgs->choosingDate=choosingDate_;
        moreArgs->strikeCall=strikeCall_;
        moreArgs->strikePut=strikePut_;
        moreArgs->exerciseCall = exerciseCall_;
        moreArgs->exercisePut = exercisePut_;
    }

    void ComplexChooserOption::arguments::validate() const {
        OneAssetOption::arguments::validate();
        QL_REQUIRE(choosingDate != Date() , " no choosing date given");
        QL_REQUIRE(choosingDate < exerciseCall->lastDate(),
                   "choosing date later than or equal to Call maturity date");
        QL_REQUIRE(choosingDate < exercisePut->lastDate(),
                   "choosing date later than or equal to Put maturity date");
    }

}
