/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 William Day

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license. You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the license for more details.
*/

#include <ql/exercise.hpp>
#include <ql/instruments/softbarrieroption.hpp>
#include <ql/instruments/impliedvolatility.hpp>
#include <ql/pricingengines/barrier/analyticsoftbarrierengine.hpp>
#include <memory> //////// is this needed?

namespace QuantLib {

    SoftBarrierOption::SoftBarrierOption(
        SoftBarrier::Type barrierType,
        Real barrier_lo,
        Real barrier_hi,
        const ext::shared_ptr<StrikedTypePayoff>& payoff,
        const ext::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff, exercise),
      barrierType_(barrierType), barrier_lo_(barrier_lo),
      barrier_hi_(barrier_hi) {}

    void SoftBarrierOption::setupArguments(PricingEngine::arguments* args) const {
        OneAssetOption::setupArguments(args);

        auto* moreArgs = dynamic_cast<SoftBarrierOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");
        moreArgs->barrierType = barrierType_;
        moreArgs->barrier_lo = barrier_lo_;
        moreArgs->barrier_hi = barrier_hi_;
    }

    Volatility SoftBarrierOption::impliedVolatility(
             Real targetValue,
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Real accuracy,
             Size maxEvaluations,
             Volatility minVol,
             Volatility maxVol) const {

        QL_REQUIRE(!isExpired(), "option expired");

        ext::shared_ptr<SimpleQuote> volQuote(new SimpleQuote);

        ext::shared_ptr<GeneralizedBlackScholesProcess> newProcess =
            detail::ImpliedVolatilityHelper::clone(process, volQuote);

        std::unique_ptr<PricingEngine> engine;
        switch (exercise_->type()) {
          case Exercise::European:
              engine = std::make_unique<AnalyticSoftBarrierEngine>(newProcess);
              break;
          case Exercise::American:
          case Exercise::Bermudan:
            QL_FAIL("engine not available for non-European soft barrier option");
          default:
            QL_FAIL("unknown exercise type");
        }

        return detail::ImpliedVolatilityHelper::calculate(*this,
                                                          *engine,
                                                          *volQuote,
                                                          targetValue,
                                                          accuracy,
                                                          maxEvaluations,
                                                          minVol, maxVol);
    }

    SoftBarrierOption::arguments::arguments()
    : barrierType(SoftBarrier::Type(-1)),
      barrier_lo(Null<Real>()), barrier_hi(Null<Real>()) {}

    void SoftBarrierOption::arguments::validate() const {
        OneAssetOption::arguments::validate();

        QL_REQUIRE(
          barrierType == SoftBarrier::DownIn ||
          barrierType == SoftBarrier::DownOut ||
          barrierType == SoftBarrier::UpIn ||
          barrierType == SoftBarrier::UpOut,
          "Invalid soft barrier type"
        );

        QL_REQUIRE(barrier_lo != Null<Real>(), "no low barrier given");
        QL_REQUIRE(barrier_hi != Null<Real>(), "no high barrier given");
        QL_REQUIRE(barrier_lo <= barrier_hi, "low barrier must be <= high barrier");
    }

}