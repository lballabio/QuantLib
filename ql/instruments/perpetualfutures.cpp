/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Hiroto Ogawa

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

#include <ql/instruments/perpetualfutures.hpp>
//#include <memory>

namespace QuantLib {

    PerpetualFutures::PerpetualFutures(
        PerpetualFutures::PayoffType payoffType,
        PerpetualFutures::FundingType fundingType) :
        payoffType_(payoffType), fundingType_(fundingType) {
    }

    void PerpetualFutures::setupArguments(PricingEngine::arguments* args) const {
        auto* moreArgs = dynamic_cast<PerpetualFutures::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");
        moreArgs->payoffType = payoffType_;
        moreArgs->fundingType = fundingType_;
    }

    PerpetualFutures::arguments::arguments()
        : payoffType(PerpetualFutures::PayoffType(-1)),
        fundingType(PerpetualFutures::FundingType(-1)) {
    }

    void PerpetualFutures::arguments::validate() const {
        switch (payoffType) {
            case PerpetualFutures::Linear:
            case PerpetualFutures::Inverse:
            case PerpetualFutures::Quanto:
                break;
            default:
                QL_FAIL("unknown payoff type");
        }
        switch (fundingType) {
            case PerpetualFutures::AHJ:
                break;
            default:
                QL_FAIL("unknown funding type");
        }
    }
}

