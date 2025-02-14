/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/instruments/twoassetbarrieroption.hpp>
#include <ql/exercise.hpp>
#include <ql/event.hpp>

namespace QuantLib {

    TwoAssetBarrierOption::TwoAssetBarrierOption(
                           Barrier::Type barrierType,
                           Real barrier,
                           const ext::shared_ptr<StrikedTypePayoff>& payoff,
                           const ext::shared_ptr<Exercise>& exercise)
    : Option(payoff, exercise), barrierType_(barrierType), barrier_(barrier) {}

    void TwoAssetBarrierOption::setupArguments(
                                       PricingEngine::arguments* args) const {
        Option::setupArguments(args);
        auto* moreArgs = dynamic_cast<TwoAssetBarrierOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");
        moreArgs->barrierType = barrierType_;
        moreArgs->barrier = barrier_;
    }
    
    bool TwoAssetBarrierOption::isExpired() const {
        return detail::simple_event(exercise_->lastDate()).hasOccurred();
    }

    TwoAssetBarrierOption::arguments::arguments()
    : barrierType(Barrier::Type(-1)), barrier(Null<Real>()) {}

    void TwoAssetBarrierOption::arguments::validate() const {
        Option::arguments::validate();

        switch (barrierType) {
          case Barrier::DownIn:
          case Barrier::UpIn:
          case Barrier::DownOut:
          case Barrier::UpOut:
            break;
          default:
            QL_FAIL("unknown type");
        }

        QL_REQUIRE(barrier != Null<Real>(), "no barrier given");
    }

    bool TwoAssetBarrierOption::engine::triggered(Real underlying) const {
        switch (arguments_.barrierType) {
          case Barrier::DownIn:
          case Barrier::DownOut:
            return underlying < arguments_.barrier;
          case Barrier::UpIn:
          case Barrier::UpOut:
            return underlying > arguments_.barrier;
          default:
            QL_FAIL("unknown type");
        }
    }

}

