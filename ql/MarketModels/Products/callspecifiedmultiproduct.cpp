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

#include <ql/MarketModels/callspecifiedmultiproduct.hpp>

namespace QuantLib {

    namespace {

        void mergeTimes(const std::vector<Time>& v1,
                        const std::vector<Time>& v2,
                        std::vector<Time>& w,
                        std::vector<bool>& b1,
                        std::vector<bool>& b2);

    }

    CallSpecifiedMultiProduct::CallSpecifiedMultiProduct(
              const boost::shared_ptr<MarketModelMultiProduct>& underlying,
              const boost::shared_ptr<ExerciseStrategy<CurveState> >& strategy,
              const boost::shared_ptr<MarketModelMultiProduct>& rebate)
    : underlying_(underlying), strategy_(strategy), rebate_(rebate) {
        EvolutionDescription d1 = underlying->suggestedEvolution();
        const std::vector<Time>& rateTimes1 = d1.rateTimes();
        const std::vector<Time>& evolutionTimes1 = d1.evolutionTimes();
        if (rebate) {
            EvolutionDescription d2 = rebate->suggestedEvolution();
            const std::vector<Time>& rateTimes2 = d2.rateTimes();
            QL_REQUIRE(rateTimes1.size() == rateTimes2.size() &&
                       std::equal(rateTimes1.begin(), rateTimes1.end(),
                                  rateTimes2.begin()),
                       "incompatible rate times");
            const std::vector<Time>& evolutionTimes2 = d2.evolutionTimes();
            QL_REQUIRE(evolutionTimes1.size() == evolutionTimes2.size() &&
                       std::equal(evolutionTimes1.begin(),
                                  evolutionTimes1.end(),
                                  evolutionTimes2.begin()),
                       "incompatible evolution times");
        }
        
        const std::vector<Time>& exerciseTimes = strategy->exerciseTimes();

        std::vector<Time> allEvolutionTimes;
        mergeTimes(evolutionTimes1, exerciseTimes,
                   allEvolutionTimes, isProductTime_, isExerciseTime_);

        evolution_ = EvolutionDescription(rateTimes1, allEvolutionTimes,
                                          d1.numeraires()); // TODO: add
                                                            // relevant rates
        cashFlowTimes_ = underlying_->possibleCashFlowTimes();
        rebateOffset_ = cashFlowTimes_.size();
        if (rebate) {
            const std::vector<Time> rebateTimes =
                rebate_->possibleCashFlowTimes();
            std::copy(rebateTimes.begin(), rebateTimes.end(),
                      std::back_inserter(cashFlowTimes_));
        }
    }

    EvolutionDescription
    CallSpecifiedMultiProduct::suggestedEvolution() const {
        return evolution_;
    }

    std::vector<Time>
    CallSpecifiedMultiProduct::possibleCashFlowTimes() const {
        return cashFlowTimes_;
    }

    Size CallSpecifiedMultiProduct::numberOfProducts() const {
        return underlying_->numberOfProducts();
    }

    Size
    CallSpecifiedMultiProduct::maxNumberOfCashFlowsPerProductPerStep() const {
        
    }

    void CallSpecifiedMultiProduct::reset() {
        underlying_->reset();
        if (rebate)
            rebate_->reset();
        strategy_->reset();
    }



    bool CallSpecifiedMultiProduct::nextTimeStep(
            const CurveState& currentState, 
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<CashFlow> >& cashFlowsGenerated) {


    }

}

#endif
