/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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
#include <ql/instruments/barrieroption.hpp>
#include <ql/instruments/dividendbarrieroption.hpp>
#include <ql/instruments/impliedvolatility.hpp>
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <memory>

namespace QuantLib {

    BarrierOption::BarrierOption(
        Barrier::Type barrierType,
        Real barrier,
        Real rebate,
        const ext::shared_ptr<StrikedTypePayoff>& payoff,
        const ext::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff, exercise),
      barrierType_(barrierType), barrier_(barrier), rebate_(rebate) {}

    void BarrierOption::setupArguments(PricingEngine::arguments* args) const {

        OneAssetOption::setupArguments(args);

        auto* moreArgs = dynamic_cast<BarrierOption::arguments*>(args);
        QL_REQUIRE(moreArgs != nullptr, "wrong argument type");
        moreArgs->barrierType = barrierType_;
        moreArgs->barrier = barrier_;
        moreArgs->rebate = rebate_;

        /* this is a workaround in case an engine is used for both barrier
           and dividend options.  The dividends might have been set by another
           instrument and need to be cleared. */
        auto* arguments = dynamic_cast<DividendBarrierOption::arguments*>(args);
        if (arguments != nullptr) {
            arguments->cashFlow.clear();
        }
    }


    Volatility BarrierOption::impliedVolatility(
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
              engine = std::make_unique<AnalyticBarrierEngine>(newProcess);
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


    BarrierOption::arguments::arguments()
    : barrierType(Barrier::Type(-1)), barrier(Null<Real>()),
      rebate(Null<Real>()) {}

    void BarrierOption::arguments::validate() const {
        OneAssetOption::arguments::validate();

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
        QL_REQUIRE(rebate != Null<Real>(), "no rebate given");
    }

    bool BarrierOption::engine::triggered(Real underlying) const {
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

