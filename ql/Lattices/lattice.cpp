
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Lattices/lattice.hpp>
#include <ql/discretizedasset.hpp>

namespace QuantLib {

    void Lattice::computeStatePrices(Size until) {
        for (Size i=statePricesLimit_; i<until; i++) {
            statePrices_.push_back(Array(size(i+1), 0.0));
            for (Size j=0; j<size(i); j++) {
                DiscountFactor disc = discount(i,j);
                Real statePrice = statePrices_[i][j];
                for (Size l=0; l<n_; l++) {
                    statePrices_[i+1][descendant(i,j,l)] +=
                        statePrice*disc*probability(i,j,l);
                }
            }
        }
        statePricesLimit_ = until;
    }

    const Array& Lattice::statePrices(Size i) {
        if (i>statePricesLimit_)
            computeStatePrices(i);
        return statePrices_[i];
    }

    Real Lattice::presentValue(
                           const boost::shared_ptr<DiscretizedAsset>& asset) {
        Size i = t_.findIndex(asset->time());
        return DotProduct(asset->values(), statePrices(i));
    }

    void Lattice::initialize(const boost::shared_ptr<DiscretizedAsset>& asset,
                             Time t) const {

        Size i = t_.findIndex(t);
        asset->time() = t;
        asset->reset(size(i));
    }

    void Lattice::rollback(const boost::shared_ptr<DiscretizedAsset>& asset,
                           Time to) const {
        rollAlmostBack(asset,to);
        asset->postAdjustValues();
    }

    void Lattice::rollAlmostBack(
                            const boost::shared_ptr<DiscretizedAsset>& asset,
                            Time to) const {

        Time from = asset->time();

        if (close(from,to))
            return;

        QL_REQUIRE(from > to,
                   "cannot roll the asset back to" +
                   DecimalFormatter::toString(to) +
                   " (it is already at t = " +
                   DecimalFormatter::toString(from) + ")");

        Integer iFrom = Integer(t_.findIndex(from));
        Integer iTo = Integer(t_.findIndex(to));

        for (Integer i=iFrom-1; i>=iTo; i--) {
            Array newValues(size(i));
            stepback(i, asset->values(), newValues);
            asset->time() = t_[i];
            asset->values() = newValues;
            // skip the very last post-adjustment
            if (i != iTo)
                asset->adjustValues();
            else
                asset->preAdjustValues();
        }
    }


    void Lattice::stepback(Size i, const Array& values,
                           Array& newValues) const {
        for (Size j=0; j<size(i); j++) {
            Real value = 0.0;
            for (Size l=0; l<n_; l++) {
                value += probability(i,j,l)*values[descendant(i,j,l)];
            }
            value *= discount(i,j);
            newValues[j] = value;
        }
    }

}

