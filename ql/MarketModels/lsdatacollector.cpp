/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

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

#include <ql/MarketModels/lsdatacollector.hpp>

namespace QuantLib {

    namespace {

        std::vector<bool> isInSubset(const std::vector<Time>& set,
                                     const std::vector<Time>& subset) {
            std::vector<bool> result(set.size(), false);
            
            return result;
        }

        typedef MarketModelMultiProduct::CashFlow CashFlow;
    }

    void collectLongstaffSchwartzData(
                      MarketModelEvolver& evolver,
                      MarketModelMultiProduct& product,
                      MarketModelBasisSystem& basisSystem,
                      MarketModelExerciseValue& rebate,
                      MarketModelExerciseValue& control,
                      Size numberOfPaths,
                      std::vector<std::vector<LSNodeData> >& collectedData) {

        std::vector<Real> numerairesHeld;

        QL_REQUIRE(product.numberOfProducts() == 1,
                   "a single product is required");

        // TODO: check that all objects have compatible evolutions
        // (same rate times; evolution times for product, basis
        // system, rebate and control must be subsets of the passed
        // evolution times; rebate, control and basis system must have
        // the same exercise---not evolution---times)

        std::vector<Size> numberCashFlowsThisStep(1);
        std::vector<std::vector<CashFlow> > cashFlowsGenerated(1);
        cashFlowsGenerated[0].resize(
                           product.maxNumberOfCashFlowsPerProductPerStep());


        std::vector<Time> rateTimes =
            product.suggestedEvolution().rateTimes();

        std::vector<Time> cashFlowTimes = product.possibleCashFlowTimes();
        std::vector<Time> rebateTimes = rebate.possibleCashFlowTimes();
        std::vector<Time> controlTimes = control.possibleCashFlowTimes();

        std::vector<MarketModelDiscounter> productDiscounters;
        std::vector<MarketModelDiscounter> rebateDiscounters;
        std::vector<MarketModelDiscounter> controlDiscounters;
        Size i;
        for (i=0; i<cashFlowTimes.size(); ++i)
            productDiscounters.push_back(
                                     MarketModelDiscounter(cashFlowTimes[i],
                                                           rateTimes));
        for (i=0; i<rebateTimes.size(); ++i)
            rebateDiscounters.push_back(
                                     MarketModelDiscounter(rebateTimes[i],
                                                           rateTimes));
        for (i=0; i<controlTimes.size(); ++i)
            controlDiscounters.push_back(
                                     MarketModelDiscounter(controlTimes[i],
                                                           rateTimes));

        EvolutionDescription evolution = evolver.evolution();

        std::vector<Time> evolutionTimes = evolution.evolutionTimes();

        std::vector<bool> isProductTime =
            isInSubset(evolutionTimes,
                       product.suggestedEvolution().evolutionTimes());
        std::vector<bool> isRebateTime =
            isInSubset(evolutionTimes,
                       rebate.evolution().evolutionTimes());
        std::vector<bool> isControlTime =
            isInSubset(evolutionTimes,
                       control.evolution().evolutionTimes());
        std::vector<bool> isBasisTime =
            isInSubset(evolutionTimes,
                       basisSystem.evolution().evolutionTimes());
        std::vector<bool> isExerciseTime(evolutionTimes.size(),false);
        std::vector<bool> v = rebate.isExerciseTime();
        Size exercises = 0;
        for (i=0; i<evolutionTimes.size(); ++i) {
            if (isRebateTime[i]) {
                isExerciseTime[i] = v[exercises];
                ++exercises;
            }
        }
        
        collectedData.resize(exercises+1);
        for (i=0; i<collectedData.size(); ++i)
            collectedData[i].resize(numberOfPaths);

        
        for (i=0; i<numberOfPaths; ++i) {
            evolver.startNewPath();
            product.reset();
            rebate.reset();
            control.reset();
            basisSystem.reset();
            Real principalInNumerairePortfolio = 1.0;

            bool done = false;
            Size nextExercise = 0;
            collectedData[0][i].cumulatedCashFlows = 0.0;
            do {
                Size currentStep = evolver.currentStep();
                evolver.advanceStep();
                const CurveState& currentState = evolver.currentState();
                Size numeraire = evolution.numeraires()[currentStep];

                if (isRebateTime[currentStep])
                    rebate.nextStep(currentState);
                if (isControlTime[currentStep])
                    control.nextStep(currentState);
                if (isBasisTime[currentStep])
                    basisSystem.nextStep(currentState);

                if (isExerciseTime[currentStep]) {
                    LSNodeData& data = collectedData[nextExercise+1][i];

                    CashFlow exerciseValue = rebate.value(currentState);
                    data.exerciseValue =
                        exerciseValue.amount *
                        rebateDiscounters[exerciseValue.timeIndex]
                           .numeraireBonds(currentState, numeraire) /
                        principalInNumerairePortfolio;

                    basisSystem.values(currentState,
                                       data.basisFunctionValues);

                    CashFlow controlValue = control.value(currentState);
                    data.controlValue =
                        controlValue.amount *
                        controlDiscounters[controlValue.timeIndex]
                           .numeraireBonds(currentState, numeraire) /
                        principalInNumerairePortfolio;

                    data.cumulatedCashFlows = 0.0;

                    data.isValid = true;

                    ++nextExercise;
                }

                if (isProductTime[currentStep]) {
                    done = product.nextTimeStep(currentState,
                                                numberCashFlowsThisStep,
                                                cashFlowsGenerated);

                    for (Size j=0; j<numberCashFlowsThisStep[0]; ++j) {
                        const CashFlow& cf = cashFlowsGenerated[0][j];
                        collectedData[nextExercise][i].cumulatedCashFlows +=
                            cf.amount *
                            productDiscounters[cf.timeIndex]
                                .numeraireBonds(currentState, numeraire) /
                            principalInNumerairePortfolio;
                    }
                }

                if (!done) {
                    Size nextNumeraire =
                        evolution.numeraires()[currentStep+1];
                    principalInNumerairePortfolio *=
                        currentState.discountRatio(numeraire,
                                                   nextNumeraire);
                }
            }
            while (!done);

            // fill the remaining (un)collected data with nulls
            for (Size j = nextExercise; j < exercises; ++j) {
                LSNodeData& data = collectedData[j+1][i];
                data.exerciseValue = data.controlValue = 0.0;
                data.cumulatedCashFlows = 0.0;
                data.isValid = false;
            }
        }
    }

}
