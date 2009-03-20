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

#include <ql/instruments/barrieroption.hpp>
#include <ql/instruments/impliedvolatility.hpp>
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <ql/exercise.hpp>
#include <boost/scoped_ptr.hpp>

namespace QuantLib {

    BarrierOption::BarrierOption(
        Barrier::Type barrierType,
        Real barrier,
        Real rebate,
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff, exercise),
      barrierType_(barrierType), barrier_(barrier), rebate_(rebate) {}

    void BarrierOption::setupArguments(PricingEngine::arguments* args) const {

        OneAssetOption::setupArguments(args);

        BarrierOption::arguments* moreArgs =
            dynamic_cast<BarrierOption::arguments*>(args);
        QL_REQUIRE(moreArgs != 0, "wrong argument type");
        moreArgs->barrierType = barrierType_;
        moreArgs->barrier = barrier_;
        moreArgs->rebate = rebate_;
    }


    Volatility BarrierOption::impliedVolatility(
             Real targetValue,
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Real accuracy,
             Size maxEvaluations,
             Volatility minVol,
             Volatility maxVol) const {

        QL_REQUIRE(!isExpired(), "option expired");

        boost::shared_ptr<SimpleQuote> volQuote(new SimpleQuote);

        boost::shared_ptr<GeneralizedBlackScholesProcess> newProcess =
            detail::ImpliedVolatilityHelper::clone(process, volQuote);

        // engines are built-in for the time being
        boost::scoped_ptr<PricingEngine> engine;
        switch (exercise_->type()) {
          case Exercise::European:
            engine.reset(new AnalyticBarrierEngine(newProcess));
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

