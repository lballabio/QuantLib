/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

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

#include <ql/models/marketmodels/constrainedevolver.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/discounter.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/proxygreekengine.hpp>
#include <algorithm>
#include <utility>

namespace QuantLib {

    ProxyGreekEngine::ProxyGreekEngine(
        std::shared_ptr<MarketModelEvolver> evolver,
        std::vector<std::vector<std::shared_ptr<ConstrainedEvolver> > > constrainedEvolvers,
        std::vector<std::vector<std::vector<Real> > > diffWeights,
        std::vector<Size> startIndexOfConstraint,
        std::vector<Size> endIndexOfConstraint,
        const Clone<MarketModelMultiProduct>& product,
        Real initialNumeraireValue)
    : originalEvolver_(std::move(evolver)), constrainedEvolvers_(std::move(constrainedEvolvers)),
      diffWeights_(std::move(diffWeights)),
      startIndexOfConstraint_(std::move(startIndexOfConstraint)),
      endIndexOfConstraint_(std::move(endIndexOfConstraint)), product_(product),
      initialNumeraireValue_(initialNumeraireValue), numberProducts_(product->numberOfProducts()),
      numerairesHeld_(product->numberOfProducts()),
      numberCashFlowsThisStep_(product->numberOfProducts()),
      cashFlowsGenerated_(product->numberOfProducts()) {
        for (Size i=0; i<numberProducts_; ++i)
            cashFlowsGenerated_[i].resize(
                       product_->maxNumberOfCashFlowsPerProductPerStep());

        const std::vector<Time>& cashFlowTimes =
            product_->possibleCashFlowTimes();
        const std::vector<Rate>& rateTimes = product_->evolution().rateTimes();
        Size n = cashFlowTimes.size();
        discounters_.reserve(n);
        for (Size j=0; j<n; ++j)
            discounters_.emplace_back(cashFlowTimes[j], rateTimes);
        const std::vector<Rate>& evolutionTimes =
            product_->evolution().evolutionTimes();
        constraints_.resize(evolutionTimes.size());
        constraintsActive_.resize(evolutionTimes.size());
    }

    void ProxyGreekEngine::singlePathValues(
              std::vector<Real>& values,
              std::vector<std::vector<std::vector<Real> > >& modifiedValues) {
        singleEvolverValues(*originalEvolver_, values, true);
        for (Size i=0; i<constrainedEvolvers_.size(); ++i) {
            for (Size j=0; j<constrainedEvolvers_[i].size(); ++j) {
                constrainedEvolvers_[i][j]->setThisConstraint(
                                            constraints_, constraintsActive_);
                singleEvolverValues(*(constrainedEvolvers_[i][j]),
                                    modifiedValues[i][j]);
            }
        }
    }

    void ProxyGreekEngine::multiplePathValues(
                  SequenceStatisticsInc& stats,
                  std::vector<std::vector<SequenceStatisticsInc> >& modifiedStats,
                  Size numberOfPaths) {
        Size N = product_->numberOfProducts();

        std::vector<Real> values(N);
        std::vector<std::vector<std::vector<Real> > > modifiedValues;
        modifiedValues.resize(constrainedEvolvers_.size());
        for (Size i=0; i<modifiedValues.size(); ++i) {
            modifiedValues[i].resize(constrainedEvolvers_[i].size());
            for (auto& j : modifiedValues[i])
                j.resize(N);
        }

        std::vector<Real> results(N);

        for (Size i=0; i<numberOfPaths; ++i) {
            singlePathValues(values, modifiedValues);
            stats.add(values);

            for (Size j=0; j<diffWeights_.size(); ++j) {
                for (Size k=0; k<diffWeights_[j].size(); ++k) {
                    const std::vector<Real>& weights = diffWeights_[j][k];
                    for (Size l=0; l<N; ++l) {
                        results[l] = weights[0]*values[l];
                        for (Size n=1; n<weights.size(); ++n)
                            results[l] += weights[n]*modifiedValues[j][n-1][l];
                    }
                    modifiedStats[j][k].add(results);
                }
            }
        }
    }

    void ProxyGreekEngine::singleEvolverValues(MarketModelEvolver& evolver,
                                               std::vector<Real>& values,
                                               bool storeRates) {

        std::fill(numerairesHeld_.begin(), numerairesHeld_.end(), 0.0);
        Real weight = evolver.startNewPath();
        product_->reset();
        Real principalInNumerairePortfolio = 1.0;

        if (storeRates)
            constraintsActive_ =false;
        //            std::fill(constraintsActive_.begin(),
        //                    constraintsActive_.end(),
        //                  false);
        //  }

        bool done = false;
        do {
            Size thisStep = evolver.currentStep();
            weight *= evolver.advanceStep();
            done = product_->nextTimeStep(evolver.currentState(),
                                          numberCashFlowsThisStep_,
                                          cashFlowsGenerated_);
            if (storeRates) {
                constraints_[thisStep] = evolver.currentState().swapRate(
                                        startIndexOfConstraint_[thisStep],
                                        endIndexOfConstraint_[thisStep]);
                constraintsActive_[thisStep] = true;
            }

            Size numeraire =
                evolver.numeraires()[thisStep];

            // for each product...
            for (Size i=0; i<numberProducts_; ++i) {
                // ...and each cash flow...
                const std::vector<MarketModelMultiProduct::CashFlow>& cashflows =
                    cashFlowsGenerated_[i];
                for (Size j=0; j<numberCashFlowsThisStep_[i]; ++j) {
                    // ...convert the cash flow to numeraires.
                    // This is done by calculating the number of
                    // numeraire bonds corresponding to such cash flow...
                    const MarketModelDiscounter& discounter =
                        discounters_[cashflows[j].timeIndex];

                    Real bonds = cashflows[j].amount *
                        discounter.numeraireBonds(evolver.currentState(),
                                                  numeraire);

                    // ...and adding the newly bought bonds to the number
                    // of numeraires held.
                    numerairesHeld_[i] +=
                        weight*bonds/principalInNumerairePortfolio;
                }
            }

            if (!done) {

                // The numeraire might change between steps. This implies
                // that we might have to convert the numeraire bonds for
                // this step into a corresponding amount of numeraire
                // bonds for the next step. This can be done by changing
                // the principal of the numeraire and updating the number
                // of bonds in the numeraire portfolio accordingly.

                Size nextNumeraire = evolver.numeraires()[thisStep+1];

                principalInNumerairePortfolio *=
                    evolver.currentState().discountRatio(numeraire,
                                                         nextNumeraire);
            }

        } while (!done);

        for (Size i=0; i<numerairesHeld_.size(); ++i)
            values[i] = numerairesHeld_[i] * initialNumeraireValue_;

    }

}
