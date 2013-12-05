/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2013 Yue Tian

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

#include <ql/experimental/barrieroption/doublebarrieroption.hpp>
#include <ql/instruments/impliedvolatility.hpp>
#include <ql/experimental/barrieroption/analyticdoublebarrierengine.hpp>
#include <ql/exercise.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/make_shared.hpp>

namespace QuantLib {

    DoubleBarrierOption::DoubleBarrierOption(
        std::vector<Barrier::Type> barrierType,
        std::vector<Real> barrier,
        std::vector<Real> rebate,
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff, exercise),
      barrierType_(barrierType), barrier_(barrier), rebate_(rebate) {}

    void DoubleBarrierOption::setupArguments(PricingEngine::arguments* args) const {

        OneAssetOption::setupArguments(args);

        DoubleBarrierOption::arguments* moreArgs =
            dynamic_cast<DoubleBarrierOption::arguments*>(args);
        QL_REQUIRE(moreArgs != 0, "wrong argument type");
        moreArgs->barrierType = barrierType_;
        moreArgs->barrier = barrier_;
        moreArgs->rebate = rebate_;
    }


    Volatility DoubleBarrierOption::impliedVolatility(
             Real targetValue,
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Real accuracy,
             Size maxEvaluations,
             Volatility minVol,
             Volatility maxVol) const {

        QL_REQUIRE(!isExpired(), "option expired");

        boost::shared_ptr<SimpleQuote> volQuote =
            boost::make_shared<SimpleQuote>();

        boost::shared_ptr<GeneralizedBlackScholesProcess> newProcess =
            detail::ImpliedVolatilityHelper::clone(process, volQuote);

        // engines are built-in for the time being
        boost::scoped_ptr<PricingEngine> engine;
        switch (exercise_->type()) {
          case Exercise::European:
            engine.reset(new AnalyticDoubleBarrierEngine(newProcess));
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
    : barrierType(std::vector<Barrier::Type>(2, Barrier::Type(-1))), barrier(std::vector<Real>(2, 0.0)),
      rebate(std::vector<Real>(2, 0.0)) {}

    void DoubleBarrierOption::arguments::validate() const {
        OneAssetOption::arguments::validate();

        QL_REQUIRE(barrierType.size() == 2, "2 barrier type should be given");
		QL_REQUIRE(barrier.size() == 2, "2 barrier should be given");
        QL_REQUIRE(rebate.size() == 2, "2 rebate should be given");

		switch (barrierType[0]) {
          case Barrier::DownIn:
			  QL_REQUIRE(barrierType[1] == Barrier::UpIn, "both barrier type should be In");
			  break;
          case Barrier::UpIn:
			  QL_FAIL("rearrange the order of barrier types as L/H");
			  break;
          case Barrier::DownOut:
			  QL_REQUIRE(barrierType[1] == Barrier::UpOut, "both barrier type should be Out");
			  break;
          case Barrier::UpOut:
			  QL_FAIL("rearrange the order of barrier types as L/H");
            break;
          default:
            QL_FAIL("unknown type");
        }
		QL_REQUIRE(barrier[0] < barrier[1], "rearrange the order of barriers as L/H");
		
    }

    bool DoubleBarrierOption::engine::triggered(Real underlying) const {
        switch (arguments_.barrierType[0]) {
          case Barrier::DownIn:
          case Barrier::DownOut:
            return underlying < arguments_.barrier[1] && underlying < arguments_.barrier[0];
          default:
            QL_FAIL("unknown type");
        }
    }

}

