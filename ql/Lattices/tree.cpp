/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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
/*! \file tree.cpp
    \brief Tree class

    \fullpath
    ql/Lattices/%tree.cpp
*/

// $Id$

#include "ql/Lattices/tree.hpp"

namespace QuantLib {

    namespace Lattices {

        void Tree::computeStatePrices(Size until) {
            for (Size i=statePricesLimit_; i<until; i++) {
                const Column& c = column(i);
                for (Size j=0; j<column(i).size(); j++) {
                    double discount = c.discount(j);
                    double statePrice = c.statePrice(j);
                    for (Size l=0; l<n_; l++) {
                        column(i+1).addToStatePrice(
                            c.descendant(j,l),
                            statePrice*discount*c.probability(j,l));
                    }
                }
            }
            statePricesLimit_ = until;
        }

        const std::vector<double>& Tree::statePrices(Size i) {
            if (i>statePricesLimit_)
                computeStatePrices(i);
            return column(i).statePrices();
        }

        double Tree::presentValue(const Handle<DiscretizedAsset>& asset) {
            Size i = t_.findIndex(asset->time());
            if (i>statePricesLimit_)
                computeStatePrices(i);
            double value = 0.0;
            const Column& c = column(i);
            for (Size j=0; j<c.size(); j++) {
                value += asset->values()[j]*c.statePrice(j);
            }
            return value;
        }

        void Tree::initialize(const Handle<DiscretizedAsset>& asset, 
                              Time t) const {

            Size i = t_.findIndex(t);
            asset->setTime(t);
            asset->reset(column(i).size());
        }

        void Tree::rollback(const Handle<DiscretizedAsset>& asset, 
                            Time to) const {

            Time from = asset->time();

            QL_REQUIRE(from>=to, "Wrong rollback extremities");
            Size iFrom = t_.findIndex(from);
            Size iTo = t_.findIndex(to);

            for (int i=(int)(iFrom-1); i>=(int)iTo; i--) {
                const Column& c = column(i);
                Array newValues(c.size());
                for (Size j=0; j<c.size(); j++) {
                    double value = 0.0;
                    for (Size l=0; l<n_; l++) {
                        value += c.probability(j,l)*
                                 asset->values()[c.descendant(j,l)];
                    }
                    value *= c.discount(j);
                    newValues[j] = value;
                }
                asset->setTime(t_[i]);
                asset->setValues(newValues);
                asset->applyCondition();
            }
        }

    }

}

