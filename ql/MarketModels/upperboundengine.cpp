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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/MarketModels/upperboundengine.hpp>
#include <algorithm>

namespace QuantLib {

    UpperBoundEngine::UpperBoundEngine(
                         const boost::shared_ptr<MarketModelEvolver>& evolver,
                         const CallSpecifiedMultiProduct& product,
                         const MarketModelMultiProduct& hedge,
                         double initialNumeraireValue)
    : evolver_(evolver), composite_(MultiProductComposite()),
      initialNumeraireValue_(initialNumeraireValue) {

        composite_.add(product);
        composite_.subtract(hedge);
        composite_.finalize();

        underlyingSize_ = product.underlying().numberOfProducts();
        rebateSize_ = product.rebate().numberOfProducts();
        numberOfProducts_ = composite_.numberOfProducts();
        numberOfSteps_ = composite_.evolution().evolutionTimes().size();

        numberCashFlowsThisStep_.resize(numberOfProducts_);
        cashFlowsGenerated_.resize(numberOfProducts_);
        for (Size i = 0; i <numberProducts_; ++i )
            cashFlowsGenerated_[i].resize(
                          composite_.maxNumberOfCashFlowsPerProductPerStep());

        const std::vector<Time>& cashFlowTimes =
            composite_.possibleCashFlowTimes();
        const std::vector<Rate>& rateTimes =
            composite_.evolution().rateTimes();
        for (Size j = 0; j < cashFlowTimes.size(); ++j)
            discounters_.push_back(MarketModelDiscounter(cashFlowTimes[j],
                                                         rateTimes));
    }


    Real UpperBoundEngine::singlePathValue() {

        const CallSpecifiedMultiProduct& product =
            dynamic_cast<const CallSpecifiedMultiProduct&>(composite_.item(0));
        const MarketModelMultiProduct& hedge = composite_.item(1);

        Real maximumValue = QL_MIN_REAL;
        Real numerairesHeld = 0.0;
        Real weight = evolver_->startNewPath();
        composite_.reset();
        Real principalInNumerairePortfolio = 1.0;

        for (Size k=0; k<numberOfSteps_; ++k) {
            Size thisStep = evolver_->currentStep();
            weight *= evolver_->advanceStep();

            composite_.nextTimeStep(evolver_->currentState(),
                                    numberCashFlowsThisStep_,
                                    cashFlowsGenerated_);
            Size numeraire =
                evolver_->numeraires()[thisStep];

            // First, we accumulate cash flows from both the
            // unexercised product and the hedge
            for (Size i=0; i<numberProducts_; ++i) {
                // we discard cash flows coming from the rebate
                if (i >= underlyingSize_ && i < underlyingSize_+rebateSize_)
                    continue;

                // for each cash flow...
                const std::vector<MarketModelMultiProduct::CashFlow>& cashflows =
                    cashFlowsGenerated_[i];
                for (Size j=0; j<numberCashFlowsThisStep_[i]; ++j) {
                    // ...convert the cash flow to numeraires.
                    // This is done by calculating the number of
                    // numeraire bonds corresponding to such cash flow...
                    const MarketModelDiscounter& discounter =
                        discounters_[cashflows[j].timeIndex];

                    Real bonds =
                        cashflows[j].amount *
                        discounter.numeraireBonds(evolver_->currentState(),
                                                  numeraire);

                    // ...and adding the newly bought bonds to the number
                    // of numeraires held.
                    numerairesHeld += bonds/principalInNumerairePortfolio;
                }
            }

            // Second, we do the upper-bound thing




            // maximumValue = std::max(maximumValue, ...);


            // Lastly, we do the homework for next step
            if (k < numberOfSteps_-1) {

                // The numeraire might change between steps. This implies
                // that we might have to convert the numeraire bonds for
                // this step into a corresponding amount of numeraire
                // bonds for the next step. This can be done by changing
                // the principal of the numeraire and updating the number
                // of bonds in the numeraire portfolio accordingly.

                Size nextNumeraire =
                    evolver_->numeraires()[thisStep+1];

                principalInNumerairePortfolio *=
                    evolver_->currentState().discountRatio(numeraire,
                                                           nextNumeraire);
            }

        }

        maximumValue *= initialNumeraireValue_;

        return make_pair(maximumValue, weight);
    }

    void UpperBoundEngine::multiplePathValues(Statistics& stats,
                                              Size numberOfPaths) {
        for (Size i=0; i<numberOfPaths; ++i) {
            std::pair<Real,Real> result = singlePathValue();
            stats.add(result.first, result.second);
        }
    }

}
