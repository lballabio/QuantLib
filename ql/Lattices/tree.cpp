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
                Column::const_iterator c = column(i).begin();
                for (; c != column(i).end(); c++) {
                    double discountFactor = discount(i, c->j);
                    for (Size n=0; n<n_; n++) {
                        node(i+1,c->descendant[n]).statePrice +=
                            c->statePrice*c->probability[n]*discountFactor;
                    }
                }
            }
            statePricesLimit_ = until;
        }

        double Tree::presentValue(const Handle<Asset>& asset) {
            Size i = t_.findIndex(asset->time());
            if (i>statePricesLimit_)
                computeStatePrices(i);
            double value = 0.0;
            Size l = 0;
            Column::const_iterator n = column(i).begin();
            for (; n != column(i).end(); n++, l++) {
                value += asset->values()[l]*n->statePrice;
            }
            return value;
        }

        void Tree::initialize(const Handle<Asset>& asset, Time t) const {
            Size i = t_.findIndex(t);
            Size width = column(i).size();
            asset->setTime(t);
            asset->reset(width);
        }

        void Tree::rollback(const Handle<Asset>& asset, Time to) const {
            std::vector<Handle<Asset> > assets(1, asset);
            rollback(assets, to);
        }

        void Tree::rollback(
            const std::vector<Handle<Asset> >& assets,
            Time to) const {

            std::vector<Handle<Asset> >::const_iterator begin = assets.begin();
            Time from = (*begin)->time();
            for (++begin; begin != assets.end(); ++begin) {
                QL_REQUIRE((*begin)->time()==from,
                    "Assets must be at the same time!");
            }

            QL_REQUIRE(from>=to, "Wrong rollback extremities");
            Size iFrom = t_.findIndex(from);
            Size iTo = t_.findIndex(to);

            for (int i=(int)(iFrom-1); i>=(int)iTo; i--) {
                for (begin = assets.begin(); begin != assets.end(); ++begin) {
                    Size width = jMax(i) - jMin(i) + 1;
                    Array newValues(width);
                    Size l = 0;
                    Column::const_iterator n = column(i).begin();
                    for (; n != column(i).end(); n++, l++) {
                        double value = 0.0;
                        for (Size k=0; k<n_; k++) {
                            Size index = nodeIndex(i+1, n->descendant[k]);
                            value += n->probability[k]
                                *(*begin)->values()[index];
                        }
                        value *= discount(i, n->j);
                        newValues[l] = value;
                    }
                    (*begin)->setTime(t(i));
                    (*begin)->setValues(newValues);
                    (*begin)->applyCondition();
                }
            }
        }

    }

}

