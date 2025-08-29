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

#include <ql/models/marketmodels/products/singleproductcomposite.hpp>

namespace QuantLib {

    Size SingleProductComposite::numberOfProducts() const {
        return 1;
    }


    Size SingleProductComposite::maxNumberOfCashFlowsPerProductPerStep() const {
        Size result = 0;
        for (const auto& component : components_)
            result += component.product->maxNumberOfCashFlowsPerProductPerStep();
        return result;
    }


    bool SingleProductComposite::nextTimeStep(
                     const CurveState& currentState,
                     std::vector<Size>& numberCashFlowsThisStep,
                     std::vector<std::vector<CashFlow> >& cashFlowsGenerated) {
        QL_REQUIRE(finalized_, "composite not finalized");
        bool done = true;
        Size n = 0, totalCashflows = 0;
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
                    Size offset = totalCashflows;
                    totalCashflows += i->numberOfCashflows[j];
                    for (Size k=0; k<i->numberOfCashflows[j]; ++k) {
                        CashFlow& from = i->cashflows[j][k];
                        CashFlow& to = cashFlowsGenerated[0][k+offset];
                        to.timeIndex = i->timeIndices[from.timeIndex];
                        to.amount = from.amount * i->multiplier;
                    }
                    numberCashFlowsThisStep[0] = totalCashflows;
                }
                // finally, set done to false if this product isn't done
                done = done && thisDone;
            }
        }
        ++currentIndex_;
        return done;
    }

    std::unique_ptr<MarketModelMultiProduct>
    SingleProductComposite::clone() const {
        return std::unique_ptr<MarketModelMultiProduct>(new SingleProductComposite(*this));
    }

}

