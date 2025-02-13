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

#include <ql/instruments/holderextensibleoption.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    HolderExtensibleOption::HolderExtensibleOption(
                           Option::Type type,
                           Real premium,
                           Date secondExpiryDate,
                           Real secondStrike,
                           const ext::shared_ptr<StrikedTypePayoff>& payoff,
                           const ext::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff,exercise),
      premium_(premium),
      secondExpiryDate_(secondExpiryDate),
      secondStrike_(secondStrike) {}

    void HolderExtensibleOption::setupArguments(
                                       PricingEngine::arguments* args) const {
        OneAssetOption::setupArguments(args);
        auto* moreArgs = dynamic_cast<HolderExtensibleOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");
        moreArgs->premium = premium_;
        moreArgs->secondExpiryDate = secondExpiryDate_;
        moreArgs->secondStrike = secondStrike_;
    }

    void HolderExtensibleOption:: arguments::validate() const {
        OneAssetOption::arguments::validate();
        QL_REQUIRE(premium > 0,"negative premium not allowed");
        QL_REQUIRE(secondExpiryDate != Date() , "no extending date given");
        QL_REQUIRE(secondExpiryDate >= exercise->lastDate(),
                   "extended date is earlier than or equal to first maturity date");
    }

}
