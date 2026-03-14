/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/instruments/partialtimebarrieroption.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    PartialTimeBarrierOption::PartialTimeBarrierOption(
                           Barrier::Type barrierType,
                           PartialBarrier::Range barrierRange,
                           Real barrier,
                           Real rebate,
                           Date coverEventDate,
                           const ext::shared_ptr<StrikedTypePayoff>& payoff,
                           const ext::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff, exercise),
      barrierType_(barrierType), barrierRange_(barrierRange),
      barrier_(barrier), rebate_(rebate),
      coverEventDate_(coverEventDate) {}

    void PartialTimeBarrierOption::setupArguments(
                                       PricingEngine::arguments* args) const {
        OneAssetOption::setupArguments(args);

        auto* moreArgs = dynamic_cast<PartialTimeBarrierOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");
        moreArgs->barrierType = barrierType_;
        moreArgs->barrierRange = barrierRange_;
        moreArgs->barrier = barrier_;
        moreArgs->rebate = rebate_;
        moreArgs->coverEventDate = coverEventDate_;
    }

    PartialTimeBarrierOption::arguments::arguments()
    : barrierType(Barrier::Type(-1)),
      barrierRange(PartialBarrier::Range(-1)),
      barrier(Null<Real>()), rebate(Null<Real>()) {}

    void PartialTimeBarrierOption::arguments::validate() const {
        OneAssetOption::arguments::validate();

        QL_REQUIRE(barrier != Null<Real>(), "no barrier given");
        QL_REQUIRE(rebate != Null<Real>(), "no rebate given");
        QL_REQUIRE(coverEventDate != Date(), "no cover event date given");
        QL_REQUIRE(coverEventDate < exercise->lastDate(),
                   "cover event date equal or later than exercise date");
    }

}
