/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/marketmodels/accountingengine.hpp>
#include <ql/models/marketmodels/callability/exercisevalue.hpp>
#include <ql/models/marketmodels/callability/upperboundengine.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/discounter.hpp>
#include <ql/models/marketmodels/evolver.hpp>
#include <ql/models/marketmodels/multiproduct.hpp>
#include <ql/models/marketmodels/products/multistep/callspecifiedmultiproduct.hpp>
#include <ql/models/marketmodels/products/multistep/exerciseadapter.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <algorithm>
#include <utility>

namespace QuantLib {

    namespace {

        class DecoratedHedge : public CallSpecifiedMultiProduct {
          public:
            explicit DecoratedHedge(const CallSpecifiedMultiProduct& product)
            : CallSpecifiedMultiProduct(product) {
                savedStates_.reserve(product.evolution().numberOfSteps());

                Size N = product.numberOfProducts();
                numberCashFlowsThisStep_.resize(N);
                cashFlowsGenerated_.resize(N);
                for (Size i=0; i<N; ++i)
                    cashFlowsGenerated_[i].resize(
                            product.maxNumberOfCashFlowsPerProductPerStep());

                clear();
            }

            void reset() override {
                CallSpecifiedMultiProduct::reset();
                disableCallability();
                for (Size i=0; i<lastSavedStep_; ++i)
                    CallSpecifiedMultiProduct::nextTimeStep(
                                                     *savedStates_[i],
                                                     numberCashFlowsThisStep_,
                                                     cashFlowsGenerated_);
                enableCallability();
            }

            bool nextTimeStep(const CurveState& currentState,
                              std::vector<Size>& numberCashFlowsThisStep,
                              std::vector<std::vector<CashFlow> >& cashFlowsGenerated) override {
                if (recording_)
                    savedStates_.emplace_back(currentState);
                return CallSpecifiedMultiProduct::nextTimeStep(
                                                     currentState,
                                                     numberCashFlowsThisStep,
                                                     cashFlowsGenerated);
            }

            std::unique_ptr<MarketModelMultiProduct> clone() const override {
                return std::unique_ptr<MarketModelMultiProduct>(new DecoratedHedge(*this));
            }

            void save() {
                lastSavedStep_ = savedStates_.size();
            }

            void clear() {
                lastSavedStep_ = 0;
                savedStates_.clear();
                recording_ = true;
            }

            void startRecording() {
                recording_ = true;
            }

            void stopRecording() {
                recording_ = false;
            }
          private:
            std::vector<Clone<CurveState> > savedStates_;
            Size lastSavedStep_;
            bool recording_;
            std::vector<Size> numberCashFlowsThisStep_;
            std::vector<std::vector<CashFlow> > cashFlowsGenerated_;
        };

    }


    UpperBoundEngine::UpperBoundEngine(
        ext::shared_ptr<MarketModelEvolver> evolver,
        std::vector<ext::shared_ptr<MarketModelEvolver> > innerEvolvers,
        const MarketModelMultiProduct& underlying,
        const MarketModelExerciseValue& rebate,
        const MarketModelMultiProduct& hedge,
        const MarketModelExerciseValue& hedgeRebate,
        const ExerciseStrategy<CurveState>& hedgeStrategy,
        Real initialNumeraireValue)
    : evolver_(std::move(evolver)), innerEvolvers_(std::move(innerEvolvers)),
      composite_(MultiProductComposite()), initialNumeraireValue_(initialNumeraireValue) {

        composite_.add(underlying);
        composite_.add(ExerciseAdapter(rebate));
        composite_.add(hedge);
        composite_.add(ExerciseAdapter(hedgeRebate));
        composite_.add(DecoratedHedge(CallSpecifiedMultiProduct(
                           hedge,hedgeStrategy,ExerciseAdapter(hedgeRebate))));
        composite_.finalize();

        underlyingOffset_ = 0;
        underlyingSize_ = underlying.numberOfProducts();
        rebateOffset_ = underlyingSize_;
        rebateSize_ = 1;
        hedgeOffset_ = underlyingSize_+rebateSize_;
        hedgeSize_ = hedge.numberOfProducts();
        hedgeRebateOffset_ = underlyingSize_+rebateSize_+hedgeSize_;
        hedgeRebateSize_ = 1;


        numberOfProducts_ = composite_.numberOfProducts();

        const std::vector<Time>& evolutionTimes =
            composite_.evolution().evolutionTimes();
        numberOfSteps_ = evolutionTimes.size();

        isExerciseTime_.resize(evolutionTimes.size());
        isExerciseTime_ = isInSubset(evolutionTimes,
                                     hedgeStrategy.exerciseTimes());

        numberCashFlowsThisStep_.resize(numberOfProducts_);
        cashFlowsGenerated_.resize(numberOfProducts_);
        for (Size i=0; i<numberOfProducts_; ++i)
            cashFlowsGenerated_[i].resize(
                          composite_.maxNumberOfCashFlowsPerProductPerStep());

        const std::vector<Time>& cashFlowTimes =
            composite_.possibleCashFlowTimes();
        const std::vector<Rate>& rateTimes =
            composite_.evolution().rateTimes();
        Size n =cashFlowTimes.size();
        discounters_.reserve(n);
        for (Size j=0; j<n; ++j)
            discounters_.emplace_back(cashFlowTimes[j], rateTimes);
    }


    void UpperBoundEngine::multiplePathValues(Statistics& stats,
                                              Size outerPaths,
                                              Size innerPaths) {
        for (Size i=0; i<outerPaths; ++i) {
            std::pair<Real,Real> result = singlePathValue(innerPaths);
            stats.add(result.first, result.second);
        }
    }


    std::pair<Real,Real> UpperBoundEngine::singlePathValue(Size innerPaths) {

        auto& callable = dynamic_cast<DecoratedHedge&>(composite_.item(4));
        const ExerciseStrategy<CurveState>& strategy = callable.strategy();


        Real maximumValue = QL_MIN_REAL;
        Real numerairesHeld = 0.0;
        Real weight = evolver_->startNewPath();
        callable.clear();
        composite_.reset();
        callable.disableCallability();
        Real principalInNumerairePortfolio = 1.0;
        Size exercise = 0;

        for (Size k=0; k<numberOfSteps_; ++k) {
            weight *= evolver_->advanceStep();

            composite_.nextTimeStep(evolver_->currentState(),
                                    numberCashFlowsThisStep_,
                                    cashFlowsGenerated_);

            // First, we accumulate cash flows from both the
            // underlying...
            Real underlyingCashFlows =
                collectCashFlows(k,
                                 principalInNumerairePortfolio,
                                 underlyingOffset_,
                                 underlyingOffset_+underlyingSize_);

            // ...and the hedge
            Real hedgeCashFlows =
                collectCashFlows(k,
                                 principalInNumerairePortfolio,
                                 hedgeOffset_,
                                 hedgeOffset_+hedgeSize_);

            // we do the same for the rebates. Warning: this relies on
            // the fact that on each exercise date an ExerciseAdapter
            // generates a cash-flow equal to the exercise value
            Real rebateCashFlow =
                collectCashFlows(k,
                                 principalInNumerairePortfolio,
                                 rebateOffset_,
                                 rebateOffset_+rebateSize_);

            Real hedgeRebateCashFlow =
                collectCashFlows(k,
                                 principalInNumerairePortfolio,
                                  hedgeRebateOffset_,
                                 hedgeRebateOffset_+hedgeRebateSize_);


            numerairesHeld += underlyingCashFlows - hedgeCashFlows;

            // Second, we do the upper-bound thing
            if (isExerciseTime_[k]) {

                Real unexercisedHedgeValue = 0.0;

                if (k != numberOfSteps_-1) {

                    // Here, we setup the relevant inner evolver and
                    // the decorated callable hedge such that their
                    // reset() method brings them to the current point
                    // rather than the beginning of the path.

                    ext::shared_ptr<MarketModelEvolver> currentEvolver =
                        innerEvolvers_[exercise++];
                    currentEvolver->setInitialState(evolver_->currentState());

                    callable.stopRecording();
                    callable.enableCallability();
                    callable.save();

                    // This allows us to write:
                    AccountingEngine engine(currentEvolver, callable,
                                            1.0); // this causes the result
                                                  // to be in numeraire units
                    SequenceStatisticsInc innerStats(callable.numberOfProducts());
                    engine.multiplePathValues(innerStats, innerPaths);

                    const std::vector<Real>& values = innerStats.mean();
                    unexercisedHedgeValue =
                        std::accumulate(values.begin(), values.end(), Real(0.0))
                        / principalInNumerairePortfolio;

                    callable.disableCallability();
                    callable.startRecording();

                }

                // Now, we can calculate the total value of our hedged
                // portfolio...
                Real portfolioValue = numerairesHeld;
                if (strategy.exercise(evolver_->currentState())) {
                    // get the rebates...
                    portfolioValue +=
                        rebateCashFlow - hedgeRebateCashFlow;
                    // ...and reinvest to rehedge
                    numerairesHeld +=
                        unexercisedHedgeValue - hedgeRebateCashFlow;
                } else {
                    portfolioValue +=
                        rebateCashFlow - unexercisedHedgeValue;
                }

                // ...and use it to update the maximum value
                maximumValue = std::max(maximumValue, portfolioValue);
            }


            // Lastly, we do the homework for next step (if any)
            if (k<numberOfSteps_-1) {

                // The numeraire might change between steps. This implies
                // that we might have to convert the numeraire bonds for
                // this step into a corresponding amount of numeraire
                // bonds for the next step. This can be done by changing
                // the principal of the numeraire and updating the number
                // of bonds in the numeraire portfolio accordingly.

                Size numeraire = evolver_->numeraires()[k];
                Size nextNumeraire = evolver_->numeraires()[k+1];

                principalInNumerairePortfolio *=
                    evolver_->currentState().discountRatio(numeraire,
                                                           nextNumeraire);
            }

        }

        // finally, we update the maximum with the total accumulated
        // cash flows (in case we never exercised)
        maximumValue = std::max(maximumValue, numerairesHeld);


        // all done; we just convert the result back to cash
        maximumValue *= initialNumeraireValue_;

        return std::make_pair(maximumValue, weight);
    }


    Real UpperBoundEngine::collectCashFlows(Size currentStep,
                                            Real principalInNumerairePortfolio,
                                            Size beginProduct,
                                            Size endProduct) const {
        Size numeraire = evolver_->numeraires()[currentStep];

        Real numeraireUnits = 0.0;
        // For each product in range...
        for (Size i=beginProduct; i<endProduct; ++i) {
            // ...and for each cash flow...
            const std::vector<MarketModelMultiProduct::CashFlow>& cashflows =
                cashFlowsGenerated_[i];
            for (Size j=0; j<numberCashFlowsThisStep_[i]; ++j) {
                // ...convert the cash flow to numeraires.  This is
                // done by calculating the number of numeraire bonds
                // corresponding to such cash flow...
                const MarketModelDiscounter& discounter =
                    discounters_[cashflows[j].timeIndex];
                // ...and adding the newly bought bonds to the total
                numeraireUnits += cashflows[j].amount *
                    discounter.numeraireBonds(evolver_->currentState(),
                                              numeraire);
            }
        }
        return numeraireUnits/principalInNumerairePortfolio;
    }

}

