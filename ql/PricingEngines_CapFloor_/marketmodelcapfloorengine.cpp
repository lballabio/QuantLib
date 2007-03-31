/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/PricingEngines/CapFloor/marketmodelcapfloorengine.hpp>
#include <ql/Instruments/payoffs.hpp>
#include <ql/MarketModels/accountingengine.hpp>
#include <ql/MarketModels/Evolvers/forwardratepcevolver.hpp>
#include <ql/MarketModels/Products/OneStep/onestepoptionlets.hpp>
#include <ql/MarketModels/BrownianGenerators/mtbrowniangenerator.hpp>

namespace QuantLib {

    MarketModelCapFloorEngine::MarketModelCapFloorEngine(
                         const boost::shared_ptr<MarketModelFactory>& factory)
    : factory_(factory) {
        registerWith(factory_);
    }

    void MarketModelCapFloorEngine::update() {
        notifyObservers();
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

        OneStepOptionlets caplets(rateTimes, accruals, paymentTimes, payoffs);
        EvolutionDescription evolution = caplets.evolution();
        std::vector<Size> measure = terminalMeasure(evolution);

        boost::shared_ptr<MarketModel> model =
            factory_->create(evolution, rateTimes.size()-1);

        // all the hard-coded choices below should be left to the user
        MTBrownianGeneratorFactory generatorFactory(42);
        boost::shared_ptr<MarketModelEvolver> evolver(
                  new ForwardRatePcEvolver(model, generatorFactory, measure));

        // maybe discounts should be retrieved from the factory?
        Real initialNumeraireValue = arguments_.discounts.back();

        AccountingEngine engine(evolver, caplets, initialNumeraireValue);
        SequenceStatistics stats(caplets.numberOfProducts());
        engine.multiplePathValues(stats, 32767);
        std::vector<Real> npvs = stats.mean();
        results_.value = std::accumulate(npvs.begin(),npvs.end(),0.0);
        // additional results might be reported
    }

}
