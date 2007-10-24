/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/pricingengines/capfloor/marketmodelcapfloorengine.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/models/marketmodels/accountingengine.hpp>
#include <ql/models/marketmodels/evolvers/lognormalfwdratepc.hpp>
#include <ql/models/marketmodels/products/onestep/onestepoptionlets.hpp>
#include <ql/models/marketmodels/browniangenerators/mtbrowniangenerator.hpp>

namespace QuantLib {

    MarketModelCapFloorEngine::MarketModelCapFloorEngine(
                         const boost::shared_ptr<MarketModelFactory>& factory,
                         const Handle<YieldTermStructure>& discountCurve)
    : factory_(factory), discountCurve_(discountCurve) {
        registerWith(factory_);
        registerWith(discountCurve_);
    }

    void MarketModelCapFloorEngine::calculate() const {

        QL_REQUIRE(arguments_.fixingTimes.front() >= 0.0,
                   "seasoned cap/floors not yet managed "
                   "by market-model engine");

        std::vector<Time> rateTimes = arguments_.fixingTimes;
        rateTimes.push_back(arguments_.endTimes.back());

        // not quite correct. The real payment times should be passed.
        std::vector<Time> paymentTimes = arguments_.endTimes;

        std::vector<Real> accruals(paymentTimes.size());
        for (Size i=0; i<accruals.size(); ++i)
            accruals[i] = arguments_.nominals[i] *
                          arguments_.accrualTimes[i] *
                          arguments_.gearings[i];

        Option::Type optionType;
        std::vector<Rate> strikes;
        switch (arguments_.type) {
          case CapFloor::Cap:
            optionType = Option::Call;
            strikes = arguments_.capRates;
            break;
          case CapFloor::Floor:
            optionType = Option::Put;
            strikes = arguments_.floorRates;
            break;
          case CapFloor::Collar:
            QL_FAIL("collar not supported");
            break;
          default:
            QL_FAIL("unknown cap/floor type");
        }
        std::vector<boost::shared_ptr<Payoff> > payoffs(paymentTimes.size());
        for (Size i=0; i<payoffs.size(); ++i)
            payoffs[i] = boost::shared_ptr<Payoff>(
                              new PlainVanillaPayoff(optionType, strikes[i]));

        OneStepOptionlets optionlets(rateTimes, accruals, paymentTimes, payoffs);
        EvolutionDescription evolution = optionlets.evolution();
        std::vector<Size> measure = terminalMeasure(evolution);

        boost::shared_ptr<MarketModel> model =
            factory_->create(evolution, rateTimes.size()-1);

        // all the hard-coded choices below should be left to the user
        MTBrownianGeneratorFactory generatorFactory(42);
        boost::shared_ptr<MarketModelEvolver> evolver(
                  new LogNormalFwdRatePc(model, generatorFactory, measure));

        // maybe discounts should be retrieved from the factory?
        Real initialNumeraireValue = arguments_.discounts.back();

        AccountingEngine engine(evolver, optionlets, initialNumeraireValue);
        SequenceStatistics stats(optionlets.numberOfProducts());
        engine.multiplePathValues(stats, /*262143*/32767);
        std::vector<Real> optionletsNpv = stats.mean();
        // Cap/floor total NPV
        results_.value = std::accumulate(optionletsNpv.begin(),optionletsNpv.end(),0.0);
        results_.additionalResults["optionletsPrice"] = optionletsNpv;
        // optionlets errors
        std::vector<Real> errors = stats.errorEstimate();
        results_.errorEstimate= std::accumulate(errors.begin(),errors.end(),0.0);
        results_.additionalResults["optionletsError"] = errors;
    }

}
