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

#include <ql/instruments/twoassetcorrelationoption.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    TwoAssetCorrelationOption::TwoAssetCorrelationOption(
                           Option::Type type,
                           Real strike1,
                           Real strike2,
                           const ext::shared_ptr<Exercise>& exercise)
    : MultiAssetOption(ext::make_shared<PlainVanillaPayoff>(type, strike1),
                       exercise), X2_(strike2) {}

    void TwoAssetCorrelationOption::setupArguments(
                                       PricingEngine::arguments* args) const {
        MultiAssetOption::setupArguments(args);
        auto* moreArgs = dynamic_cast<TwoAssetCorrelationOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");

        moreArgs->X2 = X2_;
    }

}

