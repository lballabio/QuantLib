/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/MarketModels/Products/marketmodelcomposite.hpp>

namespace QuantLib {

    MarketModelComposite::MarketModelComposite()
    : finalized_(false) {}


    EvolutionDescription MarketModelComposite::suggestedEvolution() const {
        QL_REQUIRE(finalized_, "composite not finalized");
        return EvolutionDescription(rateTimes_, evolutionTimes_,
                                    numeraires_, relevanceRates_);
    }


    std::vector<Time> MarketModelComposite::possibleCashFlowTimes() const {
        QL_REQUIRE(finalized_, "composite not finalized");
        return cashflowTimes_;
    }


    Size MarketModelComposite::numberOfProducts() const {
        Size result = 0;
        for (const_iterator i=components_.begin(); i!=components_.end(); ++i)
            result += i->product->numberOfProducts();
        return result;
    }


    Size MarketModelComposite::maxNumberOfCashFlowsPerProductPerStep() const {
        Size result = 0;
        for (const_iterator i=components_.begin(); i!=components_.end(); ++i)
            result = std::max(result,
                              i->product
                               ->maxNumberOfCashFlowsPerProductPerStep());
        return result;
    }


    void MarketModelComposite::reset() {
        for (iterator i=components_.begin(); i!=components_.end(); ++i) {
            i->product->reset();
            i->done = false;
        }
    }


    bool MarketModelComposite::nextTimeStep(
                     const CurveState& currentState,
                     std::vector<Size>& numberCashFlowsThisStep,
                     std::vector<std::vector<CashFlow> >& cashFlowsGenerated) {
        QL_REQUIRE(finalized_, "composite not finalized");
        bool done = true;
        // for each sub-product...
        for (iterator i=components_.begin(); i!=components_.end(); ++i) {
            if (!i->done) {
                // ...make it evolve...
                bool thisDone = i->product->nextTimeStep(currentState,
                                                         i->numberOfCashflows,
                                                         i->cashflows);
                // ...and copy the results. Time indices need to be remapped
                // so that they point into all cash-flow times. Amounts need
                // to be adjusted by the corresponding multiplier.
                for (Size j=0; j<i->product->numberOfProducts(); ++j) {
                    numberCashFlowsThisStep[j+i->offset] =
                        i->numberOfCashflows[j];
                    for (Size k=0; k<i->numberOfCashflows[j]; ++k) {
                        CashFlow& from = i->cashflows[j][k];
                        CashFlow& to = cashFlowsGenerated[j+i->offset][k];
                        to.timeIndex = i->timeIndices[from.timeIndex];
                        to.amount = from.amount * i->multiplier;
                    }
                }
                // finally, set done to false if this product isn't done
                done = done && thisDone;
            }
        }
        return done;
    }


    void MarketModelComposite::add(
                         const boost::shared_ptr<MarketModelProduct>& product,
                         Real multiplier) {
		QL_REQUIRE(!finalized_, "product already finalized");
        if (!components_.empty()) {
            // enforce preconditions
            EvolutionDescription d1 =
                components_.front().product->suggestedEvolution();
            EvolutionDescription d2 = product->suggestedEvolution();
            const std::vector<Time>& rateTimes1 = d1.rateTimes();
            const std::vector<Time>& rateTimes2 = d2.rateTimes();
            QL_REQUIRE(rateTimes1.size() == rateTimes2.size() &&
                       std::equal(rateTimes1.begin(), rateTimes1.end(),
                                  rateTimes2.begin()),
                       "incompatible rate times");
            const std::vector<Time>& evolutionTimes1 = d1.evolutionTimes();
            const std::vector<Time>& evolutionTimes2 = d2.evolutionTimes();
            QL_REQUIRE(evolutionTimes1.size() == evolutionTimes2.size() &&
                       std::equal(evolutionTimes1.begin(),
                                  evolutionTimes1.end(),
                                  evolutionTimes2.begin()),
                       "incompatible evolution times");
        }
        components_.push_back(SubProduct());
        components_.back().product = product;
        components_.back().multiplier = multiplier;
        components_.back().done = false;
    }

    void MarketModelComposite::subtract(
                         const boost::shared_ptr<MarketModelProduct>& product,
                         Real multiplier) {
        add(product, -multiplier);
    }


    void MarketModelComposite::finalize() {
		QL_REQUIRE(!finalized_, "product already finalized");
        QL_REQUIRE(!components_.empty(), "no sub-product provided");

        // fetch the time vectors from the first subproduct (we checked
        // they're all the same)
        EvolutionDescription description =
            components_.front().product->suggestedEvolution();
        rateTimes_ = description.rateTimes();
        evolutionTimes_ = description.evolutionTimes();

        // fetch the candidate numeraires, too...
        std::vector<Size> commonNumeraires_ = description.numeraires();
        // ...and declare a few work variables.
        bool sameNumeraire = true;
        Size offset = 0;
        std::vector<Time> allCashflowTimes;
        std::vector<std::pair<Size,Size> > allRelevanceRates =
            description.relevanceRates();

        // now, for each subproduct...
        for (iterator i=components_.begin(); i!=components_.end(); ++i) {
            EvolutionDescription d = i->product->suggestedEvolution();
            // ...check that the numeraires are the same...
            const std::vector<Size>& numeraires = d.numeraires();
            if (!std::equal(numeraires.begin(), numeraires.end(),
                            commonNumeraires_.begin())) {
                sameNumeraire = false;
            }
            // ...collect all possible cash-flow times...
            const std::vector<Time>& cashflowTimes =
                i->product->possibleCashFlowTimes();
            allCashflowTimes.insert(allCashflowTimes.end(),
                                    cashflowTimes.begin(),
                                    cashflowTimes.end());
            // ..extend the range of relevant rates as needed...
            const std::vector<std::pair<Size,Size> >& relevanceRates =
                d.relevanceRates();
            for (Size j=0; j<relevanceRates.size(); ++j) {
                allRelevanceRates[j] =
                    std::make_pair(std::min(allRelevanceRates[j].first,
                                            relevanceRates[j].first),
                                   std::max(allRelevanceRates[j].second,
                                            relevanceRates[j].second));
            }
            // ...allocate working vectors...
            i->numberOfCashflows =
                std::vector<Size>(i->product->numberOfProducts());
            i->cashflows =
                std::vector<std::vector<CashFlow> >(
                     i->product->numberOfProducts(),
                     std::vector<CashFlow>(i->product
                                  ->maxNumberOfCashFlowsPerProductPerStep()));
            // ...and set the offset at which they will be copied.
            i->offset = offset;
            offset += i->product->numberOfProducts();
        }

        // all information having been collected, we can set the numeraires...
        if (sameNumeraire)
            numeraires_ = commonNumeraires_;
        else
            numeraires_ = std::vector<Size>(evolutionTimes_.size(),
                                            rateTimes_.size()-1);
        // ...sort and compact the vector of all cash-flow times...
        std::sort(allCashflowTimes.begin(), allCashflowTimes.end());
        std::vector<Time>::iterator end = std::unique(allCashflowTimes.begin(),
                                                      allCashflowTimes.end());
        std::copy(allCashflowTimes.begin(), end,
                  std::back_inserter(cashflowTimes_));
        // ...and map each product's cash-flow time into the total vector.
        for (iterator i=components_.begin(); i!=components_.end(); ++i) {
            const std::vector<Time>& productTimes =
                i->product->possibleCashFlowTimes();
            i->timeIndices = std::vector<Size>(productTimes.size());
            for (Size j=0; j<productTimes.size(); ++j) {
                i->timeIndices[j] =
                    std::find(cashflowTimes_.begin(), cashflowTimes_.end(),
                              productTimes[j]) - cashflowTimes_.begin();
            }
        }

        // all done.
        finalized_ = true;
    }

}

