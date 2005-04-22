/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Instruments/barrieroption.hpp>
#include <ql/PricingEngines/Barrier/analyticbarrierengine.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>

namespace QuantLib {

    BarrierOption::BarrierOption(
        Barrier::Type barrierType,
        Real barrier,
        Real rebate,
        const boost::shared_ptr<GenericStochasticProcess>& process,
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise,
        const boost::shared_ptr<PricingEngine>& engine)
    : OneAssetStrikedOption(process, payoff, exercise, engine),
      barrierType_(barrierType), barrier_(barrier), rebate_(rebate) {

        if (!engine)
            setPricingEngine(
                 boost::shared_ptr<PricingEngine>(new AnalyticBarrierEngine));

    }

    void BarrierOption::setupArguments(Arguments* args) const {

        BarrierOption::arguments* moreArgs =
            dynamic_cast<BarrierOption::arguments*>(args);
        QL_REQUIRE(moreArgs != 0, "wrong argument type");
        moreArgs->barrierType = barrierType_;
        moreArgs->barrier = barrier_;
        moreArgs->rebate = rebate_;

        OneAssetStrikedOption::arguments* arguments =
            dynamic_cast<OneAssetStrikedOption::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");
        OneAssetStrikedOption::setupArguments(arguments);

    }

    void BarrierOption::performCalculations() const {
        // enforce in this class any check on engine/payoff
        OneAssetStrikedOption::performCalculations();
    }

    void BarrierOption::arguments::validate() const {
        #if defined(QL_PATCH_MSVC6)
        OneAssetStrikedOption::arguments copy = *this;
        copy.validate();
        #else
        OneAssetStrikedOption::arguments::validate();
        #endif

        // assuming, as always, that the underlying is the first of
        // the state variables...
        Real underlying = stochasticProcess->initialValues()[0];
        switch (barrierType) {
          case Barrier::DownIn:
            QL_REQUIRE(underlying >= barrier,
                       "underlying (" << underlying
                       << ") < barrier (" << barrier
                       << "): down-and-in barrier undefined");
            break;
          case Barrier::UpIn:
            QL_REQUIRE(underlying <= barrier,
                       "underlying (" << underlying
                       << ") > barrier (" << barrier
                       << "): up-and-in barrier undefined");
            break;
          case Barrier::DownOut:
            QL_REQUIRE(underlying >= barrier,
                       "underlying (" << underlying
                       << ") < barrier (" << barrier
                       << "): down-and-out barrier undefined");
            break;
          case Barrier::UpOut:
            QL_REQUIRE(underlying <= barrier,
                       "underlying (" << underlying
                       << ") > barrier (" << barrier
                       << "): up-and-out barrier undefined");
            break;
          default:
            QL_FAIL("unknown type");
        }
    }

}

