/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

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

#include <ql/models/marketmodels/products/compositeproduct.hpp>
#include <ql/models/marketmodels/utilities.hpp>

namespace QuantLib {

    MarketModelComposite::MarketModelComposite()
    : finalized_(false) {}

    const EvolutionDescription& MarketModelComposite::evolution() const {
        QL_REQUIRE(finalized_, "composite not finalized");
        return evolution_;
    }

    std::vector<Size> MarketModelComposite::suggestedNumeraires() const {
        QL_REQUIRE(finalized_, "composite not finalized");
        return terminalMeasure(evolution_);
    }

    std::vector<Time> MarketModelComposite::possibleCashFlowTimes() const {
        QL_REQUIRE(finalized_, "composite not finalized");
        return cashflowTimes_;
    }

    void MarketModelComposite::reset() {
        for (iterator i=components_.begin(); i!=components_.end(); ++i) {
            i->product->reset();
            i->done = false;
        }
        currentIndex_ = 0;
    }



    void MarketModelComposite::add(
                                const Clone<MarketModelMultiProduct>& product,
                                Real multiplier) {
        QL_REQUIRE(!finalized_, "product already finalized");
        EvolutionDescription d = product->evolution();
        if (!components_.empty()) {
            // enforce preconditions
            EvolutionDescription d1 =
                components_.front().product->evolution();
            const std::vector<Time>& rateTimes1 = d1.rateTimes();
            const std::vector<Time>& rateTimes2 = d.rateTimes();
            QL_REQUIRE(rateTimes1.size() == rateTimes2.size() &&
                       std::equal(rateTimes1.begin(), rateTimes1.end(),
                                  rateTimes2.begin()),
                       "incompatible rate times");
        }
        components_.push_back(SubProduct());
        components_.back().product = product;
        components_.back().multiplier = multiplier;
        components_.back().done = false;
        allEvolutionTimes_.push_back(d.evolutionTimes());
    }

    void MarketModelComposite::subtract(
                                const Clone<MarketModelMultiProduct>& product,
                                Real multiplier) {
        add(product, -multiplier);
    }

    void MarketModelComposite::finalize() {
        QL_REQUIRE(!finalized_, "product already finalized");
        QL_REQUIRE(!components_.empty(), "no sub-product provided");

        // fetch the rate times from the first subproduct (we checked
        // they're all the same)
        EvolutionDescription description =
            components_.front().product->evolution();
        rateTimes_ = description.rateTimes();

        mergeTimes(allEvolutionTimes_, evolutionTimes_, isInSubset_);

        std::vector<Time> allCashflowTimes;

        // now, for each subproduct...
        iterator i;
        for (i=components_.begin(); i!=components_.end(); ++i) {
            EvolutionDescription d = i->product->evolution();
            // ...collect all possible cash-flow times...
            const std::vector<Time>& cashflowTimes =
                i->product->possibleCashFlowTimes();
            allCashflowTimes.insert(allCashflowTimes.end(),
                                    cashflowTimes.begin(),
                                    cashflowTimes.end());
            // ...allocate working vectors...
            i->numberOfCashflows =
                std::vector<Size>(i->product->numberOfProducts());
            i->cashflows =
                std::vector<std::vector<CashFlow> >(
                     i->product->numberOfProducts(),
                     std::vector<CashFlow>(i->product
                                  ->maxNumberOfCashFlowsPerProductPerStep()));
        }

        // all information having been collected, we can sort and
        // compact the vector of all cash-flow times...
        std::sort(allCashflowTimes.begin(), allCashflowTimes.end());
        std::vector<Time>::iterator end = std::unique(allCashflowTimes.begin(),
                                                      allCashflowTimes.end());
        //std::copy(allCashflowTimes.begin(), end,
        //          std::back_inserter(cashflowTimes_));
        cashflowTimes_.insert(cashflowTimes_.end(),
                              allCashflowTimes.begin(), end);
        // ...and map each product's cash-flow time into the total vector.
        for (i=components_.begin(); i!=components_.end(); ++i) {
            const std::vector<Time>& productTimes =
                i->product->possibleCashFlowTimes();
            i->timeIndices = std::vector<Size>(productTimes.size());
            for (Size j=0; j<productTimes.size(); ++j) {
                i->timeIndices[j] =
                    std::find(cashflowTimes_.begin(), cashflowTimes_.end(),
                              productTimes[j]) - cashflowTimes_.begin();
            }
        }

        evolution_ = EvolutionDescription(rateTimes_, evolutionTimes_);

        // all done.
        finalized_ = true;
    }

    Size MarketModelComposite::size() const {
        return components_.size();
    }

    const MarketModelMultiProduct& MarketModelComposite::item(Size i) const {
        return *(components_.at(i).product);
    }

    MarketModelMultiProduct& MarketModelComposite::item(Size i) {
        return *(components_.at(i).product);
    }

    Real MarketModelComposite::multiplier(Size i) const {
        return components_.at(i).multiplier;
    }

}
