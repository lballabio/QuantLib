/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file lattice.cpp
    \brief Lattice class

    \fullpath
    ql/Lattices/%lattice.cpp
*/

// $Id$

#include "ql/Lattices/lattice.hpp"

namespace QuantLib {

    namespace Lattices {

        void Lattice::computeStatePrices(Size until) {
            for (Size i=statePricesLimit_; i<until; i++) {
                statePrices_.push_back(Array(size(i+1), 0.0));
                for (Size j=0; j<size(i); j++) {
                    DiscountFactor disc = discount(i,j);
                    double statePrice = statePrices_[i][j];
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

        double Lattice::presentValue(
            const Handle<DiscretizedAsset>& asset) {
            Size i = t_.findIndex(asset->time());
            if (i>statePricesLimit_)
                computeStatePrices(i);
            return DotProduct(asset->values(), statePrices_[i]);
        }

        void Lattice::initialize(const Handle<DiscretizedAsset>& asset, 
                                 Time t) const {

            Size i = t_.findIndex(t);
            asset->setTime(t);
            asset->reset(size(i));
        }

        void Lattice::rollback(const Handle<DiscretizedAsset>& asset, 
                               Time to) const {

            Time from = asset->time();

            QL_REQUIRE(from >= to, 
                       "Lattice: Wrong rollback extremities");
            Size iFrom = t_.findIndex(from);
            Size iTo = t_.findIndex(to);

            for (int i=(int)(iFrom-1); i>=(int)iTo; i--) {
                Array newValues(size(i));
                stepback(i, asset->values(), newValues);
                asset->setTime(t_[i]);
                asset->setValues(newValues);
                asset->adjustValues();
            }
        }

        void Lattice::stepback(
            Size i, const Array& values, Array& newValues) const {
            for (Size j=0; j<size(i); j++) {
                double value = 0.0;
                for (Size l=0; l<n_; l++) {
                    value += probability(i,j,l)*values[descendant(i,j,l)];
                }
                value *= discount(i,j);
                newValues[j] = value;
            }
        }

    }

}

