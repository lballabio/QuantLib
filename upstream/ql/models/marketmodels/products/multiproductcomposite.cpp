/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/models/marketmodels/products/multiproductcomposite.hpp>

namespace QuantLib {

    Size MultiProductComposite::numberOfProducts() const {
        Size result = 0;
        for (const auto& component : components_)
            result += component.product->numberOfProducts();
        return result;
    }


    Size MultiProductComposite::maxNumberOfCashFlowsPerProductPerStep() const {
        Size result = 0;
        for (const auto& component : components_)
            result = std::max(result, component.product->maxNumberOfCashFlowsPerProductPerStep());
        return result;
    }


    bool MultiProductComposite::nextTimeStep(
                     const CurveState& currentState,
                     std::vector<Size>& numberCashFlowsThisStep,
                     std::vector<std::vector<CashFlow> >& cashFlowsGenerated) {
        QL_REQUIRE(finalized_, "composite not finalized");
        bool done = true;
        Size n = 0, offset = 0;
        // for each sub-product...
        for (auto i = components_.begin(); i != components_.end(); ++i, ++n) {
            if (isInSubset_[n][currentIndex_] && !i->done) {
                // ...make it evolve...
                bool thisDone = i->product->nextTimeStep(currentState,
                                                         i->numberOfCashflows,
                                                         i->cashflows);
                // ...and copy the results. Time indices need to be remapped
                // so that they point into all cash-flow times. Amounts need
                // to be adjusted by the corresponding multiplier.
                for (Size j=0; j<i->product->numberOfProducts(); ++j) {
                    numberCashFlowsThisStep[j+offset] =
                        i->numberOfCashflows[j];
                    for (Size k=0; k<i->numberOfCashflows[j]; ++k) {
                        CashFlow& from = i->cashflows[j][k];
                        CashFlow& to = cashFlowsGenerated[j+offset][k];
                        to.timeIndex = i->timeIndices[from.timeIndex];
                        to.amount = from.amount * i->multiplier;
                    }
                }
                // finally, set done to false if this product isn't done
                done = done && thisDone;
            }
            else
                for (Size j=0; j<i->product->numberOfProducts(); ++j)
                    numberCashFlowsThisStep[j+offset] =0;

            // the offset is updated whether or not the product was evolved
            offset += i->product->numberOfProducts();
        }
        ++currentIndex_;
        return done;
    }

    std::unique_ptr<MarketModelMultiProduct>
    MultiProductComposite::clone() const {
        return std::unique_ptr<MarketModelMultiProduct>(new MultiProductComposite(*this));
    }

}
