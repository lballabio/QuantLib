/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2009 Mark Joshi

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

#include <ql/math/matrix.hpp>
#include <ql/models/marketmodels/products/pathwise/pathwiseproductcallspecified.hpp>
#include <ql/models/marketmodels/products/pathwise/pathwiseproductcashrebate.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <utility>

namespace QuantLib {

    CallSpecifiedPathwiseMultiProduct::CallSpecifiedPathwiseMultiProduct(
        const Clone<MarketModelPathwiseMultiProduct>& underlying,
        const Clone<ExerciseStrategy<CurveState> >& strategy,
        Clone<MarketModelPathwiseMultiProduct> rebate)
    : underlying_(underlying), strategy_(strategy), rebate_(std::move(rebate)), callable_(true) {

        Size products = underlying_->numberOfProducts();
        EvolutionDescription d1 = underlying->evolution();
        const std::vector<Time>& rateTimes1 = d1.rateTimes();
        const std::vector<Time>& evolutionTimes1 = d1.evolutionTimes();
        const std::vector<Time>& exerciseTimes = strategy->exerciseTimes();

        if (!rebate_.empty()) {
            EvolutionDescription d2 = rebate_->evolution();
            const std::vector<Time>& rateTimes2 = d2.rateTimes();
            QL_REQUIRE(rateTimes1.size() == rateTimes2.size() &&
                           std::equal(rateTimes1.begin(), rateTimes1.end(), rateTimes2.begin()),
                       "incompatible rate times");

            QL_REQUIRE(underlying_->alreadyDeflated() == rebate_->alreadyDeflated(),
                       "incompatible deflations");
        } else {
            EvolutionDescription description(rateTimes1, exerciseTimes);
            Matrix amounts(products, exerciseTimes.size(), 0.0);

            rebate_ = MarketModelPathwiseCashRebate(description, exerciseTimes, amounts, products);
        }

        std::vector<Time> mergedEvolutionTimes;
        std::vector<std::vector<Time> > allEvolutionTimes(4);
        allEvolutionTimes[0] = evolutionTimes1;
        allEvolutionTimes[1] = exerciseTimes;
        allEvolutionTimes[2] = rebate_->evolution().evolutionTimes();
        allEvolutionTimes[3] = strategy->relevantTimes();

        mergeTimes(allEvolutionTimes, mergedEvolutionTimes, isPresent_);

        // TODO: add relevant rates
        evolution_ = EvolutionDescription(rateTimes1, mergedEvolutionTimes);

        cashFlowTimes_ = underlying_->possibleCashFlowTimes();
        rebateOffset_ = cashFlowTimes_.size();
        const std::vector<Time> rebateTimes = rebate_->possibleCashFlowTimes();
        cashFlowTimes_.insert(cashFlowTimes_.end(), rebateTimes.begin(), rebateTimes.end());

        dummyCashFlowsThisStep_ = std::vector<Size>(products, 0);
        Size n = rebate_->maxNumberOfCashFlowsPerProductPerStep();

        CashFlow modelCashFlow;
        modelCashFlow.amount.resize(d1.numberOfRates() + 1);

        dummyCashFlowsGenerated_ =
            std::vector<std::vector<CashFlow> >(products, std::vector<CashFlow>(n, modelCashFlow));
    }

    bool CallSpecifiedPathwiseMultiProduct::alreadyDeflated() const {
        return underlying_->alreadyDeflated();
    }

    std::vector<Size> CallSpecifiedPathwiseMultiProduct::suggestedNumeraires() const {
        return underlying_->suggestedNumeraires();
    }

    const EvolutionDescription& CallSpecifiedPathwiseMultiProduct::evolution() const {
        return evolution_;
    }

    std::vector<Time> CallSpecifiedPathwiseMultiProduct::possibleCashFlowTimes() const {
        return cashFlowTimes_;
    }

    Size CallSpecifiedPathwiseMultiProduct::numberOfProducts() const {
        return underlying_->numberOfProducts();
    }

    Size CallSpecifiedPathwiseMultiProduct::maxNumberOfCashFlowsPerProductPerStep() const {
        return std::max(underlying_->maxNumberOfCashFlowsPerProductPerStep(),
                        rebate_->maxNumberOfCashFlowsPerProductPerStep());
    }

    void CallSpecifiedPathwiseMultiProduct::reset() {
        underlying_->reset();
        rebate_->reset();
        strategy_->reset();
        currentIndex_ = 0;
        wasCalled_ = false;
    }


    bool CallSpecifiedPathwiseMultiProduct::nextTimeStep(
        const CurveState& currentState,
        std::vector<Size>& numberCashFlowsThisStep,
        std::vector<std::vector<CashFlow> >& cashFlowsGenerated) {

        bool isUnderlyingTime = isPresent_[0][currentIndex_];
        bool isExerciseTime = isPresent_[1][currentIndex_];
        bool isRebateTime = isPresent_[2][currentIndex_];
        bool isStrategyRelevantTime = isPresent_[3][currentIndex_];

        bool done = false;

        if (!wasCalled_ && isStrategyRelevantTime)
            strategy_->nextStep(currentState);


        if (!wasCalled_ && isExerciseTime && callable_)
            wasCalled_ = strategy_->exercise(currentState);

        if (wasCalled_) {
            if (isRebateTime) {
                done = rebate_->nextTimeStep(currentState, numberCashFlowsThisStep,
                                             cashFlowsGenerated);
                for (Size i = 0; i < numberCashFlowsThisStep.size(); ++i)
                    for (Size j = 0; j < numberCashFlowsThisStep[i]; ++j)
                        cashFlowsGenerated[i][j].timeIndex += rebateOffset_;
            }
        } else {
            if (isRebateTime)
                rebate_->nextTimeStep(currentState, dummyCashFlowsThisStep_,
                                      dummyCashFlowsGenerated_);
            if (isUnderlyingTime)
                done = underlying_->nextTimeStep(currentState, numberCashFlowsThisStep,
                                                 cashFlowsGenerated);
        }

        ++currentIndex_;
        return done || currentIndex_ == evolution_.evolutionTimes().size();
    }

    std::unique_ptr<MarketModelPathwiseMultiProduct>
    CallSpecifiedPathwiseMultiProduct::clone() const {
        return std::unique_ptr<MarketModelPathwiseMultiProduct>(new CallSpecifiedPathwiseMultiProduct(*this));
    }

    const MarketModelPathwiseMultiProduct& CallSpecifiedPathwiseMultiProduct::underlying() const {
        return *underlying_;
    }

    const ExerciseStrategy<CurveState>& CallSpecifiedPathwiseMultiProduct::strategy() const {
        return *strategy_;
    }

    const MarketModelPathwiseMultiProduct& CallSpecifiedPathwiseMultiProduct::rebate() const {
        return *rebate_;
    }

    void CallSpecifiedPathwiseMultiProduct::enableCallability() { callable_ = true; }

    void CallSpecifiedPathwiseMultiProduct::disableCallability() { callable_ = false; }

    }





