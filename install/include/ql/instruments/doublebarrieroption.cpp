/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Thema Consulting SA

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

#include <ql/exercise.hpp>
#include <ql/instruments/doublebarrieroption.hpp>
#include <ql/instruments/impliedvolatility.hpp>
#include <ql/pricingengines/barrier/analyticdoublebarrierengine.hpp>
#include <memory>

namespace QuantLib {

    DoubleBarrierOption::DoubleBarrierOption(
        DoubleBarrier::Type barrierType,
        Real barrier_lo,
        Real barrier_hi,
        Real rebate,
        const ext::shared_ptr<StrikedTypePayoff>& payoff,
        const ext::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff, exercise),
      barrierType_(barrierType), barrier_lo_(barrier_lo), 
      barrier_hi_(barrier_hi), rebate_(rebate) {}

    void DoubleBarrierOption::setupArguments(PricingEngine::arguments* args) const {

        OneAssetOption::setupArguments(args);

        auto* moreArgs = dynamic_cast<DoubleBarrierOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");
        moreArgs->barrierType = barrierType_;
        moreArgs->barrier_lo = barrier_lo_;
        moreArgs->barrier_hi = barrier_hi_;
        moreArgs->rebate = rebate_;
    }


    Volatility DoubleBarrierOption::impliedVolatility(
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

        // engines are built-in for the time being
        std::unique_ptr<PricingEngine> engine;
        switch (exercise_->type()) {
          case Exercise::European:
              engine = std::make_unique<AnalyticDoubleBarrierEngine>(newProcess);
              break;
          case Exercise::American:
          case Exercise::Bermudan:
            QL_FAIL("engine not available for non-European barrier option");
            break;
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


    DoubleBarrierOption::arguments::arguments()
    : barrierType(DoubleBarrier::Type(-1)), barrier_lo(Null<Real>()),
      barrier_hi(Null<Real>()), rebate(Null<Real>()) {}

    void DoubleBarrierOption::arguments::validate() const {
        OneAssetOption::arguments::validate();

        QL_REQUIRE(barrierType == DoubleBarrier::KnockIn ||
                   barrierType == DoubleBarrier::KnockOut ||
                   barrierType == DoubleBarrier::KIKO ||
                   barrierType == DoubleBarrier::KOKI,
                   "Invalid barrier type");

        QL_REQUIRE(barrier_lo != Null<Real>(), "no low barrier given");
        QL_REQUIRE(barrier_hi != Null<Real>(), "no high barrier given");
        QL_REQUIRE(rebate != Null<Real>(), "no rebate given");
    }

    bool DoubleBarrierOption::engine::triggered(Real underlying) const {
        return underlying <= arguments_.barrier_lo || underlying >= arguments_.barrier_hi;
    }

}

